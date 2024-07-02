#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundExternalCallExpression final : public BoundExpression
    {
    public:
        BoundExternalCallExpression(Types::type type, const std::string& name, std::vector<std::unique_ptr<const BoundExpression>> arguments);

        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundExpression>>& getArguments() const { return m_arguments; }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_name;
        const std::vector<std::unique_ptr<const BoundExpression>> m_arguments;
    };
}