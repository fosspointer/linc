#pragma once
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/system/Types.hpp>

namespace linc
{
    struct LoopLabel final
    {
        Token specifier;
        std::unique_ptr<const IdentifierExpression> identifier;
        
        LoopLabel copy() const
        {
            auto identifier_clone = Types::uniqueCast<const IdentifierExpression>(identifier->clone());
            return LoopLabel{specifier, std::move(identifier_clone)};
        }
    };
}