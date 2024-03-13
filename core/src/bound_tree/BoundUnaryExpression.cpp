#include <linc/bound_tree/BoundUnaryExpression.hpp>

namespace linc
{
    BoundUnaryOperator::BoundUnaryOperator(Kind kind, Types::type operand_type)
        :m_kind(kind), m_operandType(operand_type), 
        m_returnType(getReturnType(kind, operand_type))
    {}

    BoundUnaryOperator::BoundUnaryOperator(Kind kind, Types::type operand_type, Types::type return_type)
        :m_kind(kind), m_operandType(operand_type), m_returnType(return_type)
    {}

    std::unique_ptr<const BoundUnaryOperator> BoundUnaryOperator::cloneConst() const
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
        case Kind::Typeof: return "Typeof Operator";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(BoundUnaryOperator::Kind);
        }
    
    }
    Types::type BoundUnaryOperator::getReturnType(Kind operator_kind, Types::type operand_type)
    {
        if(operator_kind == Kind::Stringify)
            return Types::fromKind(Types::Kind::string);
        else if(operator_kind == Kind::Typeof)
            return Types::fromKind(Types::Kind::type);
        
        if(operand_type.isArray)
        {
            if(operator_kind == Kind::UnaryPlus)
                return Types::fromKind(Types::Kind::u64);
            else return Types::invalidType;
        }

        switch(operator_kind)
        {
        case Kind::Increment:
        case Kind::Decrement:
            if(Types::isNumeric(operand_type.kind))
                return operand_type;
            else return Types::invalidType;
        case Kind::UnaryPlus:
            if(Types::isNumeric(operand_type.kind))
                return operand_type;
            else if(operand_type.kind == Types::Kind::_char)
                return Types::fromKind(Types::Kind::i32);
            else if(operand_type.kind == Types::Kind::string)
                return Types::fromKind(Types::Kind::u64);
            else return Types::invalidType;
        case Kind::UnaryMinus:
            if(Types::isSigned(operand_type.kind) || Types::isFloating(operand_type.kind))
                return operand_type;
            else return Types::invalidType;
        case Kind::LogicalNot:
            if(Types::isNumeric(operand_type.kind) || operand_type.kind == Types::Kind::_bool)
                return Types::fromKind(Types::Kind::_bool);
            else return Types::invalidType;
        default: return Types::invalidType;
        }
    }

    BoundUnaryExpression::BoundUnaryExpression(std::unique_ptr<const BoundUnaryOperator> _operator, std::unique_ptr<const BoundExpression> operand)
        :BoundExpression(_operator->getReturnType()), m_operator(std::move(_operator)), m_operand(std::move(operand))
    {}

    std::unique_ptr<const BoundExpression> BoundUnaryExpression::cloneConst() const
    {
        return std::make_unique<const BoundUnaryExpression>(std::move(m_operator->cloneConst()), std::move(m_operand->cloneConst()));
    }

    std::string BoundUnaryExpression::toStringInner() const
    {
        return linc::Logger::format("Bound Unary Expression (@'$')", BoundUnaryOperator::kindToString(m_operator->getKind()));
    }
}