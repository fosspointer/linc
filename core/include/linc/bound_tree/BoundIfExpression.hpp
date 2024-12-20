#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundIfExpression final : public BoundExpression
    {
    public:
        BoundIfExpression(Types::type type, std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundExpression> if_body, 
            std::unique_ptr<const BoundExpression> else_body);

        [[nodiscard]] inline const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        [[nodiscard]] inline const BoundExpression* const getIfBody() const { return m_ifBody.get(); }
        [[nodiscard]] inline const BoundExpression* const getElseBody() const { return m_elseBody? m_elseBody.get(): nullptr; }
        [[nodiscard]] inline const bool hasElse() const { return (bool)m_elseBody; }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;

        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            if(m_elseBody)
                return {m_testExpression.get(), m_ifBody.get(), m_elseBody.get()};
            else
                return {m_testExpression.get(), m_ifBody.get()};
        }
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundExpression> m_testExpression, m_ifBody, m_elseBody;
    };
}