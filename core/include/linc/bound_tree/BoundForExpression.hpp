#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>
#include <linc/bound_tree/BoundVariableDeclaration.hpp>

namespace linc
{
    class BoundForExpression final : public BoundExpression 
    {
    public:
        struct BoundVariableForSpecifier final
        {
            std::unique_ptr<const BoundVariableDeclaration> variableDeclaration;
            std::unique_ptr<const BoundExpression> expression;
            std::unique_ptr<const BoundStatement> statement;
        };

        struct BoundRangeForSpecifier final
        {
            std::unique_ptr<const BoundIdentifierExpression> valueIdentifier, arrayIdentifier;
        };

        BoundForExpression(std::unique_ptr<const BoundVariableDeclaration> declaration, std::unique_ptr<const BoundExpression> expression,
            std::unique_ptr<const BoundStatement> statement, std::unique_ptr<const BoundStatement> body);

        BoundForExpression(std::unique_ptr<const BoundIdentifierExpression> value_identifier,
            std::unique_ptr<const BoundIdentifierExpression> array_identifier, std::unique_ptr<const BoundStatement> body);

        [[nodiscard]] inline const std::variant<const BoundVariableForSpecifier, const BoundRangeForSpecifier>& getSpecifier() const { return m_specifier; }
        [[nodiscard]] inline const BoundStatement* const getBody() const { return m_body.get(); }

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
        virtual std::string toStringInner() const final override;
    private:
        const std::variant<const BoundVariableForSpecifier, const BoundRangeForSpecifier> m_specifier;
        const std::unique_ptr<const BoundStatement> m_body;
    };
}