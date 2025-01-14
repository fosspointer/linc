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

    std::unique_ptr<const BoundUnaryOperator> BoundUnaryOperator::clone() const
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
        case Kind::BitwiseNot: return "Bitwise NOT Operator";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
        }
    
    }
    Types::type BoundUnaryOperator::getReturnType(Kind operator_kind, Types::type operand_type)
    {
        if(operator_kind == Kind::Stringify)
            return Types::fromKind(Types::Kind::string);
        else if(operator_kind == Kind::Typeof)
            return Types::fromKind(Types::Kind::type);
        
        if(operand_type.kind == Types::type::Kind::Array)
        {
            if(operator_kind == Kind::UnaryPlus)
                return Types::fromKind(Types::Kind::u64);
            else return Types::invalidType;
        }
        else if(operand_type.kind == Types::type::Kind::Structure)
        {
            if(operand_type.structure.size() != 3ul || operator_kind != Kind::UnaryMinus)
                return Types::invalidType; 

            auto begin_type_mutable = operand_type.structure[0ul].first;
            begin_type_mutable.isMutable = true;
            if(begin_type_mutable.isCompatible(operand_type.structure[1ul].first)
                && BoundUnaryOperator(BoundUnaryOperator::Kind::Increment, begin_type_mutable).getReturnType() != Types::invalidType
                && operand_type.structure[2ul].first == Types::fromKind(Types::Kind::_bool))
                return operand_type;
        }

        switch(operator_kind)
        {
        case Kind::Increment:
        case Kind::Decrement:
            if(!operand_type.isMutable)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                    .message = "Cannot use unary increment/decrement operators on non-mutable operand."
                });
                return Types::invalidType;
            }
            if(Types::isNumeric(operand_type.primitive))
                return operand_type;
            else return Types::invalidType;
        case Kind::UnaryPlus:
            if(Types::isNumeric(operand_type.primitive))
                return Types::fromKind(operand_type.primitive);
            else if(operand_type.primitive == Types::Kind::_char || operand_type.primitive == Types::Kind::_bool)
                return Types::fromKind(Types::Kind::i32);
            else if(operand_type.primitive == Types::Kind::string)
                return Types::fromKind(Types::Kind::u64);
            else return Types::invalidType;
        case Kind::UnaryMinus:
            if(Types::isSigned(operand_type.primitive) || Types::isFloating(operand_type.primitive))
                return operand_type;
            else return Types::invalidType;
        case Kind::LogicalNot:
            if(Types::isIntegral(operand_type.primitive) || operand_type.primitive == Types::Kind::_bool)
                return Types::fromKind(Types::Kind::_bool);
            else return Types::invalidType;
        case Kind::BitwiseNot:
            if(Types::isIntegral(operand_type.primitive) || operand_type.primitive == Types::Kind::_bool)
                return Types::fromKind(operand_type.primitive);
            else return Types::invalidType;
        default: return Types::invalidType;
        }
    }

    BoundUnaryExpression::BoundUnaryExpression(std::unique_ptr<const BoundUnaryOperator> _operator, std::unique_ptr<const BoundExpression> operand)
        :BoundExpression(_operator->getReturnType()), m_operator(std::move(_operator)), m_operand(std::move(operand))
    {}

    std::unique_ptr<const BoundExpression> BoundUnaryExpression::clone() const
    {
        return std::make_unique<const BoundUnaryExpression>(std::move(m_operator->clone()), std::move(m_operand->clone()));
    }

    std::string BoundUnaryExpression::toStringInner() const
    {
        return linc::Logger::format("Unary Expression (@$:$:$)", Colors::toANSI(Colors::Color::Green),
            BoundUnaryOperator::kindToString(m_operator->getKind()), Colors::toANSI(Colors::getCurrentColor()));
    }
}
