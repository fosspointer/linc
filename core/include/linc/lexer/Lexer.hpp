#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <linc/system/Reporting.hpp>
#include <linc/lexer/Token.hpp>
#include <linc/lexer/Operators.hpp>
#include <linc/lexer/Brackets.hpp>
#include <linc/lexer/Keywords.hpp>

namespace linc
{
    class Lexer final
    {
    public:
        explicit Lexer(const std::string& source_code)
            :m_sourceCode(source_code)
        {}

        std::vector<Token> operator()() const
        {
            std::vector<Token> tokens;

            while(peek().has_value())
            {
                std::string value_buffer;

                if(peek().value() == '\n')
                {
                    m_lineNumber++;
                    m_lineBuffer = m_sourceCode.substr(m_index + 1, m_sourceCode.find_first_of('\n'));
                }

                if(tokenizeSpace());
                else if(tokenizeLiterals(tokens, value_buffer));
                else if(tokenizeWords(tokens, value_buffer));
                else if(tokenizeComments());
                else if(tokenizeBrackets(tokens, value_buffer)); 
                else tokenizeOperators(tokens, value_buffer);
            }

            tokens.push_back(Token{.type = Token::Type::EndOfFile, .lineNumber = m_lineNumber});

            m_lineNumber = m_index = {};
            return tokens;
        }
    private:
        static bool isSymbol(char c)
        {
            return std::string("!@#$%^&*-=+~`|<>:/").contains(c);
        }

        static bool digitHandle(char c, size_t* decimal_count)
        {
            if(c == '.')
                return ++(*decimal_count);
            
            else return std::isdigit(c);
        }

        [[nodiscard]] inline bool tokenizeSpace() const
        {
            if(std::isspace(peek().value()))
            {
                consume();
                return true;
            }
            return false;
        }

        [[nodiscard]] inline bool tokenizeComments() const
        {
            if(peek().value() == '#')
            {
                while (peek().has_value() && peek().value() != '\n')
                    consume();
                return true;
            }
            return false;
        }

        // Tokenization for all literals (int literal, string literal, etc...)
        [[nodiscard]] inline bool tokenizeLiterals(std::vector<Token>& tokens, std::string& value_buffer) const
        {
            if(tokenizeLiteralNumber(tokens, value_buffer))
                return true;
            else if(tokenizeLiteralString(tokens, value_buffer))
                return true;
            else if(tokenizeLiteralCharacter(tokens, value_buffer))
                return true;
            return false;
        }

        [[nodiscard]] inline bool tokenizeLiteralNumber(std::vector<Token>& tokens, std::string& value_buffer) const
        {
            std::string type_string;
            size_t decimal_count{};
            if(digitHandle(peek().value(), &decimal_count))
            {
                do
                {
                    value_buffer.push_back(consume());
                } while(peek().has_value() && digitHandle(peek().value(), &decimal_count));
                
                while(peek().has_value() && isalnum(peek().value()))
                    type_string.push_back(consume());

                if(type_string.empty())
                {
                    if(decimal_count == 0)
                        tokens.push_back(Token{.type = Token::Type::I32Literal, .value = value_buffer, .lineNumber = m_lineNumber});
                    else if(decimal_count == 1)
                        tokens.push_back(Token{.type = Token::Type::F32Literal, .value = value_buffer, .lineNumber = m_lineNumber});
                    else
                        tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer, .lineNumber = m_lineNumber});
                    return true;
                }

                auto type = Types::fromUserStringSuffix(type_string);

                switch(type)
                {
                case Types::Type::i8: tokens.push_back(Token{.type = Token::Type::I8Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::i16: tokens.push_back(Token{.type = Token::Type::I16Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::i32: tokens.push_back(Token{.type = Token::Type::I32Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::i64: tokens.push_back(Token{.type = Token::Type::I64Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::u8: tokens.push_back(Token{.type = Token::Type::U8Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::u16: tokens.push_back(Token{.type = Token::Type::U16Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::u32: tokens.push_back(Token{.type = Token::Type::U32Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::u64: tokens.push_back(Token{.type = Token::Type::U64Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::f32: tokens.push_back(Token{.type = Token::Type::F32Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::f64: tokens.push_back(Token{.type = Token::Type::F64Literal, .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::_bool: tokens.push_back(Token{.type = Types::parseBoolean(value_buffer)? Token::Type::KeywordTrue: Token::Type::KeywordFalse,
                    .value = value_buffer, .lineNumber = m_lineNumber}); break;
                case Types::Type::_char: tokens.push_back(Token{.type = Token::Type::CharacterLiteral, .value = value_buffer}); break;
                default: tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer + type_string, .lineNumber = m_lineNumber}); break;
                }
                return true;
            }
            else return false;
        }

        [[nodiscard]] inline bool tokenizeLiteralString(std::vector<Token>& tokens, std::string& value_buffer) const
        {
            if(peek().value() == '"')
            {
                consume(); // Consume beginning quote
                do
                {
                    if(peek() == '\n' || peek() == '\0')
                    {
                        tokens.push_back(linc::Token{.type = linc::Token::Type::InvalidToken, .value = value_buffer, .lineNumber = m_lineNumber});
                        
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                            .span = TextSpan{.text = m_lineBuffer, .spanStart = 0, .spanEnd = 0},
                            .message = "Unmatched double-quotes"});
                        
                        break;
                    }
                    else if(peek(1).has_value() && peek(2).has_value()
                        && peek().value() != '\\' && peek(1).value() == '\\' && peek(2).value() == '"')
                    {
                        value_buffer.push_back(consume()); // Push the first character (non-escape)
                        consume(); // Consume the '\' character (do not push)
                        value_buffer.push_back(consume()); // Push the quote
                        continue;
                    }

                    value_buffer.push_back(consume());
                } while (peek().has_value() && peek().value() != '"');

                tokens.push_back(linc::Token{.type = linc::Token::Type::StringLiteral, .value = value_buffer, .lineNumber = m_lineNumber});
                consume(); // Consume ending quote
                return true;
            }
            return false;
        }

        [[nodiscard]] inline bool tokenizeLiteralCharacter(std::vector<Token>& tokens, std::string& value_buffer) const
        {
            if(peek().value() == '\'')
            {
                consume(); // Consume beginning quote
                do
                {
                    if(peek() == '\n')
                    {
                        tokens.push_back(linc::Token{.type = linc::Token::Type::InvalidToken, .value = value_buffer, .lineNumber = m_lineNumber});
                        
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                            .message = "Unmatched single-quotes"});
                        
                        break;
                    }
                    else if(peek(1).has_value() && peek(2).has_value()
                        && peek().value() != '\\' && peek(1).value() == '\\' && peek(2).value() == '\'')
                    {
                        value_buffer.push_back(consume()); // Push the first character (non-escape)
                        consume(); // Consume the '\' character (do not push)
                        value_buffer.push_back(consume()); // Push the quote
                        continue;
                    }

                    value_buffer.push_back(consume());
                } while (peek().has_value() && peek().value() != '\'');

                tokens.push_back(linc::Token{.type = linc::Token::Type::CharacterLiteral, .value = value_buffer, .lineNumber = m_lineNumber});
                consume(); // Consume ending quote
                return true;
            }
            return false;
        }

        // Tokenization method for keywords and identifiers
        [[nodiscard]] inline bool tokenizeWords(std::vector<Token>& tokens, std::string& value_buffer) const
        {
            if(std::isalpha(peek().value()) || peek().value() == '_')
            {
                do
                {
                    value_buffer.push_back(consume());
                } while (peek().has_value() && (std::isalnum(peek().value()) || peek().value() == '_'));

                auto token_type = Keywords::get(value_buffer);
                
                if(token_type == Token::Type::KeywordTrue || token_type == Token::Type::KeywordFalse)
                    tokens.push_back(Token{.type = token_type, .value = value_buffer, .lineNumber = m_lineNumber});
                else if(token_type != Token::Type::InvalidToken)
                    tokens.push_back(Token{.type = token_type, .lineNumber = m_lineNumber});
                else
                    tokens.push_back(Token{.type = Token::Type::Identifier, .value = value_buffer, .lineNumber = m_lineNumber});
                return true;
            }
            return false;
        }

        [[nodiscard]] inline bool tokenizeBrackets(std::vector<Token>& tokens, std::string& value_buffer) const
        {
            auto bracket = Brackets::getBracket(peek().value());

            if(bracket != Token::Type::InvalidToken)
            {
                tokens.push_back(Token{.type = bracket, .lineNumber = m_lineNumber});
                consume();
                return true;
            }
            return false;
        }

        inline void tokenizeOperators(std::vector<Token>& tokens, std::string& value_buffer) const
        {
            std::string symbol;
            while (peek().has_value() && isSymbol(peek().value()))
                symbol.push_back(consume());

            if(!symbol.empty())
                tokens.push_back(Token{.type = Operators::get(symbol), .lineNumber = m_lineNumber});
            else 
            {
                consume();
                tokens.push_back(Token{.type = Token::Type::InvalidToken, .lineNumber = m_lineNumber});
            }
        }

        [[nodiscard]] inline std::optional<char> peek(std::string::size_type offset) const
        {
            if(m_index + offset > m_sourceCode.size() - 1)
                return std::nullopt; 
            return m_sourceCode[m_index + offset];
        }

        [[nodiscard]] inline std::optional<char> peek() const
        {
            if(m_index >= m_sourceCode.size())
                return std::nullopt; 
            return m_sourceCode[m_index];
        }

        inline char consume() const
        {
            return m_sourceCode[m_index++];
        }

        const std::string m_sourceCode;
        mutable std::string::size_type m_index{};
        mutable size_t m_lineNumber{0};
        mutable std::string m_lineBuffer;
    };
}