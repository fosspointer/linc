#include <linc/bound_tree/Binder.hpp>

template <typename T>
static std::optional<T> parseString(const std::string& str)
{
    using namespace linc;
    
    if(str.empty())
        return static_cast<T>(0);

    char* p;
    if constexpr(std::is_integral<T>::value)
    {
        T result{};
        bool negative{str[0ull] == '-'};

        for(std::size_t i{negative? 1ull: 0ull}; i < str.size(); ++i)
            if(str[i] >= '0' && str[i] <= '9')
                result = result * static_cast<T>(10) + (str[i] - '0');
            else
                return std::nullopt;

        return negative? -result: result;
    }
    else if constexpr(std::is_same<T, Types::f32>::value)
    {
        Types::f32 value = strtof(str.c_str(), &p);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr(std::is_same<T, Types::f64>::value)
    {
        Types::f64 value = strtod(str.c_str(), &p);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr(std::is_same<T, Types::_bool>::value)
    {
        Types::_bool value = strtoll(str.c_str(), &p, 10);
        
        if(str == "false")
            return false;
        else if(str == "true")
            return true;
        else if(*p)
            return std::nullopt;
        else value;
    }
    else if constexpr(std::is_same<T, Types::_char>::value)
    {
        Types::_char value = str.empty()? '\0': str.at(0); 
        return value;
    }
    else return std::nullopt;
}

namespace linc
{
    BoundProgram Binder::bind(const Program* program)
    {
        BoundProgram bound_program;

        for(const auto& declaration: program->declarations)
            bound_program.declarations.push_back(bindDeclaration(declaration.get()));

        return std::move(bound_program);
    }

    std::unique_ptr<const BoundNode> Binder::bindNode(const Node* node)
    {
        if(!node)
            return nullptr;

        if(auto expression = dynamic_cast<const Expression*>(node))
            return bindExpression(expression);
        
        else if(auto statement = dynamic_cast<const Statement*>(node))
            return bindStatement(statement);
        
        else if(auto declaration = dynamic_cast<const Declaration*>(node))
            return bindDeclaration(declaration);

        else throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized node.");
    }

    std::unique_ptr<const BoundStatement> Binder::bindStatement(const Statement* statement)
    {
        if(!statement)
            return nullptr;

        else if(auto declaration_statement = dynamic_cast<const DeclarationStatement*>(statement))
            return bindDeclarationStatement(declaration_statement);

        else if(auto expression_statement = dynamic_cast<const ExpressionStatement*>(statement))
            return bindExpressionStatement(expression_statement);
        
        else if(auto scope_statement = dynamic_cast<const ScopeStatement*>(statement))
            return bindScopeStatement(scope_statement);

        else if(auto put_character_statement = dynamic_cast<const PutCharacterStatement*>(statement))
            return bindPutCharacterStatement(put_character_statement);

        else if(auto put_string_statement = dynamic_cast<const PutStringStatement*>(statement))
            return bindPutStringStatement(put_string_statement);

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized statement.");
    }

    std::unique_ptr<const BoundDeclaration> Binder::bindDeclaration(const Declaration* declaration)
    {
        if(auto variable_declaration = dynamic_cast<const VariableDeclaration*>(declaration))
            return bindVariableDeclaration(variable_declaration);

        else if(auto function_declaration = dynamic_cast<const FunctionDeclaration*>(declaration))
            return bindFunctionDeclaration(function_declaration);
        
        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized declaration.");
    }

    std::unique_ptr<const BoundExpression> Binder::bindExpression(const Expression* expression)
    {
        if(auto literal_expression = dynamic_cast<const LiteralExpression*>(expression))
            return bindLiteralExpression(literal_expression);
        
        else if(auto unary_expression = dynamic_cast<const UnaryExpression*>(expression))
            return bindUnaryExpression(unary_expression);
        
        else if(auto binary_expression = dynamic_cast<const BinaryExpression*>(expression))
            return bindBinaryExpression(binary_expression);
        
        else if(auto identifier_expression = dynamic_cast<const IdentifierExpression*>(expression))
            return bindIdentifierExpression(identifier_expression);

        else if(auto type_expression = dynamic_cast<const TypeExpression*>(expression))
            return bindTypeExpression(type_expression);
        
        else if(auto parenthesis_expression = dynamic_cast<const ParenthesisExpression*>(expression))
            return bindExpression(parenthesis_expression->getExpression());

        else if(auto if_else_expression = dynamic_cast<const IfElseExpression*>(expression))
            return bindIfElseExpression(if_else_expression);

        else if(auto while_expression = dynamic_cast<const WhileExpression*>(expression))
            return bindWhileExpression(while_expression);

        else if(auto for_expression = dynamic_cast<const ForExpression*>(expression))
            return bindForExpression(for_expression);

        else if(auto function_call_expression = dynamic_cast<const FunctionCallExpression*>(expression))
            return bindFunctionCallExpression(function_call_expression);

        else if(auto conversion_expression = dynamic_cast<const ConversionExpression*>(expression))
            return bindConversionExpression(conversion_expression);

        else if(auto array_initializer_expression = dynamic_cast<const ArrayInitializerExpression*>(expression))
            return bindArrayInitializerExpression(array_initializer_expression);

        else if(auto array_index_expression = dynamic_cast<const ArrayIndexExpression*>(expression))
            return bindArrayIndexExpression(array_index_expression);

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized expression.");
    }

    void Binder::reportInvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::type left_type, Types::type right_type)
    {
        Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Undefined binary operator '$' for operands '$' and '$'.",
            BoundBinaryOperator::kindToString(operator_kind), Types::toString(left_type), Types::toString(right_type))});
    }

    void Binder::reportInvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::type operand_type)
    {
        Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Undefined unary operator '$' for operand of type '$'.",
            BoundUnaryOperator::kindToString(operator_kind), Types::toString(operand_type))});
    }

    const std::unique_ptr<const BoundStatement> Binder::bindDeclarationStatement(const DeclarationStatement* statement)
    {
        auto declaration = bindDeclaration(statement->getDeclaration());
        return std::make_unique<const BoundDeclarationStatement>(std::move(declaration));
    }

    const std::unique_ptr<const BoundStatement> Binder::bindExpressionStatement(const ExpressionStatement* statement)
    {
        auto expression = bindExpression(statement->getExpression());
        return std::make_unique<const BoundExpressionStatement>(std::move(expression));
    }

    const std::unique_ptr<const BoundStatement> Binder::bindScopeStatement(const ScopeStatement* statement)
    {
        std::vector<std::unique_ptr<const BoundStatement>> statements;

        m_boundDeclarations.beginScope();

        for(const auto& statement : statement->getStatements())
            statements.push_back(bindStatement(statement.get()));
        
        m_boundDeclarations.endScope();

        return std::make_unique<const BoundScopeStatement>(std::move(statements));
    }

    const std::unique_ptr<const BoundStatement> Binder::bindPutCharacterStatement(const PutCharacterStatement* statement)
    {
        auto expression = bindExpression(statement->getExpression());
        return std::make_unique<const BoundPutCharacterStatement>(std::move(expression));
    }

    const std::unique_ptr<const BoundStatement> Binder::bindPutStringStatement(const PutStringStatement* statement)
    {
        auto expression = bindExpression(statement->getExpression());
        return std::make_unique<const BoundPutStringStatement>(std::move(expression));
    }

    const std::unique_ptr<const BoundDeclaration> Binder::bindVariableDeclaration(const VariableDeclaration* declaration, bool is_argument)
    {
        auto type = Types::unique_cast<const BoundTypeExpression>(bindTypeExpression(declaration->getType()))->getActualType(); 

        auto name = declaration->getIdentifier()->getIdentifierToken().value.value();
        auto default_value = declaration->getDefaultValue()? std::make_optional(bindExpression(declaration->getDefaultValue()->getExpression())): std::nullopt;

        auto variable = std::make_unique<const BoundVariableDeclaration>(type, name, std::move(default_value));
        
        if(variable->getDefaultValue() && !variable->getDefaultValue().value()->getType().isAssignableTo(type))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Cannot assign expression of type '$' to variable of type '$'", 
                    Types::toString(variable->getDefaultValue().value()->getType()), Types::toString(type))});

        else if(!is_argument && !type.isMutable && !default_value)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Cannot declare immutable variable '$' without default value.",
                    name)
            });

        else if(!m_boundDeclarations.push(variable->cloneConst()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Cannot redeclare symbol '$' as variable of type '$'.", 
                    name, Types::toString(variable->getDefaultValue().value()->getType()))});

        return std::move(variable);
    }

    const std::unique_ptr<const BoundDeclaration> Binder::bindFunctionDeclaration(const FunctionDeclaration* declaration)
    {
        auto return_type = Types::unique_cast<const BoundTypeExpression>(bindTypeExpression(declaration->getReturnType()))->getActualType();
        auto name = declaration->getIdentifier()->getValue();

        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;

        m_boundDeclarations.beginScope();

        bool has_default_value{false}, has_error{false};

        for(std::size_t i = 0; i < declaration->getArguments().size(); ++i)
        {
            const auto& argument = declaration->getArguments()[i];
            auto bound_argument = Types::unique_cast<const BoundVariableDeclaration>(bindVariableDeclaration(argument.get(), true));

            if(bound_argument->getDefaultValue().has_value())
                has_default_value = true;
            else if(has_default_value)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = "Function argument with default value cannot be followed by non-default-value argument."
                });

                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Argument $ in function '$'.", i, name)
                });

                has_error = true;
                break;
            }

            arguments.push_back(std::move(bound_argument));
        }

        auto body = bindStatement(declaration->getBody());        
        auto function = std::make_unique<const BoundFunctionDeclaration>(return_type, name, std::move(arguments), std::move(body));
        
        if(!function->getBody()->getType().isAssignableTo(function->getReturnType()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Function '$' declared with return type '$', but it evaluates to type '$'.",
                    name, Types::toString(function->getReturnType()), Types::toString(function->getBody()->getType()))});
        
        m_boundDeclarations.endScope();

        if(!has_error && !m_boundDeclarations.push(std::move(function->cloneConst())))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot redeclare symbol '$'.", name)});

        return function;
    }
 
    const std::unique_ptr<const BoundExpression> Binder::bindIdentifierExpression(const IdentifierExpression* expression)
    {
        auto value = expression->getValue();
        auto find = m_boundDeclarations.find(value);
        
        if(find == m_boundDeclarations.end())
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Undeclared identifier '$'.", value)});

            return std::make_unique<const BoundIdentifierExpression>(value, Types::invalidType);
        }
        else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(find->get()))
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Cannot reference identifier '$', as it is a function.", value)});

            return std::make_unique<const BoundIdentifierExpression>(value, Types::invalidType);
        }
        else if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(find->get()))
            return std::make_unique<const BoundIdentifierExpression>(value, variable->getActualType());
        
        return std::make_unique<const BoundIdentifierExpression>(value, Types::invalidType);
    }

    const std::unique_ptr<const BoundExpression> Binder::bindTypeExpression(const TypeExpression* expression)
    {
        auto type = Types::kindFromUserString(expression->getTypeIdentifier().value.value());
        auto is_array = expression->getLeftBracket() && expression->getRightBracket();
        auto is_mutable = expression->getMutabilityKeyword().has_value();
        
        if(auto _array_size = expression->getArraySize())
        {
            auto array_size = Types::unique_cast<const BoundLiteralExpression>(bindLiteralExpression(*_array_size));
            return std::make_unique<const BoundTypeExpression>(type, is_mutable, is_array, array_size->getValue().getU64());
        }

        else return std::make_unique<const BoundTypeExpression>(type, is_mutable, is_array, std::nullopt);
    }

    const std::unique_ptr<const BoundExpression> Binder::bindIfElseExpression(const IfElseExpression* expression)
    {
        auto test_expression = bindExpression(expression->getTestExpression());
        auto body_if_statement = bindStatement(expression->getIfBodyStatement());
        auto _body_else_statement = expression->getElseBodyStatement();

        if(_body_else_statement)
        {
            auto body_else_statement = bindStatement(_body_else_statement.value());
            auto type = body_if_statement->getType() == body_else_statement->getType()? body_if_statement->getType(): Types::voidType;
            return std::make_unique<const BoundIfElseExpression>(std::move(test_expression), std::move(body_if_statement), std::move(body_else_statement), type);
        }
        else return std::make_unique<const BoundIfElseExpression>(std::move(test_expression), std::move(body_if_statement), Types::voidType);
    }

    const std::unique_ptr<const BoundExpression> Binder::bindWhileExpression(const WhileExpression* expression)
    {
        auto test_expression = bindExpression(expression->getTestExpression());
        auto body_statement = bindStatement(expression->getWhileBodyStatement());
        auto type = body_statement->getType();

        auto _body_finally_statement = expression->getFinallyBodyStatement();
        auto _body_else_statement = expression->getElseBodyStatement();

        if(_body_finally_statement)
        {
            auto body_finally_statement = bindStatement(_body_finally_statement.value());
            auto finally_type = type.isCompatible(body_finally_statement->getType())? type: Types::voidType;

            if(_body_else_statement)
            {
                auto body_else_statement = bindStatement(_body_else_statement.value());
                auto else_type = finally_type.isCompatible(body_else_statement->getType())? finally_type: Types::voidType;

                return std::make_unique<const BoundWhileExpression>(else_type, std::move(test_expression), std::move(body_statement), 
                    std::move(body_finally_statement), std::move(body_else_statement));
            }
            else return std::make_unique<const BoundWhileExpression>(finally_type, std::move(test_expression), std::move(body_statement), 
                std::move(body_finally_statement));
        }
        else if(_body_else_statement)
        {
            auto body_else_statement = bindStatement(_body_else_statement.value());
            auto else_type = type.isCompatible(body_else_statement->getType())? type: Types::voidType;

            return std::make_unique<const BoundWhileExpression>(else_type, std::move(test_expression), std::move(body_statement), 
                std::nullopt, std::move(body_else_statement));
        }

        else return std::make_unique<const BoundWhileExpression>(Types::voidType, std::move(test_expression), std::move(body_statement));
    }

    const std::unique_ptr<const BoundExpression> Binder::bindForExpression(const ForExpression* expression)
    {
        m_boundDeclarations.beginScope();
        const auto& specifier = expression->getSpecifier();

        if(auto variable_specifier = std::get_if<const ForExpression::VariableForSpecifier>(&specifier))
        {
            auto variable_declaration = Types::unique_cast<const BoundVariableDeclaration>(
                bindVariableDeclaration(variable_specifier->variableDeclaration.get()));

            auto _expression = bindExpression(variable_specifier->expression.get());
            auto statement = bindStatement(variable_specifier->statement.get());
            auto body = bindStatement(expression->getBody());
            
            m_boundDeclarations.endScope(); 
            return std::make_unique<const BoundForExpression>(std::move(variable_declaration), std::move(_expression), std::move(statement), std::move(body));
        }
        else if(auto range_specifier = std::get_if<const ForExpression::RangeForSpecifier>(&specifier))
        {
            auto array_identifier = Types::unique_cast<const BoundIdentifierExpression>(bindIdentifierExpression(range_specifier->arrayIdentifier.get()));
            auto variable_declaration = std::make_unique<const BoundVariableDeclaration>(
                Types::type{.kind = array_identifier->getType().kind, .isMutable = true}, range_specifier->valueIdentifier->getValue(), std::nullopt);
            
            if(!m_boundDeclarations.push(variable_declaration->cloneConst()))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Cannot name range-for-loop variable '$'.", 
                        range_specifier->valueIdentifier->getValue())
                });

            auto value_identifier = Types::unique_cast<const BoundIdentifierExpression>(bindIdentifierExpression(range_specifier->valueIdentifier.get())); 
            auto body = bindStatement(expression->getBody());

            m_boundDeclarations.endScope();
            return std::make_unique<const BoundForExpression>(std::move(value_identifier), std::move(array_identifier), std::move(body));
            
        }
        else throw LINC_EXCEPTION_OUT_OF_BOUNDS(std::variant);
    }

    const std::unique_ptr<const BoundExpression> Binder::bindFunctionCallExpression(const FunctionCallExpression* expression)
    {
        auto name = expression->getIdentifierToken().value.value();
        std::vector<BoundFunctionCallExpression::Argument> arguments;
        
        auto find = m_boundDeclarations.find(name);

        if(find == m_boundDeclarations.end())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot call undeclared function '$'.", name)});

        else if(dynamic_cast<const BoundVariableDeclaration*>(find->get()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot call identifier '$', as it is a variable.", name)});

        else 
        {
            auto function = dynamic_cast<const BoundFunctionDeclaration*>(find->get());

            if(function->getArguments().size() < expression->getArguments().size()
            || function->getArguments().size() - function->getDefaultArgumentCount() > expression->getArguments().size())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Tried to call function '$' with $ arguments, when it takes $ (with $ default arguments).", 
                        name, expression->getArguments().size(), function->getArguments().size(), function->getDefaultArgumentCount())});

            using _size = std::vector<std::unique_ptr<const Expression>>::size_type;

            for(_size i = 0ull; i < std::min(expression->getArguments().size(), function->getArguments().size()); i++)
            {
                const auto& argument = expression->getArguments()[i];
                const auto& declared_argument = function->getArguments()[i];
                auto bound_argument = bindExpression(argument.get());

                if(!declared_argument->getActualType().isCompatible(bound_argument->getType()))
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                        .message = Logger::format("Invalid argument type in function '$'.", name)});
                    
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                        .message = Logger::format("Argument $ called with type '$', expected '$'.",
                            i, Types::toString(bound_argument->getType()), Types::toString(declared_argument->getActualType()))});
                }

                else arguments.push_back(BoundFunctionCallExpression::Argument{
                    .name = declared_argument->getName(),
                    .value = std::move(bound_argument),
                    .isMutable = declared_argument->getActualType().isMutable
                });
            }

            for(_size i = expression->getArguments().size(); i < function->getArguments().size()
                && i >= function->getArguments().size() - function->getDefaultArgumentCount(); ++i)
            {
                const auto& declared_argument = function->getArguments()[i];
                
                arguments.push_back(BoundFunctionCallExpression::Argument{
                    .name = declared_argument->getName(),
                    .value = declared_argument->getDefaultValue().value()->cloneConst(),
                    .isMutable = declared_argument->getType().isMutable
                });
            }

            return std::make_unique<const BoundFunctionCallExpression>(function->getReturnType(), name, std::move(arguments), 
                std::move(function->getBody()->cloneConst()));
        }

        return std::make_unique<const BoundFunctionCallExpression>(Types::invalidType, name, std::move(arguments),
            std::make_unique<const BoundExpressionStatement>(std::make_unique<const BoundLiteralExpression>(PrimitiveValue::invalidValue, Types::invalidType)));
    }

    const std::unique_ptr<const BoundExpression> Binder::bindLiteralExpression(const LiteralExpression* expression)
    {
        switch(expression->getType())
        {
        case Token::Type::U8Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u8>(expression->getValue()).value_or(static_cast<Types::u8>(0)),
                Types::fromKind(Types::Kind::u8));
        case Token::Type::U16Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u16>(expression->getValue()).value_or(static_cast<Types::u16>(0)),
                Types::fromKind(Types::Kind::u16));
        case Token::Type::U32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u32>(expression->getValue()).value_or(static_cast<Types::u32>(0)),
                Types::fromKind(Types::Kind::u32));
        case Token::Type::U64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u64>(expression->getValue()).value_or(static_cast<Types::u64>(0)),
                Types::fromKind(Types::Kind::u64));
        case Token::Type::I8Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i8>(expression->getValue()).value_or(static_cast<Types::i8>(0)),
                Types::fromKind(Types::Kind::i8));
        case Token::Type::I16Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i16>(expression->getValue()).value_or(static_cast<Types::i16>(0)),
                Types::fromKind(Types::Kind::i16));
        case Token::Type::I32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i32>(expression->getValue()).value_or(static_cast<Types::i32>(0)),
                Types::fromKind(Types::Kind::i32));
        case Token::Type::I64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i64>(expression->getValue()).value_or(static_cast<Types::i64>(0)),
                Types::fromKind(Types::Kind::i64));
        case Token::Type::F32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::f32>(expression->getValue()).value_or(static_cast<Types::f32>(0)),
                Types::fromKind(Types::Kind::f32));
        case Token::Type::F64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::f64>(expression->getValue()).value_or(static_cast<Types::f64>(0)),
                Types::fromKind(Types::Kind::f64));
        case Token::Type::CharacterLiteral:
            return std::make_unique<const BoundLiteralExpression>(Types::parseUserCharacter(expression->getValue()), Types::fromKind(Types::Kind::_char));
        case Token::Type::StringLiteral:
            return std::make_unique<const BoundLiteralExpression>(expression->getValue(), Types::fromKind(Types::Kind::string));
        case Token::Type::KeywordTrue:
        case Token::Type::KeywordFalse:
            return std::make_unique<const BoundLiteralExpression>(Types::parseBoolean(expression->getValue()), Types::fromKind(Types::Kind::_bool));
        default:
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = "Unrecognized literal expression."});
        return std::make_unique<const BoundLiteralExpression>(PrimitiveValue::invalidValue, Types::invalidType);
        }
    }

    BoundUnaryOperator::Kind Binder::bindUnaryOperatorKind(Token::Type token_type)
    {
        switch(token_type)
        {
        case Token::Type::OperatorPlus: return BoundUnaryOperator::Kind::UnaryPlus;
        case Token::Type::OperatorMinus: return BoundUnaryOperator::Kind::UnaryMinus;
        case Token::Type::OperatorLogicalNot: return BoundUnaryOperator::Kind::LogicalNot;
        case Token::Type::OperatorStringify: return BoundUnaryOperator::Kind::Stringify;
        case Token::Type::OperatorIncrement: return BoundUnaryOperator::Kind::Increment;
        case Token::Type::OperatorDecrement: return BoundUnaryOperator::Kind::Decrement;
        case Token::Type::TypeSpecifier: return BoundUnaryOperator::Kind::Typeof;
        default: return BoundUnaryOperator::Kind::Invalid;
        }
    }

    const std::unique_ptr<const BoundExpression> Binder::bindUnaryExpression(const UnaryExpression* expression)
    {
        auto operand = bindExpression(expression->getOperand());
        auto kind = bindUnaryOperatorKind(expression->getOperatorToken().type);
        auto _operator = std::make_unique<const BoundUnaryOperator>(kind, operand->getType());

        if(_operator->getReturnType().kind == Types::Kind::invalid)
            reportInvalidUnaryOperator(kind, operand->getType());

        return std::make_unique<const BoundUnaryExpression>(std::move(_operator), std::move(operand));
    }

    BoundBinaryOperator::Kind Binder::bindBinaryOperatorKind(Token::Type token_type)
    {
        switch(token_type)
        {
        case Token::Type::OperatorPlus: return BoundBinaryOperator::Kind::Addition;
        case Token::Type::OperatorMinus: return BoundBinaryOperator::Kind::Subtraction;
        case Token::Type::OperatorAsterisk: return BoundBinaryOperator::Kind::Multiplication;
        case Token::Type::OperatorSlash: return BoundBinaryOperator::Kind::Division;
        case Token::Type::OperatorLogicalAnd: return BoundBinaryOperator::Kind::LogicalAnd;
        case Token::Type::OperatorLogicalOr: return BoundBinaryOperator::Kind::LogicalOr;
        case Token::Type::OperatorEquals: return BoundBinaryOperator::Kind::Equals;
        case Token::Type::OperatorNotEquals: return BoundBinaryOperator::Kind::NotEquals;
        case Token::Type::OperatorGreater: return BoundBinaryOperator::Kind::Greater;
        case Token::Type::OperatorLess: return BoundBinaryOperator::Kind::Less;
        case Token::Type::OperatorGreaterEqual: return BoundBinaryOperator::Kind::GreaterEqual;
        case Token::Type::OperatorLessEqual: return BoundBinaryOperator::Kind::LessEqual;
        case Token::Type::OperatorAssignment: return BoundBinaryOperator::Kind::Assignment;
        default: return BoundBinaryOperator::Kind::Invalid;
        }
    }

    const std::unique_ptr<const BoundExpression> Binder::bindBinaryExpression(const BinaryExpression* expression)
    {
        auto left = bindExpression(expression->getLeft());
        auto right = bindExpression(expression->getRight());
        auto kind = bindBinaryOperatorKind(expression->getOperatorToken().type);
        auto _operator = std::make_unique<const BoundBinaryOperator>(kind, left->getType(), right->getType());

        if(_operator->getReturnType().kind == Types::Kind::invalid)
            reportInvalidBinaryOperator(kind, left->getType(), right->getType());

        return std::make_unique<const BoundBinaryExpression>(std::move(_operator), std::move(left), std::move(right));
    }

    const std::unique_ptr<const BoundExpression> Binder::bindConversionExpression(const ConversionExpression* expression)
    {
        auto inner_expression = bindExpression(expression->getExpression());
        auto target_type = Types::unique_cast<const BoundTypeExpression>(bindExpression(expression->getType()))->getActualType();

        auto conversion = std::make_unique<const BoundConversion>(inner_expression->getType(), target_type);

        if(conversion->getReturnType().kind == Types::Kind::invalid)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Invalid conversion expression to type '$' (from '$').",
                    Types::toString(target_type), Types::toString(inner_expression->getType()))
            });

        return std::make_unique<const BoundConversionExpression>(std::move(inner_expression), std::move(conversion));
    }

    const std::unique_ptr<const BoundExpression> Binder::bindArrayInitializerExpression(const ArrayInitializerExpression* expression)
    {
        std::vector<std::unique_ptr<const BoundExpression>> values{};
        auto type = Types::voidType;

        for(const auto& value: expression->getValues())
        {
            auto bound_value = bindExpression(value.get());

            if(type == Types::voidType);
            else if(type != bound_value->getType())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = "All members of an array initializer must be of the same type."
                });
            type = bound_value->getType();
            
            values.push_back(std::move(bound_value));
        }

        return std::make_unique<const BoundArrayInitializerExpression>(std::move(values), Types::type{
            .kind = type.kind,
            .isMutable = type.isMutable,
            .isArray = true,
            .arraySize = expression->getValues().size()
        });
    }

    const std::unique_ptr<const BoundExpression> Binder::bindArrayIndexExpression(const ArrayIndexExpression* expression)
    {
        auto identifier = Types::unique_cast<const BoundIdentifierExpression>(bindIdentifierExpression(expression->getIdentifier()));
        auto index = bindExpression(expression->getIndex());

        if(index->getType().isArray || index->getType().kind != Types::Kind::u64)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Incompatible type in array indexing: expected '$', found '$'.",
                    Types::toString(Types::fromKind(Types::Kind::u64)), Types::toString(index->getType()))
            });
            return std::make_unique<const BoundArrayIndexExpression>(std::move(identifier), std::move(index), Types::invalidType);
        }
        else if(auto literal = dynamic_cast<const BoundLiteralExpression*>(index.get()))
        {
            auto value = literal->getValue().getIfU64().value_or(0ull);

            if(identifier->getType().arraySize && value >= identifier->getType().arraySize.value())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Warning, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Accessing invalid array index $ is undefined behavior (array '$' has $ elements).",
                        value, identifier->getValue(), identifier->getType().arraySize.value())
                });
        }

        if(!identifier->getType().isArray && identifier->getType().kind != Types::Kind::string)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot index non-array non-string operand '$'.",
                    identifier->getValue())
            });
            
            return std::make_unique<const BoundArrayIndexExpression>(std::move(identifier), std::move(index), Types::invalidType);
        }

        return std::make_unique<const BoundArrayIndexExpression>(std::move(identifier), std::move(index), Types::type{
            .kind = identifier->getType().kind,
            .isMutable = identifier->getType().isMutable,
        });

        return std::make_unique<const BoundArrayIndexExpression>(std::move(identifier), std::move(index), Types::invalidType);
    }
}