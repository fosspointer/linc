#include <linc/bound_tree/BoundBinaryExpression.hpp>

namespace linc
{
    BoundBinaryOperator::BoundBinaryOperator(Kind kind, Types::Type left_type, Types::Type right_type)
        :m_kind(kind), m_leftType(left_type), m_rightType(right_type), 
        m_returnType(getReturnType(kind, left_type, right_type))
    {}

    BoundBinaryOperator::BoundBinaryOperator(Kind kind, Types::Type left_type, Types::Type right_type, Types::Type return_type)
        :m_kind(kind), m_leftType(left_type), m_rightType(right_type), m_returnType(return_type)
    {}

    std::unique_ptr<const BoundBinaryOperator> BoundBinaryOperator::clone_const() const 
    {
        return std::make_unique<const BoundBinaryOperator>(m_kind, m_leftType, m_rightType, m_returnType);
    }

    std::string BoundBinaryOperator::kindToString(Kind kind) 
    {
        switch(kind)
        {
        case Kind::Invalid: return "Invalid Binary Operator";
        case Kind::Addition: return "Arithmetic Addition Operator";
        case Kind::Subtraction: return "Arithmetic Subtraction Operator";
        case Kind::Multiplication: return "Arithmetic Multiplication Operator";
        case Kind::Division: return "Arithmetic Division Operator";
        case Kind::LogicalAnd: return "Logical AND Operator";
        case Kind::LogicalOr: return "Logical OR Operator";
        case Kind::Equals: return "Relational Equality Operator";
        case Kind::NotEquals: return "Relational Inequality Operator";
        case Kind::Greater: return "Relational Greater-Than Operator";
        case Kind::Less: return "Relational Less-Than Operator";
        case Kind::GreaterEqual: return "Relational Greater-or-Equal Operator";
        case Kind::LessEqual: return "Relational Less-or-Equal Operator";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(BoundBinaryOperator::Kind);
        }
    }

    Types::Type BoundBinaryOperator::getReturnType(Kind kind, Types::Type left_type, Types::Type right_type)
    {
        switch(kind)
        {
        case Kind::Addition:
            if((left_type == Types::Type::string && right_type == Types::Type::_char)
            || (left_type == Types::Type::_char && right_type == Types::Type::string)
            || (left_type == right_type && right_type == Types::Type::_char))
                return Types::Type::string;
            else if(left_type == right_type)
                return right_type;
            else return Types::Type::invalid;
        case Kind::Subtraction:
        case Kind::Multiplication:
        case Kind::Division:
            if(left_type == right_type)
                return right_type;
            else return Types::Type::invalid;
            break;
        case Kind::LogicalAnd:
        case Kind::LogicalOr:
            if(left_type == Types::Type::_bool && right_type == Types::Type::_bool)
                return Types::Type::_bool;
            else return Types::Type::invalid;
        case Kind::Equals:
        case Kind::NotEquals:
            if(left_type == right_type)
                return Types::Type::_bool;
            else return Types::Type::invalid;
        case Kind::Greater:
        case Kind::GreaterEqual:
        case Kind::Less:
        case Kind::LessEqual:
            if(Types::isNumeric(left_type) && left_type == right_type)
                return Types::Type::_bool;
            else return Types::Type::invalid;
            break;
        default: return Types::Type::invalid;
        }
    }

    BoundBinaryExpression::BoundBinaryExpression(std::unique_ptr<const BoundBinaryOperator> _operator, std::unique_ptr<const BoundExpression> left,
        std::unique_ptr<const BoundExpression> right)
        :BoundExpression(_operator->getReturnType()), m_operator(std::move(_operator)), m_left(std::move(left)), m_right(std::move(right))
    {}

    std::unique_ptr<const BoundExpression> BoundBinaryExpression::clone_const() const
    {
        return std::make_unique<const BoundBinaryExpression>(m_operator->clone_const(), m_left->clone_const(), m_right->clone_const());
    }

    std::string BoundBinaryExpression::toStringInner() const
    {
        return linc::Logger::format("Bound Binary Expression (@'$')", BoundBinaryOperator::kindToString(m_operator->getKind()));
    }
}