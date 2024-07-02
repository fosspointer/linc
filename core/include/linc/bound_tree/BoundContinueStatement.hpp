#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundContinueStatement final : public BoundStatement
    {
    public:
        explicit BoundContinueStatement(Types::u64 scope)
            :BoundStatement(Types::voidType), m_scope(scope)
        {}

        [[nodiscard]] inline Types::u64 getScope() const { return m_scope; }

        virtual std::unique_ptr<const BoundStatement> clone() const final override
        {
            return std::make_unique<const BoundContinueStatement>(m_scope);
        }
    private:
        virtual std::string toStringInner() const final override { return "Continue Statement"; }
        const Types::u64 m_scope;
    };
}