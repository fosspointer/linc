#pragma once
#include <linc/system/Files.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/system/Code.hpp>
#include <linc/Include.hpp>

#define LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER ';'

namespace linc
{
    class Preprocessor final 
    {
    public:
        Preprocessor(const std::string& raw_source_code, const std::string& filepath = "")
            :m_sourceCode(Code::toSource(raw_source_code, filepath)), m_filepath(filepath)
        {}

        Preprocessor(const Code::Source& source_code, const std::string& filepath = "")
            :m_sourceCode(source_code), m_filepath(filepath)
        {}
        
        Code::Source operator()() const
        {
            Code::Source output{Code::Line{
                .text = "",
                .file = m_filepath,
                .line = m_lineIndex + 1
            }};

            output.reserve(m_sourceCode.size());

            while(peek().has_value())
            {
                if(peek().value() == LINC_PREPROCESSOR_DIRECTIVE_SPECIFIER)
                {
                    consume();
                    auto directive_name = readWord();

                    if(directive_name == "include")
                    {
                        auto relative_filepath = readStringLiteral();
                        auto filepath = filepathToDirectory(m_filepath) + "/" + relative_filepath;

                        std::string raw_source = Files::read(filepath);
                        Code::Source source = Code::toSource(raw_source, filepath);

                        Preprocessor preprocessor(std::move(source), filepath);
                        auto new_source = preprocessor();
                        
                        Code::append(output, std::move(new_source));
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
                else if(peek().value() == '\n')
                {
                    consume();
                    output.push_back(Code::Line{
                        .text = "",
                        .file = m_filepath,
                        .line = m_lineIndex + 1
                    });
                }
                else
                    output.back().text.push_back(consume());
            }

            for(const auto& macro: m_macros)
            {
                if(macro.strict)
                    strictReplace(output, macro.to, macro.from);
                else wordReplace(output, macro.to, macro.from);
            }

            m_characterIndex = m_lineIndex = {};
            return output;
        }
    private:
        static std::string filepathToDirectory(const std::string& path)
        {
            return path.substr(0ull, path.find_last_of('/'));
        }

        struct Macro
        {
            std::string to, from;
            bool strict;
        };

        inline static void strictReplace(Code::Source& output, std::string_view to, std::string_view from)
        {
            std::string::size_type index{};
            
            for(auto& line : output)
                while (std::string::npos != (index = line.text.find(from, index)))
                {
                    line.text.replace(index, from.size(), to);
                    index += to.size();
                }
        }

        inline static void wordReplace(Code::Source& output, std::string_view to, std::string_view from)
        {
            std::string::size_type index{};
            
            for(auto& line : output)
                while (std::string::npos != (index = line.text.find(from, index)))
                {
                    auto next_index = index + from.size();

                    if((index != 0 && isWordCharacter(line.text.at(index - 1))) || (next_index < line.text.size() && isWordCharacter(line.text.at(next_index))))
                    {
                        index = next_index;
                        continue;
                    }

                    line.text.replace(index, from.size(), to);
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

        [[nodiscard]] inline std::optional<char> peek(std::string::size_type offset = 0ull) const
        {
            return Code::peek(m_sourceCode, m_characterIndex, m_lineIndex, offset);
        }

        inline char consume() const
        {
            return Code::consume(m_sourceCode, m_characterIndex, m_lineIndex);
        }

        [[nodiscard]] inline static bool isWordCharacter(char character)
        {
            return std::isalnum(character) || character == '_';
        }

        const Code::Source m_sourceCode;
        const std::string m_filepath;
        mutable std::string::size_type m_characterIndex{}, m_lineIndex{};
        mutable std::vector<Macro> m_macros;
    };
}