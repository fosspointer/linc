#include <linc/bound_tree/BoundFunctionDeclaration.hpp>
#include <linc/bound_tree/BoundVariableDeclaration.hpp>
#include <linc/bound_tree/BoundFunctionPrototypeDeclaration.hpp>

namespace linc
{
    BoundFunctionDeclaration::BoundFunctionDeclaration(std::unique_ptr<const BoundFunctionPrototypeDeclaration> prototype, std::unique_ptr<const BoundExpression> body)
        :BoundDeclaration(prototype->getName()), m_prototype(std::move(prototype)), m_body(std::move(body))
    {}
    BoundFunctionDeclaration::~BoundFunctionDeclaration() = default;

    std::unique_ptr<const BoundDeclaration> BoundFunctionDeclaration::clone() const
    {
        auto prototype = Types::uniqueCast<const BoundFunctionPrototypeDeclaration>(m_prototype->clone());
        return std::make_unique<const BoundFunctionDeclaration>(std::move(prototype), m_body->clone());
    }

    std::vector<const BoundNode*> BoundFunctionDeclaration::getChildren() const
    {
        return std::vector<const BoundNode*>{m_prototype.get(), m_body.get()};
    }

    std::string BoundFunctionDeclaration::toStringInner() const
    {
        return Logger::format("Function Declaration (=$) (:$)", PrimitiveValue(m_name), PrimitiveValue(m_prototype->getFunctionType()));
    }
}