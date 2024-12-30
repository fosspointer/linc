#include <linc/parser/Parser.hpp>
#include <linc/lexer/Operators.hpp>
#include <linc/system/Internals.hpp>
#include <linc/Tree.hpp>
#define LAMBDA_PARSE(function) [this](){ return this->parse##function(); }

namespace linc
{
    Parser::Parser()
    {
        beginScope();
        m_definitions.top().reserve(Internals::get().size());
        
        for(const auto& internal: Internals::get())
            m_definitions.top().push_back(Definition{Definition::Kind::External, internal.name});
    }

    Program Parser::operator()() const
    {
        Program program;

        while(auto declaration = parseDeclaration())
            program.declarations.push_back(std::move(declaration));

        auto end_of_file_token = match(Token::Type::EndOfFile);
        m_index = {};
        
        return program;
    }

    void Parser::set(std::vector<Token> tokens, std::string_view filepath)
    {
        m_tokens = std::move(tokens);
        m_filepath = filepath;
        m_index = {};
        m_matchFailed = {};
    }

    std::unique_ptr<const MatchClause> Parser::parseMatchClause() const
    {
        auto value_list = parseNodeListClause(LAMBDA_PARSE(Expression), Token::Type::Arrow);
        auto then_keyword = match(Token::Type::Arrow);
        auto expression = parseExpression();

        return std::make_unique<const MatchClause>(then_keyword, std::move(expression), std::move(value_list));
    }

    std::unique_ptr<const EnumeratorClause> Parser::parseEnumeratorClause() const
    {
        auto identifier = parseIdentifierExpression();
        
        if(!identifier)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(peekInfo()),
                .message = Logger::format("$ Expected identifier in enumerator clause.", peekInfo())
            }), nullptr);
        
        auto has_parenthesis = peek()->type == Token::Type::ParenthesisLeft;
        auto left_parenthesis = has_parenthesis? consume(): Token{.type = Token::Type::ParenthesisLeft, .info = peekInfo()};
        auto actual_type = has_parenthesis? parseTypeExpression(): std::make_unique<const TypeExpression>(std::nullopt, std::make_unique<const IdentifierExpression>(
            Token{.type = Token::Type::Identifier, .value = "void"}
        ), std::vector<TypeExpression::ArraySpecifier>{});
        auto right_parenthesis = has_parenthesis? match(Token::Type::ParenthesisRight): Token{.type = Token::Type::ParenthesisRight, .info = peekInfo()};

        if(!actual_type)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(peekInfo()),
                .message = Logger::format("$ Expected type expression in enumerator clause.", peekInfo())
            }), nullptr);

        return std::make_unique<const EnumeratorClause>(left_parenthesis, right_parenthesis, std::move(identifier), std::move(actual_type));
    }

    std::unique_ptr<const VariantClause<LegacyForClause, RangedForClause>> Parser::parseForClause() const
    {
        if(peek()->isIdentifier() && peek(1ul)->type == Token::Type::KeywordIn)
            return std::make_unique<const VariantClause<LegacyForClause, RangedForClause>>(parseRangedForClause());
        else return std::make_unique<const VariantClause<LegacyForClause, RangedForClause>>(parseLegacyForClause());
    }

    std::unique_ptr<const LegacyForClause> Parser::parseLegacyForClause() const
    {
        auto info = peekInfo();
        auto declaration = parseDeclaration();
        auto first_terminator = match(Token::Type::Terminator);
        auto test_expression = parseExpression();
        auto second_terminator = match(Token::Type::Terminator);
        auto end_expression = parseExpression();

        if(!declaration)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(info),
                .message = Logger::format("$ Invalid identifier in legacy for clause.", info)
            }), nullptr);
        else if(!test_expression)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(first_terminator.info),
                .message = Logger::format("$ Invalid test expression in legacy for clause.", first_terminator.info)
            }), nullptr);
        else if(!end_expression)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(second_terminator.info),
                .message = Logger::format("$ Invalid end expression in legacy for clause.", second_terminator.info)
            }), nullptr);

        return std::make_unique<const LegacyForClause>(first_terminator, second_terminator, std::move(declaration), std::move(test_expression), std::move(end_expression));
    }

    std::unique_ptr<const RangedForClause> Parser::parseRangedForClause() const
    {
        auto identifier = parseIdentifierExpression();
        auto in_keyword = match(Token::Type::KeywordIn);
        auto expression = parseExpression();

        if(!identifier)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(in_keyword.info),
                .message = Logger::format("$ Invalid identifier in ranged for clause.", in_keyword.info)
            }), nullptr);
        else if(!expression)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(peekInfo()),
                .message = Logger::format("$ Invalid expression in ranged for clause.", peekInfo())
            }), nullptr);

        return std::make_unique<const RangedForClause>(in_keyword, std::move(identifier), std::move(expression));
    }

    std::optional<LoopLabel> Parser::parseLoopLabel() const
    {
        if(peek()->type != Token::Type::Tilde) return std::nullopt;

        auto label_specifier = consume();
        auto identifier = parseIdentifierExpression();

        if(!identifier)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid identifier in loop label.", label_specifier.info)
            });
            return std::nullopt;
        }

        return LoopLabel{.specifier = label_specifier, .identifier = std::move(identifier)};
    }
    
    std::unique_ptr<const Statement> Parser::parseStatement() const 
    {
        if(peek()->type == Token::Type::KeywordBreak)
        {
            auto break_keyword = consume();
            auto identifier = peek()->type == Token::Type::Terminator? nullptr: parseIdentifierExpression();
            auto terminator = match(Token::Type::Terminator);

            return std::make_unique<const BreakStatement>(terminator, break_keyword, std::move(identifier));
        }
        else if(peek()->type == Token::Type::KeywordContinue)
        {
            auto continue_keyword = consume();
            auto identifier = peek()->type == Token::Type::Terminator? nullptr: parseIdentifierExpression();
            auto terminator = match(Token::Type::Terminator);

            return std::make_unique<const ContinueStatement>(terminator, continue_keyword, std::move(identifier));

        }
        else if(peek()->type == Token::Type::KeywordReturn)
        {
            auto return_keyword = consume();
            auto expression = parseExpression();
            auto terminator = match(Token::Type::Terminator);

            return std::make_unique<const ReturnStatement>(terminator, return_keyword, std::move(expression));
        }
        else if(auto declaration = parseDeclaration())
            return std::make_unique<const DeclarationStatement>(match(Token::Type::Terminator), std::move(declaration));
        else if(auto expression = parseExpression())
            return std::make_unique<const ExpressionStatement>(match(Token::Type::Terminator), std::move(expression));
        else return nullptr;
    }

    std::unique_ptr<const Expression> Parser::parseExpression(uint16_t parent_precedence) const
    {
        std::unique_ptr<const Expression> expression;

        if(peek()->isUnaryOperator() && Operators::getUnaryPrecedence(peek()->type) > parent_precedence)
        {
            const auto _operator = consume();
            const auto operand = parseRangeExpression();
            if(!operand)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .span = TextSpan::fromTokenInfo(_operator.info),
                    .message = Logger::format("$ Invalid syntax given as unary operator operand.", _operator.info)
                });
                return nullptr;
            }
            expression = std::make_unique<const UnaryExpression>(_operator, operand->clone());
        }
        else expression = parseRangeExpression();

        while(peek()->isBinaryOperator())
        {
            if(!expression) break;
            const auto precedence = Operators::getBinaryPrecedence(peek()->type);
            const auto associativity = Operators::getAssociativity(peek()->type);

            if(precedence < parent_precedence)
                break;

            const auto _operator = consume();
            auto right = parseExpression(precedence + (associativity == Operators::Associativity::Left? 1u: 0u));
            if(!right)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .span = TextSpan::fromTokenInfo(_operator.info),
                    .message = Logger::format("$ Invalid syntax given as binary operator right operand.", _operator.info)
                });
                return nullptr;
            }

            expression = std::make_unique<const BinaryExpression>(_operator, expression->clone(), std::move(right));

            if(peek()->type == Token::Type::EndOfFile)
                break;               
        }
        
        return expression;
    }

    std::unique_ptr<const TypeExpression> Parser::parseTypeExpression() const
    {
        auto has_mutability_keyword = peek()->type == Token::Type::KeywordMutability;
        auto root_peek_offset = has_mutability_keyword? 1ul: 0ul;
        auto has_function_root = peek(root_peek_offset)->type == Token::Type::KeywordFunction && peek(root_peek_offset + 1ul)->type == Token::Type::ParenthesisLeft;
        if(has_mutability_keyword || isTypeIdentifier(*peek()) || has_function_root)
        {
            auto mutability_keyword = has_mutability_keyword? std::make_optional(consume()): std::nullopt;
            TypeExpression::Root root{nullptr};
            auto root_info = peekInfo();

            if(has_function_root)
            {
                auto function_keyword = consume();
                auto left_parenthesis = consume();
                auto argument_types = parseNodeListClause(LAMBDA_PARSE(TypeExpression));
                auto right_parenthesis = match(Token::Type::ParenthesisRight);
                auto type_specifier = match(Token::Type::Colon);
                auto return_type = parseTypeExpression();

                if(!return_type)
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                        .span = TextSpan{.lineStart = type_specifier.info.line, .lineEnd = peekInfo().line,
                            .spanStart = type_specifier.info.characterStart, .spanEnd = peekInfo().characterEnd},
                        .message = Logger::format("$ Invalid return type in function pointer root.", function_keyword.info)
                    }), nullptr);

                root = TypeExpression::FunctionRoot{
                    function_keyword, type_specifier, left_parenthesis, right_parenthesis, std::move(return_type), std::move(argument_types)
                };
            }
            else root = parseIdentifierExpression(true);

            if(auto identifier_root = std::get_if<0ul>(&root); identifier_root && *identifier_root == nullptr)
                return (Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .message = Logger::format("$ Type root specified was invalid.", root_info)
                }), nullptr);

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

            return std::make_unique<const TypeExpression>(mutability_keyword, std::move(root), std::move(specifiers));
        }
        return nullptr;
    }

    std::unique_ptr<const IdentifierExpression> Parser::parseIdentifierExpression(bool type_inclusive) const
    {
        auto has_identifier = peek()->isIdentifier();
        
        if(has_identifier && (!isTypeIdentifier(*peek()) || type_inclusive))
            return std::make_unique<const IdentifierExpression>(consume());
        
        return nullptr;
    }

    std::unique_ptr<const LiteralExpression> Parser::parseLiteralExpression() const
    {
        if(peek()->isLiteral())
            return std::make_unique<const LiteralExpression>(consume());
        
        return nullptr;
    }

    std::unique_ptr<const Expression> Parser::parseRangeExpression() const
    {
        auto root = parseModifierExpression();
        auto has_range_specifier = peek()->type == Token::Type::RangeSpecifier;
        if(!has_range_specifier)
            return root;
        auto range_specifier = consume();
        auto end_expression = parseModifierExpression();
        if(!end_expression)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfoRange(range_specifier.info, peekInfo()),
                .message = Logger::format("$ Invalid end expression in range.", peekInfo())
            }), nullptr);
        
        return std::make_unique<const RangeExpression>(range_specifier, std::move(root), std::move(end_expression));
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
                        .message = Logger::format("$ Invalid literal in index expression.", left_bracket.info)
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
                        .message = Logger::format("$ Invalid identifier in access expression.", dot.info)
                    });
                    return nullptr;
                }

                base = std::make_unique<const AccessExpression>(dot, std::move(base), std::move(identifier));
            }
            else return base;
        }
    }

    std::unique_ptr<const BlockExpression> Parser::parseBlockExpression() const
    {
        if(peek()->type != Token::Type::BraceLeft)
            return nullptr;

        auto left_brace = consume();
        std::unique_ptr<const Expression> tail;
        std::vector<std::unique_ptr<const Statement>> statements;

        beginScope();
        while(!peek()->isEndOfFile() && peek()->type != Token::Type::BraceRight)
        {
            auto token = peek();
            auto member = parseVariant();
            if(!member) break;
            else if(auto expression = Types::uniqueCastDynamic<const Expression>(member->clone()))
            {
                tail = std::move(expression);
                break;
            }
            
            auto statement = Types::uniqueCast<const Statement>(std::move(member));
            statements.push_back(std::move(statement));

            if(!peek() || !token || peek()->info == token->info)
                break;
        }
        endScope();

        auto right_brace = match(Token::Type::BraceRight);
        return std::make_unique<const BlockExpression>(left_brace, right_brace, std::move(statements), std::move(tail));
    }

    std::unique_ptr<const ArrayInitializerExpression> Parser::parseArrayInitializerExpression() const 
    {
        if(peek()->type != Token::Type::SquareLeft)
            return nullptr;
            
        auto left_bracket = consume();
        auto expression_list = parseNodeListClause(LAMBDA_PARSE(Expression), Token::Type::SquareRight);
        auto right_bracket = match(Token::Type::SquareRight);

        return std::make_unique<const ArrayInitializerExpression>(left_bracket, right_bracket, std::move(expression_list));
    }

    std::unique_ptr<const ParenthesisExpression> Parser::parseParenthesisExpression() const
    {
        if(peek()->type != Token::Type::ParenthesisLeft)
            return nullptr;

        auto info = peekInfo();
        auto left_parenthesis = consume();
        auto expression = parseExpression();

        if(!expression)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(info),
                .message = Logger::format("$ Parentheses require a valid expression.", info)
            });
            return nullptr;
        }

        auto right_parenthesis = match(Token::Type::ParenthesisRight);
        return std::make_unique<const ParenthesisExpression>(left_parenthesis, right_parenthesis, std::move(expression));
    }
    
    std::unique_ptr<const IfExpression> Parser::parseIfExpression() const
    {
        if(peek()->type != Token::Type::KeywordIf)
            return nullptr;

        auto info = peekInfo();
        auto if_keyword = consume();
        auto test_expression = parseExpression();
        auto if_body = parseExpression();

        if(!test_expression)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(info),
                .message = Logger::format("$ Invalid check expression in `if` conditional.", info)
            });
            return nullptr;
        }

        else if(!if_body)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(info),
                .message = Logger::format("$ Invalid body in `if` expression.", info)
            });
            return nullptr;
        }

        if(peek() && peek()->type == Token::Type::KeywordElse)
        {
            auto else_keyword = consume();
            auto else_body = parseExpression();

            if(!else_body)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .span = TextSpan::fromTokenInfo(info),
                    .message = Logger::format("$ Invalid body in `else` clause of `if` conditional.", info)
                });
                return nullptr;
            }

            return std::make_unique<const IfExpression>(if_keyword, else_keyword, std::move(test_expression), std::move(if_body), std::move(else_body));
        }
        else return std::make_unique<const IfExpression>(if_keyword, std::nullopt, std::move(test_expression), std::move(if_body), nullptr);
    }

    std::unique_ptr<const ForExpression> Parser::parseForExpression(std::optional<LoopLabel> label) const
    {
        if(peek()->type != Token::Type::KeywordFor)
            return nullptr;

        auto for_keyword = consume();
        auto clause = parseForClause();
        auto body = parseExpression();

        return std::make_unique<const ForExpression>(std::move(label), for_keyword, std::move(clause), std::move(body));
    }

    std::unique_ptr<const WhileExpression> Parser::parseWhileExpression(std::optional<LoopLabel> label) const
    {
        if(peek()->type != Token::Type::KeywordWhile)
            return nullptr;

        auto while_keyword = consume();
        auto test_expression = parseExpression();
        auto while_body = parseExpression();

        if(!test_expression)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfoRange(while_keyword.info, peekInfo()),
                .message = Logger::format("$ Invalid test expression in while expression.", while_keyword.info)
            }), nullptr);
        else if(!while_body)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfoRange(while_keyword.info, peekInfo()),
                .message = Logger::format("$ Invalid body in while expression.", while_keyword.info)
            }), nullptr);
        
        auto has_finally = peek() && peek()->type == Token::Type::KeywordFinally;
        auto finally_keyword = has_finally? std::make_optional(consume()): std::nullopt;
        auto finally_body = has_finally? parseExpression(): nullptr;
        
        auto has_else = peek()->type == Token::Type::KeywordElse;
        auto else_keyword = has_else? std::make_optional(consume()): std::nullopt;
        auto else_body = has_else? parseExpression(): nullptr;

        return std::make_unique<const WhileExpression>(std::move(label), while_keyword, finally_keyword, else_keyword, std::move(test_expression),
            std::move(while_body), std::move(finally_body), std::move(else_body));
    }

    std::unique_ptr<const MatchExpression> Parser::parseMatchExpression() const
    {
        if(peek()->type != Token::Type::KeywordMatch)
            return nullptr;

        auto match_keyword = consume();
        auto test_expression = parseExpression();

        if(!test_expression)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfoRange(match_keyword.info, peekInfo()),
                .message = Logger::format("$ Invalid value in match expression.", peekInfo())
            }), nullptr);

        auto left_brace = match(Token::Type::BraceLeft);
        auto clauses = parseNodeListClause(LAMBDA_PARSE(MatchClause), Token::Type::BraceRight);
        auto right_brace = match(Token::Type::BraceRight);
        
        return std::make_unique<const MatchExpression>(match_keyword, left_brace, right_brace, std::move(test_expression), std::move(clauses));
    }

    std::unique_ptr<const ConversionExpression> Parser::parseConversionExpression() const
    {
        if(peek()->type != Token::Type::KeywordAs)
            return nullptr;

        auto as_keyword = consume();
        auto type = parseTypeExpression();
        auto left_parenthesis = match(Token::Type::ParenthesisLeft);
        auto expression = parseExpression();
        auto right_parenthesis = match(Token::Type::ParenthesisRight);

        if(!type)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid type in conversion expression.", as_keyword.info)
            }), nullptr);

        if(!expression)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid argument in conversion expression.", as_keyword.info)
            }), nullptr);

        return std::make_unique<const ConversionExpression>(as_keyword, left_parenthesis, right_parenthesis, std::move(type), std::move(expression));
    }

    std::unique_ptr<const StructureInitializerExpression> Parser::parseStructureInitializerExpression() const
    {
        if(!peek(1ul) || peek()->type != Token::Type::Identifier || !isValidTypeDefinition(*peek()->value)
            || peek(1ul)->type != Token::Type::BraceLeft)
            return nullptr;

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

    std::unique_ptr<const CallExpression> Parser::parseCallExpression() const
    {
        if(!peek(1ul) || !peek()->isIdentifier() || peek(1ul)->type != Token::Type::ParenthesisLeft)
            return nullptr;

        auto identifier = consume();
        auto left_parenthesis = consume();
        auto arguments = parseNodeListClause(LAMBDA_PARSE(Expression));
        auto right_parenthesis = match(Token::Type::ParenthesisRight);
        auto definition = findDefinition(identifier.value.value_or(""));
        
        if(!definition)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(identifier.info),
                .message = Logger::format("$ Call to undeclared function `$`.", identifier.info, identifier.value.value_or(""))
            }), nullptr);

        else if(definition != Definition::Kind::Function && definition != Definition::Kind::External && definition != Definition::Kind::Variable)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(identifier.info),
                .message = Logger::format("$ Call to non-callable symbol `$`.", identifier.info, identifier.value.value_or(""))
            }), nullptr);

        return std::make_unique<const CallExpression>(identifier, left_parenthesis, right_parenthesis, std::move(arguments),
            definition == Definition::Kind::External);
    }

    std::unique_ptr<const EnumeratorExpression> Parser::parseEnumeratorExpression() const
    {
        if(peek()->type != Token::Type::Identifier || !isValidTypeDefinition(peek()->value.value_or("")) || peek(1ul)->type != Token::Type::DoubleColon)
            return nullptr;

        auto enumerator_identifier = parseIdentifierExpression(true);
        auto namespace_access_specifier = consume();
        auto identifier = parseIdentifierExpression();
        auto has_parenthesis = peek()->type == Token::Type::ParenthesisLeft;
        auto left_parenthesis = has_parenthesis? std::make_optional(consume()): std::nullopt;
        auto value = has_parenthesis? parseExpression(): nullptr;
        auto right_parenthesis = has_parenthesis? std::make_optional(match(Token::Type::ParenthesisRight)): std::nullopt;

        if(!identifier)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(namespace_access_specifier.info),
                .message = Logger::format("$ Invalid identifier in enumerator access expression.", namespace_access_specifier.info)
            }), nullptr);

        if(has_parenthesis && !value)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfoRange(left_parenthesis->info, peekInfo()),
                .message = Logger::format("$ Invalid value in enumerator access expression.", namespace_access_specifier.info)
            }), nullptr);

        return std::make_unique<const EnumeratorExpression>(namespace_access_specifier, left_parenthesis, right_parenthesis, std::move(enumerator_identifier),
            std::move(identifier), std::move(value));
    }

    std::unique_ptr<const Expression> Parser::parsePrimaryExpression() const
    {
        auto loop_label = parseLoopLabel();

        if(auto for_expression = parseForExpression(loop_label? std::make_optional(loop_label->copy()): std::nullopt))
            return for_expression;

        else if(auto while_expression = parseWhileExpression(loop_label? std::make_optional(loop_label->copy()): std::nullopt))
            return while_expression;

        else if(auto match_expression = parseMatchExpression())
            return match_expression;

        if(auto array_initializer = parseArrayInitializerExpression())
            return array_initializer;

        if(auto block = parseBlockExpression())
            return block;

        else if(auto parenthesis = parseParenthesisExpression())
            return parenthesis;
        
        else if(auto if_expression = parseIfExpression())
            return if_expression;

        else if(auto conversion = parseConversionExpression())
            return conversion;

        else if(auto structure_initializer = parseStructureInitializerExpression())
            return structure_initializer;
        
        else if(auto function_call = parseCallExpression())
            return function_call;

        else if(auto namespace_access = parseEnumeratorExpression())
            return namespace_access;

        else if(auto identifier = parseIdentifierExpression())
            return identifier;

        else if(auto type_expression = parseTypeExpression())
            return std::move(type_expression);

        else return parseLiteralExpression();
    }

    std::unique_ptr<const VariableDeclaration> Parser::parseVariableDeclaration() const
    {
        if(!peek() || !peek(1ul) || !peek()->isIdentifier() || peek(1ul)->type != Token::Type::Colon)
            return nullptr;

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
                .message = Logger::format("$ Invalid identifier expression in variable declaration.", type_specifier.info)});
            return nullptr;
        }

        if(!type)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid type expression in variable declaration.", type_specifier.info)});
            return nullptr;
        }

        m_definitions.top().push_back(Definition{.kind = Definition::Kind::Variable, .identifier = identifier->getValue()});

        return std::make_unique<const VariableDeclaration>(type_specifier, std::move(type), std::move(identifier), std::move(default_value));
    }

    std::unique_ptr<const DirectVariableDeclaration> Parser::parseDirectVariableDeclaration() const
    {
        if(!peek() || !peek(1ul) || !peek()->isIdentifier() || (peek(1ul)->type != Token::Type::ColonEquals && peek(1ul)->type != Token::Type::KeywordMutability))
            return nullptr;
        
        auto identifier = parseIdentifierExpression();
        auto mutability_specifier = peek() && peek()->type == Token::Type::KeywordMutability? std::make_optional(consume()): std::nullopt;
        auto direct_assignment = consume();
        auto value = parseExpression();

        if(!identifier)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid identifier expression in direct variable declaration.", direct_assignment.info)});
            return nullptr;
        }

        if(!value)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid value expression in direct variable declaration.", direct_assignment.info)});
            return nullptr;
        }

        return std::make_unique<const DirectVariableDeclaration>(direct_assignment, mutability_specifier, std::move(identifier), std::move(value));
    }

    std::unique_ptr<const FunctionDeclaration> Parser::parseFunctionDeclaration() const
    {
        if(peek() && peek()->type != Token::Type::KeywordFunction)
            return nullptr;
        
        auto function_keyword = consume();
        auto function_name = parseIdentifierExpression();
        auto left_parenthesis = match(Token::Type::ParenthesisLeft);
        auto arguments = parseNodeListClause(LAMBDA_PARSE(VariableDeclaration));
        auto right_parenthesis = match(Token::Type::ParenthesisRight);

        auto has_type_specifier = peek()->type == Token::Type::Colon;
        auto type_specifier = has_type_specifier? consume(): Token{Token::Type::Colon};

        std::unique_ptr<const Expression> body{nullptr};
        std::unique_ptr<const TypeExpression> return_type{nullptr};

        if(!has_type_specifier)
            body = parseExpression();
        
        else
        {
            return_type = parseTypeExpression();
            body = parseExpression();
        }

        if(!function_name)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid identifier expression in function declaration (function name).", function_keyword.info)
            });
            return nullptr;
        }

        if(has_type_specifier && !return_type)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid type expression in function declaration (return type).", function_keyword.info)
            });
            return nullptr;
        }

        if(!body)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Invalid body statement in function declaration.", function_keyword.info)
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
                .message = Logger::format("$ Expected identifier in external declaration.", peek()->info)
            });
            return nullptr;
        }

        auto left_parenthesis = match(Token::Type::ParenthesisLeft);
        auto arguments = parseNodeListClause(LAMBDA_PARSE(TypeExpression));
        auto right_parenthesis = match(Token::Type::ParenthesisRight);
        auto type_specifier = match(Token::Type::Colon);
        auto actual_type = parseTypeExpression();

        if(!actual_type)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Expected return type in external declaration.", peek()->info)
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
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .message = Logger::format("$ Expected identifier in structure declaration.", structure_keyword.info)
            }), nullptr);
        else m_definitions.top().push_back(Definition{.kind = Definition::Kind::Typename, .identifier = identifier->getValue()});

        auto left_brace = match(Token::Type::BraceLeft);
        std::vector<std::unique_ptr<const VariableDeclaration>> fields;

        while(auto variable_declaration = parseVariableDeclaration())
            fields.push_back(std::move(variable_declaration));

        auto right_brace = match(Token::Type::BraceRight);

        return std::make_unique<const StructureDeclaration>(structure_keyword, left_brace, right_brace, std::move(identifier), std::move(fields));
    }

    std::unique_ptr<const EnumerationDeclaration> Parser::parseEnumerationDeclaration() const
    {
        if(peek()->type != Token::Type::KeywordEnumeration)
            return nullptr;

        auto enumeration_keyword = consume();
        auto identifier = parseIdentifierExpression();

        if(!identifier)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfoRange(enumeration_keyword.info, peekInfo()),
                .message = Logger::format("$ Expected identifier in enumeration declaration.", enumeration_keyword.info)
            }), nullptr);
        else m_definitions.top().push_back(Definition{.kind = Definition::Kind::Typename, .identifier = identifier->getValue()});

        auto left_brace = match(Token::Type::BraceLeft);
        auto enumerators = parseNodeListClause(LAMBDA_PARSE(EnumeratorClause), Token::Type::BraceRight);
        auto right_brace = match(Token::Type::BraceRight);

        return std::make_unique<const EnumerationDeclaration>(enumeration_keyword, left_brace, right_brace, std::move(identifier), std::move(enumerators));
    }

    std::unique_ptr<const Declaration> Parser::parseDeclaration() const
    {
        if(auto variable_declaration = parseVariableDeclaration())
            return std::move(variable_declaration);
        else if(auto direct_variable_declaration = parseDirectVariableDeclaration())
            return std::move(direct_variable_declaration);
        else if(auto function_declaration = parseFunctionDeclaration())
            return std::move(function_declaration);
        else if(auto external_declaration = parseExternalDeclaration())
            return std::move(external_declaration);
        else if(auto structure_declaration = parseStructureDeclaration())
            return std::move(structure_declaration);
        else if(auto enumeration_declaration = parseEnumerationDeclaration())
            return std::move(enumeration_declaration);
        else return nullptr;
    }

    std::unique_ptr<const Node> Parser::parseVariant() const
    {
        if(peek()->isIdentifier() && (peek(1ul)->type == Token::Type::Colon || peek(1ul)->type == Token::Type::ColonEquals ||
            peek(1ul)->type == Token::Type::KeywordMutability))
            return parseStatement();

        auto expression = parseExpression();

        if(!expression)
        {
            auto statement = parseStatement();
            if(!statement)
            {
                auto info = peekInfo();
                if(m_tokens.size() == 1ul)
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                        .message = Logger::format("$ Statement or expression cannot be empty.", info)
                    });
                
                else Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                    .span = TextSpan::fromTokenInfo(peekInfo()),
                    .message = Logger::format("$ No valid expression or statement syntax found.", info)
                });
            }
            return statement;
        }
        
        if(peek()->type == Token::Type::Terminator)
            return std::make_unique<const ExpressionStatement>(consume(), std::move(expression));

        else return std::move(expression);
    }
}