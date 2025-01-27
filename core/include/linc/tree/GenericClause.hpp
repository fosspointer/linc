#pragma once
#include <linc/tree/Clause.hpp>
#include <linc/tree/TypeExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class GenericClause final : public Clause<GenericClause>
    {
    public:
        GenericClause(const Token& left_angled_bracket, const Token& right_angled_bracket, std::unique_ptr<const NodeListClause<TypeExpression>> type_identifiers)
            :Clause<GenericClause>(type_identifiers->getTokenInfo()), m_leftAngledBracket(left_angled_bracket), m_rightAngledBracket(right_angled_bracket), m_typeIdentifiers(std::move(type_identifiers))
        {
            this->addToken(m_leftAngledBracket);
            this->addTokens(m_typeIdentifiers->getTokens());
            this->addToken(m_rightAngledBracket);
        }

        inline const Token& getLeftAngledBracket() const { return m_leftAngledBracket; }
        inline const Token& getRightAngledBracket() const { return m_rightAngledBracket; }
        const NodeListClause<TypeExpression>* const getTypes() const { return m_typeIdentifiers.get(); }

        virtual std::unique_ptr<const GenericClause> clone() const final override
        {
            return std::make_unique<const GenericClause>(m_leftAngledBracket, m_rightAngledBracket, m_typeIdentifiers->clone());
        }
    private:
        const Token m_leftAngledBracket, m_rightAngledBracket;
        const std::unique_ptr<const NodeListClause<TypeExpression>> m_typeIdentifiers;
    };
}