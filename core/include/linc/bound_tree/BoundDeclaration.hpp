#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundDeclaration : public BoundNode
    {
    public:
        BoundDeclaration(Types::Type type)
            :BoundNode(type)
        {}
        
        virtual ~BoundDeclaration() = default;
        virtual std::unique_ptr<const BoundDeclaration> clone_const() const = 0;
    };
}