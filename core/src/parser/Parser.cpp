#include <linc/parser/Parser.hpp>

namespace linc
{
    Parser::Parser(std::vector<Token> tokens)
            :m_tokens(std::move(tokens))
    {}

    Program Parser::operator()() const
    {
        Program program;

        while(auto declaration = parseDeclaration())
            program.declarations.push_back(std::move(declaration));

        auto end_of_file_token = match(Token::Type::EndOfFile);
        m_index = {};
        
        return std::move(program);
    }
    
    std::unique_ptr<const Statement> Parser::parseStatement() const 
    {
        if(peek()->type == Token::Type::BraceLeft)
        {
            auto left_brace = consume();
            std::vector<std::unique_ptr<const Statement>> statements;
            
            while(!peek()->isEndOfFile() && peek()->type != Token::Type::BraceRight)
                statements.push_back(std::move(parseStatement()));
            
            auto right_brace = match(Token::Type::BraceRight);
            return std::make_unique<const ScopeStatement>(left_brace, right_brace, std::move(statements));
        }
        else if(peek()->type == Token::Type::Identifier && peek()->value.has_value() && peek()->value == "putc")
        {
            auto identifier = consume();
            auto left_parenthesis = match(Token::Type::ParenthesisLeft);
            auto expression = parseExpression();
            auto right_parenthesis = match(Token::Type::ParenthesisRight);

            return std::make_unique<const PutCharacterStatement>(identifier, left_parenthesis, right_parenthesis, std::move(expression));
        }
        else if(peek()->type == Token::Type::Identifier && peek()->value.has_value() && peek()->value == "puts")
        {
            auto identifier = consume();
            auto left_parenthesis = match(Token::Type::ParenthesisLeft);
            auto expression = parseExpression();
            auto right_parenthesis = match(Token::Type::ParenthesisRight);

            return std::make_unique<const PutStringStatement>(identifier, left_parenthesis, right_parenthesis, std::move(expression));
        }
        else if(auto declaration = parseDeclaration())
            return std::make_unique<const DeclarationStatement>(std::move(declaration));
        else 
            return std::make_unique<const ExpressionStatement>(std::move(parseExpression()));
    }

    std::unique_ptr<const Expression> Parser::parseExpression(uint16_t parent_precedence) const
    {
        std::unique_ptr<const Expression> left;

        if (peek()->isUnaryOperator() && Operators::getUnaryPrecedence(peek()->type) > parent_precedence)
        {
            auto _operator = consume();
            auto operand = parsePrimaryExpression();
            left = std::make_unique<const UnaryExpression>(_operator, std::move(operand->clone_const()));
        }
        else left = parsePrimaryExpression();

        while(peek()->isBinaryOperator())
        {
            const auto precedence = Operators::getBinaryPrecedence(peek()->type);
            
            if(precedence <= parent_precedence)
                break;

            auto _operator = consume();
            auto right = parseExpression(precedence);
            left = std::make_unique<const BinaryExpression>(_operator, std::move(left->clone_const()), std::move(right));
            
            if(peek()->type == Token::Type::EndOfFile)
                break;               
        }
        
        return left;
    }

    std::unique_ptr<const Expression> Parser::parsePrimaryExpression() const
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
            auto literal = consume();
            return std::make_unique<const LiteralExpression>(literal);
        }

        auto identifier = match(Token::Type::Identifier);
            
        if(peek().has_value() && peek()->type == Token::Type::ParenthesisLeft)
        {
            auto left_parenthesis = consume();
            std::vector<std::unique_ptr<const Expression>> arguments;

            while(peek().has_value() && peek()->type != Token::Type::ParenthesisRight)
                arguments.push_back(parseExpression());
                
            auto right_parenthesis = match(Token::Type::ParenthesisRight);

            return std::make_unique<const FunctionCallExpression>(identifier, left_parenthesis, right_parenthesis, std::move(arguments));
        }
        
        else if(peek().has_value() && peek()->type == Token::Type::OperatorAssignment)
        {
            auto equals = consume();
            auto expression = parseExpression();

            return std::make_unique<const VariableAssignmentExpression>(equals, identifier, std::move(expression));
        }
        else return std::make_unique<const IdentifierExpression>(identifier);
    }

    std::unique_ptr<const Declaration> Parser::parseDeclaration() const
    {
        bool is_mutable{};
        if((peek()->isIdentifier() && peek(1).has_value() && peek(1)->isIdentifier()) 
        || (is_mutable = peek()->type == Token::Type::KeywordMutability))
        {
            std::optional<Token> mut_keyword;
            if(is_mutable)
                mut_keyword = consume();

            auto typename_identifier = match(Token::Type::Identifier);
            auto varname_identifier = match(Token::Type::Identifier);
            
            auto assignment__operator = match(Token::Type::OperatorAssignment);
            auto value = parseExpression();
                
            auto varname_identifier_expression = std::make_unique<const IdentifierExpression>(varname_identifier);

            return std::make_unique<const VariableDeclaration>
                (typename_identifier, assignment__operator, mut_keyword, std::move(value), std::move(varname_identifier_expression));
        }
        else if (peek().value().type == Token::Type::KeywordFunction)
        {
            auto function_keyword = consume();
            auto function_name = match(Token::Type::Identifier);
            auto left_parenthesis = match(Token::Type::ParenthesisLeft);
            
            std::vector<std::unique_ptr<const ArgumentDeclaration>> arguments;

            while(peek().has_value() && peek()->type != Token::Type::ParenthesisRight)
            {
                std::optional<Token> mutability_keyword{}, assignment_operator{};
                std::optional<std::unique_ptr<const Expression>> default_value{};
                
                if(peek().has_value() && peek()->type == Token::Type::KeywordMutability)
                    mutability_keyword = consume();

                auto typename_identifier = match(Token::Type::Identifier);
                auto varname_identifier = match(Token::Type::Identifier);

                if(peek().has_value() && peek()->type == Token::Type::OperatorAssignment)
                {
                    assignment_operator = consume();
                    default_value = parseExpression();
                }

                arguments.push_back(std::make_unique<const ArgumentDeclaration>(typename_identifier, varname_identifier,
                    mutability_keyword, assignment_operator, default_value.has_value()? std::move(default_value.value()):
                    std::optional<std::unique_ptr<const Expression>>{}));
            }

            auto right_parenthesis = match(Token::Type::ParenthesisRight);
            auto type_operator = match(Token::Type::OperatorType);
            auto return_type = match(Token::Type::Identifier);
            auto body = parseStatement();

            return std::make_unique<const FunctionDeclaration>(function_keyword, function_name, left_parenthesis, right_parenthesis,
                type_operator, return_type, std::move(arguments), std::move(body));
        }
        else return nullptr;
    }
}