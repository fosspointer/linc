#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundBreakStatement final : public BoundStatement
    {
    public:
        explicit BoundBreakStatement(std::string_view label_identifier)
            :m_label(label_identifier)
        {}

        [[nodiscard]] inline const std::string& getLabel() const { return m_label; }

        virtual std::unique_ptr<const BoundStatement> clone() const final override
        {
            return std::make_unique<const BoundBreakStatement>(m_label);
        }
    private:
        virtual std::string toStringInner() const final override { return "Break Statement"; }
        const std::string m_label;
    };
}