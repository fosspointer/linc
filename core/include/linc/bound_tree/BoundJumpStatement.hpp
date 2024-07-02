#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundJumpStatement final : public BoundStatement
    {
    public:
        explicit BoundJumpStatement(Types::u64 block_index, Types::u64 scope)
            :BoundStatement(Types::voidType), m_blockIndex(block_index), m_scope(scope)
        {}

        [[nodiscard]] inline Types::u64 getBlockIndex() const { return m_blockIndex; }
        [[nodiscard]] inline Types::u64 getScope() const { return m_scope; }

        virtual std::unique_ptr<const BoundStatement> clone() const final override
        {
            return std::make_unique<const BoundJumpStatement>(m_blockIndex, m_scope);
        }
    private:
        virtual std::string toStringInner() const final override { return "Jump Statement"; }
        const Types::u64 m_blockIndex, m_scope;
    };
}