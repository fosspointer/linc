#pragma once
#include <linc/BoundTree.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct UnreachableBlock final{};
    struct ControlBlock final
    {
        std::size_t edge{-1ul};
    };
    
    struct BasicBlock final
    {
        std::vector<std::unique_ptr<const BoundStatement>> statements;
        std::size_t edge{-1ul};
    };

    struct ConditionalBlock final
    {
        std::unique_ptr<const BoundExpression> condition;
        std::size_t edgeTrue, edgeFalse;
    };

    struct MapBlock final
    {
        std::vector<std::pair<std::unique_ptr<const BoundExpression>, std::size_t>> cases;
    };

    using Block = std::variant<ControlBlock, BasicBlock, ConditionalBlock, UnreachableBlock>;
    struct ControlFlowGraph final
    {
        const BoundFunctionPrototypeDeclaration* prototype;
        std::vector<Block> blocks;
        std::unique_ptr<const BoundExpression> returnValue;  
    };
    
    using ControlFlowProgram = std::vector<ControlFlowGraph>;
}