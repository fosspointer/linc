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

        BoundForExpression(std::string_view label, std::unique_ptr<const BoundVariableDeclaration> declaration, std::unique_ptr<const BoundExpression> expression,
            std::unique_ptr<const BoundStatement> statement, std::unique_ptr<const BoundExpression> body);

        BoundForExpression(std::string_view label, std::unique_ptr<const BoundIdentifierExpression> value_identifier,
            std::unique_ptr<const BoundIdentifierExpression> array_identifier, std::unique_ptr<const BoundExpression> body);

        [[nodiscard]] inline const std::string& getLabel() const { return m_label; }
        [[nodiscard]] inline const std::variant<const BoundVariableForSpecifier, const BoundRangeForSpecifier>& getSpecifier() const { return m_specifier; }
        [[nodiscard]] inline const BoundExpression* const getBody() const { return m_body.get(); }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
        
        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            return {m_body.get()};
        }
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_label;
        const std::variant<const BoundVariableForSpecifier, const BoundRangeForSpecifier> m_specifier;
        const std::unique_ptr<const BoundExpression> m_body;
    };
}