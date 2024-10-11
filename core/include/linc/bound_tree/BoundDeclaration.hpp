#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundDeclaration : public BoundNode
    {
    public:        
        virtual ~BoundDeclaration() = default;
        virtual std::unique_ptr<const BoundDeclaration> clone() const = 0;
    };
}