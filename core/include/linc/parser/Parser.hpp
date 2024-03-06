#pragma once
#include <linc/parser/Program.hpp>
#include <linc/lexer/Operators.hpp>
#include <linc/Tree.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Parser final 
    {
    public:
        using TokenList = std::vector<Token>;
        using TokenSize = TokenList::size_type;

        Parser(std::vector<Token> tokens);
        Program operator()() const;
        std::unique_ptr<const Statement> parseStatement() const;

        std::unique_ptr<const Declaration> parseDeclaration() const;
        std::unique_ptr<const VariableDeclaration> parseVariableDeclaration() const;
        std::unique_ptr<const FunctionDeclaration> parseFunctionDeclaration() const;

        std::unique_ptr<const Expression> parseExpression(uint16_t parent_precedence = 0) const;
        std::unique_ptr<const Expression> parsePrimaryExpression() const;
        std::unique_ptr<const LiteralExpression> parseLiteralExpression() const;
        std::unique_ptr<const IdentifierExpression> parseIdentifierExpression() const;
        std::unique_ptr<const TypeExpression> parseTypeExpression() const;

        inline auto parseEndOfFile() const { return match(Token::Type::EndOfFile); }
    private:
        [[nodiscard]] inline std::optional<Token> peek(TokenSize offset) const
        {
            if(m_index + offset > m_tokens.size() - 1)
                return std::nullopt; 
            return m_tokens[m_index + offset];
        }

        [[nodiscard]] inline std::optional<Token> peek() const
        {
            if(m_index > m_tokens.size() - 1)
                return std::nullopt; 
            return m_tokens[m_index];
        }

        [[nodiscard]] inline Token consume() const
        {
            if(m_index + 1 >= m_tokens.size())
                return Token{.type = Token::Type::EndOfFile};
            return m_tokens[m_index++];
        }

        [[nodiscard]] inline Token match(Token::Type type) const
        {
            if(peek().has_value() && peek()->type == type)
                return consume();

            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$::$ Expected token of type '$', got '$'.", peek()->info.file, peek()->info.line, Token::typeToString(type),
                    Token::typeToString(peek()->type))}, !m_matchFailed);
            
            m_matchFailed = true;
            
            return Token{.type = type, .info = peek()->info};
        }

        mutable bool m_matchFailed{};
        TokenList m_tokens;
        mutable TokenSize m_index{0};
    };
}