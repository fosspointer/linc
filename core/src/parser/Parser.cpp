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
        else if(auto expression = parseExpression())
            return std::make_unique<const ExpressionStatement>(std::move(expression));
        else return nullptr;
    }

    std::unique_ptr<const Expression> Parser::parseExpression(uint16_t parent_precedence) const
    {
        std::unique_ptr<const Expression> left;

        if(peek()->isUnaryOperator() && Operators::getUnaryPrecedence(peek()->type) > parent_precedence)
        {
            auto _operator = consume();
            auto operand = parsePrimaryExpression();
            left = std::make_unique<const UnaryExpression>(_operator, operand->cloneConst());
        }
        else left = parsePrimaryExpression();

        while(peek()->isBinaryOperator())
        {
            const auto precedence = Operators::getBinaryPrecedence(peek()->type);
            
            if(precedence <= parent_precedence)
                break;

            auto _operator = consume();
            auto right = parseExpression(precedence);

            left = std::make_unique<const BinaryExpression>(_operator, left->cloneConst(), std::move(right));

            if(peek()->type == Token::Type::EndOfFile)
                break;               
        }
        
        return left;
    }

    std::unique_ptr<const TypeExpression> Parser::parseTypeExpression() const
    {
        auto has_mutability_keyword = peek()->type == Token::Type::KeywordMutability;
        if(has_mutability_keyword || (peek()->isIdentifier() && Types::kindFromUserString(peek()->value.value()) != Types::Kind::invalid))
        {
            auto mutability_keyword = has_mutability_keyword? std::make_optional(consume()): std::nullopt;
            auto typename_identifier = match(Token::Type::Identifier);

            auto has_array_specifier = peek() && peek()->type == Token::Type::SquareLeft;
            auto left_bracket = has_array_specifier? std::make_optional(consume()): std::nullopt;
            auto array_size_literal = peek()->isLiteral()? std::make_optional(parseLiteralExpression()): std::nullopt;
            auto right_bracket = has_array_specifier && peek()->type == Token::Type::SquareRight? std::make_optional(consume()): std::nullopt;

            if(array_size_literal)
                return std::make_unique<const TypeExpression>(typename_identifier, mutability_keyword, left_bracket, right_bracket, std::move(array_size_literal.value()));
            else return std::make_unique<const TypeExpression>(typename_identifier, mutability_keyword, left_bracket, right_bracket);
        }
        return nullptr;
    }

    std::unique_ptr<const IdentifierExpression> Parser::parseIdentifierExpression() const
    {
        auto has_identifier = peek()->isIdentifier() && Types::kindFromUserString(peek()->value.value()) == Types::Kind::invalid;
        
        if(has_identifier)
            return std::make_unique<const IdentifierExpression>(consume());
        
        return nullptr;
    }

    std::unique_ptr<const LiteralExpression> Parser::parseLiteralExpression() const
    {
        if(peek()->isLiteral())
            return std::make_unique<const LiteralExpression>(consume());
        
        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
            .message = Logger::format("$::$ Expected literal expression, got '$'.", 
                peek()->info.file, peek()->info.line, Token::typeToString(peek()->type))
        });
        
        return std::make_unique<const LiteralExpression>(Token{.type = Token::Type::I32Literal, .value = "0", .info = peek()->info});
    }

    std::unique_ptr<const Expression> Parser::parsePrimaryExpression() const
    {
        if(peek()->type == Token::Type::SquareLeft)
        {
            auto left_bracket = consume();
            
            std::vector<std::unique_ptr<const Expression>> values;
            
            while(peek()->type != Token::Type::SquareRight)
                if(auto expression = parseExpression())
                    values.push_back(std::move(expression));
                else break;

            auto right_bracket = match(Token::Type::SquareRight);

            return std::make_unique<const ArrayInitializerExpression>(left_bracket, right_bracket, std::move(values));
        }
        else if(peek()->type == Token::Type::ParenthesisLeft)
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

            if(peek() && peek()->type == Token::Type::KeywordElse)
            {
                auto else_keyword = consume();
                auto else_body_statement = parseStatement();
                return std::make_unique<const IfElseExpression>(if_keyword, std::move(check_expression), std::move(if_body_statement), else_keyword,
                    std::move(else_body_statement));
            }
            else return std::make_unique<const IfElseExpression>(if_keyword, std::move(check_expression), std::move(if_body_statement));
        }
        else if(peek()->type == Token::Type::KeywordFor)
        {
            auto for_keyword = consume();
            auto left_parenthesis = match(Token::Type::ParenthesisLeft);

            if(peek()->type == Token::Type::KeywordIn)
            {
                auto in_keyword = consume();
                auto array_identifier = parseIdentifierExpression();
                auto value_identifier = parseIdentifierExpression();

                if(!array_identifier || !value_identifier)
                    return nullptr;

                auto right_parenthesis = match(Token::Type::ParenthesisRight);
                auto body = parseStatement();

                return std::make_unique<const ForExpression>(for_keyword, left_parenthesis, right_parenthesis,
                    in_keyword, std::move(value_identifier), std::move(array_identifier), std::move(body));
            }

            auto declaration = parseVariableDeclaration();
            auto expression = parseExpression();
            auto statement = parseStatement();

            if(!declaration || !expression || !statement)
                return nullptr;

            auto right_parenthesis = match(Token::Type::ParenthesisRight);
            auto body = parseStatement();

            return std::make_unique<const ForExpression>(for_keyword, left_parenthesis, right_parenthesis,
                std::move(declaration), std::move(expression), std::move(statement), std::move(body));
        }
        else if(peek()->type == Token::Type::KeywordWhile)
        {
            auto while_keyword = consume();
            auto check_expression = parseExpression();
            auto body_while_statement = parseStatement();

            if(peek() && peek()->type == Token::Type::KeywordFinally)
            {
                auto finally_keyword = consume();
                auto body_finally_statement = parseStatement();
                
                if(peek() && peek()->type == Token::Type::KeywordElse)
                {
                    auto else_keyword = consume();
                    auto body_else_statement = parseStatement();

                    return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement), 
                        finally_keyword, std::move(body_finally_statement), else_keyword, std::move(body_else_statement));
                }
                else return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement), 
                    finally_keyword, std::move(body_finally_statement));
            }
            else if(peek() && peek()->type == Token::Type::KeywordElse)
            {
                auto else_keyword = consume();
                auto body_else_statement = parseStatement();

                return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement), 
                    std::nullopt, std::nullopt, else_keyword, std::move(body_else_statement));
            }

            else return std::make_unique<const WhileExpression>(while_keyword, std::move(check_expression), std::move(body_while_statement));
        }

        else if(peek()->type == Token::Type::KeywordAs)
        {
            auto as_keyword = consume();
            auto type = parseTypeExpression();
            auto left_parenthesis = match(Token::Type::ParenthesisLeft);
            auto expression = parseExpression();
            auto right_parenthesis = match(Token::Type::ParenthesisRight);

            return std::make_unique<const ConversionExpression>(as_keyword, left_parenthesis, right_parenthesis, std::move(type), std::move(expression));
        }

        else if(auto type_expression = parseTypeExpression())
            return std::move(type_expression);

        else if(peek()->type == Token::Type::Identifier)
        {
            auto identifier = consume();

            if(peek()->type == Token::Type::SquareLeft)
            {
                auto left_bracket = consume();
                auto index = parseExpression();
                auto right_bracket = consume();

                return std::make_unique<const ArrayIndexExpression>(left_bracket, right_bracket,
                    std::make_unique<IdentifierExpression>(identifier), std::move(index));
            }

            else if(peek()->type == Token::Type::ParenthesisLeft)
            {
                auto left_parenthesis = consume();
                std::vector<std::unique_ptr<const Expression>> arguments;

                while(peek() && peek()->type != Token::Type::ParenthesisRight)
                    if(auto expression = parseExpression())
                        arguments.push_back(std::move(expression));
                    else break;
                    
                auto right_parenthesis = match(Token::Type::ParenthesisRight);

                return std::make_unique<const FunctionCallExpression>(identifier, left_parenthesis, right_parenthesis, std::move(arguments));
            }
            else return std::make_unique<const IdentifierExpression>(identifier);
        }
        else return parseLiteralExpression();
    }

    std::unique_ptr<const VariableDeclaration> Parser::parseVariableDeclaration() const
    {
        std::unique_ptr<const TypeExpression> type;
        
        if(peek() && peek(1) && peek()->isIdentifier() && peek(1)->type == Token::Type::TypeSpecifier)
        {
            auto identifier = parseIdentifierExpression();
            auto type_specifier = consume();
            auto type = parseTypeExpression();
            auto has_default_value = peek() && peek()->type == Token::Type::OperatorAssignment;

            std::optional<VariableDeclaration::ValueAssignment> default_value{};

            if(has_default_value)
            {
                auto assignment_operator = match(Token::Type::OperatorAssignment);
                auto expression = parseExpression();

                default_value = VariableDeclaration::ValueAssignment(assignment_operator, std::move(expression));
            }

            if(!identifier)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = "Invalid identifier expression in variable declaration"});
                return nullptr;
            }

            if(!type)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = "Invalid type expression in variable declaration"});
                return nullptr;
            }

            return std::make_unique<const VariableDeclaration>(type_specifier, std::move(type), std::move(identifier), std::move(default_value));
        }
        else return nullptr;
    }

    std::unique_ptr<const FunctionDeclaration> Parser::parseFunctionDeclaration() const
    {
        if(peek().value().type == Token::Type::KeywordFunction)
        {
            auto function_keyword = consume();
            auto function_name = parseIdentifierExpression();
            auto left_parenthesis = match(Token::Type::ParenthesisLeft);
            
            std::vector<std::unique_ptr<const VariableDeclaration>> arguments;

            while(peek() && peek()->type != Token::Type::ParenthesisRight)
            {
                auto argument = parseVariableDeclaration();

                if(!argument)
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                        .message = "Expected argument in function declaration."
                    });
                    break;
                }

                arguments.push_back(std::move(argument));
            }

            auto right_parenthesis = match(Token::Type::ParenthesisRight);
            auto type_specifier = match(Token::Type::TypeSpecifier);
            auto return_type = parseTypeExpression();
            auto body = parseStatement();

            if(!function_name)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = "Invalid identifier expression in function declaration (function name)."
                });
                return nullptr;
            }

            if(!return_type)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = "Invalid type expression in function declaration (return type)."
                });
                return nullptr;
            }

            if(!body)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = "Invalid body statement in function declaration."
                });
                return nullptr;
            }

            return std::make_unique<const FunctionDeclaration>(function_keyword, type_specifier, left_parenthesis, right_parenthesis,
                std::move(function_name), std::move(return_type), std::move(arguments), std::move(body));
        }
        else return nullptr;
    }


    std::unique_ptr<const Declaration> Parser::parseDeclaration() const
    {
        if(auto variable_declaration = parseVariableDeclaration())
            return std::move(variable_declaration);
        else if(auto function_declaration = parseFunctionDeclaration())
            return std::move(function_declaration);
        else return nullptr;
    }
}