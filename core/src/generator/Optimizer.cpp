#include <linc/generator/Optimizer.hpp>

namespace linc
{
    std::unique_ptr<const BoundNode> Optimizer::optimizeNode(const BoundNode* node)
    {
        if(auto expression = dynamic_cast<const BoundExpression*>(node))
            return optimizeExpression(std::move(expression));

        else if(auto statement = dynamic_cast<const BoundStatement*>(node))
            return optimizeStatement(std::move(statement));
        
        else if(auto declaration = dynamic_cast<const BoundDeclaration*>(node))
            return optimizeDeclaration(std::move(declaration));

        else throw LINC_EXCEPTION_INVALID_INPUT("Encountered unrecognized node while optimizing");
    }
}