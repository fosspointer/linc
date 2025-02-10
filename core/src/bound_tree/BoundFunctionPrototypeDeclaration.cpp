#include <linc/bound_tree/BoundFunctionPrototypeDeclaration.hpp>
#include <linc/bound_tree/BoundVariableDeclaration.hpp>

namespace linc
{
    BoundFunctionPrototypeDeclaration::BoundFunctionPrototypeDeclaration(const Types::type& function_type, const std::string& name,
        std::unique_ptr<const BoundNodeListClause<BoundVariableDeclaration>> arguments)
        :BoundDeclaration(name), m_functionType(function_type), m_arguments(std::move(arguments))
    {}

    BoundFunctionPrototypeDeclaration::~BoundFunctionPrototypeDeclaration() = default;

    std::unique_ptr<const BoundDeclaration> BoundFunctionPrototypeDeclaration::clone() const
    {
        return std::make_unique<const BoundFunctionPrototypeDeclaration>(m_functionType, m_name, m_arguments->clone());
    }

    std::size_t BoundFunctionPrototypeDeclaration::getDefaultArgumentCount() const
    {
        std::vector<std::unique_ptr<const BoundVariableDeclaration>>::size_type count{};
        
        for(const auto& argument: m_arguments->getList())
            if(argument->getDefaultValue())
                ++count;
        
        return count;
    }

    std::vector<const BoundNode*> BoundFunctionPrototypeDeclaration::getChildren() const 
    {
        std::vector<const BoundNode*> nodes;
        nodes.reserve(m_arguments->getList().size());
        for(const auto& argument: m_arguments->getList())
            nodes.push_back(argument.get());
        
        return nodes;
    }

    std::string BoundFunctionPrototypeDeclaration::toStringInner() const
    {
        return "Function Prototype Declaration";
    }
}