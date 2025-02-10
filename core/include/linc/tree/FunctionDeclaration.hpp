#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/NodeListClause.hpp>
#include <linc/system/Types.hpp>

namespace linc
{
    class FunctionPrototypeDeclaration;
    class VariableDeclaration;
    class FunctionDeclaration final : public Declaration
    {
    public:
        FunctionDeclaration(std::unique_ptr<const FunctionPrototypeDeclaration> prototype, std::unique_ptr<const Expression> body);
        ~FunctionDeclaration();
        virtual std::unique_ptr<const Declaration> clone() const final override;
        virtual std::unique_ptr<const Declaration> cloneRename(std::unique_ptr<const IdentifierExpression> identifier) const;
        
        inline const FunctionPrototypeDeclaration* const getPrototype() const { return m_prototype.get(); }
        inline const Expression* const getBody() const { return m_body.get(); }
    private:
        const std::unique_ptr<const FunctionPrototypeDeclaration> m_prototype;
        const std::unique_ptr<const Expression> m_body;
    };
}