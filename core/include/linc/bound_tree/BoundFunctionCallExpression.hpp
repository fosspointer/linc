#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundFunctionCallExpression final : public BoundExpression
    {
    public:
        struct Argument final
        {
            std::string name;
            std::unique_ptr<const BoundExpression> value;
        };

        BoundFunctionCallExpression(Types::type type, const std::string& name, std::vector<Argument> arguments);

        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const std::vector<Argument>& getArguments() const { return m_arguments; }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_name;
        const std::vector<Argument> m_arguments;
    };
}