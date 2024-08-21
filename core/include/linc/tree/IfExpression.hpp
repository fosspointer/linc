#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class IfExpression final : public Expression
    {
    public:
        IfExpression(const Token& if_keyword, std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Expression> if_body,
            const Token& else_keyword, std::unique_ptr<const Expression> else_body)
            :Expression(if_keyword.info), m_ifKeyword(if_keyword), m_testExpression(std::move(test_expression)),
            m_ifBody(std::move(if_body)), m_elseKeyword(else_keyword), m_elseBody(std::move(else_body))
        {
            addToken(m_ifKeyword);
            addTokens(m_testExpression->getTokens());
            addTokens(m_ifBody->getTokens());
            addToken(m_elseKeyword.value());
            addTokens(m_elseBody.value()->getTokens());
        }

        IfExpression(const Token& if_keyword, std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Expression> if_body)
            :Expression(if_keyword.info), m_ifKeyword(if_keyword), m_testExpression(std::move(test_expression)),
            m_ifBody(std::move(if_body)), m_elseKeyword(std::nullopt), m_elseBody(std::nullopt)
        {
            addToken(m_ifKeyword);
            addTokens(m_testExpression->getTokens());
            addTokens(m_ifBody->getTokens());
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            if(m_elseBody.has_value() && m_elseKeyword.has_value())
                return std::make_unique<const IfExpression>(m_ifKeyword, std::move(m_testExpression->clone()), 
                    std::move(m_ifBody->clone()), m_elseKeyword.value(), std::move(m_elseBody.value()->clone()));
            else return std::make_unique<const IfExpression>(m_ifKeyword, std::move(m_testExpression->clone()), 
                    std::move(m_ifBody->clone()));
        }

        const Token& getIfKeyword() const { return m_ifKeyword; } 
        const std::optional<const Token>& getElseKeyword() const { return m_elseKeyword; }

        const Expression* const getTestExpression() const { return m_testExpression.get(); } 
        const Expression* const getIfBody() const { return m_ifBody.get(); } 
        const std::optional<const Expression* const> getElseBody() const 
        { 
            return m_elseBody.has_value()? std::make_optional(m_elseBody->get()): std::nullopt;
        } 
    private:
        const Token m_ifKeyword;
        const std::unique_ptr<const Expression> m_testExpression;
        const std::unique_ptr<const Expression> m_ifBody;
        const std::optional<const Token> m_elseKeyword;
        const std::optional<const std::unique_ptr<const Expression>> m_elseBody;
    };
}