#pragma once
#include <linc/Tree.hpp>
#include <linc/BoundTree.hpp>
#include <stack>

namespace linc
{
    class BoundVariableDeclarations final
    {
    public:
        BoundVariableDeclarations()
        {
            beginScope();
        }
        
        struct Variable final
        {
            Types::Type type;
            std::string name;
            bool isMutable;
        };

        struct Scope final
        {
            std::vector<Variable> variables; 
        };

        std::vector<Variable>::const_iterator end() const
        {
            return m_scopes.top().variables.end();
        }

        std::vector<Variable>::const_iterator find(const std::string& name) const
        {
            for(auto it = m_scopes.top().variables.begin(); it != m_scopes.top().variables.end(); ++it)
                if(it->name == name)
                    return it;
                
            return m_scopes.top().variables.end();
        }

        bool push(Types::Type type, const std::string& name, bool is_mutable)
        {
            auto find = this->find(name);

            if(find == m_scopes.top().variables.end())
            {
                m_scopes.top().variables.push_back(Variable{.type = type, .name = name, .isMutable = is_mutable});
                return true;
            }
            else return false;
        }

        void beginScope()
        {
            auto variables = m_scopes.empty()? std::vector<Variable>{}: m_scopes.top().variables;
            m_scopes.push(Scope{variables});
        }

        void endScope()
        {
            m_scopes.pop();
        }

        const std::vector<Variable>& getVariableList() const
        {
            return m_scopes.top().variables;
        } 
    private:
        std::stack<Scope> m_scopes;
    };

    class Binder final
    {
    public:
        const std::vector<BoundVariableDeclarations::Variable>& getVariableList()
        {
            return m_boundVariableDeclarations.getVariableList();
        }

        std::unique_ptr<const BoundStatement> bindStatement(const Statement* statement)
        {
            if(auto* expression_statement = dynamic_cast<const ExpressionStatement*>(statement))
                return bindExpressionStatement(expression_statement);
            
            else if(auto* scope_statement = dynamic_cast<const ScopeStatement*>(statement))
                return bindScopeStatement(scope_statement);

            else if(auto* variable_declaration_statement = dynamic_cast<const VariableDeclarationStatement*>(statement))
                return bindVariableDeclarationStatement(variable_declaration_statement);
            
            else if(auto* put_character_statement = dynamic_cast<const PutCharacterStatement*>(statement))
                return bindPutCharacterStatement(put_character_statement);

            else if(auto* put_string_statement = dynamic_cast<const PutStringStatement*>(statement))
                return bindPutStringStatement(put_string_statement);

            throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized statement");
        }

        const std::unique_ptr<const BoundExpression> bindExpression(const Expression* expression)
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

            throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized expression");
        }
    private:
        void reportinvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::Type left_type, Types::Type right_type)
        {
            Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Undefined binary operator '$' for operands '$' and '$'.",
                BoundBinaryOperator::kindToString(operator_kind), Types::toString(left_type), Types::toString(right_type))
            });
        }

        void reportinvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::Type operand_type)
        {
            Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Undefined unary operator '$' for operand of type '$'.",
                BoundUnaryOperator::kindToString(operator_kind), Types::toString(operand_type))
            });
        }

        std::unique_ptr<const BoundStatement> bindExpressionStatement(const ExpressionStatement* statement)
        {
            auto expression = bindExpression(statement->getExpression());
            return std::make_unique<const BoundExpressionStatement>(std::move(expression));
        }

        std::unique_ptr<const BoundStatement> bindScopeStatement(const ScopeStatement* statement)
        {
            std::vector<std::unique_ptr<const BoundStatement>> statements;

            m_boundVariableDeclarations.beginScope();

            for(const auto& statement : statement->getStatements())
                statements.push_back(bindStatement(statement.get()));
            
            m_boundVariableDeclarations.endScope();

            return std::make_unique<const BoundScopeStatement>(std::move(statements));
        }

        std::unique_ptr<const BoundStatement> bindVariableDeclarationStatement(const VariableDeclarationStatement* statement)
        {
            Types::Type type = Types::fromUserString(statement->getTypeNameIdentifierToken().value.value());
            auto name = statement->getIdentifierExpression()->getValue();
            auto value_expression = bindExpression(statement->getExpression());
            auto variable = statement->getVariableKeywordToken().has_value();

            if(value_expression->getType() != type)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Cannot assign expression of type '$' to variable of type '$'", 
                        Types::toString(value_expression->getType()), Types::toString(type))
                });

            else if(!m_boundVariableDeclarations.push(type, name, variable))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Cannot redeclare variable '$' with type '$'.", 
                        name, Types::toString(value_expression->getType()))
                });

            return std::make_unique<const BoundVariableDeclarationStatement>(type, name, variable, std::move(value_expression));
        }

        std::unique_ptr<const BoundStatement> bindPutCharacterStatement(const PutCharacterStatement* statement)
        {
            auto expression = bindExpression(statement->getExpression());
            return std::make_unique<const BoundPutCharacterStatement>(std::move(expression));
        }

        std::unique_ptr<const BoundStatement> bindPutStringStatement(const PutStringStatement* statement)
        {
            auto expression = bindExpression(statement->getExpression());
            return std::make_unique<const BoundPutStringStatement>(std::move(expression));
        }
 
        std::unique_ptr<const BoundExpression> bindIdentifierExpression(const IdentifierExpression* expression)
        {
            auto value = expression->getValue();
            auto find = m_boundVariableDeclarations.find(value);
            
            if(find == m_boundVariableDeclarations.end())
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Undeclared identifier '$'.", value)
                });

                return std::make_unique<const BoundIdentifierExpression>(value, Types::Type::invalid);
            }
            
            return std::make_unique<const BoundIdentifierExpression>(value, find->type);
        }

        const std::unique_ptr<const BoundExpression> bindIfElseExpression(const IfElseExpression* expression)
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

        const std::unique_ptr<const BoundExpression> bindWhileExpression(const WhileExpression* expression)
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

        const std::unique_ptr<const BoundExpression> bindVariableAssignmentExpression(const VariableAssignmentExpression* expression)
        {
            auto identifier = *expression->getIdentifierToken().value;
            auto value = bindExpression(expression->getValue());
            auto find = m_boundVariableDeclarations.find(identifier);

            if(find == m_boundVariableDeclarations.end())
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Variable '$' has not been declared and cannot be reassigned.", identifier)
                });
            
            else if(!find->isMutable)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("Cannot reassign immutable variable '$' of type '$'", identifier, Types::toString(find->type))
                });

            return std::make_unique<const BoundVariableAssignmentExpression>(identifier, std::move(value));
        }

        template <typename T>
        std::optional<T> parseString(const std::string& str)
        {
            char* p;
            if constexpr (std::is_same_v<T, Types::u8>)
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
            else return std::nullopt;
        }

        const std::unique_ptr<const BoundExpression> bindLiteralExpression(const LiteralExpression* expression)
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
                return std::make_unique<const BoundLiteralExpression>(Types::parseCharacter(expression->getValue()));
            case Token::Type::StringLiteral:
                return std::make_unique<const BoundLiteralExpression>(expression->getValue());
            case Token::Type::KeywordTrue:
            case Token::Type::KeywordFalse:
                return std::make_unique<const BoundLiteralExpression>(Types::parseBoolean(expression->getValue()));
            default:
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = "Unrecognized literal expression."
                });
            return nullptr;
            }
        }

        BoundUnaryOperator::Kind bindUnaryOperatorKind(Token::Type token_type)
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

        const std::unique_ptr<const BoundExpression> bindUnaryExpression(const UnaryExpression* expression)
        {
            auto operand = bindExpression(expression->getOperand());
            auto kind = bindUnaryOperatorKind(expression->getOperatorToken().type);
            auto _operator = std::make_unique<BoundUnaryOperator>(kind, operand->getType());

            if(_operator->getReturnType() == Types::Type::invalid)
                reportinvalidUnaryOperator(kind, operand->getType());

            return std::make_unique<const BoundUnaryExpression>(std::move(_operator), std::move(operand));
        }

        BoundBinaryOperator::Kind bindBinaryOperatorKind(Token::Type token_type)
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

        std::unique_ptr<const BoundExpression> bindBinaryExpression(const BinaryExpression* expression)
        {
            auto left = bindExpression(expression->getLeft());
            auto right = bindExpression(expression->getRight());
            auto kind = bindBinaryOperatorKind(expression->getOperatorToken().type);
            auto _operator = std::make_unique<const BoundBinaryOperator>(kind, left->getType(), right->getType());

            if(_operator->getReturnType() == Types::Type::invalid)
                reportinvalidBinaryOperator(kind, left->getType(), right->getType());

            return std::make_unique<const BoundBinaryExpression>(std::move(_operator), std::move(left), std::move(right));
        }

        BoundVariableDeclarations m_boundVariableDeclarations;
    };
}