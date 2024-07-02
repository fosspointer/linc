#pragma once
#include <linc/BoundTree.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Optimizer final
    {
    public:
        Optimizer() = delete;
        static std::unique_ptr<const BoundExpression> optimizeExpression(const BoundExpression* expression)
        {
            if(auto if_expression = dynamic_cast<const BoundIfExpression*>(expression))
            {
                auto test_expression = optimizeExpression(if_expression->getTestExpression());

                if(auto literal = dynamic_cast<const BoundLiteralExpression*>(test_expression.get()))
                {
                    if(literal->getValue().getBool())
                    {
                        std::vector<std::unique_ptr<const BoundStatement>> statement;
                        statement.push_back(if_expression->getIfBodyStatement()->clone());
                        return std::make_unique<const BoundBlockExpression>(std::move(statement));
                    }
                    else if(auto else_body_statement = if_expression->getElseBodyStatement())
                    {
                        std::vector<std::unique_ptr<const BoundStatement>> statement;
                        statement.push_back(else_body_statement.value()->clone());
                        return std::make_unique<const BoundBlockExpression>(std::move(statement));
                    }
                    else return std::make_unique<const BoundBlockExpression>(std::vector<std::unique_ptr<const BoundStatement>>{});
                }
                auto if_body = optimizeStatement(if_expression->getIfBodyStatement());

                if(auto _else_body = if_expression->getElseBodyStatement())
                {
                    auto else_body = optimizeStatement(_else_body.value());
                    return std::make_unique<const BoundIfExpression>(std::move(test_expression), std::move(if_body), std::move(else_body), if_expression->getType());
                }

                else return std::make_unique<const BoundIfExpression>(std::move(test_expression), std::move(if_body), if_expression->getType());
            }
            else if(auto unary_expression = dynamic_cast<const BoundUnaryExpression*>(expression))
            {
                auto operand = optimizeExpression(unary_expression->getOperand());

                if(auto literal = dynamic_cast<const BoundLiteralExpression*>(operand.get()))
                    return std::make_unique<const BoundLiteralExpression>(
                        optimizeConstantUnaryExpression(unary_expression->getOperator()->getKind(), literal->getValue(), unary_expression->getType()),
                        unary_expression->getType());
                
                else return std::make_unique<const BoundUnaryExpression>(unary_expression->getOperator()->clone(), std::move(operand));
            }
            else if(auto binary_expression = dynamic_cast<const BoundBinaryExpression*>(expression))
            {
                if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::LogicalAnd)
                {
                    auto left = optimizeExpression(binary_expression->getLeft());
                    if(auto left_literal = dynamic_cast<const BoundLiteralExpression*>(left.get()))
                    {
                        if(!left_literal->getValue().getBool())
                            return std::make_unique<const BoundLiteralExpression>(false, binary_expression->getType());
                        else return optimizeExpression(binary_expression->getRight());
                    }
                    else return std::make_unique<const BoundBinaryExpression>(binary_expression->getOperator()->clone(),
                        std::move(left), binary_expression->getRight()->clone());
                }
                else if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::LogicalOr)
                {
                    auto left = optimizeExpression(binary_expression->getLeft());
                    if(auto left_literal = dynamic_cast<const BoundLiteralExpression*>(left.get()))
                    {
                        if(left_literal->getValue().getBool())
                            return std::make_unique<const BoundLiteralExpression>(true, binary_expression->getType());
                        else return optimizeExpression(binary_expression->getRight());
                    }
                    else return std::make_unique<const BoundBinaryExpression>(binary_expression->getOperator()->clone(),
                        std::move(left), binary_expression->getRight()->clone());
                }
  
                auto left = optimizeExpression(binary_expression->getLeft());
                auto right = optimizeExpression(binary_expression->getRight());

                if(auto left_literal = dynamic_cast<const BoundLiteralExpression*>(left.get()), right_literal = dynamic_cast<const BoundLiteralExpression*>(right.get()); left_literal && right_literal)
                    return std::make_unique<const BoundLiteralExpression>(
                        optimizeConstantBinaryExpression(binary_expression->getOperator()->getKind(), left_literal->getValue(), right_literal->getValue()),
                        binary_expression->getType());
                
                else return std::make_unique<const BoundBinaryExpression>(binary_expression->getOperator()->clone(), std::move(left), std::move(right));
            }

            return expression->clone();
        }

        static std::unique_ptr<const BoundStatement> optimizeStatement(const BoundStatement* statement)
        {
            if(auto expression = dynamic_cast<const BoundExpressionStatement*>(statement))
                return std::make_unique<const BoundExpressionStatement>(optimizeExpression(expression->getExpression()));
            
            else if(auto declaration = dynamic_cast<const BoundDeclarationStatement*>(statement))
                return std::make_unique<const BoundDeclarationStatement>(optimizeDeclaration(declaration->getDeclaration()));

            return statement->clone();
        }
        
        static std::unique_ptr<const BoundDeclaration> optimizeDeclaration(const BoundDeclaration* declaration)
        {
            if(auto variable_declaration = dynamic_cast<const BoundVariableDeclaration*>(declaration))
            {
                auto value = variable_declaration->getDefaultValue()? std::make_optional(optimizeExpression(variable_declaration->getDefaultValue().value()))
                    :std::nullopt;

                return std::make_unique<const BoundVariableDeclaration>(variable_declaration->getActualType(), variable_declaration->getName(), std::move(value));
            }
            return declaration->clone();
        }

        static std::unique_ptr<const BoundProgram> optimizeProgram(const BoundProgram* program)
        {
            std::vector<std::unique_ptr<const BoundDeclaration>> declarations;
            
            for(const auto& declaration: program->declarations)
                declarations.push_back(optimizeDeclaration(declaration.get()));

            return std::make_unique<const BoundProgram>(std::move(declarations));
        }
    private:
        static PrimitiveValue optimizeConstantUnaryExpression(BoundUnaryOperator::Kind kind, const PrimitiveValue& operand, const Types::type& type)
        {
            switch(kind)
            {
            case BoundUnaryOperator::Kind::Stringify: return operand.toApplicationString();
            case BoundUnaryOperator::Kind::Typeof: return type;
            case BoundUnaryOperator::Kind::LogicalNot: return !operand.getBool();
            case BoundUnaryOperator::Kind::UnaryPlus: return +operand;
            case BoundUnaryOperator::Kind::UnaryMinus: return -operand;
            default: return PrimitiveValue::invalidValue;
            }
        }

        static PrimitiveValue optimizeConstantBinaryExpression(BoundBinaryOperator::Kind kind, const PrimitiveValue& left, const PrimitiveValue& right)
        {
            switch(kind)
            {
            case BoundBinaryOperator::Kind::Addition: return left + right;
            case BoundBinaryOperator::Kind::Equals: return left == right;
            case BoundBinaryOperator::Kind::NotEquals: return left != right;
            case BoundBinaryOperator::Kind::Subtraction: return left - right;
            case BoundBinaryOperator::Kind::Multiplication: return left * right;
            case BoundBinaryOperator::Kind::Division: return left / right;
            case BoundBinaryOperator::Kind::Greater: return left > right;
            case BoundBinaryOperator::Kind::Less: return left < right;
            case BoundBinaryOperator::Kind::GreaterEqual: return left >= right;
            case BoundBinaryOperator::Kind::LessEqual: return left <= right;
            case BoundBinaryOperator::Kind::Modulo: return left % right;
            default: return PrimitiveValue::invalidValue;
            }
        }
    };
}