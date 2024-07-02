#include <linc/bound_tree/Binder.hpp>
#include <linc/Lexer.hpp>
#include <linc/system/Internals.hpp>

template <typename T>
static std::optional<T> parseString(const std::string& str, std::optional<linc::Token::NumberBase> base)
{
    using namespace linc;
    
    if(str.empty())
        return static_cast<T>(0);

    char* p;
    if constexpr(std::is_integral<T>::value)
    {
        T result{};
        bool negative{str[0ul] == '-'};

        for(std::size_t i{negative? 1ul: 0ul}; i < str.size(); ++i)
            if(linc::Lexer::isDigit(str[i], base.value()))
            {
                auto displacement = std::isdigit(str[i])? 0: 39;
                result = result * static_cast<T>(linc::Token::baseToInt(base.value())) + (str[i] - '0' - displacement);
            }
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
        else return value;
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
            return Types::unique_cast<const BoundStatement>(bindDeclarationStatement(declaration_statement));

        else if(auto expression_statement = dynamic_cast<const ExpressionStatement*>(statement))
            return Types::unique_cast<const BoundStatement>(bindExpressionStatement(expression_statement));

        else if(auto label_statement = dynamic_cast<const LabelStatement*>(statement))
            return Types::unique_cast<const BoundStatement>(bindLabelStatement(label_statement));

        else if(auto jump_statement = dynamic_cast<const JumpStatement*>(statement))
            return Types::unique_cast<const BoundStatement>(bindJumpStatement(jump_statement));

        else if(auto return_statement = dynamic_cast<const ReturnStatement*>(statement))
            return Types::unique_cast<const BoundStatement>(bindReturnStatement(return_statement));

        else if(auto continue_statement = dynamic_cast<const ContinueStatement*>(statement))
            return Types::unique_cast<const BoundStatement>(bindContinueStatement(continue_statement));

        else if(auto break_statement = dynamic_cast<const BreakStatement*>(statement))
            return Types::unique_cast<const BoundStatement>(bindBreakStatement(break_statement));

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized statement");
    }

    std::unique_ptr<const BoundDeclaration> Binder::bindDeclaration(const Declaration* declaration)
    {
        if(auto variable_declaration = dynamic_cast<const VariableDeclaration*>(declaration))
            return Types::unique_cast<const BoundDeclaration>(bindVariableDeclaration(variable_declaration));

        else if(auto function_declaration = dynamic_cast<const FunctionDeclaration*>(declaration))
            return Types::unique_cast<const BoundDeclaration>(bindFunctionDeclaration(function_declaration));

        else if(auto external_declaration = dynamic_cast<const ExternalDeclaration*>(declaration))
            return Types::unique_cast<const BoundDeclaration>(bindExternalDeclaration(external_declaration));

        else if(auto structure_declaration = dynamic_cast<const StructureDeclaration*>(declaration))
            return Types::unique_cast<const BoundDeclaration>(bindStructureDeclaration(structure_declaration));
        
        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized declaration");
    }

    std::unique_ptr<const BoundExpression> Binder::bindExpression(const Expression* expression)
    {
        if(auto literal_expression = dynamic_cast<const LiteralExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindLiteralExpression(literal_expression));
        
        else if(auto unary_expression = dynamic_cast<const UnaryExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindUnaryExpression(unary_expression));
        
        else if(auto binary_expression = dynamic_cast<const BinaryExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindBinaryExpression(binary_expression));
        
        else if(auto identifier_expression = dynamic_cast<const IdentifierExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindIdentifierExpression(identifier_expression));

        else if(auto type_expression = dynamic_cast<const TypeExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindTypeExpression(type_expression));
        
        else if(auto block_expression = dynamic_cast<const BlockExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindBlockExpression(block_expression));

        else if(auto parenthesis_expression = dynamic_cast<const ParenthesisExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindExpression(parenthesis_expression->getExpression()));

        else if(auto if_else_expression = dynamic_cast<const IfExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindIfExpression(if_else_expression));

        else if(auto while_expression = dynamic_cast<const WhileExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindWhileExpression(while_expression));

        else if(auto for_expression = dynamic_cast<const ForExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindForExpression(for_expression));
            
        else if(auto function_call_expression = dynamic_cast<const CallExpression*>(expression))
        {
            if(function_call_expression->isExternal())
                return Types::unique_cast<const BoundExpression>(bindExternalCallExpression(function_call_expression));

            else return Types::unique_cast<const BoundExpression>(bindFunctionCallExpression(function_call_expression));
        }

        else if(auto conversion_expression = dynamic_cast<const ConversionExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindConversionExpression(conversion_expression));

        else if(auto array_initializer_expression = dynamic_cast<const ArrayInitializerExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindArrayInitializerExpression(array_initializer_expression));

        else if(auto index_expression = dynamic_cast<const IndexExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindIndexExpression(index_expression));

        else if(auto access_exprsesion = dynamic_cast<const AccessExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindAccessExpression(access_exprsesion));

        else if(auto structure_initializer_expression = dynamic_cast<const StructureInitializerExpression*>(expression))
            return Types::unique_cast<const BoundExpression>(bindStructureInitializerExpression(structure_initializer_expression));

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized expression");
    }

    void Binder::reportInvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::type left_type, Types::type right_type,
        const Token::Info& info)
    {
        Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("$::$ Undefined binary operator '$' for operands `$` and `$`.",
                info.file, info.line, BoundBinaryOperator::kindToString(operator_kind), left_type, right_type)});
    }

    void Binder::reportInvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::type operand_type, const Token::Info& info)
    {
        Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("$::$ Undefined unary operator '$' for operand of type `$`.",
                info.file, info.line, BoundUnaryOperator::kindToString(operator_kind), operand_type)});
    }

    const std::unique_ptr<const BoundDeclarationStatement> Binder::bindDeclarationStatement(const DeclarationStatement* statement)
    {
        auto declaration = bindDeclaration(statement->getDeclaration());
        return std::make_unique<const BoundDeclarationStatement>(std::move(declaration));
    }

    const std::unique_ptr<const BoundExpressionStatement> Binder::bindExpressionStatement(const ExpressionStatement* statement)
    {
        auto expression = bindExpression(statement->getExpression());
        return std::make_unique<const BoundExpressionStatement>(std::move(expression));
    }

    const std::unique_ptr<const BoundBlockExpression> Binder::bindBlockExpression(const BlockExpression* statement)
    {
        std::vector<std::unique_ptr<const BoundStatement>> statements;

        m_boundDeclarations.beginScope();

        for(std::size_t i{0ul}; i < statement->getStatements().size(); ++i)
        {
            m_blockIndex = i;
            statements.push_back(bindStatement(statement->getStatements()[i].get()));
        }
        
        m_blockIndex = -1ul;
        m_boundDeclarations.endScope();

        return std::make_unique<const BoundBlockExpression>(std::move(statements));
    }

    const std::unique_ptr<const BoundLabelStatement> Binder::bindLabelStatement(const LabelStatement* statement)
    {
        auto name = statement->getIdentifier()->getValue();

        if(m_blockIndex == -1ul)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot define label '$' outside of scope statement.", 
                   statement->getInfoString(), statement->getIdentifier()->getValue())
            });
            return std::make_unique<const BoundLabelStatement>(name, bindStatement(statement->getNext()), m_blockIndex);
        }
        
        bool is_loop = false;
        if(auto expression = dynamic_cast<const ExpressionStatement*>(statement->getNext())) 
            is_loop = dynamic_cast<const WhileExpression*>(expression->getExpression()) 
                || dynamic_cast<const ForExpression*>(expression->getExpression());

        if(!m_boundDeclarations.pushLabel(name, m_blockIndex, m_boundDeclarations.getScope(), is_loop))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot redefine label '$'", statement->getInfoString(), name)
            });

        auto next = bindStatement(statement->getNext());
        return std::make_unique<const BoundLabelStatement>(name, std::move(next), m_blockIndex);
    }
    
    const std::unique_ptr<const BoundJumpStatement> Binder::bindJumpStatement(const JumpStatement* statement)
    {
        const auto& name = statement->getIdentifier()->getValue();
        const auto find = m_boundDeclarations.findLabel(name);

        if(find == m_boundDeclarations.labelEnd())
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot jump to undefined label '$'.", name)
            });
            return std::make_unique<const BoundJumpStatement>(-1ul, m_boundDeclarations.getScope());
        }
        
        return std::make_unique<const BoundJumpStatement>(find->second.blockIndex, find->second.scope);
    }

    const std::unique_ptr<const BoundReturnStatement> Binder::bindReturnStatement(const ReturnStatement* statement)
    {
        auto expression = bindExpression(statement->getExpression());
        
        if(!m_inFunction)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Return statement used outside of function scope.",
                    statement->getInfoString())
            });

        else if(!expression->getType().isAssignableTo(m_currentFunctionType))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Return statement doesn't match the function's return type (`$` -> `$`).",
                    statement->getInfoString(), expression->getType(), m_currentFunctionType)
            });

        return std::make_unique<const BoundReturnStatement>(std::move(expression));
    }

    const std::unique_ptr<const BoundContinueStatement> Binder::bindContinueStatement(const ContinueStatement* statement)
    {
        if(!m_inLoop)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot use continue statement outside of a loop.", statement->getInfoString())
            });

        if(!statement->getLabel())
            return std::make_unique<const BoundContinueStatement>(m_boundDeclarations.getScope());
        
        auto name = statement->getLabel()->getValue();
        auto find = m_boundDeclarations.findLabel(name);

        if(find == m_boundDeclarations.labelEnd())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Undefined label in continue statement.", statement->getInfoString())
            });
        else if(!find->second.isLoop)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot continue with label '$', as it does not name a loop.",
                    statement->getInfoString(), name)
            });

        return std::make_unique<const BoundContinueStatement>(m_boundDeclarations.getScope());
    }
    
    const std::unique_ptr<const BoundBreakStatement> Binder::bindBreakStatement(const BreakStatement* statement)
    {
        if(!m_inLoop)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot use break statement outside of a loop.", statement->getInfoString())
            });
            
        if(!statement->getLabel())
            return std::make_unique<const BoundBreakStatement>(m_boundDeclarations.getScope());
        
        auto name = statement->getLabel()->getValue();
        auto find = m_boundDeclarations.findLabel(name);

        if(find == m_boundDeclarations.labelEnd())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Undefined label in break statement.", statement->getInfoString())
            });
        else if(!find->second.isLoop)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot break with label '$', as it does not name a loop.",
                    statement->getInfoString(), name)
            });

        return std::make_unique<const BoundBreakStatement>(find->second.scope);
    }

    const std::unique_ptr<const BoundVariableDeclaration> Binder::bindVariableDeclaration(const VariableDeclaration* declaration, bool is_argument)
    {
        auto type = bindTypeExpression(declaration->getType())->getActualType(); 

        auto name = declaration->getIdentifier()->getIdentifierToken().value.value();
        auto default_value = declaration->getDefaultValue()? std::make_optional(bindExpression(declaration->getDefaultValue()->getExpression())):
            std::nullopt;

        auto variable = std::make_unique<const BoundVariableDeclaration>(type, name, std::move(default_value));
        
        if(variable->getDefaultValue() && !variable->getDefaultValue().value()->getType().isAssignableTo(type))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("$ Cannot assign expression of type `$` to variable of type `$`", 
                    declaration->getInfoString(), variable->getDefaultValue().value()->getType(), type)});

        else if(!is_argument && !type.isMutable && !default_value)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("$ Cannot declare immutable variable '$' without default value.",
                    declaration->getInfoString(), name)
            });

        else if(!m_boundDeclarations.push(variable->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("$ Cannot redeclare symbol '$' as variable of type `$`.", 
                    declaration->getInfoString(), name, variable->getDefaultValue().value()->getType())});

        return std::move(variable);
    }

    const std::unique_ptr<const BoundFunctionDeclaration> Binder::bindFunctionDeclaration(const FunctionDeclaration* declaration)
    {
        auto return_type = bindTypeExpression(declaration->getReturnType())->getActualType();
        auto name = declaration->getIdentifier()->getValue();

        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;

        m_boundDeclarations.beginScope();

        bool has_default_value{false}, has_error{false};

        for(std::size_t i = 0; i < declaration->getArguments().size(); ++i)
        {
            const auto& argument = declaration->getArguments()[i];
            auto bound_argument = bindVariableDeclaration(argument.variable.get(), true);

            if(bound_argument->getDefaultValue().has_value())
                has_default_value = true;
            else if(has_default_value)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Function argument with default value cannot be followed by non-default-value argument.",
                        declaration->getInfoString())
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

        m_inFunction = true;
        m_currentFunctionType = return_type;
        auto body = bindStatement(declaration->getBody());
        m_currentFunctionType = Types::voidType;
        m_inFunction = false;

        auto function = std::make_unique<const BoundFunctionDeclaration>(return_type, name, std::move(arguments), std::move(body));
        
        if(!function->getBody()->getType().isAssignableTo(function->getReturnType()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Function '$' declared with return type `$`, but it evaluates to type `$`.",
                    declaration->getInfoString(), name, function->getReturnType(), function->getBody()->getType())});
        
        m_boundDeclarations.endScope();

        if(!has_error && !m_boundDeclarations.push(function->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Redefinition of symbol '$' as function declaration.",
                    declaration->getInfoString(), name)});

        return std::move(function);
    }

    const std::unique_ptr<const BoundExternalDeclaration> Binder::bindExternalDeclaration(const ExternalDeclaration* declaration)
    {
        auto name = declaration->getIdentifier()->getValue();
        auto actual_type = bindTypeExpression(declaration->getActualType());
        
        std::vector<std::unique_ptr<const BoundTypeExpression>> arguments;
        for(const auto& argument : declaration->getArguments())
        {
            auto type = bindTypeExpression(argument.get());
            arguments.push_back(std::move(type));
        }
        
        auto external_function = std::make_unique<const BoundExternalDeclaration>(name, std::move(actual_type), std::move(arguments));

        if(!m_boundDeclarations.push(external_function->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot redeclare symbol '$' as external function declaration.", declaration->getInfoString(), name)
            });

        return std::move(external_function);
    }

    const std::unique_ptr<const BoundStructureDeclaration> Binder::bindStructureDeclaration(const StructureDeclaration* declaration)
    {
        const auto name = declaration->getIdentifier()->getValue(); 

        std::vector<std::unique_ptr<const BoundVariableDeclaration>> fields;
        fields.reserve(declaration->getFields().size());

        m_boundDeclarations.beginScope();

        for(const auto& field: declaration->getFields())
        {
            auto new_field = bindVariableDeclaration(field.get(), true);
            fields.push_back(std::move(new_field));
        }

        m_boundDeclarations.endScope();

        auto structure = std::make_unique<const BoundStructureDeclaration>(name, std::move(fields));
        
        if(!m_boundDeclarations.push(structure->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Redefinition of symbol '$' as structure declaration.",
                    declaration->getInfoString(), name)
            });

        return std::move(structure);
    }
 
    const std::unique_ptr<const BoundIdentifierExpression> Binder::bindIdentifierExpression(const IdentifierExpression* expression)
    {
        auto value = expression->getValue();
        auto find = m_boundDeclarations.find(value);
        
        if(find == m_boundDeclarations.end())
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("$ Undeclared identifier '$'.", expression->getInfoString(), value)});

            return std::make_unique<const BoundIdentifierExpression>(value, Types::invalidType);
        }
        else if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(find->get()))
            return std::make_unique<const BoundIdentifierExpression>(value, variable->getActualType());

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
            .message = linc::Logger::format("$ Cannot reference identifier '$', as it is not a variable.", expression->getInfoString(), value)});

        return std::make_unique<const BoundIdentifierExpression>(value, Types::invalidType);
    }

    const std::unique_ptr<const BoundTypeExpression> Binder::bindTypeExpression(const TypeExpression* expression)
    {
        const auto kind = Types::kindFromUserString(expression->getTypeIdentifier().value.value());
        auto specifiers = bindArraySpecifiers(expression->getArraySpecifiers());

        if(kind != Types::Kind::invalid)
            return std::make_unique<const BoundTypeExpression>(kind, expression->getMutabilityKeyword().has_value(), std::move(specifiers));

        else
        {
            auto name = *expression->getTypeIdentifier().value;
            auto find = m_boundDeclarations.find(name);
            
            if(find == m_boundDeclarations.end())
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Undeclared identifier '$' does not result to a type.", expression->getInfoString(), name)
                });
                return std::make_unique<const BoundTypeExpression>(Types::Kind::invalid, expression->getMutabilityKeyword().has_value(),
                    std::move(specifiers));
            }
            else if(auto structure_declaration = dynamic_cast<const BoundStructureDeclaration*>(find->get()))
            {
                Types::type::Structure types;
                types.reserve(structure_declaration->getFields().size());

                for(const auto& field: structure_declaration->getFields())
                    types.push_back(std::pair(field->getName(), field->getActualType().clone()));

                return std::make_unique<const BoundTypeExpression>(std::move(types), expression->getMutabilityKeyword().has_value(), std::move(specifiers));
            }
            
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Identifier '$' does not refer to a type.", expression->getInfoString(), name)
            });

            return std::make_unique<const BoundTypeExpression>(Types::Kind::invalid, expression->getMutabilityKeyword().has_value(), std::move(specifiers));
        }
    }

    const std::unique_ptr<const BoundIfExpression> Binder::bindIfExpression(const IfExpression* expression)
    {
        auto test_expression = bindExpression(expression->getTestExpression());
        auto body_if_statement = bindStatement(expression->getIfBodyStatement());
        auto _body_else_statement = expression->getElseBodyStatement();

        if(_body_else_statement)
        {
            auto body_else_statement = bindStatement(_body_else_statement.value());
            auto type = body_if_statement->getType() == body_else_statement->getType()? body_if_statement->getType(): Types::voidType;
            return std::make_unique<const BoundIfExpression>(std::move(test_expression), std::move(body_if_statement),
                std::move(body_else_statement), type);
        }
        else return std::make_unique<const BoundIfExpression>(std::move(test_expression), std::move(body_if_statement), Types::voidType);
    }

    const std::unique_ptr<const BoundWhileExpression> Binder::bindWhileExpression(const WhileExpression* expression)
    {
        m_boundDeclarations.beginScope();
        auto test_expression = bindExpression(expression->getTestExpression());

        m_inLoop = true;
        auto body_statement = bindStatement(expression->getWhileBodyStatement());
        m_inLoop = false;
        
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

                m_boundDeclarations.endScope();
                return std::make_unique<const BoundWhileExpression>(else_type, std::move(test_expression), std::move(body_statement), 
                    std::move(body_finally_statement), std::move(body_else_statement));
            }
            else
            {
                m_boundDeclarations.endScope();
                return std::make_unique<const BoundWhileExpression>(finally_type, std::move(test_expression), std::move(body_statement), 
                    std::move(body_finally_statement));
            }
        }
        else if(_body_else_statement)
        {
            auto body_else_statement = bindStatement(_body_else_statement.value());
            auto else_type = type.isCompatible(body_else_statement->getType())? type: Types::voidType;

            m_boundDeclarations.endScope();
            return std::make_unique<const BoundWhileExpression>(else_type, std::move(test_expression), std::move(body_statement), 
                std::nullopt, std::move(body_else_statement));
        }

        else
        {
            m_boundDeclarations.endScope();
            return std::make_unique<const BoundWhileExpression>(Types::voidType, std::move(test_expression), std::move(body_statement));
        };
    }

    const std::unique_ptr<const BoundForExpression> Binder::bindForExpression(const ForExpression* expression)
    {
        m_boundDeclarations.beginScope();
        const auto& specifier = expression->getSpecifier();

        if(auto variable_specifier = std::get_if<const ForExpression::VariableForSpecifier>(&specifier))
        {
            auto variable_declaration = bindVariableDeclaration(variable_specifier->variableDeclaration.get());

            auto _expression = bindExpression(variable_specifier->expression.get());
            auto statement = bindStatement(variable_specifier->statement.get());\
            
            m_inLoop = true;
            auto body = bindStatement(expression->getBody());
            m_inLoop = false;

            m_boundDeclarations.endScope(); 
            return std::make_unique<const BoundForExpression>(std::move(variable_declaration), std::move(_expression), std::move(statement), std::move(body));
        }
        else if(auto range_specifier = std::get_if<const ForExpression::RangeForSpecifier>(&specifier))
        {
            auto array_identifier = bindIdentifierExpression(range_specifier->arrayIdentifier.get());
            auto range_type = Types::type(array_identifier->getType().kind == Types::type::Kind::Primitive
                && array_identifier->getType().primitive == Types::Kind::string?
                Types::fromKind(Types::Kind::_char): *array_identifier->getType().array.base_type);
            auto variable_declaration = std::make_unique<const BoundVariableDeclaration>(range_type, range_specifier->valueIdentifier->getValue(), std::nullopt);

            if(!m_boundDeclarations.push(variable_declaration->clone()))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Cannot redefine symbol '$' as ranged for loop identifier.", 
                        expression->getInfoString(), range_specifier->valueIdentifier->getValue())
                });

            auto value_identifier = bindIdentifierExpression(range_specifier->valueIdentifier.get());
            
            m_inLoop = true;
            auto body = bindStatement(expression->getBody());
            m_inLoop = false;

            m_boundDeclarations.endScope();
            return std::make_unique<const BoundForExpression>(std::move(value_identifier), std::move(array_identifier), std::move(body));
            
        }
        else throw LINC_EXCEPTION_OUT_OF_BOUNDS(std::variant);
    }

    const std::unique_ptr<const BoundFunctionCallExpression> Binder::bindFunctionCallExpression(const CallExpression* expression)
    {
        auto name = expression->getIdentifier().value.value();
        std::vector<BoundFunctionCallExpression::Argument> arguments;
        
        auto find = m_boundDeclarations.find(name);

        if(find == m_boundDeclarations.end())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot call undeclared function '$'.", expression->getInfoString(), name)});

        else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(find->get()); !function)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot call identifier '$', as it is not a function.", expression->getInfoString(), name)});

        else 
        {
            if(function->getArguments().size() < expression->getArguments().size()
            || function->getArguments().size() - function->getDefaultArgumentCount() > expression->getArguments().size())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Tried to call function '$' with $ arguments, when it takes $ (with $ default arguments).", 
                        expression->getInfoString(), name, expression->getArguments().size(), function->getArguments().size(),
                        function->getDefaultArgumentCount())});

            using _size = std::vector<std::unique_ptr<const Expression>>::size_type;

            for(_size i = 0ul; i < std::min(expression->getArguments().size(), function->getArguments().size()); i++)
            {
                const auto& argument = expression->getArguments()[i];
                const auto& declared_argument = function->getArguments()[i];
                auto bound_argument = bindExpression(argument.expression.get());

                if(!declared_argument->getActualType().isCompatible(bound_argument->getType()))
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                        .message = Logger::format("$ Invalid argument type in function '$'.", expression->getInfoString(), name)});
                    
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                        .message = Logger::format("$ Argument $ called with type `$`, expected `$`.",
                            expression->getInfoString(), i, bound_argument->getType(), declared_argument->getActualType())});
                }

                else arguments.push_back(BoundFunctionCallExpression::Argument{
                    .name = declared_argument->getName(),
                    .value = std::move(bound_argument)
                });
            }

            for(_size i = expression->getArguments().size(); i < function->getArguments().size()
                && i >= function->getArguments().size() - function->getDefaultArgumentCount(); ++i)
            {
                const auto& declared_argument = function->getArguments()[i];
                
                arguments.push_back(BoundFunctionCallExpression::Argument{
                    .name = declared_argument->getName(),
                    .value = declared_argument->getDefaultValue().value()->clone()
                });
            }

            return std::make_unique<const BoundFunctionCallExpression>(function->getReturnType(), name, std::move(arguments), 
                std::move(function->getBody()->clone()));
        }

        return std::make_unique<const BoundFunctionCallExpression>(Types::invalidType, name, std::move(arguments),
            std::make_unique<const BoundExpressionStatement>(std::make_unique<const BoundLiteralExpression>(PrimitiveValue::invalidValue, Types::invalidType)));
    }

    const std::unique_ptr<const BoundExternalCallExpression> Binder::bindExternalCallExpression(const CallExpression* expression)
    {
        auto name = expression->getIdentifier().value.value();
        std::vector<std::unique_ptr<const BoundExpression>> arguments;

        auto find = m_boundDeclarations.find(name);

        if(Internals::isInternal(name))
        {
            std::vector<std::unique_ptr<const BoundExpression>> arguments;
            arguments.reserve(expression->getArguments().size());
            
            auto internal = Internals::find(name);
            if(expression->getArguments().size() != internal->arguments.size())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Incorrect number of arguments given to external function '$' (given $ but it requires $).",
                        expression->getInfoString(), name, expression->getArguments().size(), internal->arguments.size())
                });

            else for(std::size_t index{0ul}; index < expression->getArguments().size(); ++index)
            {
                const auto& declared_argument_type = internal->arguments[index];
                auto argument = bindExpression(expression->getArguments()[index].expression.get());

                if(!argument->getType().isCompatible(declared_argument_type))
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                        .message = Logger::format("$ Incorrect type for external function argument $ (expected `$`, got `$`).",
                            expression->getInfoString(), index + 1ul, PrimitiveValue(declared_argument_type), PrimitiveValue(argument->getType()))
                    });

                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                        .message = Logger::format("In external function $.",
                            PrimitiveValue(name))
                    });
                    break;
                }

                arguments.push_back(std::move(argument));
            }

            return std::make_unique<const BoundExternalCallExpression>(internal->returnType, internal->name, std::move(arguments));
        }
        else if(find == m_boundDeclarations.end())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot call undeclared external function '$'.", expression->getInfoString(), name)});

        else if(auto external = dynamic_cast<const BoundExternalDeclaration*>(find->get()); !external)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot call identifier '$', as it is not an external function.", expression->getInfoString(), name)});

        return std::make_unique<const BoundExternalCallExpression>(Types::invalidType, name, std::move(arguments));
    }

    const std::unique_ptr<const BoundLiteralExpression> Binder::bindLiteralExpression(const LiteralExpression* expression)
    {
        switch(expression->getType())
        {
        case Token::Type::U8Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u8>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::u8>(0)),
                Types::fromKind(Types::Kind::u8));
        case Token::Type::U16Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u16>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::u16>(0)),
                Types::fromKind(Types::Kind::u16));
        case Token::Type::U32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u32>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::u32>(0)),
                Types::fromKind(Types::Kind::u32));
        case Token::Type::U64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::u64>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::u64>(0)),
                Types::fromKind(Types::Kind::u64));
        case Token::Type::I8Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i8>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::i8>(0)),
                Types::fromKind(Types::Kind::i8));
        case Token::Type::I16Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i16>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::i16>(0)),
                Types::fromKind(Types::Kind::i16));
        case Token::Type::I32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i32>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::i32>(0)),
                Types::fromKind(Types::Kind::i32));
        case Token::Type::I64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::i64>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::i64>(0)),
                Types::fromKind(Types::Kind::i64));
        case Token::Type::F32Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::f32>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::f32>(0)),
                Types::fromKind(Types::Kind::f32));
        case Token::Type::F64Literal:
            return std::make_unique<const BoundLiteralExpression>(parseString<Types::f64>(expression->getValue(), expression->getNumberBase().value()).value_or(static_cast<Types::f64>(0)),
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
                .message = Logger::format("$ Unrecognized literal expression.", expression->getInfoString())});
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
        case Token::Type::Colon: return BoundUnaryOperator::Kind::Typeof;
        case Token::Type::OperatorBitwiseNot: return BoundUnaryOperator::Kind::BitwiseNot;
        default: return BoundUnaryOperator::Kind::Invalid;
        }
    }

    const std::unique_ptr<const BoundUnaryExpression> Binder::bindUnaryExpression(const UnaryExpression* expression)
    {
        auto operand = bindExpression(expression->getOperand());
        auto kind = bindUnaryOperatorKind(expression->getOperatorToken().type);
        auto _operator = std::make_unique<const BoundUnaryOperator>(kind, operand->getType());

        if(_operator->getReturnType().primitive == Types::Kind::invalid)
            reportInvalidUnaryOperator(kind, operand->getType(), expression->getInfo().info);

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
        case Token::Type::OperatorPercent: return BoundBinaryOperator::Kind::Modulo;
        case Token::Type::OperatorLogicalAnd: return BoundBinaryOperator::Kind::LogicalAnd;
        case Token::Type::OperatorLogicalOr: return BoundBinaryOperator::Kind::LogicalOr;
        case Token::Type::OperatorEquals: return BoundBinaryOperator::Kind::Equals;
        case Token::Type::OperatorNotEquals: return BoundBinaryOperator::Kind::NotEquals;
        case Token::Type::OperatorGreater: return BoundBinaryOperator::Kind::Greater;
        case Token::Type::OperatorLess: return BoundBinaryOperator::Kind::Less;
        case Token::Type::OperatorGreaterEqual: return BoundBinaryOperator::Kind::GreaterEqual;
        case Token::Type::OperatorLessEqual: return BoundBinaryOperator::Kind::LessEqual;
        case Token::Type::OperatorAssignment: return BoundBinaryOperator::Kind::Assignment;
        case Token::Type::OperatorAsignmentAddition: return BoundBinaryOperator::Kind::AdditionAssignment;
        case Token::Type::OperatorAsignmentSubstraction: return BoundBinaryOperator::Kind::SubtractionAssignment;
        case Token::Type::OperatorAssignmentMultiplication: return BoundBinaryOperator::Kind::MultiplicationAssignment;
        case Token::Type::OperatorAssignmentDivision: return BoundBinaryOperator::Kind::DivisionAssignment;
        case Token::Type::OperatorAssignmentModulo: return BoundBinaryOperator::Kind::ModuloAssignment;
        case Token::Type::OperatorBitwiseAnd: return BoundBinaryOperator::Kind::BitwiseAnd;
        case Token::Type::OperatorBitwiseOr: return BoundBinaryOperator::Kind::BitwiseOr;
        case Token::Type::OperatorBitwiseXor: return BoundBinaryOperator::Kind::BitwiseXor;
        case Token::Type::OperatorBitwiseShiftLeft: return BoundBinaryOperator::Kind::BitwiseShiftLeft;
        case Token::Type::OperatorBitwiseShiftRight: return BoundBinaryOperator::Kind::BitwiseShiftRight;
        default: return BoundBinaryOperator::Kind::Invalid;
        }
    }

    const std::unique_ptr<const BoundBinaryExpression> Binder::bindBinaryExpression(const BinaryExpression* expression)
    {
        auto left = bindExpression(expression->getLeft());
        auto right = bindExpression(expression->getRight());
        auto kind = bindBinaryOperatorKind(expression->getOperatorToken().type);
        auto _operator = std::make_unique<const BoundBinaryOperator>(kind, left->getType(), right->getType());

        if(_operator->getReturnType().primitive == Types::Kind::invalid)
            reportInvalidBinaryOperator(kind, left->getType(), right->getType(), expression->getInfo().info);

        return std::make_unique<const BoundBinaryExpression>(std::move(_operator), std::move(left), std::move(right));
    }

    const std::unique_ptr<const BoundConversionExpression> Binder::bindConversionExpression(const ConversionExpression* expression)
    {
        auto inner_expression = bindExpression(expression->getExpression());
        auto target_type = bindTypeExpression(expression->getType())->getActualType();

        auto conversion = std::make_unique<const BoundConversion>(inner_expression->getType(), target_type);

        if(conversion->getReturnType().primitive == Types::Kind::invalid)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Invalid conversion expression to type `$` (from `$`).",
                    expression->getInfoString(), target_type, inner_expression->getType())
            });

        return std::make_unique<const BoundConversionExpression>(std::move(inner_expression), std::move(conversion));
    }

    const std::unique_ptr<const BoundArrayInitializerExpression> Binder::bindArrayInitializerExpression(const ArrayInitializerExpression* expression)
    {
        std::vector<std::unique_ptr<const BoundExpression>> values{};
        auto type = Types::voidType;

        for(const auto& value: expression->getValues())
        {
            auto bound_value = bindExpression(value.value.get());

            if(type == Types::voidType);
            else if(type != bound_value->getType())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ All members of an array initializer must be of the same type.",
                        expression->getInfoString())
                });
            type = bound_value->getType();
            
            values.push_back(std::move(bound_value));
        }

        return std::make_unique<const BoundArrayInitializerExpression>(std::move(values), Types::type(Types::type::Array{
            .base_type = type.clone(),
            .count = values.size() 
        }));
    }

    const std::unique_ptr<const BoundIndexExpression> Binder::bindIndexExpression(const IndexExpression* expression)
    {
        auto array = bindExpression(expression->getArray());
        auto index = bindExpression(expression->getIndex());

        if(index->getType().kind != Types::type::Kind::Primitive || index->getType().primitive != Types::Kind::u64)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Incompatible type in array indexing: expected `$`, found `$`.",
                    expression->getInfoString(), Types::fromKind(Types::Kind::u64), index->getType())
            });
            return std::make_unique<const BoundIndexExpression>(std::move(array), std::move(index), Types::invalidType);
        }
        else if(auto literal = dynamic_cast<const BoundLiteralExpression*>(index.get()))
        {
            auto value = literal->getValue().getIfU64().value_or(0ul);

            if(array->getType().kind == Types::type::Kind::Array && array->getType().array.count && value >= *array->getType().array.count)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Warning, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Accessing out of bounds array index $ (array has $ elements).",
                        expression->getInfoString(), value, PrimitiveValue(*array->getType().array.count))
                });
        }

        if(array->getType().kind == Types::type::Kind::Primitive && array->getType().primitive != Types::Kind::string)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot index non-array non-string operand.",
                    expression->getInfoString())
            });
            
            return std::make_unique<const BoundIndexExpression>(std::move(array), std::move(index), Types::invalidType);
    }

        auto type = array->getType().kind == Types::type::Kind::Primitive && array->getType().primitive == Types::Kind::string?
            Types::type(Types::Kind::_char, array->getType().isMutable): *array->getType().array.base_type; 
        
        return std::make_unique<const BoundIndexExpression>(std::move(array), std::move(index), type);
    }

    const std::unique_ptr<const BoundAccessExpression> Binder::bindAccessExpression(const AccessExpression* expression)
    {
        auto name = expression->getIdentifier()->getValue();
        auto find = m_boundDeclarations.find(name);
        auto base = bindExpression(expression->getBase());

        if(base->getType().kind != Types::type::Kind::Structure)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot access field '$' of non-structure typed operand.", expression->getInfoString(), name)
            });
            
            return std::make_unique<const BoundAccessExpression>(std::move(base), -1ul, Types::invalidType);
        }
        auto structure = base->getType().structure;

        for(std::size_t index{0ul}; index < structure.size(); ++index)
            if(structure[index].first == name)
                return std::make_unique<const BoundAccessExpression>(std::move(base), index, *structure[index].second);

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
            .message = Logger::format("$ Accessing undefined field '$' in structure.", expression->getInfoString(), name)
        });

        return std::make_unique<const BoundAccessExpression>(std::move(base), -1ul, Types::invalidType);
    }

    BoundTypeExpression::BoundArraySpecifiers Binder::bindArraySpecifiers(const std::vector<TypeExpression::ArraySpecifier>& specifiers)
    {
        BoundTypeExpression::BoundArraySpecifiers result;

        for(const auto& specifier: specifiers)
        {
            std::unique_ptr<const BoundLiteralExpression> count_literal = specifier.count? bindLiteralExpression(specifier.count.get()): nullptr;
            result.push_back(count_literal? std::make_optional(count_literal->getValue().getU64()): std::nullopt);
        }

        return result;
    }

    const std::unique_ptr<const BoundStructureInitializerExpression> Binder::bindStructureInitializerExpression(const StructureInitializerExpression* expression)
    {
        auto name = expression->getIdentifier()->getValue();
        auto find = m_boundDeclarations.find(name);

        if(find == m_boundDeclarations.end())
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot use undefined identifier $ as a structure initializer, as it does not name a type.",
                    expression->getInfoString(), PrimitiveValue(name))
            });
            return std::make_unique<const BoundStructureInitializerExpression>(name, std::vector<std::unique_ptr<const BoundExpression>>{},
                Types::invalidType);
        }
        else if(!dynamic_cast<const BoundStructureDeclaration*>(find->get()))
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Identifier $ in structure initializer is not a structure type.",
                    expression->getInfoString(), PrimitiveValue(name))
            });
            return std::make_unique<const BoundStructureInitializerExpression>(name, std::vector<std::unique_ptr<const BoundExpression>>{},
                Types::invalidType);
        }

        std::vector<std::unique_ptr<const BoundExpression>> fields{};
        auto structure = static_cast<const BoundStructureDeclaration*>(find->get());
        fields.reserve(structure->getFields().size());

        if(structure->getFields().size() != expression->getArguments().size())
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Structure initializer's types do not align with structure $ (expected size: $, got $).",
                    expression->getInfoString(), PrimitiveValue(name))
            });
            return std::make_unique<const BoundStructureInitializerExpression>(name, std::vector<std::unique_ptr<const BoundExpression>>{},
                Types::invalidType);
        }

        for(std::size_t i{0ul}; i < structure->getFields().size(); ++i)
        {
            auto bound_value = bindExpression(expression->getArguments()[i].value.get());
            auto field_name = expression->getArguments()[i].identifier->getValue();
            
            if(field_name != structure->getFields()[i]->getName())
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Structure's and initializer's corresponding field names do not match (expected $, got $).",
                        expression->getInfoString(), PrimitiveValue(structure->getFields()[i]->getName()), PrimitiveValue(field_name))
                });
                return std::make_unique<const BoundStructureInitializerExpression>(name, std::vector<std::unique_ptr<const BoundExpression>>{},
                    Types::invalidType);
            }

            fields.push_back(std::move(bound_value));
        }
        return std::make_unique<const BoundStructureInitializerExpression>(name, std::move(fields), structure->getActualType());
    }
}
