#pragma once
#include <memory>
#include <linc/Tree.hpp>

namespace linc
{
    class Parser final 
    {
    public:
        using TokenList = std::vector<Token>;
        using TokenSize = TokenList::size_type;

        Parser(std::vector<Token> tokens)
            :m_tokens(std::move(tokens))
        {}

        auto operator()() const 
        {
            auto statement = parseStatement();
            auto end_of_file = match(Token::Type::EndOfFile);
            m_index = 0;
            
            return statement;
        }

        inline std::unique_ptr<const Statement> parseStatement() const 
        {
            if(peek()->type == Token::Type::BraceLeft)
            {
                auto left_brace_token = consume();
                std::vector<std::unique_ptr<const Statement>> statements;
                
                do
                {
                    statements.push_back(std::move(parseStatement()));
                } while(!peek()->isEndOfFile() && peek()->type != Token::Type::BraceRight);
                
                auto right_brace_token = match(Token::Type::BraceRight);
                return std::make_unique<ScopeStatement>(left_brace_token, right_brace_token, std::move(statements));
            }
            else if(peek()->isIdentifier() && peek(1).has_value() && peek(1)->isIdentifier())
            {
                auto typename_identifier_token = consume();
                auto varname_identifier_token = consume();
                
                auto assignment_operator_token = match(Token::Type::OperatorAssignment);
                auto value = parseExpression();
                    
                auto varname_identifier_expression = std::make_unique<const IdentifierExpression>(varname_identifier_token);

                return std::make_unique<const VariableDeclarationStatement>
                    (typename_identifier_token, assignment_operator_token, std::move(value), std::move(varname_identifier_expression));
            }
            else return std::make_unique<const ExpressionStatement>(std::move(parseExpression()));
        }

        inline std::unique_ptr<const Expression> parseExpression(uint16_t parent_precedence = 0) const
        {
            std::unique_ptr<const Expression> left;

            if (peek()->isUnaryOperator() && Operators::getUnaryPrecedence(peek()->type) > parent_precedence)
            {
                auto operator_token = consume();
                auto operand = parsePrimaryExpression();
                left = std::make_unique<const UnaryExpression>(operator_token, std::move(operand->clone_const()));
            }
            else left = parsePrimaryExpression();

            while(peek()->isBinaryOperator())
            {
                const auto precedence = Operators::getBinaryPrecedence(peek()->type);
                
                if(precedence <= parent_precedence)
                    break;

                auto operator_token = consume();
                auto right = parseExpression(precedence);
                left = std::make_unique<const BinaryExpression>(operator_token, std::move(left->clone_const()), std::move(right));
                
                if(peek()->type == Token::Type::EndOfFile)
                    break;               
            }
            
            return left;
        }

        std::unique_ptr<const Expression> parsePrimaryExpression() const
        {
            if(peek()->type == Token::Type::ParenthesisLeft)
            {   
                auto left_parenthesis = consume();
                auto expression = parseExpression();
                auto right_parenthesis = match(Token::Type::ParenthesisRight);
                return std::make_unique<ParenthesisExpression>(left_parenthesis, right_parenthesis, std::move(expression));
            }
            if(peek()->isLiteral())
            {
                auto literal_token = consume();
                return std::make_unique<LiteralExpression>(literal_token);
            }

            auto identifier_token = match(Token::Type::Identifier);
            return std::make_unique<IdentifierExpression>(identifier_token);
        }

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

        inline Token consume() const
        {
            if(m_index + 1 >= m_tokens.size())
                return Token{.type = Token::Type::EndOfFile};
            return m_tokens[m_index++];
        }

        Token match(Token::Type type) const
        {
            if(peek().has_value() && peek()->type == type)
                return consume();

            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("Expected token of type '$', got '$'.", Token::typeToString(type), Token::typeToString(peek()->type))});
            
            return Token{.type = type, .lineNumber = peek()->lineNumber};
        }

        TokenList m_tokens;
        mutable TokenSize m_index{0};
    };
}