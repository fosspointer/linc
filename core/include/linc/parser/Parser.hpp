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
                
                while(!peek()->isEndOfFile() && peek()->type != Token::Type::BraceRight)
                    statements.push_back(std::move(parseStatement()));
                
                auto right_brace_token = match(Token::Type::BraceRight);
                return std::make_unique<const ScopeStatement>(left_brace_token, right_brace_token, std::move(statements));
            }
            else if(peek()->isIdentifier() && peek(1).has_value() && peek(1)->isIdentifier())
            {
                auto typename_identifier_token = consume();
                auto varname_identifier_token = consume();
                
                std::optional<Token> var_keyword_token;
                if(peek().has_value() && peek()->type == Token::Type::KeywordMutability)
                    var_keyword_token = consume();
                
                auto assignment_operator_token = match(Token::Type::OperatorAssignment);
                auto value = parseExpression();
                    
                auto varname_identifier_expression = std::make_unique<const IdentifierExpression>(varname_identifier_token);

                return std::make_unique<const VariableDeclarationStatement>
                    (typename_identifier_token, assignment_operator_token, var_keyword_token, std::move(value), std::move(varname_identifier_expression));
            }
            else if(peek()->type == Token::Type::Identifier && peek()->value.has_value() && peek()->value == "putc")
            {
                auto identifier_token = consume();
                auto left_parenthesis = match(Token::Type::ParenthesisLeft);
                auto expression = parseExpression();
                auto right_parenthesis = match(Token::Type::ParenthesisRight);

                return std::make_unique<const PutCharacterStatement>(identifier_token, left_parenthesis, right_parenthesis, std::move(expression));
            }
            else if(peek()->type == Token::Type::Identifier && peek()->value.has_value() && peek()->value == "puts")
            {
                auto identifier_token = consume();
                auto left_parenthesis = match(Token::Type::ParenthesisLeft);
                auto expression = parseExpression();
                auto right_parenthesis = match(Token::Type::ParenthesisRight);

                return std::make_unique<const PutStringStatement>(identifier_token, left_parenthesis, right_parenthesis, std::move(expression));
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
                return std::make_unique<const ParenthesisExpression>(left_parenthesis, right_parenthesis, std::move(expression));
            }
            else if(peek()->type == Token::Type::KeywordIf)
            {
                auto if_keyword = consume();
                auto check_expression = parseExpression();
                auto if_body_statement = parseStatement();

                if(peek().has_value() && peek()->type == Token::Type::KeywordElse)
                {
                    auto else_keyword = consume();
                    auto else_body_statement = parseStatement();
                    return std::make_unique<const IfElseExpression>(if_keyword, std::move(check_expression), std::move(if_body_statement), else_keyword,
                        std::move(else_body_statement));
                }
                else return std::make_unique<const IfElseExpression>(if_keyword, std::move(check_expression), std::move(if_body_statement));
            }
            else if(peek()->type == Token::Type::KeywordWhile)
            {
                auto while_keyword = consume();
                auto check_expression = parseExpression();
                auto body_while_statement = parseStatement();

                if(peek().has_value() && peek()->type == Token::Type::KeywordFinally)
                {
                    auto finally_keyword = consume();
                    auto body_finally_statement = parseStatement();
                    
                    if(peek().has_value() && peek()->type == Token::Type::KeywordElse)
                    {
                        auto else_keyword = consume();
                        auto body_else_statement = parseStatement();

                        return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement), 
                            finally_keyword, std::move(body_finally_statement), else_keyword, std::move(body_else_statement));
                    }
                    else return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement), 
                        finally_keyword, std::move(body_finally_statement));
                }
                else if(peek().has_value() && peek()->type == Token::Type::KeywordElse)
                {
                    auto else_keyword = consume();
                    auto body_else_statement = parseStatement();

                    return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement), 
                        std::nullopt, std::nullopt, else_keyword, std::move(body_else_statement));
                }

                else return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement));
            }
            else if(peek()->isLiteral())
            {
                auto literal_token = consume();
                return std::make_unique<const LiteralExpression>(literal_token);
            }

            auto identifier_token = match(Token::Type::Identifier);

            if(peek().has_value() && peek()->type == Token::Type::OperatorAssignment)
            {
                auto equals_token = consume();
                auto expression = parseExpression();

                return std::make_unique<const VariableAssignmentExpression>(equals_token, identifier_token, std::move(expression));
            }
            else return std::make_unique<const IdentifierExpression>(identifier_token);
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