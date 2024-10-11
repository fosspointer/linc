#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class WhileExpression final : public Expression
    {
    public:
        WhileExpression(const Token& while_keyword, const std::optional<Token>& finally_keyword, const std::optional<Token>& else_keyword,
            std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Expression> while_body,
            std::unique_ptr<const Expression> finally_body, std::unique_ptr<const Expression> else_body)
            :Expression(while_keyword.info), m_whileKeyword(while_keyword), m_finallyKeyword(finally_keyword), m_testExpression(std::move(test_expression)),
            m_whileBody(std::move(while_body)), m_finallyBody(std::move(finally_body)), m_elseBody(std::move(else_body))
        {
            addToken(m_whileKeyword);
            addTokens(m_testExpression->getTokens());
            addTokens(m_whileBody->getTokens());

            if(m_finallyKeyword && m_finallyBody)
            {
                addToken(*m_finallyKeyword);
                addTokens(m_finallyBody->getTokens());
            }
            
            if(m_elseKeyword && m_elseBody)
            {
                addToken(*m_elseKeyword);
                addTokens(m_elseBody->getTokens());
            }
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const WhileExpression>(m_whileKeyword, m_finallyKeyword, m_elseKeyword, m_testExpression->clone(),
                m_whileBody->clone(), m_finallyBody? m_finallyBody->clone(): nullptr, m_elseBody? m_elseBody->clone(): nullptr);
        }

        const Token& getWhileKeyword() const { return m_whileKeyword; }
        const std::optional<const Token>& getElseKeyword() const { return m_elseKeyword; }
        const std::optional<const Token>& getFinallyKeyword() const { return m_finallyKeyword; }
        const Expression* const getTestExpression() const { return m_testExpression.get(); } 
        const Expression* const getWhileBody() const { return m_whileBody.get(); }
        const Expression* const getFinallyBody() const { return m_finallyBody? m_finallyBody.get(): nullptr; }
        const Expression* const getElseBody() const { return m_elseBody? m_elseBody.get(): nullptr; }
        inline bool hasFinally() const { return (bool)m_finallyBody; }
        inline bool hasElse() const { return (bool)m_elseBody; }

    private:
        const Token m_whileKeyword;
        const std::optional<const Token> m_finallyKeyword, m_elseKeyword;
        const std::unique_ptr<const Expression> m_testExpression, m_whileBody, m_finallyBody, m_elseBody;
    };
}