#pragma once
#include <linc/system/Files.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/Include.hpp>

#define LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER ';'

namespace linc
{
    class Preprocessor final 
    {
    public:
        Preprocessor(const std::string& source_code, const std::string& working_directory = "")
            :m_sourceCode(source_code), m_workingDirectory(working_directory)
        {}
        
        std::string operator()() const
        {
            std::string output{};

            while(peek().has_value())
            {
                if(peek().value() == LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER)
                {
                    consume();
                    auto directive_name = readWord();

                    if(directive_name == "include")
                    {
                        std::string filepath_string = readStringLiteral();
                        std::string raw_source = Files::read(m_workingDirectory + "/" + filepath_string);
                        Preprocessor preprocessor(std::move(raw_source));
                        auto source = preprocessor();
                        output.append(std::move(source));
                    }
                    else if(directive_name == "instance")
                    {
                        auto replace_from = readArgument();
                        auto replace_to = readArgument();

                        if(replace_from.empty())
                            Reporting::push(Reporting::Report{
                                .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                                .message = "Preprocessor instance directive cannot have empty argument"});

                        else m_macros.push_back(Macro{.to = std::move(replace_to), .from = std::move(replace_from), .strict = true});
                    }
                    else if(directive_name == "define")
                    {
                        auto replace_from = readWord();
                        
                        if(replace_from.empty())
                            Reporting::push(Reporting::Report{
                                .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                                .message = "Preprocessor definition directive cannot have empty argument"
                            });

                        else if(!peek().has_value() || peek().value() != LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER)
                            Reporting::push(Reporting::Report{
                                .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                                .message = Logger::format("Preprocessor definition directive argument must end in '$'", 
                                    LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER)});

                        else 
                        {
                            consume();
                            auto replace_to = readArgument();
                            m_macros.push_back(Macro{.to = std::move(replace_to), .from = std::move(replace_from), .strict = false});
                        }    
                    }
                    else if(directive_name.empty())
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                            .message = "Preprocessor directive cannot be empty"
                        });
                    else
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                            .message = Logger::format("Invalid preprocessor directive '$'", directive_name)
                        });
                }
                else output.push_back(consume());
            }

            for(const auto& macro: m_macros)
            {
                if(macro.strict)
                    strictReplace(output, macro.to, macro.from);
                else wordReplace(output, macro.to, macro.from);
            }

            m_index = {};
            return output;
        }
    private:
        struct Macro
        {
            std::string to, from;
            bool strict;
        };

        inline static void strictReplace(std::string& str, std::string_view to, std::string_view from)
        {
            std::string::size_type index{};
            
            while (std::string::npos != (index = str.find(from, index)))
            {
                str.replace(index, from.size(), to);
                index += to.size();
            }
        }

        inline static void wordReplace(std::string& str, std::string_view to, std::string_view from)
        {
            std::string::size_type index{};
            
            while (std::string::npos != (index = str.find(from, index)))
            {
                auto next_index = index + from.size();

                if((index != 0 && isWordCharacter(str.at(index - 1))) || (next_index < str.size() && isWordCharacter(str.at(next_index))))
                {
                    index = next_index;
                    continue;
                }

                str.replace(index, from.size(), to);
                index += to.size();
            }
        }

        inline std::string readWord() const
        {
            ignoreSpace();
            std::string str{};

            while(peek().has_value() && isWordCharacter(peek().value()))
                str.push_back(consume());
        
            return std::move(str);
        }

        inline std::string readStringLiteral() const
        {
            ignoreSpace();
            std::string result{};

            if(peek().value() == '"')
            {
                consume();
                do
                {
                    if(peek() == '\n' || peek() == '\0')
                    {
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                            .message = "Unmatched double-quote in preprocessor directive"});
                        
                        break;
                    }
                    else if(peek(1).has_value() && peek(2).has_value()
                        && peek().value() != '\\' && peek(1).value() == '\\' && peek(2).value() == '"')
                    {
                        result.push_back(consume()); // Push the first character (non-escape)
                        consume(); // Consume the '\' character (do not push)
                        result.push_back(consume()); // Push the quote
                        continue;
                    }

                    result.push_back(consume());
                } while (peek().has_value() && peek().value() != '"');

                consume(); // Consume ending quote
            }
            return result;
        }

        inline std::string readUntilSpace() const 
        {
            ignoreSpace();
            std::string result{};

            while(peek().has_value() && !std::isspace(peek().value()))
                result.push_back(consume());

            return result;
        }

        inline std::string readArgument() const 
        {
            ignoreSpace();
            std::string result{};

            while(peek().has_value() && peek().value() != LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER)
                result.push_back(consume());

            if(!peek().has_value())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Preprocessor,
                    .message = Logger::format("Preprocessor directive argument must end in '$'", LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER)});
            
            else consume();

            return result;
        }

        inline void ignoreSpace() const
        {
            while(peek().has_value() && std::isspace(peek().value()))
                consume();
        }

        [[nodiscard]] inline std::optional<char> peek() const
        {
            if(m_index >= m_sourceCode.size())
                return std::nullopt; 
            return m_sourceCode[m_index];
        }

        [[nodiscard]] inline std::optional<char> peek(std::string::size_type offset) const
        {
            if(m_index + offset > m_sourceCode.size() - 1)
                return std::nullopt; 
            return m_sourceCode[m_index + offset];
        }

        inline char consume() const
        {
            return m_sourceCode[m_index++];
        }

        [[nodiscard]] inline static bool isWordCharacter(char character)
        {
            return std::isalnum(character) || character == '_';
        }

        const std::string m_sourceCode, m_workingDirectory;
        mutable std::string::size_type m_index{};
        mutable std::vector<Macro> m_macros;
    };
}