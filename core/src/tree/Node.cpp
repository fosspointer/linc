#include <linc/tree/Node.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/tree/Declaration.hpp>

namespace linc
{
    std::unique_ptr<const Node> Node::cloneConst() const
    {
        if(auto expression = dynamic_cast<const Expression*>(this))
            return expression->cloneConst();

        else if(auto statement = dynamic_cast<const Statement*>(this))
            return statement->cloneConst();

        else if(auto declaration = dynamic_cast<const Declaration*>(this))
            return declaration->cloneConst();

        else throw LINC_EXCEPTION_OUT_OF_BOUNDS(Node);
    }
}