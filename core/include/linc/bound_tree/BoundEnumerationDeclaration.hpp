#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundNodeListClause.hpp>
#include <linc/bound_tree/BoundEnumeratorClause.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundEnumerationDeclaration final: public BoundDeclaration
    {
    public:
        BoundEnumerationDeclaration(const std::string& name, std::unique_ptr<const BoundNodeListClause<BoundEnumeratorClause>> enumerators)
            :BoundDeclaration(name), m_enumerators(std::move(enumerators)), m_actualType(calculateActualType(m_enumerators.get()))
        {}

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override
        {
            return std::make_unique<const BoundEnumerationDeclaration>(m_name, m_enumerators->clone());
        }

        [[nodiscard]] inline const Types::type& getActualType() const { return m_actualType; }
        [[nodiscard]] inline const auto* const getEnumerators() const { return m_enumerators.get(); }
    private:
        static Types::type calculateActualType(const BoundNodeListClause<BoundEnumeratorClause>* enumerators)
        {
            Types::type::Enumeration result;
            result.reserve(enumerators->getList().size());
            for(const auto& enumerator: enumerators->getList())
                result.push_back(std::pair(enumerator->getName(), enumerator->getActualType()));
            return Types::type(std::move(result));
        }

        virtual std::string toStringInner() const final override
        {
            return "Enumeration Declaration";
        }
        const std::unique_ptr<const BoundNodeListClause<BoundEnumeratorClause>> m_enumerators;
        const Types::type m_actualType;
    };
}