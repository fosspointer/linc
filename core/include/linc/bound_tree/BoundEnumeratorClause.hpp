#pragma once
#include <linc/bound_tree/BoundClause.hpp>

namespace linc
{
    class BoundEnumeratorClause final : public BoundClause<BoundEnumeratorClause>
    {
    public:
        BoundEnumeratorClause(const Types::type& actual_type, const std::string& name, const Token::Info& info)
            :BoundClause(info), m_actualType(actual_type), m_name(name)
        {}

        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const Types::type& getActualType() const { return m_actualType; }

        virtual std::unique_ptr<const BoundEnumeratorClause> clone() const final override
        {
            return std::make_unique<const BoundEnumeratorClause>(m_actualType, m_name, getInfo());
        }
    private:
        const Types::type m_actualType;
        const std::string m_name;
    };
}
