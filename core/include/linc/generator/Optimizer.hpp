#pragma once
#include <linc/BoundTree.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Optimizer final
    {
    public:
        Optimizer() = delete;

        static std::unique_ptr<const BoundNode> optimizeNode(const BoundNode* node);
        
        static std::unique_ptr<const BoundExpression> optimizeBlockExpression(const BoundBlockExpression* expression)
        {
            std::vector<std::unique_ptr<const linc::BoundStatement>> statements;
            statements.reserve(expression->getStatements().size());

            for(const auto& statement: expression->getStatements())
                statements.push_back(optimizeStatement(statement.get()));

            auto tail = expression->getTail()? optimizeExpression(expression->getTail()): nullptr;
            return std::make_unique<const BoundBlockExpression>(std::move(statements), std::move(tail));
        }

        static std::unique_ptr<const BoundExpression> optimizeIfExpression(const BoundIfExpression* expression)
        {
            auto test_expression = optimizeExpression(expression->getTestExpression());

            if(auto literal = dynamic_cast<const BoundLiteralExpression*>(test_expression.get()))
            {
                if(literal->getValue().getBool())
                    return expression->getIfBody()->clone();
                else if(auto else_body = expression->getElseBody())
                    return else_body->clone();
                else return std::make_unique<const BoundBlockExpression>(std::vector<std::unique_ptr<const BoundStatement>>{}, nullptr);
            }
            auto if_body = optimizeExpression(expression->getIfBody());
            auto else_body = expression->hasElse()? optimizeExpression(expression->getElseBody()): nullptr;
            return std::make_unique<const BoundIfExpression>(expression->getType(), std::move(test_expression), std::move(if_body),
                std::move(else_body));
        }

        static std::unique_ptr<const BoundExpression> optimizeForExpression(const BoundForExpression* expression)
        {
            const auto& clause = expression->getForClause();
            if(auto value = clause->getIfFirst())
            {
                auto declaration = optimizeDeclaration(value->getDeclaration());
                auto test_expression = optimizeExpression(value->getTestExpression());
                auto end_expression = optimizeExpression(value->getEndExpression());
                auto legacy_for_clause = std::make_unique<const BoundLegacyForClause>(std::move(declaration), std::move(test_expression), std::move(end_expression));
                auto body = optimizeExpression(expression->getBody());
                return std::make_unique<const BoundForExpression>(expression->getLabel(), std::make_unique<const BoundForExpression::ForClause>(std::move(legacy_for_clause)), std::move(body));
            }
            
            auto value = clause->getSecond();
            auto iterated_expression = optimizeExpression(value->getExpression());
            auto identifier = Types::uniqueCast<const BoundIdentifierExpression>(value->getIdentifier()->clone());
            auto ranged_for_clause = std::make_unique<const BoundRangedForClause>(std::move(identifier), std::move(iterated_expression));
            auto body = optimizeExpression(expression->getBody());
            return std::make_unique<const BoundForExpression>(expression->getLabel(), std::make_unique<const BoundForExpression::ForClause>(std::move(ranged_for_clause)), std::move(body));
        }
        
        static std::unique_ptr<const BoundExpression> optimizeExpression(const BoundExpression* expression)
        {
            if(auto if_expression = dynamic_cast<const BoundIfExpression*>(expression))
                return optimizeIfExpression(if_expression);

            else if(auto while_expression = dynamic_cast<const BoundWhileExpression*>(expression))
            {
                auto test = optimizeExpression(while_expression->getTestExpression());
                auto while_body = optimizeExpression(while_expression->getWhileBody());
                auto finally_body = while_expression->hasFinally()? optimizeExpression(while_expression->getFinallyBody()): nullptr;
                auto else_body = while_expression->hasElse()? optimizeExpression(while_expression->getElseBody()): nullptr;
                auto label = while_expression->getLabel();

                return std::make_unique<const BoundWhileExpression>(label, while_expression->getType(), std::move(test), std::move(while_body),
                    std::move(finally_body), std::move(else_body));
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
            else if(auto block_expression = dynamic_cast<const BoundBlockExpression*>(expression))
            {
                std::vector<std::unique_ptr<const BoundStatement>> statements;
                for(const auto& statement: block_expression->getStatements())
                    statements.push_back(optimizeStatement(statement.get()));
                auto tail = block_expression->getTail()? optimizeExpression(block_expression->getTail()): nullptr;

                return std::make_unique<const BoundBlockExpression>(std::move(statements), std::move(tail));
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
        
        static std::unique_ptr<const BoundVariableDeclaration> optimizeVariableDeclaration(const BoundVariableDeclaration* declaration)
        {
            auto value = declaration->getDefaultValue()? optimizeExpression(declaration->getDefaultValue()): nullptr;
            return std::make_unique<const BoundVariableDeclaration>(declaration->getActualType(), declaration->getName(), std::move(value));
        }

        static std::unique_ptr<const BoundDeclaration> optimizeDeclaration(const BoundDeclaration* declaration)
        {
            if(auto variable_declaration = dynamic_cast<const BoundVariableDeclaration*>(declaration))
                return optimizeVariableDeclaration(std::move(variable_declaration));

            else if(auto function_declaration = dynamic_cast<const BoundFunctionDeclaration*>(declaration))
            {
                auto body = optimizeExpression(function_declaration->getBody());
                std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;
                arguments.reserve(function_declaration->getArguments().size());
                for(const auto& argument: function_declaration->getArguments())
                {
                    auto optimized_argument = Types::uniqueCast<const BoundVariableDeclaration>(optimizeDeclaration(argument.get()));
                    arguments.push_back(std::move(optimized_argument));
                }

                auto function_type = Types::type{Types::type::Function{function_declaration->getReturnType().clone(), function_declaration->getFunctionType().function.argumentTypes}};
                return std::make_unique<const BoundFunctionDeclaration>(function_type, function_declaration->getName(), std::move(arguments), std::move(body));
            }
            return declaration->clone();
        }

        static BoundProgram optimizeProgram(BoundProgram& program)
        {
            std::vector<std::unique_ptr<const BoundDeclaration>> declarations;
            
            for(const auto& declaration: program.declarations)
                declarations.push_back(optimizeDeclaration(declaration.get()));

            return BoundProgram{.declarations = std::move(declarations)};
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
            case BoundUnaryOperator::Kind::BitwiseNot: return ~operand;
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