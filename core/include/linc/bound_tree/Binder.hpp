#pragma once
#include <linc/Tree.hpp>
#include <linc/BoundTree.hpp>

namespace linc
{
    class BoundVariableDeclarations
    {
    public:
        bool insert(const std::string& name, Types::Type type)
        {
            auto find = m_typeMap.find(name);

            if(find == m_typeMap.end())
            {
                m_typeMap[name] = type;
                return true;
            }
            else return false;
        }

        Types::Type get(const std::string& name)
        {
            auto find = m_typeMap.find(name);

            if(find != m_typeMap.end())
                return find->second;
            
            else return Types::Type::invalid;
        }

        std::vector<std::pair<std::string, Types::Type>> getVariableList()
        {
            std::vector<std::pair<std::string, Types::Type>> ret;

            for(const auto& var: m_typeMap)
                ret.push_back(var);
            
            return std::move(ret);
        }
    private:
        std::unordered_map<std::string, Types::Type> m_typeMap;
    };

    class Binder final
    {
    public:
        std::vector<std::pair<std::string, Types::Type>> getVariableList()
        {
            return std::move(m_boundVariableDeclarations.getVariableList());
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

            for(const auto& statement : statement->getStatements())
                statements.push_back(bindStatement(statement.get()));
            
            return std::make_unique<const BoundScopeStatement>(std::move(statements));
        }

        std::unique_ptr<const BoundStatement> bindVariableDeclarationStatement(const VariableDeclarationStatement* statement)
        {
            Types::Type type = Types::fromUserString(statement->getTypeNameIdentifierToken().value.value());
            auto name = statement->getIdentifierExpression()->getValue();
            auto value_expression = bindExpression(statement->getExpression());
            auto variable = statement->getVariableKeywordToken();

            if(value_expression->getType() != type)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Cannot assign expression of type '$' to variable of type '$'", 
                        Types::toString(value_expression->getType()), Types::toString(type))
                });
            }

            else if(!m_boundVariableDeclarations.insert(name, type))
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Cannot redeclare variable '$' with type '$'.", 
                        name, Types::toString(value_expression->getType()))
                });

            return std::make_unique<const BoundVariableDeclarationStatement>(type, name, variable.has_value(), std::move(value_expression));
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
            Types::Type type = m_boundVariableDeclarations.get(value);
            
            if(type == Types::Type::invalid)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Undeclared identifier '$'.", value)
                });
            
            return std::make_unique<const BoundIdentifierExpression>(value, type);
        }

        const std::unique_ptr<const BoundExpression> bindIfElseExpression(const IfElseExpression* expression)
        {
            auto test_expression = bindExpression(expression->getTestExpression());
            auto body_if_statement = bindStatement(expression->getBodyIfStatement());
            auto _body_else_statement = expression->getBodyElseStatement();

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
            auto body_statement = bindStatement(expression->getBodyStatement());
            auto type = body_statement->getType();

            return std::make_unique<const BoundWhileExpression>(std::move(test_expression), std::move(body_statement), type);
        }

        const std::unique_ptr<const BoundExpression> bindLiteralExpression(const LiteralExpression* expression)
        {
            switch(expression->getType())
            {
            case Token::Type::U8Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::u8)std::stoul(expression->getValue()));
            case Token::Type::U16Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::u16)std::stoul(expression->getValue()));
            case Token::Type::U32Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::u32)std::stoul(expression->getValue()));
            case Token::Type::U64Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::u64)std::stoull(expression->getValue()));
            case Token::Type::I8Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::i8)std::stoull(expression->getValue()));
            case Token::Type::I16Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::i16)std::stoull(expression->getValue()));
            case Token::Type::I32Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::i32)std::stoi(expression->getValue()));
            case Token::Type::I64Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::i64)std::stoll(expression->getValue()));
            case Token::Type::F32Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::f32)std::stof(expression->getValue()));
            case Token::Type::F64Literal:
                return std::make_unique<const BoundLiteralExpression>((Types::f64)std::stod(expression->getValue()));
            case Token::Type::CharacterLiteral:
                return std::make_unique<const BoundLiteralExpression>(Types::parseCharacter(expression->getValue()));
            case Token::Type::StringLiteral:
                return std::make_unique<const BoundLiteralExpression>(expression->getValue().c_str());
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