#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundWhileExpression final : public BoundExpression
    {
    public:
        BoundWhileExpression(std::string_view label, Types::type type, std::unique_ptr<const BoundExpression> test_expression,
            std::unique_ptr<const BoundExpression> while_body, std::unique_ptr<const BoundExpression> finally_body, std::unique_ptr<const BoundExpression> else_body);

        [[nodiscard]] inline const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        const BoundExpression* const getWhileBody() const { return m_whileBody.get(); }

        [[nodiscard]] inline const BoundExpression* const getFinallyBody() const 
        {
            return m_finallyBody? m_finallyBody.get(): nullptr;
        }
        
        [[nodiscard]] inline const BoundExpression* const getElseBody() const
        {
            return m_elseBody? m_elseBody.get(): nullptr;
        }

        [[nodiscard]] inline bool hasFinally() const { return (bool)m_finallyBody; }
        [[nodiscard]] inline bool hasElse() const { return (bool)m_elseBody; }

        [[nodiscard]] inline const std::string& getLabel() const { return m_label; }

        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            if(m_finallyBody)
            {
                if(m_elseBody)
                    return {m_testExpression.get(), m_whileBody.get(), m_finallyBody.get(), m_elseBody.get()};
                else
                    return {m_testExpression.get(), m_whileBody.get(), m_finallyBody.get()};
            }
            else if(m_elseBody)
                return {m_testExpression.get(), m_whileBody.get(), m_elseBody.get()};
            else
                return {m_testExpression.get(), m_whileBody.get()};
            
        }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_label;
        const std::unique_ptr<const BoundExpression> m_testExpression, m_whileBody, m_finallyBody, m_elseBody;
    };
}