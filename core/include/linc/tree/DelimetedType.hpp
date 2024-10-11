#pragma once
#include <linc/tree/TypeExpression.hpp>

namespace linc
{
    struct DelimitedType final
    {
        std::unique_ptr<const TypeExpression> type;
        std::optional<const Token> delimeter;
    };
}