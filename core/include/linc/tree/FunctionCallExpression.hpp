#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class CallExpression final : public Expression
    {
    public:
        struct Argument final
        {
            const std::optional<Token> separator;
            std::unique_ptr<const Expression> expression;
        };

        CallExpression(const Token& identifier, const Token& left_parenthesis, const Token& right_parenthesis,
            std::vector<Argument> arguments, bool is_external)
            :Expression(identifier.info), m_identifier(identifier), m_leftParenthesis(left_parenthesis),
            m_rightParenthesis(right_parenthesis), m_arguments(std::move(arguments)), m_isExternal(is_external)
        {
            addTokens(std::vector<Token>{m_identifier, m_leftParenthesis});
            for(const auto& argument: m_arguments)
            {
                addTokens(argument.expression->getTokens());
                if(argument.separator)
                    addToken(*argument.separator);
            }
            addToken(m_rightParenthesis);
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            std::vector<Argument> arguments;

            for(const auto& argument: m_arguments)
                arguments.push_back(Argument{argument.separator, std::move(argument.expression->clone())});

            return std::make_unique<const CallExpression>(m_identifier, m_leftParenthesis, m_rightParenthesis,
                std::move(arguments), m_isExternal);
        }

        const Token& getIdentifier() const { return m_identifier; }
        const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        const Token& getRightParenthesis() const { return m_rightParenthesis; }
        const std::vector<Argument>& getArguments() const { return m_arguments; }
        bool isExternal() const { return m_isExternal; }
    private:
        const Token m_identifier, m_leftParenthesis, m_rightParenthesis;
        const std::vector<Argument> m_arguments;
        const bool m_isExternal;
    };
}