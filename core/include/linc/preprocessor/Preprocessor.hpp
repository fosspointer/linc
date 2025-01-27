#pragma once
#include <linc/system/Files.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/system/Code.hpp>
#include <linc/lexer/Lexer.hpp>
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Preprocessor final 
    {
    public:
        using TokenList = std::vector<Token>;
        using TokenSize = std::vector<Token>::size_type;
        Preprocessor() = default;

        void set(TokenList tokens, std::string_view filepath)
        {
            m_tokens = std::move(tokens);
            m_filepath = filepath;
        }

        inline static void reset()
        {
            s_guardedFiles.clear();
        }
        
        std::vector<Token> operator()() const
        {
            std::vector<Token> output;
            m_index = {};

            if(m_tokens.size() >= 2ul && m_tokens[0ul].type == Token::Type::PreprocessorSpecifier && m_tokens[1ul].isIdentifier()
            && m_tokens[1ul].value && *m_tokens[1ul].value == "guard")
            {
                auto _specifier = consume();
                auto _directive = consume();

                s_guardedFiles.insert(Files::toAbsolute(m_filepath));
            }

            while(peek())
            {
                if(peek()->isIdentifier())
                {
                    auto identifier = consume();

                    if(!identifier.value)
                    {
                        output.push_back(identifier);
                        continue;
                    }

                    bool found{false};
                    for(const auto& definition: m_definitions)
                        if(definition.name == *identifier.value)
                        {
                            output.insert(output.end(), definition.body.begin(), definition.body.end());
                            found = true;
                            break;
                        }

                    if(found) continue;

                    for(const auto& macro: m_macros)
                        if(macro.name == *identifier.value)
                        {
                            std::vector<TokenList> arguments{TokenList{}};
                            match(Token::Type::ParenthesisLeft);

                            using ssize_t = std::make_signed<size_t>::type;
                            ssize_t parentheses_count{0l};
                            while(peek() && (peek()->type != Token::Type::ParenthesisRight || parentheses_count > 0l))
                            {
                                if(peek()->type == Token::Type::ParenthesisLeft)
                                    ++parentheses_count;
                                else if(peek()->type == Token::Type::ParenthesisRight)
                                    --parentheses_count;

                                arguments.back().push_back(consume());
                                if(peek()->type == Token::Type::Comma && parentheses_count == 0l)
                                {
                                    consume();
                                    arguments.push_back(TokenList{});
                                }
                            }
                            if(parentheses_count != 0l)
                                Reporting::push(Reporting::Report{
                                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                                    .span = TextSpan::fromTokenInfo(peekInfo()),
                                    .message = Logger::format("$ Unmatched parentheses in macro expansion.", peekInfo())
                                });
                            match(Token::Type::ParenthesisRight); 
                            std::vector<Token> body = embedMacroArguments(macro, arguments);
                            body.push_back(Token{.type = Token::Type::EndOfFile, .info = peekInfo()});
                            Preprocessor preprocessor;
                            preprocessor.set(body, m_filepath);
                            preprocessor.appendDefinitions(m_definitions, m_macros);
                            body = preprocessor();
                            body.pop_back();

                            output.insert(output.end(), body.begin(), body.end());
                            found = true;
                            break;
                        }

                    if(!found) output.push_back(identifier);
                    continue;
                }

                else if(peek()->type != Token::Type::PreprocessorSpecifier)
                {
                    output.push_back(consume());
                    continue;
                }

                auto specifier = consume();
                auto directive = match(Token::Type::Identifier, "Expected identifier in preprocessor directive.");
                if(*directive.value == "include")
                {
                    auto literal = match(Token::Type::StringLiteral);
                #ifdef LINC_WINDOWS
                    bool is_absolute = literal.value->starts_with("C:");
                #else
                    bool is_absolute = literal.value->starts_with('/');
                #endif
                    auto filepath = is_absolute? *literal.value: filepathToDirectory(m_filepath) + "/" + *literal.value;

                    if(!Files::exists(filepath))
                    {
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Warning, .stage = Reporting::Stage::Preprocessor,
                            .message = Logger::format("$ Include directive target path '$' does not exist (`$`).",
                                literal.info, *literal.value, filepath)
                        });
                        break;
                    }

                    if(s_guardedFiles.contains(Files::toAbsolute(filepath)))
                        continue;

                    auto raw_source = Files::read(filepath);
                    auto source = Code::toSource(raw_source, filepath);
                    Lexer lexer(source, false);
                    Preprocessor preprocessor;
                    preprocessor.set(lexer(), filepath);
                    auto tokens = preprocessor();

                    output.insert(output.end(), tokens.begin(), tokens.end() - 1ul);
                    continue;
                }
                else if(*directive.value == "define")
                {
                    auto identifier = match(Token::Type::Identifier);
                    std::vector<Token> body;

                    while(peek() && peek()->type != Token::Type::PreprocessorSpecifier)
                        body.push_back(consume());

                    consume();
                    m_definitions.push_back(Definition{.name = *identifier.value, .body = std::move(body)});
                    continue;
                }
                else if(*directive.value == "macro")
                {
                    auto identifier = match(Token::Type::Identifier);
                    std::vector<std::string> arguments;
                    std::vector<Token> body;
                    match(Token::Type::ParenthesisLeft);

                    while(peek() && peek()->type == Token::Type::Identifier)
                    {
                        bool end_parenthesis{false};

                        auto argument = consume();
                        auto delimiter = peek() && peek()->type == Token::Type::ParenthesisRight? (end_parenthesis = true, consume()): match(Token::Type::Comma);
                        arguments.push_back(*argument.value);
                        
                        if(end_parenthesis)
                            break;
                    }

                    while(peek() && peek()->type != Token::Type::PreprocessorSpecifier)
                        body.push_back(consume());

                    match(Token::Type::PreprocessorSpecifier);
                    m_macros.push_back(Macro{.name = *identifier.value, .arguments = arguments, .body = std::move(body)});
                    continue;
                }
                else if(*directive.value == "guard")
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                        .message = Logger::format("$ Include guard must be at the beginning of the file.", directive.info)
                    });
                    continue;
                }

                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                    .message = Logger::format("$ Invalid preprocessor directive '$'.", directive.info, *directive.value)
                });
            }

            for(std::size_t index{0ul}; index + 1ul < output.size(); ++index)
                if(output[index].type == Token::Type::Identifier && output[index + 1ul].type == Token::Type::GlueSpecifier)
                {
                    auto identifier = output[index];
                    auto glue_specifier = output[index + 1ul];
                    index += 2ul;

                    auto glued_identifier = index < output.size() && output[index].type == Token::Type::Identifier? output[index]:
                        Token{.type = Token::Type::Identifier};
                    
                    if(!glued_identifier.value)
                    {
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                            .message = Logger::format("$ Cannot glue invalid identifier.", identifier.info)
                        });
                        continue;
                    }

                    *output[index -2ul].value = *identifier.value + *glued_identifier.value;
                    output.erase(output.begin() + index - 1ul, output.begin() + index + 1ul);
                    index -= 2ul;
                    continue;
                }
            
            m_index = {};
            m_matchFailed = {};

            return output;
        }

    private:
        struct Definition
        {
            std::string name;
            std::vector<Token> body;
        };

        struct Macro
        {
            std::string name;
            std::vector<std::string> arguments;
            std::vector<Token> body;
        };

        inline void appendDefinitions(std::vector<Definition>& definitions, std::vector<Macro>& macros)
        {
            m_definitions.insert(m_definitions.end(), definitions.begin(), definitions.end());
            m_macros.insert(m_macros.end(), macros.begin(), macros.end());
        }

        static std::string filepathToDirectory(const std::string& path)
        {
        #ifdef LINC_WINDOWS
            return path.substr(0ul, path.find_last_of('\\'));
        #else
            return path.substr(0ul, path.find_last_of('/'));
        #endif
        }

        static inline std::vector<Token> embedMacroArguments(const Macro& macro, const std::vector<TokenList>& arguments)
        {
            std::vector<Token> result;
            result.reserve(std::max(macro.body.size() * 2ul, macro.body.size()));

            for(const auto& token: macro.body)
            {
                if(token.type == Token::Type::Identifier)
                {
                    bool find{false};
                    for(TokenSize i{0ul}; i < arguments.size(); ++i)
                        if(token.value == macro.arguments[i])
                        {
                            result.insert(result.end(), arguments[i].begin(), arguments[i].end());
                            find = true;
                            break;
                        }
                    
                    if(!find)
                        result.push_back(token);
                } else result.push_back(token);
            }

            return result;
        }

        [[nodiscard]] inline std::optional<Token> peek(TokenSize offset) const
        {
            if(m_index + offset > m_tokens.size() - 1ul)
                return std::nullopt; 
            return m_tokens[m_index + offset];
        }

        [[nodiscard]] inline std::optional<Token> peek() const
        {
            if(m_index > m_tokens.size() - 1ul)
                return std::nullopt; 
            return m_tokens[m_index];
        }

        inline Token consume() const
        {
            Token::Info info = peekInfo();

            if(m_index > m_tokens.size())
                return (++m_index, Token{.type = Token::Type::EndOfFile, .info = info});
            return m_tokens[m_index++];
        }

        inline Token match(Token::Type type, const std::string& error_message = "") const
        {
            Token::Info info = peekInfo();

            if(peek() && peek()->type == type)
                return consume();

            auto complete_message = error_message.empty()? Logger::format("Expected token of type '$', got '$'.",
                Token::typeToString(type), Token::typeToString(peek()->type)):
                error_message;

            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                .span = TextSpan::fromTokenInfo(info),
                .message = Logger::format("$ $", info, complete_message)
            }, !m_matchFailed);
            m_matchFailed = true;
            
            return Token{.type = type, .info = info};
        }

        [[nodiscard]] inline Token::Info peekInfo() const
        {
            return m_index < m_tokens.size()? m_tokens[m_index].info: m_tokens.back().info;
        }

        TokenList m_tokens;
        std::string m_filepath;
        mutable std::vector<std::string> m_includeDirectories{"/usr/include/", "/usr/local/include/", LINC_INSTALL_PATH "/include/"};
        mutable std::vector<Definition> m_definitions;
        mutable std::vector<Macro> m_macros;
        mutable TokenSize m_index{0ul};
        mutable bool m_matchFailed{false};
        static std::unordered_set<std::string> s_guardedFiles;
    };

    std::unordered_set<std::string> Preprocessor::s_guardedFiles;
}