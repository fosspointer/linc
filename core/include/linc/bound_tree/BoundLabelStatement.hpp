#pragma once
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundLabelStatement final : public BoundStatement
    {
    public:
        BoundLabelStatement(const std::string& name, std::unique_ptr<const BoundStatement> next, Types::u64 block_index,
            Types::u64 identifier_index)
            :m_name(name), m_next(std::move(next)), m_blockIndex(block_index),
            m_identifierIndex(identifier_index)
        {}

        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const BoundStatement* getNext() const { return m_next.get(); }
        [[nodiscard]] inline const Types::u64 getBlockIndex() const { return m_blockIndex; }
        [[nodiscard]] inline const Types::u64 getIdentifierIndex() const { return m_identifierIndex; }

        virtual std::unique_ptr<const BoundStatement> clone() const final override
        {
            return std::make_unique<const BoundLabelStatement>(m_name, m_next->clone(), m_blockIndex, m_identifierIndex);
        }

        virtual std::string toStringInner() const final override { return "Label Statement"; }
    private:
        const std::string m_name;
        const std::unique_ptr<const BoundStatement> m_next;
        const Types::u64 m_blockIndex, m_identifierIndex;
    };
}