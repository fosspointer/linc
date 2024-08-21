#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class WhileExpression final : public Expression
    {
    public:
        WhileExpression(const Token& while_keyword, std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Expression> while_body,
            const std::optional<Token>& finally_keyword, std::optional<std::unique_ptr<const Expression>> finally_body,
            const std::optional<Token>& else_keyword, std::optional<std::unique_ptr<const Expression>> else_body)
            :Expression(while_keyword.info), m_whileKeyword(while_keyword), m_testExpression(std::move(test_expression)),
            m_whileBody(std::move(while_body)), m_finallyKeyword(finally_keyword), m_elseKeyword(else_keyword),
            m_finallyBody(finally_body? std::make_optional(std::move(finally_body.value())): std::nullopt),
            m_elseBody(else_body? std::make_optional(std::move(else_body.value())): std::nullopt)
        {
            addToken(m_whileKeyword);
            addTokens(m_testExpression->getTokens());
            addTokens(m_whileBody->getTokens());

            if(m_finallyBody.has_value())
                addTokens(m_finallyBody->get()->getTokens());
            
            if(m_elseBody.has_value())
                addTokens(m_elseBody->get()->getTokens());
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const WhileExpression>(m_whileKeyword,
                std::move(m_testExpression->clone()), std::move(m_whileBody->clone()), m_finallyKeyword, 
                m_finallyBody.has_value()? std::make_optional(m_finallyBody.value()->clone()): std::nullopt,
                m_elseKeyword,
                m_elseBody.has_value()? std::make_optional(m_elseBody.value()->clone()): std::nullopt);
        }

        const Token& getWhileKeywordToken() const { return m_whileKeyword; }
        const Expression* const getTestExpression() const { return m_testExpression.get(); } 
        
        const Expression* const getWhileBody() const { return m_whileBody.get(); }

        const std::optional<const Expression* const> getFinallyBody() const
        {
            return m_elseBody.has_value()? std::make_optional(m_finallyBody->get()): std::nullopt;
        }  
        
        const std::optional<const Expression* const> getElseBody() const
        {
            return m_elseBody.has_value()? std::make_optional(m_elseBody->get()): std::nullopt;
        } 
    private:
        const Token m_whileKeyword;
        const std::unique_ptr<const Expression> m_testExpression;
        const std::unique_ptr<const Expression> m_whileBody;
        std::optional<const Token> m_finallyKeyword, m_elseKeyword;
        std::optional<const std::unique_ptr<const Expression>> m_finallyBody, m_elseBody;
    };
}