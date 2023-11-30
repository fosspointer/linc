#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundBinaryOperator final 
    {
    public:
        enum class Kind: char
        {
            Invalid, Addition, Subtraction, Multiplication, Division, LogicalAnd, LogicalOr,
            Equals, NotEquals, Greater, Less, GreaterEqual, LessEqual
        };

        BoundBinaryOperator(Kind kind, Types::Type left_type, Types::Type right_type)
            :m_kind(kind), m_leftType(left_type), m_rightType(right_type), 
            m_returnType(getReturnType(kind, left_type, right_type))
        {}

        BoundBinaryOperator(Kind kind, Types::Type left_type, Types::Type right_type, Types::Type return_type)
            :m_kind(kind), m_leftType(left_type), m_rightType(right_type), m_returnType(return_type)
        {}

        inline Kind getKind() const { return m_kind; }
        inline Types::Type getLeftType() const { return m_leftType; }
        inline Types::Type getRightType() const { return m_rightType; }
        inline Types::Type getReturnType() const { return m_returnType; }

        static std::string kindToString(Kind kind) 
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
    private:
        static Types::Type getReturnType(Kind kind, Types::Type left_type, Types::Type right_type)
        {
            switch(kind)
            {
            case Kind::Addition:
            case Kind::Subtraction:
            case Kind::Multiplication:
            case Kind::Division:
                if(left_type == right_type)
                    return right_type;
                else return Types::Type::Invalid;
                break;
            case Kind::LogicalAnd:
            case Kind::LogicalOr:
                if(left_type == Types::Type::_bool && right_type == Types::Type::_bool)
                    return Types::Type::_bool;
                else return Types::Type::Invalid;
            case Kind::Equals:
            case Kind::NotEquals:
                if(left_type == right_type)
                    return Types::Type::_bool;
                else return Types::Type::Invalid;
            case Kind::Greater:
            case Kind::GreaterEqual:
            case Kind::Less:
            case Kind::LessEqual:
                if(Types::isNumeric(left_type) && left_type == right_type)
                    return Types::Type::_bool;
                else return Types::Type::Invalid;
                break;
            default: return Types::Type::Invalid;
            }
        }
        Kind m_kind;
        Types::Type m_leftType, m_rightType, m_returnType;
    };

    class BoundBinaryExpression final : public BoundExpression
    {
    public:
        BoundBinaryExpression(std::unique_ptr<const BoundBinaryOperator> _operator, std::unique_ptr<const BoundExpression> left,
            std::unique_ptr<const BoundExpression> right)
            :BoundExpression(_operator->getReturnType()), m_operator(std::move(_operator)), m_left(std::move(left)), m_right(std::move(right))
        {}

        const BoundBinaryOperator* getOperator() const { return m_operator.get(); }
        const BoundExpression* getLeft() const { return m_left.get(); }
        const BoundExpression* getRight() const { return m_right.get(); }
    private:
        std::unique_ptr<const BoundBinaryOperator> m_operator;
        std::unique_ptr<const BoundExpression> m_left, m_right;
    };
}