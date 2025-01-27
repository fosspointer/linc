#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundTypeExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundAliasDeclaration final : public BoundDeclaration
    {
    public:
        BoundAliasDeclaration(const std::string& name, std::unique_ptr<const BoundTypeExpression> type)
            :BoundDeclaration(name), m_type(std::move(type))
        {}

        inline const BoundTypeExpression* const getType() const { return m_type.get(); }

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override
        {
            auto type = Types::uniqueCast<const BoundTypeExpression>(m_type->clone());
            return std::make_unique<const BoundAliasDeclaration>(m_name, std::move(type));
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return Logger::format("Bound Alias Declaration (=$) (::$)", PrimitiveValue(m_name), PrimitiveValue(m_type->getActualType()));
        }
        const std::unique_ptr<const BoundTypeExpression> m_type;
    };
}