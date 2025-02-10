#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundFunctionPrototypeDeclaration;
    class BoundFunctionDeclaration final : public BoundDeclaration
    {
    public:
    BoundFunctionDeclaration(std::unique_ptr<const BoundFunctionPrototypeDeclaration> prototype, std::unique_ptr<const BoundExpression> body);
    ~BoundFunctionDeclaration();

        [[nodiscard]] inline const BoundFunctionPrototypeDeclaration* const getPrototype() const { return m_prototype.get(); };
        [[nodiscard]] inline const BoundExpression* const getBody() const { return m_body.get(); }

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override;
        virtual std::vector<const BoundNode*> getChildren() const final override;
    private:
        virtual std::string toStringInner() const final override;
    
        const std::unique_ptr<const BoundFunctionPrototypeDeclaration> m_prototype;
        const std::unique_ptr<const BoundExpression> m_body;
    };
}