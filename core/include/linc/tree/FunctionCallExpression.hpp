#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class FunctionCallExpression final : public Expression
    {
    public:
        FunctionCallExpression(const Token& identifier_token, const Token& left_parenthesis_token, const Token& right_parenthesis_token,
            std::vector<std::unique_ptr<const Expression>> arguments)
            :m_identifierToken(identifier_token), m_leftParenthesisToken(left_parenthesis_token), m_rightParenthesisToken(right_parenthesis_token),
            m_arguments(std::move(arguments))
        {}

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {};
        }

        virtual std::unique_ptr<const Expression> cloneConst() const final override
        {
            std::vector<std::unique_ptr<const Expression>> arguments;

            for(const auto& argument: m_arguments)
                arguments.push_back(std::move(argument->cloneConst()));

            return std::make_unique<const FunctionCallExpression>(m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken,
                std::move(arguments));
        }

        const Token& getIdentifierToken() const { return m_identifierToken; }
        const Token& getLeftParenthesis() const { return m_leftParenthesisToken; }
        const Token& getRightParenthesis() const { return m_rightParenthesisToken; }
        const std::vector<std::unique_ptr<const Expression>>& getArguments() const { return m_arguments; }
    private:
        const Token m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken;
        const std::vector<std::unique_ptr<const Expression>> m_arguments;
    };
}