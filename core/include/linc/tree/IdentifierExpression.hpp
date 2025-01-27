#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class GenericClause;
    class IdentifierExpression final : public Expression 
    {
    public:
        IdentifierExpression(const Token& token, std::unique_ptr<const GenericClause> generic);
        ~IdentifierExpression();
        virtual std::unique_ptr<const Expression> clone() const final override;
        [[nodiscard]] const Token& getIdentifierToken() const { return m_identifierToken; }
        [[nodiscard]] std::string getValue() const { return m_identifierToken.value.value_or(std::string{}); }
        [[nodiscard]] const GenericClause* const getGeneric() const { return m_generic? m_generic.get(): nullptr; }
    private:
        const Token m_identifierToken;
        const std::unique_ptr<const GenericClause> m_generic;
    };
}