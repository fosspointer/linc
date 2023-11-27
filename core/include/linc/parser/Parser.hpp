#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/LiteralExpression.hpp>
#include <linc/tree/UnaryExpression.hpp>
#include <linc/tree/BinaryExpression.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/ParenthesisExpression.hpp>
#include <linc/tree/ExpressionStatement.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/tree/ScopeStatement.hpp>
#include <linc/tree/VariableDeclarationStatement.hpp>

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

        inline const Statement* parseStatement() const 
        {
            if(peek()->type == Token::Type::BraceLeft)
            {
                auto left_brace_token = consume();
                std::vector<const Statement*> statements;
                
                do
                {
                    statements.push_back(parseStatement());
                } while(!peek()->isEndOfFile() && peek()->type != Token::Type::BraceRight);
                
                auto right_brace_token = match(Token::Type::BraceRight);
                return new ScopeStatement(left_brace_token, right_brace_token, statements);
            }
            else if(peek()->isIdentifier() && peek(1).has_value() && peek(1)->isIdentifier())
            {
                auto typename_identifier_token = consume();
                auto varname_identifier_token = consume();
                
                auto assignment_operator_token = match(Token::Type::OperatorAssignment);
                auto value = parseExpression();
                    
                auto varname_identifier_expression = new IdentifierExpression(varname_identifier_token);

                return new VariableDeclarationStatement(typename_identifier_token, assignment_operator_token, value, varname_identifier_expression);
            }
            else return new ExpressionStatement(parseExpression());
        }

        inline Expression* parseExpression(uint16_t parent_precedence = 0) const
        {
            Expression* left;

            if (peek()->isUnaryOperator() && Operators::getUnaryPrecedence(peek()->type) > parent_precedence)
            {
                auto operator_token = consume();
                auto operand = parsePrimaryExpression();
                left = new UnaryExpression(operator_token, operand);
            }
            else left = parsePrimaryExpression();

            while(peek()->isBinaryOperator())
            {
                const auto precedence = Operators::getBinaryPrecedence(peek()->type);
                
                if(precedence <= parent_precedence)
                    break;

                auto operator_token = consume();
                auto right = parseExpression(precedence);
                left = new BinaryExpression(operator_token, left, right);
                
                if(peek()->type == Token::Type::EndOfFile)
                    break;               
            }
            
            return left;
        }

        Expression* parsePrimaryExpression() const
        {
            if(peek()->type == Token::Type::ParenthesisLeft)
            {   
                auto left_parenthesis = consume();
                auto expression = parseExpression();
                auto right_parenthesis = match(Token::Type::ParenthesisRight);
                return new ParenthesisExpression(left_parenthesis, right_parenthesis, expression);
            }
            if(peek()->isLiteral())
            {
                auto literal_token = consume();
                return new LiteralExpression(literal_token);
            }
            if(peek()->isIdentifier())
            {
                auto identifier_token = consume();
                return new IdentifierExpression(identifier_token);
            }
            else
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = linc::Logger::format("Expected primary expression, got '$'.", Token::typeToString(peek()->type))
                });
                consume();
                return new LiteralExpression(Token{.type = Token::Type::InvalidToken, .value = "0"});
            }
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