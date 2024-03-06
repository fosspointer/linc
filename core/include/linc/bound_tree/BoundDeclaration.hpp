#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundDeclaration : public BoundNode
    {
    public:
        BoundDeclaration(const Types::type& type)
            :BoundNode(type)
        {}
        
        virtual ~BoundDeclaration() = default;
        virtual std::unique_ptr<const BoundDeclaration> cloneConst() const = 0;
    };
}