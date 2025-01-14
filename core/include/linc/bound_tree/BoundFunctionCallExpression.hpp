#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundFunctionCallExpression final : public BoundExpression
    {
    public:
        BoundFunctionCallExpression(Types::type type, std::unique_ptr<const BoundExpression> function, std::vector<std::unique_ptr<const BoundExpression>> arguments);

        [[nodiscard]] inline const BoundExpression* const getFunction() const { return m_function.get(); }
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundExpression>>& getArguments() const { return m_arguments; }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundExpression> m_function;
        const std::vector<std::unique_ptr<const BoundExpression>> m_arguments;
    };
}