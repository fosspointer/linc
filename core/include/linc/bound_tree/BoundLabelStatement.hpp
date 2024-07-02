#pragma once
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundLabelStatement final : public BoundStatement
    {
    public:
        BoundLabelStatement(const std::string& name, std::unique_ptr<const BoundStatement> next, Types::u64 block_index)
            :BoundStatement(next->getType()), m_name(name), m_next(std::move(next)), m_blockIndex(block_index)
        {}

        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const BoundStatement* getNext() const { return m_next.get(); }
        [[nodiscard]] inline const Types::u64 getBlockIndex() const { return m_blockIndex; }

        virtual std::unique_ptr<const BoundStatement> clone() const final override
        {
            return std::make_unique<const BoundLabelStatement>(m_name, m_next->clone(), m_blockIndex);
        }

        virtual std::string toStringInner() const final override { return "Label Statement"; }
    private:
        const std::string m_name;
        const std::unique_ptr<const BoundStatement> m_next;
        const Types::u64 m_blockIndex;
    };
}