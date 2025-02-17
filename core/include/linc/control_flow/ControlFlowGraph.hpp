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
        struct Case final
        {
            std::unique_ptr<const BoundExpression> candidateExpression;
            std::size_t testIndex, resultIndex;
        };
        std::unique_ptr<const BoundExpression> testExpression;
        std::vector<Case> cases;
        std::size_t edgeExit;
    };

    using Block = std::variant<ControlBlock, BasicBlock, ConditionalBlock, UnreachableBlock, MapBlock>;
    struct ControlFlowGraph final
    {
        std::unique_ptr<const BoundFunctionPrototypeDeclaration> prototype;
        std::vector<Block> blocks;
        std::unique_ptr<const BoundExpression> returnValue;  
    };
    
    struct ControlFlowProgram
    {
        std::vector<std::pair<std::string, Types::type>> globals;
        std::vector<ControlFlowGraph> functions;
    };
}