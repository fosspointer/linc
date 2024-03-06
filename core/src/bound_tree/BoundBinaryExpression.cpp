#include <linc/bound_tree/BoundBinaryExpression.hpp>

namespace linc
{
    BoundBinaryOperator::BoundBinaryOperator(Kind kind, Types::type left_type, Types::type right_type)
        :m_kind(kind), m_leftType(left_type), m_rightType(right_type), 
        m_returnType(getReturnType(kind, left_type, right_type))
    {}

    std::unique_ptr<const BoundBinaryOperator> BoundBinaryOperator::cloneConst() const 
    {
        return std::make_unique<const BoundBinaryOperator>(m_kind, m_leftType, m_rightType);
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
        case Kind::Assignment: return "Assignment Operator";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(BoundBinaryOperator::Kind);
        }
    }

    Types::type BoundBinaryOperator::getReturnType(Kind kind, Types::type left_type, Types::type right_type)
    {
        if(left_type.isArray != right_type.isArray)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot call operator '$' on primitive-array operand pair. ('$' and '$')",
                    kindToString(kind), Types::toString(left_type), Types::toString(right_type))
            });
            return Types::invalidType;
        }
        else if(left_type.isArray)
        {
            switch (kind)
            {
            case Kind::Addition:
                if(left_type.kind == right_type.kind)
                {
                    if(left_type.arraySize && right_type.arraySize)
                        return Types::type{
                            .kind = left_type.kind,
                            .isArray = true,
                            .arraySize = left_type.arraySize.value() + right_type.arraySize.value()
                        };
                    else return Types::type{
                            .kind = left_type.kind,
                            .isArray = true,
                        };
                }
                else return Types::invalidType;
            default: return Types::invalidType;
            }
        }

        switch(kind)
        {
        case Kind::Addition:
            if((left_type.kind == Types::Kind::_char && right_type.kind == Types::Kind::_char)
            || (left_type.kind == Types::Kind::string && right_type.kind == Types::Kind::string)
            || (left_type.kind == Types::Kind::_char && right_type.kind == Types::Kind::string)
            || (left_type.kind == Types::Kind::string && right_type.kind == Types::Kind::_char))
                return Types::fromKind(Types::Kind::string);
            else if(Types::isNumeric(left_type.kind) && left_type.kind == right_type.kind)
                return right_type;
            else return Types::invalidType;
        case Kind::Subtraction:
        case Kind::Multiplication:
        case Kind::Division:
            if(Types::isNumeric(left_type.kind) && left_type.kind == right_type.kind)
                return right_type;
            else return Types::invalidType;
        case Kind::LogicalAnd:
        case Kind::LogicalOr:
            if(left_type.kind == Types::Kind::_bool && right_type.kind == Types::Kind::_bool)
                return Types::fromKind(Types::Kind::_bool);
            else return Types::invalidType;
        case Kind::Equals:
        case Kind::NotEquals:
            if(left_type.kind == right_type.kind)
                return Types::fromKind(Types::Kind::_bool);
            else return Types::invalidType;
        case Kind::Greater:
        case Kind::GreaterEqual:
        case Kind::Less:
        case Kind::LessEqual:
            if(Types::isNumeric(left_type.kind) && left_type.kind == right_type.kind)
                return Types::fromKind(Types::Kind::_bool);
            else return Types::invalidType;
        case Kind::Assignment:
            if(!left_type.isMutable)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                    .message = "Cannot use assignment operator on non-mutable operand."
                });
                return Types::invalidType;
            }
            else if(left_type.kind == right_type.kind)
                return left_type;
            else return Types::invalidType;
        default: return Types::invalidType;
        }
    }

    BoundBinaryExpression::BoundBinaryExpression(std::unique_ptr<const BoundBinaryOperator> _operator, std::unique_ptr<const BoundExpression> left,
        std::unique_ptr<const BoundExpression> right)
        :BoundExpression(_operator->getReturnType()), m_operator(std::move(_operator)), m_left(std::move(left)), m_right(std::move(right))
    {}

    std::unique_ptr<const BoundExpression> BoundBinaryExpression::cloneConst() const
    {
        return std::make_unique<const BoundBinaryExpression>(m_operator->cloneConst(), m_left->cloneConst(), m_right->cloneConst());
    }

    std::string BoundBinaryExpression::toStringInner() const
    {
        return linc::Logger::format("Bound Binary Expression (@'$')", BoundBinaryOperator::kindToString(m_operator->getKind()));
    }
}