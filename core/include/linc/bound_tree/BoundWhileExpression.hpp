#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundWhileExpression final : public BoundExpression
    {
    public:
        BoundWhileExpression(Types::type type, std::unique_ptr<const BoundExpression> test_expression,
            std::unique_ptr<const BoundExpression> while_body, 
            std::optional<std::unique_ptr<const BoundExpression>> finally_body = {},
            std::optional<std::unique_ptr<const BoundExpression>> else_body = {});

        [[nodiscard]] inline const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        const BoundExpression* const getWhileBody() const { return m_whileBody.get(); }

        [[nodiscard]] inline const std::optional<const BoundExpression* const> getFinallyBody() const 
        {
            return m_finallyBody.has_value()? std::make_optional(m_finallyBody->get()): std::nullopt;
        }
        
        [[nodiscard]] inline const std::optional<const BoundExpression* const> getElseBody() const
        {
            return m_elseBody.has_value()? std::make_optional(m_elseBody->get()): std::nullopt;
        }

        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            if(m_finallyBody.has_value())
                if(m_elseBody.has_value())
                    return {m_testExpression.get(), m_whileBody.get(), m_finallyBody.value().get(), m_elseBody.value().get()};
                else
                    return {m_testExpression.get(), m_whileBody.get(), m_finallyBody.value().get()};
            else if(m_elseBody.has_value())
                return {m_testExpression.get(), m_whileBody.get(), m_elseBody.value().get()};
            else
                return {m_testExpression.get(), m_whileBody.get()};
            
        }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        
        const std::unique_ptr<const BoundExpression> m_testExpression;
        const std::unique_ptr<const BoundExpression> m_whileBody;
        const std::optional<const std::unique_ptr<const BoundExpression>> m_finallyBody, m_elseBody;
    };
}