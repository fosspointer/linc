#include <linc/bound_tree/Binder.hpp>

template <typename T>
static std::optional<T> parseString(const std::string& str)
{
    using namespace linc;

    char* p;
    if constexpr (std::is_same_v<T, linc::Types::u8>)
    {
        Types::u8 value = strtol(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::u16>)
    {
        Types::u16 value = strtol(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::u32>)
    {
        Types::u32 value = strtol(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::u64>)
    {
        Types::u64 value = strtoll(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::i8>)
    {
        Types::i8 value = strtoul(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::i16>)
    {
        Types::i16 value = strtoul(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::i32>)
    {
        Types::i32 value = strtoul(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::i64>)
    {
        Types::i64 value = strtoull(str.c_str(), &p, 10);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::f32>)
    {
        Types::f32 value = strtof(str.c_str(), &p);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::f64>)
    {
        Types::f64 value = strtod(str.c_str(), &p);
        
        if(*p)
            return std::nullopt;
        else return value;
    }
    else if constexpr (std::is_same_v<T, Types::_bool>)
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
    else if constexpr (std::is_same_v<T, Types::_char>)
    {
        Types::_char value = str.empty()? '\77': str.at(0); 
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
        if(auto expression = dynamic_cast<const Expression*>(node))
            return bindExpression(expression);
        else if(auto statement = dynamic_cast<const Statement*>(node))
            return bindStatement(statement);
        else if(auto declaration = dynamic_cast<const Declaration*>(node))
            return bindDeclaration(declaration);

        else throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized node");
    }

    std::unique_ptr<const BoundStatement> Binder::bindStatement(const Statement* statement)
    {
        if(auto declaration_statement = dynamic_cast<const DeclarationStatement*>(statement))
            return bindDeclarationStatement(declaration_statement);

        else if(auto expression_statement = dynamic_cast<const ExpressionStatement*>(statement))
            return bindExpressionStatement(expression_statement);
        
        else if(auto scope_statement = dynamic_cast<const ScopeStatement*>(statement))
            return bindScopeStatement(scope_statement);

        else if(auto put_character_statement = dynamic_cast<const PutCharacterStatement*>(statement))
            return bindPutCharacterStatement(put_character_statement);

        else if(auto put_string_statement = dynamic_cast<const PutStringStatement*>(statement))
            return bindPutStringStatement(put_string_statement);

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized statement");
    }

    std::unique_ptr<const BoundDeclaration> Binder::bindDeclaration(const Declaration* declaration)
    {
        if(auto variable_declaration = dynamic_cast<const VariableDeclaration*>(declaration))
            return bindVariableDeclaration(variable_declaration);

        else if(auto argument_declaration = dynamic_cast<const ArgumentDeclaration*>(declaration))
            return bindArgumentDeclaration(argument_declaration);

        else if(auto function_declaration = dynamic_cast<const FunctionDeclaration*>(declaration))
            return bindFunctionDeclaration(function_declaration);
        
        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized declaration");
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
        
        else if(auto parenthesis_expression = dynamic_cast<const ParenthesisExpression*>(expression))
            return bindExpression(parenthesis_expression->getExpression());

        else if(auto if_else_expression = dynamic_cast<const IfElseExpression*>(expression))
            return bindIfElseExpression(if_else_expression);

        else if(auto while_expression = dynamic_cast<const WhileExpression*>(expression))
            return bindWhileExpression(while_expression);

        else if(auto variable_assignment_expression = dynamic_cast<const VariableAssignmentExpression*>(expression))
            return bindVariableAssignmentExpression(variable_assignment_expression);

        else if(auto function_call_expression = dynamic_cast<const FunctionCallExpression*>(expression))
            return bindFunctionCallExpression(function_call_expression);

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized expression");
    }

    void Binder::reportInvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::Type left_type, Types::Type right_type)
    {
        Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Undefined binary operator '$' for operands '$' and '$'.",
            BoundBinaryOperator::kindToString(operator_kind), Types::toString(left_type), Types::toString(right_type))});
    }

    void Binder::reportInvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::Type operand_type)
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

    const std::unique_ptr<const BoundDeclaration> Binder::bindVariableDeclaration(const VariableDeclaration* declaration)
    {
        Types::Type type = Types::fromUserString(declaration->getTypeNameIdentifierToken().value.value());
        auto name = declaration->getIdentifierExpression()->getValue();
        auto value_expression = bindExpression(declaration->getExpression());
        auto is_mutable = declaration->getMutableKeywordToken().has_value();

        auto variable = std::make_unique<const BoundVariableDeclaration>(type, name, is_mutable, std::move(value_expression));
        
        if(variable->getValueExpression()->getType() != type)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Cannot assign expression of type '$' to variable of type '$'", 
                    Types::toString(variable->getValueExpression()->getType()), Types::toString(type))});

        else if(!m_boundDeclarations.push(std::move(variable->clone_const())))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Cannot redeclare symbol '$' with type '$'.", 
                    name, Types::toString(variable->getValueExpression()->getType()))});

        return std::move(variable);
    }

    const std::unique_ptr<const BoundDeclaration> Binder::bindArgumentDeclaration(const ArgumentDeclaration* declaration)
    {
        auto name = declaration->getVarnameIdentifierToken().value.value();
        auto type = Types::fromUserString(declaration->getTypenameIdentifierToken().value.value());
        auto is_mutable = declaration->getMutableToken().has_value();
        auto value = declaration->getDefaultValueExpression().has_value()? bindExpression(declaration->getDefaultValueExpression().value()):
            std::optional<std::unique_ptr<const BoundExpression>>{};

        auto argument = std::make_unique<const BoundArgumentDeclaration>(type, name, is_mutable, std::move(value));

        if(!m_boundDeclarations.push(argument->clone_const()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Argument name '$' has already been declared in this scope.", name)});

        return std::move(argument);
    }

    const std::unique_ptr<const BoundDeclaration> Binder::bindFunctionDeclaration(const FunctionDeclaration* declaration)
    {
        auto return_type = Types::fromUserString(declaration->getReturnTypeToken().value.value());
        auto name = declaration->getIdentifierToken().value.value();

        std::vector<std::unique_ptr<const BoundArgumentDeclaration>> arguments;

        m_boundDeclarations.beginScope();

        for(const auto& argument: declaration->getArguments())
        {
            auto bound_argument = bindArgumentDeclaration(argument.get());
            arguments.push_back(std::unique_ptr<const BoundArgumentDeclaration>(dynamic_cast<const BoundArgumentDeclaration*>(bound_argument.get())));
            bound_argument.release();
        }


        auto body = bindStatement(declaration->getBody());        
        auto function = std::make_unique<const BoundFunctionDeclaration>(return_type, name, std::move(arguments), std::move(body));
        
        if(function->getReturnType() != function->getBody()->getType())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Function '$' declared with return type $, but it evaluates to type $.",
                    name, Types::toString(function->getReturnType()), Types::toString(function->getBody()->getType()))});
        
        m_boundDeclarations.endScope();

        if(!m_boundDeclarations.push(std::move(function->clone_const())))
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

            return std::make_unique<const BoundIdentifierExpression>(value, Types::Type::invalid);
        }
        else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(find->get()))
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Cannot reference identifier '$', as it is a function.", value)});

            return std::make_unique<const BoundIdentifierExpression>(value, Types::Type::invalid);
        }
        else if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(find->get()))
            return std::make_unique<const BoundIdentifierExpression>(value, variable->getType());
        
        else if(auto argument = dynamic_cast<const BoundArgumentDeclaration*>(find->get()))
            return std::make_unique<const BoundIdentifierExpression>(value, argument->getType());
        
        return std::make_unique<const BoundIdentifierExpression>(value, Types::Type::invalid);
    }

    const std::unique_ptr<const BoundExpression> Binder::bindIfElseExpression(const IfElseExpression* expression)
    {
        auto test_expression = bindExpression(expression->getTestExpression());
        auto body_if_statement = bindStatement(expression->getIfBodyStatement());
        auto _body_else_statement = expression->getElseBodyStatement();

        if(_body_else_statement)
        {
            auto body_else_statement = bindStatement(_body_else_statement.value());
            auto type = body_if_statement->getType() == body_else_statement->getType()? body_if_statement->getType(): Types::Type::_void;
            return std::make_unique<const BoundIfElseExpression>(std::move(test_expression), std::move(body_if_statement), std::move(body_else_statement), type);
        }
        else return std::make_unique<const BoundIfElseExpression>(std::move(test_expression), std::move(body_if_statement), Types::Type::_void);
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
            auto finally_type = type == body_finally_statement->getType()? type: Types::Type::_void;

            if(_body_else_statement)
            {
                auto body_else_statement = bindStatement(_body_else_statement.value());
                auto else_type = finally_type == body_else_statement->getType()? finally_type: Types::Type::_void;

                return std::make_unique<const BoundWhileExpression>(else_type, std::move(test_expression), std::move(body_statement), 
                    std::move(body_finally_statement), std::move(body_else_statement));
            }
            else return std::make_unique<const BoundWhileExpression>(finally_type, std::move(test_expression), std::move(body_statement), 
                std::move(body_finally_statement));
        }
        else if(_body_else_statement)
        {
            auto body_else_statement = bindStatement(_body_else_statement.value());
            auto else_type = type == body_else_statement->getType()? type: Types::Type::_void;

            return std::make_unique<const BoundWhileExpression>(else_type, std::move(test_expression), std::move(body_statement), 
                std::nullopt, std::move(body_else_statement));
        }

        else return std::make_unique<const BoundWhileExpression>(type, std::move(test_expression), std::move(body_statement));
    }

    const std::unique_ptr<const BoundExpression> Binder::bindVariableAssignmentExpression(const VariableAssignmentExpression* expression)
    {
        auto identifier = *expression->getIdentifierToken().value;
        auto value = bindExpression(expression->getValue());
        auto find = m_boundDeclarations.find(identifier);

        if(find == m_boundDeclarations.end())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Variable '$' has not been declared and cannot be reassigned.", identifier)});
        
        else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(find->get()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot reassign identifier '$', as it is a function.", identifier)});

        else if(auto argument = dynamic_cast<const BoundArgumentDeclaration*>(find->get()))
        {
            if(!argument->getMutable())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Cannot reassign immutable function argument '$' of type '$'", identifier, 
                        Types::toString(argument->getType()))});
        }
        else 
        {
            auto variable = dynamic_cast<const BoundVariableDeclaration*>(find->get());

            if(!variable->getMutable())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Cannot reassign immutable variable '$' of type '$'", identifier, Types::toString(variable->getType()))});
        }

        return std::make_unique<const BoundVariableAssignmentExpression>(identifier, std::move(value));
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
        else if(dynamic_cast<const BoundArgumentDeclaration*>(find->get()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot call identifier '$', as it is a function argument.", name)});

        else 
        {
            auto function = dynamic_cast<const BoundFunctionDeclaration*>(find->get());

            if(function->getArguments().size() != expression->getArguments().size())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Tried to call function '$' with $ arguments, when it takes $.", 
                        name, expression->getArguments().size(), function->getArguments().size())});

            for(std::vector<std::unique_ptr<const Expression>>::size_type i = 0; i < expression->getArguments().size(); i++)
            {
                const auto& argument = expression->getArguments()[i];
                const auto& declared_argument = function->getArguments()[i];
                auto bound_argument = bindExpression(argument.get());

                if(declared_argument->getType() != bound_argument->getType())
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                        .message = Logger::format("Cannot pass value of type $ to argument $ of function '$', as it is of type $", 
                            Types::toString(bound_argument->getType()), i, name, Types::toString(declared_argument->getType()))});

                else arguments.push_back(BoundFunctionCallExpression::Argument{
                    .name = declared_argument->getName(),
                    .value = std::move(bound_argument),
                    .isMutable = declared_argument->getMutable()
                });
            }

            return std::make_unique<const BoundFunctionCallExpression>(function->getReturnType(), name, std::move(arguments), 
                std::move(function->getBody()->clone_const()));
        }

        return std::make_unique<const BoundFunctionCallExpression>(Types::Type::invalid, name, std::move(arguments),
            std::make_unique<const BoundExpressionStatement>(std::make_unique<const BoundLiteralExpression>(TypedValue::invalidValue)));
    }

    const std::unique_ptr<const BoundExpression> Binder::bindLiteralExpression(const LiteralExpression* expression)
    {
        switch(expression->getType())
        {
        case Token::Type::U8Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u8>(expression->getValue()).value_or(static_cast<Types::u8>(0)));
        case Token::Type::U16Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u16>(expression->getValue()).value_or(static_cast<Types::u16>(0)));
        case Token::Type::U32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u32>(expression->getValue()).value_or(static_cast<Types::u32>(0)));
        case Token::Type::U64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u64>(expression->getValue()).value_or(static_cast<Types::u64>(0)));
        case Token::Type::I8Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i8>(expression->getValue()).value_or(static_cast<Types::i8>(0)));
        case Token::Type::I16Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i16>(expression->getValue()).value_or(static_cast<Types::i16>(0)));
        case Token::Type::I32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i32>(expression->getValue()).value_or(static_cast<Types::i32>(0)));
        case Token::Type::I64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i64>(expression->getValue()).value_or(static_cast<Types::i64>(0)));
        case Token::Type::F32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::f32>(expression->getValue()).value_or(static_cast<Types::f32>(0)));
        case Token::Type::F64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::f64>(expression->getValue()).value_or(static_cast<Types::f64>(0)));
        case Token::Type::CharacterLiteral:
            return std::make_unique<const BoundLiteralExpression>(Types::parseUserCharacter(expression->getValue()));
        case Token::Type::StringLiteral:
            return std::make_unique<const BoundLiteralExpression>(expression->getValue());
        case Token::Type::KeywordTrue:
        case Token::Type::KeywordFalse:
            return std::make_unique<const BoundLiteralExpression>(Types::parseBoolean(expression->getValue()));
        default:
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = "Unrecognized literal expression."});
        return std::make_unique<const BoundLiteralExpression>(TypedValue::invalidValue);
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
        default: return BoundUnaryOperator::Kind::Invalid;
        }
    }

    const std::unique_ptr<const BoundExpression> Binder::bindUnaryExpression(const UnaryExpression* expression)
    {
        auto operand = bindExpression(expression->getOperand());
        auto kind = bindUnaryOperatorKind(expression->getOperatorToken().type);
        auto _operator = std::make_unique<BoundUnaryOperator>(kind, operand->getType());

        if(_operator->getReturnType() == Types::Type::invalid)
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
        default: return BoundBinaryOperator::Kind::Invalid;
        }
    }

    const std::unique_ptr<const BoundExpression> Binder::bindBinaryExpression(const BinaryExpression* expression)
    {
        auto left = bindExpression(expression->getLeft());
        auto right = bindExpression(expression->getRight());
        auto kind = bindBinaryOperatorKind(expression->getOperatorToken().type);
        auto _operator = std::make_unique<const BoundBinaryOperator>(kind, left->getType(), right->getType());

        if(_operator->getReturnType() == Types::Type::invalid)
            reportInvalidBinaryOperator(kind, left->getType(), right->getType());

        return std::make_unique<const BoundBinaryExpression>(std::move(_operator), std::move(left), std::move(right));
    }
}