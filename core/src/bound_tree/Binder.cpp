#include <linc/bound_tree/Binder.hpp>
#include <linc/BoundTree.hpp>
#include <linc/Tree.hpp>
#include <linc/system/Internals.hpp>
#include <linc/Lexer.hpp>

namespace linc
{
    template <typename T>
    static std::optional<T> parseString(const std::string& str, std::optional<Token::NumberBase> base)
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
                if(Lexer::isDigit(str[i], base.value()))
                {
                    auto displacement = std::isdigit(str[i])? 0: 'A' - '9' - 1;
                    result = result * static_cast<T>(Token::baseToInt(base.value())) + (str[i] - '0' - displacement);
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

    BoundSymbols::BoundSymbols()
    {
        beginScope();
    }

    void BoundSymbols::clear()
    {
        m_scopes = ScopeStack<std::unique_ptr<const BoundDeclaration>>{};
        m_labels = StringStack{};
    }

    std::unique_ptr<const BoundDeclaration> BoundSymbols::find(const std::string& name, bool top_only) const
    {
        if(auto find = top_only? m_scopes.findTop(name): m_scopes.find(name); find)
        {
            if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(find->get()))
            {
                if(variable->getName() == name)
                    return variable->clone();
            }
            else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(find->get()))
            {
                if(function->getName() == name)
                    return function->clone();
            }
            else if(auto external_function = dynamic_cast<const BoundExternalDeclaration*>(find->get()))
            {
                if(external_function->getName() == name)
                    return external_function->clone();
            }
            else if(auto structure_declaration = dynamic_cast<const BoundStructureDeclaration*>(find->get()))
            {
                if(structure_declaration->getName() == name)
                    return structure_declaration->clone();
            }
            else if(auto enumeration_declaration = dynamic_cast<const BoundEnumerationDeclaration*>(find->get()))
            {
                if(enumeration_declaration->getName() == name)
                    return enumeration_declaration->clone();
            }
            else throw LINC_EXCEPTION_ILLEGAL_STATE(find);
        }   
        return nullptr;
    }

    bool BoundSymbols::push(std::unique_ptr<const BoundDeclaration> symbol)
    {
        std::unique_ptr<const BoundDeclaration> find{nullptr};
        std::string name;
        
        if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(symbol.get()))
        {
            name = variable->getName();
            find = this->find(name, true);
        }
        else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(symbol.get()))
        {
            name = function->getName();
            find = this->find(name);
        }
        else if(auto external_function = dynamic_cast<const BoundExternalDeclaration*>(symbol.get()))
        {
            name = external_function->getName();
            find = this->find(name);
        }
        else if(auto structure = dynamic_cast<const BoundStructureDeclaration*>(symbol.get()))
        {
            name = structure->getName();
            find = this->find(name);
        }
        else if(auto enumeration = dynamic_cast<const BoundEnumerationDeclaration*>(symbol.get()))
        {
            name = enumeration->getName();
            find = this->find(name);
        }
        else throw LINC_EXCEPTION_ILLEGAL_VALUE(symbol);       

        if(!find)
        {
            m_scopes.append(name, std::move(symbol));
            return true;
        }
        else return false;
    }

    BoundProgram Binder::bindProgram(const Program* program)
    {
        BoundProgram bound_program;

        for(const auto& declaration: program->declarations)
            bound_program.declarations.push_back(bindDeclaration(declaration.get()));

        return bound_program;
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

        else throw LINC_EXCEPTION_INVALID_INPUT("Encountered unrecognized node while binding");
    }

    std::unique_ptr<const BoundStatement> Binder::bindStatement(const Statement* statement)
    {
        if(!statement)
            return nullptr;

        else if(auto declaration_statement = dynamic_cast<const DeclarationStatement*>(statement))
            return Types::uniqueCast<const BoundStatement>(bindDeclarationStatement(declaration_statement));

        else if(auto expression_statement = dynamic_cast<const ExpressionStatement*>(statement))
            return Types::uniqueCast<const BoundStatement>(bindExpressionStatement(expression_statement));

        else if(auto return_statement = dynamic_cast<const ReturnStatement*>(statement))
            return Types::uniqueCast<const BoundStatement>(bindReturnStatement(return_statement));

        else if(auto continue_statement = dynamic_cast<const ContinueStatement*>(statement))
            return Types::uniqueCast<const BoundStatement>(bindContinueStatement(continue_statement));

        else if(auto break_statement = dynamic_cast<const BreakStatement*>(statement))
            return Types::uniqueCast<const BoundStatement>(bindBreakStatement(break_statement));

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized statement");
    }

    std::unique_ptr<const BoundDeclaration> Binder::bindDeclaration(const Declaration* declaration)
    {
        if(!declaration)
            return nullptr;

        if(auto variable_declaration = dynamic_cast<const VariableDeclaration*>(declaration))
            return Types::uniqueCast<const BoundDeclaration>(bindVariableDeclaration(variable_declaration));

        else if(auto direct_variable_declaration = dynamic_cast<const DirectVariableDeclaration*>(declaration))
            return Types::uniqueCast<const BoundDeclaration>(bindDirectVariableDeclaration(direct_variable_declaration));

        else if(auto function_declaration = dynamic_cast<const FunctionDeclaration*>(declaration))
            return Types::uniqueCast<const BoundDeclaration>(bindFunctionDeclaration(function_declaration));

        else if(auto external_declaration = dynamic_cast<const ExternalDeclaration*>(declaration))
            return Types::uniqueCast<const BoundDeclaration>(bindExternalDeclaration(external_declaration));

        else if(auto structure_declaration = dynamic_cast<const StructureDeclaration*>(declaration))
            return Types::uniqueCast<const BoundDeclaration>(bindStructureDeclaration(structure_declaration));
        
        else if(auto enumeration_declaration = dynamic_cast<const EnumerationDeclaration*>(declaration))
            return Types::uniqueCast<const BoundDeclaration>(bindEnumerationDeclaration(enumeration_declaration));

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized declaration");
    }

    std::unique_ptr<const BoundExpression> Binder::bindExpression(const Expression* expression)
    {
        if(!expression)
            return nullptr;
        else if(auto literal_expression = dynamic_cast<const LiteralExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindLiteralExpression(literal_expression));
        
        else if(auto unary_expression = dynamic_cast<const UnaryExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindUnaryExpression(unary_expression));
        
        else if(auto binary_expression = dynamic_cast<const BinaryExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindBinaryExpression(binary_expression));
        
        else if(auto identifier_expression = dynamic_cast<const IdentifierExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindIdentifierExpression(identifier_expression));

        else if(auto type_expression = dynamic_cast<const TypeExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindTypeExpression(type_expression));
        
        else if(auto block_expression = dynamic_cast<const BlockExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindBlockExpression(block_expression));

        else if(auto parenthesis_expression = dynamic_cast<const ParenthesisExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindExpression(parenthesis_expression->getExpression()));

        else if(auto if_else_expression = dynamic_cast<const IfExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindIfExpression(if_else_expression));

        else if(auto while_expression = dynamic_cast<const WhileExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindWhileExpression(while_expression));

        else if(auto match_expression = dynamic_cast<const MatchExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindMatchExpression(match_expression));

        else if(auto for_expression = dynamic_cast<const ForExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindForExpression(for_expression));
            
        else if(auto function_call_expression = dynamic_cast<const CallExpression*>(expression))
        {
            if(function_call_expression->isExternal())
                return Types::uniqueCast<const BoundExpression>(bindExternalCallExpression(function_call_expression));

            else return Types::uniqueCast<const BoundExpression>(bindFunctionCallExpression(function_call_expression));
        }

        else if(auto conversion_expression = dynamic_cast<const ConversionExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindConversionExpression(conversion_expression));

        else if(auto array_initializer_expression = dynamic_cast<const ArrayInitializerExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindArrayInitializerExpression(array_initializer_expression));

        else if(auto index_expression = dynamic_cast<const IndexExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindIndexExpression(index_expression));

        else if(auto access_exprsesion = dynamic_cast<const AccessExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindAccessExpression(access_exprsesion));
        
        else if(auto namespace_access_expression = dynamic_cast<const EnumeratorExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindEnumeratorExpression(namespace_access_expression));

        else if(auto structure_initializer_expression = dynamic_cast<const StructureInitializerExpression*>(expression))
            return Types::uniqueCast<const BoundExpression>(bindStructureInitializerExpression(structure_initializer_expression));

        throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized expression");
    }

    void Binder::reportInvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::type left_type, Types::type right_type,
        const Token::Info& info)
    {
        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
            .span = TextSpan::fromTokenInfo(info),
            .message = Logger::format("$ Invalid binary operator '$' for operands `$` and `$`.",
            info, BoundBinaryOperator::kindToString(operator_kind), left_type, right_type)});

        if(left_type.kind == Types::type::Kind::Primitive && right_type.kind == Types::type::Kind::Primitive
        && Types::isNumeric(left_type.primitive) && Types::isNumeric(right_type.primitive) && left_type.primitive != right_type.primitive){
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Note that Linc does not allow for implicit type conversions between numeric types. "
                    "Did you forget to do a type-cast?", info, BoundBinaryOperator::kindToString(operator_kind), left_type, right_type)});
        }
    }

    void Binder::reportInvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::type operand_type, const Token::Info& info)
    {
        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
            .span = TextSpan::fromTokenInfo(info),
            .message = Logger::format("$ Invalid unary operator '$' for operand of type `$`.",
            info, BoundUnaryOperator::kindToString(operator_kind), operand_type)});
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

    const std::unique_ptr<const BoundBlockExpression> Binder::bindBlockExpression(const BlockExpression* expression)
    {
        std::vector<std::unique_ptr<const BoundStatement>> statements;

        m_boundDeclarations.beginScope();

        for(std::size_t i{0ul}; i < expression->getStatements().size(); ++i)
            statements.push_back(bindStatement(expression->getStatements()[i].get()));
        auto tail = expression->getTail()? bindExpression(expression->getTail()): nullptr;
        m_boundDeclarations.endScope();

        return std::make_unique<const BoundBlockExpression>(std::move(statements), std::move(tail));
    }

    const std::unique_ptr<const BoundReturnStatement> Binder::bindReturnStatement(const ReturnStatement* statement)
    {
        auto expression = statement->getExpression()? bindExpression(statement->getExpression()): nullptr;
        
        if(!m_inFunction)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Return statement used outside of function scope.",
                    statement->getInfoString())
            });

        else if(m_currentFunctionType == Types::invalidType)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot use return statements in functions with implicit return types.",
                    statement->getInfoString())
            });

        else if(expression && !expression->getType().isAssignableTo(m_currentFunctionType))
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
            return std::make_unique<const BoundContinueStatement>(std::string{});
        
        auto name = statement->getLabel()->getValue();
        auto find = m_boundDeclarations.findLabel(name);

        if(find.empty())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Undefined label in continue statement.", statement->getInfoString())
            });

        return std::make_unique<const BoundContinueStatement>(find);
    }
    
    const std::unique_ptr<const BoundBreakStatement> Binder::bindBreakStatement(const BreakStatement* statement)
    {
        if(!m_inLoop)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot use break statement outside of a loop.", statement->getInfoString())
            });
            
        if(!statement->getLabel())
            return std::make_unique<const BoundBreakStatement>(std::string{});
        
        auto name = statement->getLabel()->getValue();
        auto find = m_boundDeclarations.findLabel(name);

        if(find.empty())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Undefined label in break statement.", statement->getInfoString())
            });

        return std::make_unique<const BoundBreakStatement>(find);
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
                .message = Logger::format("$ Cannot assign expression of type `$` to variable of type `$`", 
                    declaration->getInfoString(), variable->getDefaultValue().value()->getType(), type)});

        else if(!is_argument && !type.isMutable && !default_value)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot declare immutable variable '$' without default value.",
                    declaration->getInfoString(), name)
            });

        else if(!is_argument && !variable->getDefaultValue() && type.kind == Types::type::Kind::Array && !type.array.count.has_value())
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot automatically infer the size of an array-typed variable with no default value.",
                    declaration->getInfoString(), name)
            });

        else if(!m_boundDeclarations.push(variable->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot redeclare symbol '$' as variable of type `$`.", 
                    declaration->getInfoString(), name, variable->getActualType())});

        return variable;
    }

    const std::unique_ptr<const BoundVariableDeclaration> Binder::bindDirectVariableDeclaration(const DirectVariableDeclaration* declaration)
    {
        auto value = bindExpression(declaration->getValue());
        auto type = value->getType();
        type.isMutable = declaration->getMutabilitySpecifier().has_value(); 
        auto name = declaration->getIdentifier()->getValue();

        auto variable = std::make_unique<const BoundVariableDeclaration>(type, name, std::move(value));

        if(!m_boundDeclarations.push(variable->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot redeclare symbol '$' as variable of type `$`.", 
                    declaration->getInfoString(), name, variable->getActualType())});

        return variable;
    }

    const std::unique_ptr<const BoundFunctionDeclaration> Binder::bindFunctionDeclaration(const FunctionDeclaration* declaration)
    {
        auto return_type = declaration->getReturnType()? bindTypeExpression(declaration->getReturnType())->getActualType(): Types::invalidType;
        auto name = declaration->getIdentifier()->getValue();

        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;
        std::vector<std::unique_ptr<const Types::type>> argument_types;
        arguments.reserve(declaration->getArguments()->getList().size());
        argument_types.reserve(declaration->getArguments()->getList().size());

        m_boundDeclarations.beginScope();
        bool has_default_value{false}, has_error{false};

        for(std::size_t i{0ul}; i < declaration->getArguments()->getList().size(); ++i)
        {
            const auto& argument = declaration->getArguments()->getList()[i];
            auto bound_argument = bindVariableDeclaration(argument.node.get(), true);

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

        for(const auto& argument: arguments)
            argument_types.push_back(argument->getActualType().clone());

        ++m_inFunction;
        m_currentFunctionType = return_type;
        auto body = bindExpression(declaration->getBody());
        if(return_type == Types::invalidType) { return_type = body->getType(); return_type.isMutable = false; }
        m_currentFunctionType = Types::voidType;
        --m_inFunction;

        if(return_type.isMutable)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Warning, .stage = Reporting::Stage::ABT,
                .span = TextSpan::fromTokenInfo(declaration->getTypeSpecifier().info),
                .message = Logger::format("$ Mutable modifier is ineffective on function return types.", declaration->getInfoString())});

        auto function_type = Types::type{Types::type::Function{return_type.clone(), std::move(argument_types)}, return_type.isMutable};
        auto function = std::make_unique<const BoundFunctionDeclaration>(function_type, name, std::move(arguments), std::move(body));

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

        return function;
    }

    const std::unique_ptr<const BoundExternalDeclaration> Binder::bindExternalDeclaration(const ExternalDeclaration* declaration)
    {
        auto name = declaration->getIdentifier()->getValue();
        auto actual_type = bindTypeExpression(declaration->getActualType());
        
        std::vector<std::unique_ptr<const BoundTypeExpression>> arguments;
        for(const auto& argument: declaration->getArguments()->getList())
        {
            auto type = bindTypeExpression(argument.node.get());
            arguments.push_back(std::move(type));
        }
        
        auto external_function = std::make_unique<const BoundExternalDeclaration>(name, std::move(actual_type), std::move(arguments));

        if(!m_boundDeclarations.push(external_function->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot redeclare symbol '$' as external function declaration.", declaration->getInfoString(), name)
            });

        return external_function;
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

        return structure;
    }

    const std::unique_ptr<const BoundEnumerationDeclaration> Binder::bindEnumerationDeclaration(const EnumerationDeclaration* declaration)
    {
        const auto name = declaration->getIdentifier()->getValue(); 

        auto enumerators = bindNodeListClause(declaration->getEnumerators(), &Binder::bindEnumeratorClause);
        auto enumeration = std::make_unique<const BoundEnumerationDeclaration>(name, std::move(enumerators));
    
        if(!m_boundDeclarations.push(enumeration->clone()))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Redefinition of symbol '$' as enumeration declaration.",
                    declaration->getInfoString(), name)
            });

        return enumeration;
    }
 
    const std::unique_ptr<const BoundIdentifierExpression> Binder::bindIdentifierExpression(const IdentifierExpression* expression)
    {
        auto value = expression->getValue();
        auto find = m_boundDeclarations.find(value);
        
        if(!find)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .span = TextSpan::fromTokenInfo(expression->getTokenInfo()),
                .message = Logger::format("$ Undeclared identifier '$'.", expression->getInfoString(), value)});

            return std::make_unique<const BoundIdentifierExpression>(value, Types::invalidType);
        }
        else if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(find.get()))
            return std::make_unique<const BoundIdentifierExpression>(value, variable->getActualType());

        else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(find.get()))
            return std::make_unique<const BoundIdentifierExpression>(value, function->getFunctionType());

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
            .message = Logger::format("$ Cannot reference identifier '$', as it is not a variable.", expression->getInfoString(), value)});

        return std::make_unique<const BoundIdentifierExpression>(value, Types::invalidType);
    }

    const std::unique_ptr<const BoundEnumeratorExpression> Binder::bindEnumeratorExpression(const EnumeratorExpression* expression)
    {
        std::string match_identifier{};
        if(!m_matchIdentifiers.empty())
        {
            match_identifier = m_matchIdentifiers.top();
            m_matchIdentifiers.pop();
        }

        auto name = expression->getEnumerationIdentifier()->getValue();
        auto find = m_boundDeclarations.find(name);
        if(!find)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Undeclared identifier '$' does not result to a namespace.", expression->getInfoString(), name)
            }), std::make_unique<const BoundEnumeratorExpression>(std::string{}, -1ul, nullptr, Types::invalidType));
        else if(auto enumeration = dynamic_cast<const BoundEnumerationDeclaration*>(find.get()); !enumeration)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot namespace-access identifier '$', which is not an enumeration.",
                    expression->getInfoString(), name)
            }), std::make_unique<const BoundEnumeratorExpression>(std::string{}, -1ul, nullptr, Types::invalidType));
        
        auto enumeration = static_cast<const BoundEnumerationDeclaration*>(find.get());
        auto enumerator_name = expression->getIdentifier()->getValue();
        auto type = enumeration->getActualType();

        Types::u64 enumerator_index{-1ul};
        for(std::vector<std::string>::size_type i{0ul}; i < enumeration->getEnumerators()->getList().size(); ++i)
            if(enumerator_name == enumeration->getEnumerators()->getList()[i]->getName())
                enumerator_index = i;

        if(expression->getValue() && !match_identifier.empty() && enumerator_index != -1ul)
        {
            const auto& enumerator = enumeration->getEnumerators()->getList().at(enumerator_index);
            if(!m_boundDeclarations.push(std::make_unique<const BoundVariableDeclaration>(
                enumerator->getActualType(), match_identifier, std::nullopt)))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .span = TextSpan::fromTokenInfo(expression->getValue()->getTokenInfo()),
                    .message = Logger::format("$ Cannot redefine identifier '$' as match parameter in enumerator expression",
                        expression->getValue()->getTokenInfo(), match_identifier)
                });  
        }
        auto value = expression->getValue()? bindExpression(expression->getValue()): nullptr;

        if(enumerator_index == -1ul)
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .span = TextSpan::fromTokenInfo(expression->getIdentifier()->getIdentifierToken().info),
                .message = Logger::format("$ Invalid enumerator '$' in enumeration '$'",
                    expression->getTokenInfo(), enumerator_name, name)
            }), std::make_unique<const BoundEnumeratorExpression>(std::string{}, -1ul, nullptr, Types::invalidType));
        else if(value && !value->getType().isCompatible(enumeration->getEnumerators()->getList()[enumerator_index]->getActualType()))
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .span = TextSpan::fromTokenInfo(expression->getIdentifier()->getIdentifierToken().info),
                .message = Logger::format("$ Enumerator type incompatible with declared enumerator '$' in enumeration '$'",
                    expression->getTokenInfo(), enumerator_name, name)
            }), std::make_unique<const BoundEnumeratorExpression>(std::string{}, -1ul, nullptr, Types::invalidType));
        else if(!value && !enumeration->getEnumerators()->getList()[enumerator_index]->getActualType().isCompatible(Types::voidType))
            return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .span = TextSpan::fromTokenInfo(expression->getIdentifier()->getIdentifierToken().info),
                .message = Logger::format("$ Cannot omit value for non-void enumerator '$' in enumeration '$'.",
                    expression->getTokenInfo(), enumerator_name, name)
            }), std::make_unique<const BoundEnumeratorExpression>(std::string{}, -1ul, nullptr, Types::invalidType));

        return std::make_unique<const BoundEnumeratorExpression>(name, enumerator_index, std::move(value), type);
    }

    const std::unique_ptr<const BoundTypeExpression> Binder::bindTypeExpression(const TypeExpression* expression)
    {
        const auto kind = expression->getIfIdentifierRoot()? Types::kindFromUserString(expression->getIfIdentifierRoot()->getValue()): Types::Kind::invalid;
        auto specifiers = bindArraySpecifiers(expression->getArraySpecifiers());

        if(kind != Types::Kind::invalid)
            return std::make_unique<const BoundTypeExpression>(kind, expression->getMutabilityKeyword().has_value(), std::move(specifiers));

        else if(auto function = expression->getIfFunctionRoot())
        {
            auto return_type = bindTypeExpression(function->returnType.get())->getActualType().clone();
            std::vector<std::unique_ptr<const Types::type>> argument_types;
            argument_types.reserve(function->argumentTypes->getList().size());

            for(const auto& type: function->argumentTypes->getList())
                argument_types.push_back(bindTypeExpression(type.node.get())->getActualType().clone());

            return std::make_unique<const BoundTypeExpression>(Types::type::Function{std::move(return_type), std::move(argument_types)}, expression->getMutabilityKeyword().has_value(),
                std::move(specifiers));
        }

        else
        {
            auto name = expression->getIfIdentifierRoot()->getValue();
            auto find = m_boundDeclarations.find(name);
            
            if(!find)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Undeclared identifier '$' does not result to a type.", expression->getInfoString(), name)
                });
                return std::make_unique<const BoundTypeExpression>(Types::Kind::invalid, expression->getMutabilityKeyword().has_value(),
                    std::move(specifiers));
            }
            else if(auto structure_declaration = dynamic_cast<const BoundStructureDeclaration*>(find.get()))
            {
                Types::type::Structure types;
                types.reserve(structure_declaration->getFields().size());

                for(const auto& field: structure_declaration->getFields())
                    types.push_back(std::pair(field->getName(), field->getActualType().clone()));

                return std::make_unique<const BoundTypeExpression>(std::move(types), expression->getMutabilityKeyword().has_value(), std::move(specifiers));
            }
            else if(auto enumeration_declaration = dynamic_cast<const BoundEnumerationDeclaration*>(find.get()))
            {
                Types::type::Enumeration types;
                types.reserve(enumeration_declaration->getEnumerators()->getList().size());
                
                for(const auto& enumerator: enumeration_declaration->getEnumerators()->getList())
                    types.push_back(std::pair(enumerator->getName(), enumerator->getActualType()));

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
        auto if_body = bindExpression(expression->getIfBody());
        auto else_body = expression->getElseBody()? bindExpression(expression->getElseBody()): nullptr;
        auto type = else_body && if_body->getType() == else_body->getType()? if_body->getType(): Types::voidType; 

        return std::make_unique<const BoundIfExpression>(type, std::move(test_expression), std::move(if_body), std::move(else_body));
    }

    const std::unique_ptr<const BoundWhileExpression> Binder::bindWhileExpression(const WhileExpression* expression)
    {
        auto test_expression = bindExpression(expression->getTestExpression());
        auto label = expression->getLabel()? expression->getLabel()->identifier->getValue(): std::string{};
        
        if(!label.empty()) m_boundDeclarations.pushLabel(label);
        ++m_inLoop;
        auto while_body = bindExpression(expression->getWhileBody());
        --m_inLoop;
        if(!label.empty()) m_boundDeclarations.popLabel();

        auto else_body = expression->hasElse()? bindExpression(expression->getElseBody()): nullptr;
        auto finally_body = expression->hasFinally()? bindExpression(expression->getFinallyBody()): nullptr;
        auto type = while_body->getType();


        if(else_body && finally_body)
        {
            if(!while_body->getType().isCompatible(finally_body->getType())
            || !finally_body->getType().isCompatible(else_body->getType())
            || !else_body->getType().isCompatible(while_body->getType()))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Incompatible typing in while-finally-else expression.", expression->getInfoString())
                });
        }
        else if(else_body)
        {
            if(!while_body->getType().isCompatible(else_body->getType()))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Incompatible typing in while-else expression.", expression->getInfoString())
                });
        }
        else if(while_body->getType() != Types::voidType || (finally_body && finally_body->getType() != Types::voidType))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ All typed while expressions must have an else clause.", expression->getInfoString())
            });

        return std::make_unique<const BoundWhileExpression>(std::move(label), type, std::move(test_expression), std::move(while_body), std::move(finally_body),
            std::move(else_body));
    }

    const std::unique_ptr<const BoundMatchExpression> Binder::bindMatchExpression(const MatchExpression* expression)
    {
        auto test_expression = bindExpression(expression->getTestExpression());
        auto clauses = bindNodeListClause<MatchClause>(expression->getClauses(), &Binder::bindMatchClause);
        auto type = clauses->getList().empty()? Types::voidType: clauses->getList()[0ul]->getExpression()->getType();

        [&]()
        {
            for(const auto& clause: clauses->getList())
            {
                if(!type.isCompatible(clause->getExpression()->getType()))
                    type = Types::voidType;

                for(const auto& value: clause->getValues()->getList())
                    if(!value->getType().isCompatible(test_expression->getType()))
                    {
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                            .span = TextSpan::fromTokenInfo(value->getInfo()),
                            .message = Logger::format("$ Expression case type incompatible with test expression in match expression.", value->getInfo())
                        });
                        return;
                    }
            }
        }();

        return std::make_unique<const BoundMatchExpression>(std::move(test_expression), std::move(clauses), type);
    }

    const std::unique_ptr<const BoundForExpression> Binder::bindForExpression(const ForExpression* expression)
    {
        m_boundDeclarations.beginScope();
        const auto& specifier = expression->getSpecifier();
        auto label = expression->getLabel()? expression->getLabel()->identifier->getValue(): std::string{};

        if(auto variable_specifier = std::get_if<const ForExpression::VariableForSpecifier>(&specifier))
        {
            auto variable_declaration = bindVariableDeclaration(variable_specifier->variableDeclaration.get());

            auto _expression = bindExpression(variable_specifier->expression.get());
            auto statement = bindStatement(variable_specifier->statement.get());\
            
            if(!label.empty()) m_boundDeclarations.pushLabel(label);
            ++m_inLoop;
            auto body = bindExpression(expression->getBody());
            --m_inLoop;
            if(!label.empty()) m_boundDeclarations.popLabel();

            m_boundDeclarations.endScope(); 
            return std::make_unique<const BoundForExpression>(label, std::move(variable_declaration), std::move(_expression), std::move(statement), std::move(body));
        }
        else if(auto range_specifier = std::get_if<const ForExpression::RangeForSpecifier>(&specifier))
        {
            auto array_identifier = bindIdentifierExpression(range_specifier->arrayIdentifier.get());
            auto range_type = Types::type(array_identifier->getType().kind == Types::type::Kind::Primitive
                && array_identifier->getType().primitive == Types::Kind::string?
                Types::fromKind(Types::Kind::_char): *array_identifier->getType().array.baseType);
            auto variable_declaration = std::make_unique<const BoundVariableDeclaration>(range_type, range_specifier->valueIdentifier->getValue(), std::nullopt);

            if(!m_boundDeclarations.push(variable_declaration->clone()))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Cannot redefine symbol '$' as ranged for loop identifier.", 
                        expression->getInfoString(), range_specifier->valueIdentifier->getValue())
                });

            auto value_identifier = bindIdentifierExpression(range_specifier->valueIdentifier.get());
            
            if(!label.empty()) m_boundDeclarations.pushLabel(label);
            ++m_inLoop;
            auto body = bindExpression(expression->getBody());
            --m_inLoop;
            if(!label.empty()) m_boundDeclarations.popLabel();

            m_boundDeclarations.endScope();
            return std::make_unique<const BoundForExpression>(label, std::move(value_identifier), std::move(array_identifier), std::move(body));
        }
        else throw LINC_EXCEPTION_OUT_OF_BOUNDS(specifier);
    }

    const std::unique_ptr<const BoundFunctionCallExpression> Binder::bindFunctionCallExpression(const CallExpression* expression)
    {
        auto name = expression->getIdentifier().value.value();
        std::vector<BoundFunctionCallExpression::Argument> arguments;
        
        auto find = m_boundDeclarations.find(name);

        if(!find)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot call undeclared function '$'.", expression->getInfoString(), name)});

        else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(find.get()); !function)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot call identifier '$', as it is not a function.", expression->getInfoString(), name)});

        else 
        {
            const auto& list = expression->getArguments()->getList();
            if(function->getArguments().size() < list.size()
            || function->getArguments().size() - function->getDefaultArgumentCount() > list.size())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Tried to call function '$' with $ arguments, when it takes $ (with $ default arguments).", 
                        expression->getInfoString(), name, list.size(), function->getArguments().size(),
                        function->getDefaultArgumentCount())});

            using _size = std::vector<std::unique_ptr<const Expression>>::size_type;

            for(_size i{0ul}; i < std::min(list.size(), function->getArguments().size()); i++)
            {
                const auto& argument = list[i];
                const auto& declared_argument = function->getArguments()[i];
                auto bound_argument = bindExpression(argument.node.get());

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

            for(_size i{list.size()}; i < function->getArguments().size()
                && i >= function->getArguments().size() - function->getDefaultArgumentCount(); ++i)
            {
                const auto& declared_argument = function->getArguments()[i];
                
                arguments.push_back(BoundFunctionCallExpression::Argument{
                    .name = declared_argument->getName(),
                    .value = declared_argument->getDefaultValue().value()->clone()
                });
            }

            return std::make_unique<const BoundFunctionCallExpression>(function->getReturnType(), name, std::move(arguments));
        }

        return std::make_unique<const BoundFunctionCallExpression>(Types::invalidType, name, std::move(arguments));
    }

    const std::unique_ptr<const BoundExternalCallExpression> Binder::bindExternalCallExpression(const CallExpression* expression)
    {
        auto name = expression->getIdentifier().value.value();
        std::vector<std::unique_ptr<const BoundExpression>> arguments;

        auto find = m_boundDeclarations.find(name);

        if(Internals::isInternal(name))
        {
            const auto& list = expression->getArguments()->getList(); 
            std::vector<std::unique_ptr<const BoundExpression>> arguments;
            arguments.reserve(list.size());
            
            auto internal = Internals::find(name);
            if(list.size() != internal->arguments.size())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("$ Incorrect number of arguments given to external function '$' (given $ but it requires $).",
                        expression->getInfoString(), name, list.size(), internal->arguments.size())
                });

            else for(std::size_t index{0ul}; index < list.size(); ++index)
            {
                const auto& declared_argument_type = internal->arguments[index];
                auto argument = bindExpression(list[index].node.get());

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
        else if(!find)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot call undeclared external function '$'.", expression->getInfoString(), name)});

        else if(auto external = dynamic_cast<const BoundExternalDeclaration*>(find.get()); !external)
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

        for(const auto& value: expression->getValues()->getList())
        {
            auto bound_value = bindExpression(value.node.get());

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
            .baseType = type.clone(),
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
            Types::type(Types::Kind::_char, array->getType().isMutable): *array->getType().array.baseType; 
        
        return std::make_unique<const BoundIndexExpression>(std::move(array), std::move(index), type);
    }

    const std::unique_ptr<const BoundAccessExpression> Binder::bindAccessExpression(const AccessExpression* expression)
    {
        auto name = expression->getIdentifier()->getValue();
        auto base = bindExpression(expression->getBase());

        if(base->getType().kind != Types::type::Kind::Structure)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot access field '$' of non-structure typed operand.", expression->getInfoString(), name)
            });
            
            return std::make_unique<const BoundAccessExpression>(std::move(base), -1ul, Types::invalidType);
        }
        auto structure = Types::type{base->getType()}.structure;

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

        if(!find)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("$ Cannot use undefined identifier $ as a structure initializer, as it does not name a type.",
                    expression->getInfoString(), PrimitiveValue(name))
            });
            return std::make_unique<const BoundStructureInitializerExpression>(name, std::vector<std::unique_ptr<const BoundExpression>>{},
                Types::invalidType);
        }
        else if(!dynamic_cast<const BoundStructureDeclaration*>(find.get()))
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
        auto structure = static_cast<const BoundStructureDeclaration*>(find.get());
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

    const std::unique_ptr<const BoundMatchClause> Binder::bindMatchClause(const MatchClause* expression)
    {
        m_boundDeclarations.beginScope();
        
        for(const auto& value: expression->getValues()->getList())
        {
            auto enumerator = dynamic_cast<const EnumeratorExpression*>(value.node.get());
            if(!enumerator || !enumerator->getValue()) continue;
            auto identifier = dynamic_cast<const IdentifierExpression*>(enumerator->getValue());
            if(!identifier || m_boundDeclarations.find(identifier->getValue())) continue;
            m_matchIdentifiers.push(identifier->getValue());
        }

        auto values = bindNodeListClause<Expression>(expression->getValues(), &Binder::bindExpression);
        auto result_expression = bindExpression(expression->getExpression());
        m_boundDeclarations.endScope();
        return std::make_unique<const BoundMatchClause>(std::move(result_expression), std::move(values));
    }

    const std::unique_ptr<const BoundEnumeratorClause> Binder::bindEnumeratorClause(const EnumeratorClause* expression)
    {
        auto actual_type = bindTypeExpression(expression->getActualType())->getActualType();
        auto name = expression->getIdentifier()->getValue();
        return std::make_unique<const BoundEnumeratorClause>(actual_type, name, expression->getTokenInfo());
    }
}
