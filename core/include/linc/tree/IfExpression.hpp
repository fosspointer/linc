#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class IfExpression final : public Expression
    {
    public:
        IfExpression(const Token& if_keyword, const std::optional<Token>& else_keyword, std::unique_ptr<const Expression> test_expression,
            std::unique_ptr<const Expression> if_body, std::unique_ptr<const Expression> else_body)
            :Expression(if_keyword.info), m_ifKeyword(if_keyword), m_elseKeyword(else_keyword), m_testExpression(std::move(test_expression)),
            m_ifBody(std::move(if_body)), m_elseBody(std::move(else_body))
        {
            addToken(m_ifKeyword);
            addTokens(m_testExpression->getTokens());
            addTokens(m_ifBody->getTokens());
            if(m_elseKeyword.has_value())
                addToken(m_elseKeyword.value());
            if(m_elseBody)
                addTokens(m_elseBody->getTokens());
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const IfExpression>(m_ifKeyword, m_elseKeyword, m_testExpression->clone(), m_ifBody->clone(),
                m_elseBody? m_elseBody->clone(): nullptr);
        }

        const Token& getIfKeyword() const { return m_ifKeyword; } 
        const std::optional<const Token>& getElseKeyword() const { return m_elseKeyword; }

        const Expression* const getTestExpression() const { return m_testExpression.get(); } 
        const Expression* const getIfBody() const { return m_ifBody.get(); } 
        const Expression* const getElseBody() const { return m_elseBody? m_elseBody.get(): nullptr; }
    private:
        const Token m_ifKeyword;
        const std::optional<const Token> m_elseKeyword;
        const std::unique_ptr<const Expression> m_testExpression, m_ifBody, m_elseBody;
    };
}