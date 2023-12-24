#include <linc/tree/Node.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>

namespace linc
{
    std::unique_ptr<const Node> Node::clone_const() const
    {
        if(auto expression = dynamic_cast<const Expression*>(this))
            return std::move(expression->clone_const());

        else if(auto statement = dynamic_cast<const Statement*>(this))
            return std::move(statement->clone_const());

        else throw LINC_EXCEPTION_OUT_OF_BOUNDS(Node);
    }
}