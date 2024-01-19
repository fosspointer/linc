#include <linc/bound_tree/BoundUnaryExpression.hpp>

namespace linc
{
    BoundUnaryOperator::BoundUnaryOperator(Kind kind, Types::Type operand_type)
        :m_kind(kind), m_operandType(operand_type), 
        m_returnType(getReturnType(kind, operand_type))
    {}

    BoundUnaryOperator::BoundUnaryOperator(Kind kind, Types::Type operand_type, Types::Type return_type)
        :m_kind(kind), m_operandType(operand_type), m_returnType(return_type)
    {}

    std::unique_ptr<const BoundUnaryOperator> BoundUnaryOperator::clone_const() const
    {
        return std::make_unique<const BoundUnaryOperator>(m_kind, m_operandType, m_returnType);
    }

    std::string BoundUnaryOperator::kindToString(Kind kind) 
    {
        switch(kind)
        {
        case Kind::Invalid: return "Invalid Unary Operator";
        case Kind::UnaryPlus: return "Unary Plus Operator";
        case Kind::UnaryMinus: return "Unary Negation Operator";
        case Kind::LogicalNot: return "Logical NOT Operator";
        case Kind::Stringify: return "Stringify Operator";
        case Kind::Increment: return "Increment Operator";
        case Kind::Decrement: return "Decrement Operator";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(BoundUnaryOperator::Kind);
        }
    
    }
    Types::Type BoundUnaryOperator::getReturnType(Kind operator_kind, Types::Type operand_type)
    {
        switch(operator_kind)
        {
        case Kind::Stringify:
            return Types::Type::string;
        case Kind::Increment:
        case Kind::Decrement:
            if(Types::isNumeric(operand_type))
                return operand_type;
            else return Types::Type::invalid;
        case Kind::UnaryPlus:
            if(Types::isNumeric(operand_type))
                return operand_type;
            else if(operand_type == Types::Type::_char)
                return Types::Type::i32;
        case Kind::UnaryMinus:
            if(Types::isSigned(operand_type) || Types::isFloating(operand_type))
                return operand_type;
            else return Types::Type::invalid;
        case Kind::LogicalNot:
            if(Types::isNumeric(operand_type) || operand_type == Types::Type::_bool)
                return Types::Type::_bool;
            else return Types::Type::invalid;
        default: return Types::Type::invalid;
        }
    }

    BoundUnaryExpression::BoundUnaryExpression(std::unique_ptr<const BoundUnaryOperator> _operator, std::unique_ptr<const BoundExpression> operand)
        :BoundExpression(_operator->getReturnType()), m_operator(std::move(_operator)), m_operand(std::move(operand))
    {}

    std::unique_ptr<const BoundExpression> BoundUnaryExpression::clone_const() const
    {
        return std::make_unique<const BoundUnaryExpression>(std::move(m_operator->clone_const()), std::move(m_operand->clone_const()));
    }

    std::string BoundUnaryExpression::toStringInner() const
    {
        return linc::Logger::format("Bound Unary Expression (@'$')", BoundUnaryOperator::kindToString(m_operator->getKind()));
    }
}