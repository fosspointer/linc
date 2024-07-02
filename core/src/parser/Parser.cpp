#include <linc/parser/Parser.hpp>

namespace linc
{
    Program Parser::operator()() const
    {
        Program program;

        while(auto declaration = parseDeclaration())
            program.declarations.push_back(std::move(declaration));

        auto end_of_file_token = match(Token::Type::EndOfFile);
        m_index = {};
        
        return std::move(program);
    }

    void Parser::set(std::vector<Token> tokens, std::string_view filepath)
    {
        m_tokens = std::move(tokens);
        m_filepath = filepath;
        m_index = {};
        m_matchFailed = {};
    }
    
    std::unique_ptr<const Statement> Parser::parseStatement() const 
    {
        if(peek(1ul) && peek()->type == Token::Type::Tilde && peek(1ul)->type == Token::Type::Identifier)
        {
            auto label_specifier = consume();
            auto identifier = parseIdentifierExpression();
            auto next = parseStatement();
            
            return std::make_unique<const LabelStatement>(label_specifier, std::move(identifier), std::move(next)); 
        }
        else if(peek()->type == Token::Type::KeywordJump)
        {
            auto jump_keyword = consume();
            auto identifier = parseIdentifierExpression();

            if(!identifier)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = Logger::format("$::$ Expected identifier in jump statement.", jump_keyword.info.file, jump_keyword.info.line)
                });
                return nullptr;
            }

            return std::make_unique<const JumpStatement>(jump_keyword, std::move(identifier));
        }
        else if(peek()->type == Token::Type::KeywordBreak)
        {
            auto break_keyword = consume();
            auto identifier = peek()->isIdentifier()? parseIdentifierExpression(): nullptr;

            return std::make_unique<const BreakStatement>(break_keyword, std::move(identifier));
        }
        else if(peek()->type == Token::Type::KeywordContinue)
        {
            auto continue_keyword = consume();
            auto identifier = peek()->isIdentifier()? parseIdentifierExpression(): nullptr;

            return std::make_unique<const ContinueStatement>(continue_keyword, std::move(identifier));

        }
        else if(peek()->type == Token::Type::KeywordReturn)
        {
            auto return_keyword = consume();
            auto expression = parseExpression();

            return std::make_unique<const ReturnStatement>(return_keyword, std::move(expression));
        }
        else if(auto declaration = parseDeclaration())
            return std::make_unique<const DeclarationStatement>(std::move(declaration));
        else if(auto expression = parseExpression())
            return std::make_unique<const ExpressionStatement>(std::move(expression));
        else return nullptr;
    }

    std::unique_ptr<const Expression> Parser::parseExpression(uint16_t parent_precedence) const
    {
        std::unique_ptr<const Expression> expression;

        if(peek()->isUnaryOperator() && Operators::getUnaryPrecedence(peek()->type) > parent_precedence)
        {
            const auto _operator = consume();
            const auto operand = parseModifierExpression();
            expression = std::make_unique<const UnaryExpression>(_operator, operand->clone());
        }
        else expression = parseModifierExpression();

        while(peek()->isBinaryOperator())
        {
            const auto precedence = Operators::getBinaryPrecedence(peek()->type);
            const auto associativity = Operators::getAssociativity(peek()->type);

            if(precedence < parent_precedence)
                break;

            const auto _operator = consume();
            auto right = parseExpression(precedence + (associativity == Operators::Associativity::Left? 1u: 0u));

            expression = std::make_unique<const BinaryExpression>(_operator, expression->clone(), std::move(right));

            if(peek()->type == Token::Type::EndOfFile)
                break;               
        }
        
        return expression;
    }

    std::unique_ptr<const TypeExpression> Parser::parseTypeExpression() const
    {
        auto has_mutability_keyword = peek()->type == Token::Type::KeywordMutability;
        if(has_mutability_keyword || isTypeIdentifier(*peek()))
        {
            auto mutability_keyword = has_mutability_keyword? std::make_optional(consume()): std::nullopt;
            auto typename_identifier = match(Token::Type::Identifier);

            std::vector<TypeExpression::ArraySpecifier> specifiers;

            while(peek()->type == Token::Type::SquareLeft)
            {
                auto left_bracket = consume();
                auto count = peek()->isLiteral()? parseLiteralExpression(): nullptr;
                auto right_bracket = match(Token::Type::SquareRight);
                
                specifiers.push_back(TypeExpression::ArraySpecifier{
                    .leftBracket = left_bracket,
                    .rightBracket = right_bracket,
                    .count = std::move(count)
                });
            }

            return std::make_unique<const TypeExpression>(typename_identifier, mutability_keyword, std::move(specifiers));
        }
        return nullptr;
    }

    std::unique_ptr<const IdentifierExpression> Parser::parseIdentifierExpression() const
    {
        auto has_identifier = peek()->isIdentifier() && !isTypeIdentifier(*peek());
        
        if(has_identifier)
            return std::make_unique<const IdentifierExpression>(consume());
        
        return nullptr;
    }

    std::unique_ptr<const LiteralExpression> Parser::parseLiteralExpression() const
    {
        if(peek()->isLiteral())
            return std::make_unique<const LiteralExpression>(consume());
        
        Token::Info info = getLastAvailableInfo();

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
            .message = Logger::format("$::$ Expected literal expression, got '$'.", 
                info.file, info.line, Token::typeToString(peek()->type))
        });
        
        return std::make_unique<const LiteralExpression>(Token{.type = Token::Type::I32Literal, .value = "0", .info = peek()->info});
    }

    std::unique_ptr<const Expression> Parser::parseModifierExpression() const
    {
        auto base = parsePrimaryExpression();
        
        while(true)
        {
            if(peek() && peek()->type == Token::Type::SquareLeft)
            {
                auto left_bracket = consume();
                auto index = parseExpression();
                auto right_bracket = match(Token::Type::SquareRight);
            
                if(!index)
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                        .message = Logger::format("$::$ Invalid literal in index expression.", left_bracket.info.file, left_bracket.info.line)
                    });

                base = std::make_unique<const IndexExpression>(left_bracket, right_bracket, std::move(base), std::move(index));
            }
            else if(peek() && peek()->type == Token::Type::Dot)
            {
                auto dot = consume();
                auto identifier = parseIdentifierExpression();

                if(!identifier)
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                        .message = Logger::format("$::$ Invalid identifier in access expression.", dot.info.file, dot.info.line)
                    });
                    return nullptr;
                }

                base = std::make_unique<const AccessExpression>(dot, std::move(base), std::move(identifier));
            }
            else return std::move(base);
        }
    }

    std::unique_ptr<const Expression> Parser::parsePrimaryExpression() const
    {
        if(peek()->type == Token::Type::SquareLeft)
        {
            auto left_bracket = consume();
            std::vector<ArrayInitializerExpression::Argument> values;

            while(peek()->type != Token::Type::SquareRight)
            {
                auto expression = parseExpression();
                auto separator = peek()->type != Token::Type::SquareRight? std::make_optional(match(Token::Type::Comma)): std::nullopt;

                values.push_back(ArrayInitializerExpression::Argument{
                    .separator = separator,
                    .value = std::move(expression)
                });
            }
            auto right_bracket = match(Token::Type::SquareRight);

            return std::make_unique<const ArrayInitializerExpression>(left_bracket, right_bracket, std::move(values));
        }
        if(peek()->type == Token::Type::BraceLeft)
        {
            auto left_brace = consume();
            std::vector<std::unique_ptr<const Statement>> statements;

            beginScope();

            while(!peek()->isEndOfFile() && peek()->type != Token::Type::BraceRight)
                statements.push_back(std::move(parseStatement()));
            
            endScope();

            auto right_brace = match(Token::Type::BraceRight);
            return std::make_unique<const BlockExpression>(left_brace, right_brace, std::move(statements));
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
                return std::make_unique<const IfExpression>(if_keyword, std::move(check_expression), std::move(if_body_statement), else_keyword,
                    std::move(else_body_statement));
            }
            else return std::make_unique<const IfExpression>(if_keyword, std::move(check_expression), std::move(if_body_statement));
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

        else if(peek(1ul) && peek()->type == Token::Type::Identifier && isValidStructure(*peek()->value)
            && peek(1ul)->type == Token::Type::BraceLeft)
        {
            auto identifier = std::make_unique<const IdentifierExpression>(consume());
            auto left_brace = consume();
            std::vector<StructureInitializerExpression::Argument> arguments;

            while(peek()->type == Token::Type::Dot)
            {
                auto access_specifier = consume();
                auto identifier = parseIdentifierExpression();
                auto equality_specifier = match(Token::Type::OperatorAssignment);
                auto expression = parseExpression();
                auto separator = peek()->type == Token::Type::Comma? consume(): match(Token::Type::BraceRight);

                arguments.push_back(StructureInitializerExpression::Argument{access_specifier, equality_specifier, separator,
                    std::move(identifier), std::move(expression)});
            }

            return std::make_unique<const StructureInitializerExpression>(left_brace, std::move(identifier), std::move(arguments));
        }
        else if(peek()->type == Token::Type::Identifier && !isTypeIdentifier(*peek()))
        {
            auto identifier = consume();

            if(peek()->type == Token::Type::ParenthesisLeft)
            {
                auto left_parenthesis = consume();
                std::vector<CallExpression::Argument> arguments;

                while(peek() && peek()->type != Token::Type::ParenthesisRight)
                {
                    auto expression = parseExpression();
                    auto separator = peek()->type != Token::Type::ParenthesisRight? std::make_optional(match(Token::Type::Comma)): std::nullopt;
                    arguments.push_back(CallExpression::Argument{separator, std::move(expression)});
                }

                auto right_parenthesis = match(Token::Type::ParenthesisRight);
                auto definition = findDefinition(identifier.value.value_or(""));
                
                if(!definition)
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                        .message = Logger::format("$::$ Call to undeclared function `$`.",
                            identifier.info.file, identifier.info.line, identifier.value.value_or(""))
                    }), nullptr);

                else if(definition != Definition::Kind::Function && definition != Definition::Kind::External)
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                        .message = Logger::format("$::$ Call to non-callable symbol `$`.",
                            identifier.info.file, identifier.info.line, identifier.value.value_or(""))
                    }), nullptr);

                return std::make_unique<const CallExpression>(identifier, left_parenthesis, right_parenthesis, std::move(arguments),
                    definition == Definition::Kind::External);
            }
            else return std::make_unique<const IdentifierExpression>(identifier);
        }
        else if(auto type_expression = parseTypeExpression())
            return std::move(type_expression);

        else return parseLiteralExpression();
    }

    std::unique_ptr<const VariableDeclaration> Parser::parseVariableDeclaration() const
    {
        std::unique_ptr<const TypeExpression> type;
        
        if(peek() && peek(1ul) && peek()->isIdentifier() && peek(1ul)->type == Token::Type::Colon)
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
                    .message = Logger::format("$::$ Invalid identifier expression in variable declaration.",
                        type_specifier.info.file, type_specifier.info.line)});
                return nullptr;
            }

            if(!type)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = Logger::format("$::$ Invalid type expression in variable declaration.",
                        type_specifier.info.file, type_specifier.info.line)});
                return nullptr;
            }

            if(isValidStructure(*type->getTypeIdentifier().value) && peek() && peek()->type == Token::Type::BraceLeft)
            {
                auto left_brace = consume();
                std::vector<StructureInitializerExpression::Argument> arguments;

                while(peek() && peek()->type == Token::Type::Dot)
                {
                    auto access_specifier = consume();
                    auto field_identifier = parseIdentifierExpression();
                    auto equality_specifier = match(Token::Type::OperatorAssignment);
                    auto expression = parseExpression();
                    auto separator = peek()->type == Token::Type::Comma? consume(): match(Token::Type::BraceRight);

                    arguments.push_back(StructureInitializerExpression::Argument{access_specifier, equality_specifier, separator,
                        std::move(field_identifier), std::move(expression)});
                }

                auto structure_identifier = std::make_unique<const IdentifierExpression>(type->getTypeIdentifier());
                default_value = VariableDeclaration::ValueAssignment{
                    left_brace, std::make_unique<const StructureInitializerExpression>(left_brace, std::move(structure_identifier), std::move(arguments))
                };
            }


            return std::make_unique<const VariableDeclaration>(type_specifier, std::move(type), std::move(identifier), std::move(default_value));
        }
        else return nullptr;
    }

    std::unique_ptr<const FunctionDeclaration> Parser::parseFunctionDeclaration() const
    {
        if(peek() && peek()->type != Token::Type::KeywordFunction)
            return nullptr;
        
        auto function_keyword = consume();
        auto function_name = parseIdentifierExpression();
        auto left_parenthesis = match(Token::Type::ParenthesisLeft);
        
        std::vector<FunctionDeclaration::Argument> arguments;

        while(auto variable_declaration = parseVariableDeclaration())
            arguments.push_back(FunctionDeclaration::Argument{peek()->type != Token::Type::ParenthesisRight?
                std::make_optional(match(Token::Type::Comma)): std::nullopt, std::move(variable_declaration)});

        auto right_parenthesis = match(Token::Type::ParenthesisRight);

        auto type_specifier = peek()->type == Token::Type::BraceLeft? Token{Token::Type::Colon}: match(Token::Type::Colon);
        auto return_type = peek()->type == Token::Type::BraceLeft
            ?std::make_unique<const TypeExpression>(Token{.type = Token::Type::Identifier, .value = "void"}, std::nullopt)
            :parseTypeExpression();

        auto body = parseStatement();

        if(!function_name)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$::$ Invalid identifier expression in function declaration (function name).",
                    type_specifier.info.file, type_specifier.info.line)
            });
            return nullptr;
        }

        if(!return_type)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$::$ Invalid type expression in function declaration (return type).",
                    type_specifier.info.file, type_specifier.info.line)
            });
            return nullptr;
        }

        if(!body)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$::$ Invalid body statement in function declaration.",
                    type_specifier.info.file, type_specifier.info.line)
            });
            return nullptr;
        }

        m_definitions.top().push_back(Definition{.kind = Definition::Kind::Function, .identifier = function_name->getValue()});

        return std::make_unique<const FunctionDeclaration>(function_keyword, type_specifier, left_parenthesis, right_parenthesis,
            std::move(function_name), std::move(return_type), std::move(arguments), std::move(body));
    }

    std::unique_ptr<const ExternalDeclaration> Parser::parseExternalDeclaration() const
    {
        if(peek()->type != Token::Type::KeywordExternal)
            return nullptr;

        auto external_keyword = consume();
        auto identifier = parseIdentifierExpression();

        if(!identifier)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$::$ Expected identifier in external declaration.",
                    peek()->info.file, peek()->info.line)
            });
            return nullptr;
        }

        auto left_parenthesis = match(Token::Type::ParenthesisLeft);

        std::vector<std::unique_ptr<const TypeExpression>> arguments;

        while(peek()->type != Token::Type::ParenthesisRight)
        {
            auto argument = parseTypeExpression();

            if(!argument)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = Logger::format("$::$ Expected argument in external declaration.", peek()->info.file, peek()->info.line)
                });
                break;
            }

            arguments.push_back(std::move(argument));
        }

        auto right_parenthesis = match(Token::Type::ParenthesisRight);
        auto type_specifier = match(Token::Type::Colon);
        auto actual_type = parseTypeExpression();

        if(!actual_type)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$::$ Expected return type in external declaration.",
                    peek()->info.file, peek()->info.line)
            });
            return nullptr;
        }

        m_definitions.top().push_back(Definition{.kind = Definition::Kind::External, .identifier = identifier->getValue()});

        return std::make_unique<const ExternalDeclaration>(external_keyword, left_parenthesis, right_parenthesis, type_specifier, std::move(identifier),
            std::move(actual_type), std::move(arguments));
    }

    std::unique_ptr<const StructureDeclaration> Parser::parseStructureDeclaration() const
    {
        if(peek()->type != Token::Type::KeywordStructure)
            return nullptr;

        auto structure_keyword = consume();
        auto identifier = parseIdentifierExpression();

        if(!identifier)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$::$ Expected identifier in structure declaration.", structure_keyword.info.file, structure_keyword.info.line)
            });
        else m_definitions.top().push_back(Definition{.kind = Definition::Kind::Typename, .identifier = identifier->getValue()});

        auto left_brace = match(Token::Type::BraceLeft);
        std::vector<std::unique_ptr<const VariableDeclaration>> fields;

        while(auto variable_declaration = parseVariableDeclaration())
            fields.push_back(std::move(variable_declaration));

        auto right_brace = match(Token::Type::BraceRight);

        return std::make_unique<const StructureDeclaration>(structure_keyword, left_brace, right_brace, std::move(identifier), std::move(fields));

    }

    std::unique_ptr<const Declaration> Parser::parseDeclaration() const
    {
        if(auto variable_declaration = parseVariableDeclaration())
            return std::move(variable_declaration);
        else if(auto function_declaration = parseFunctionDeclaration())
            return std::move(function_declaration);
        else if(auto external_declaration = parseExternalDeclaration())
            return std::move(external_declaration);
        else if(auto structure_declaration = parseStructureDeclaration())
            return std::move(structure_declaration);
        else return nullptr;
    }
}