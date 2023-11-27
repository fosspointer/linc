#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundUnaryOperator final 
    {
    public:
        enum class Kind: char
        {
            Invalid, UnaryPlus, UnaryMinus, LogicalNot
        };

        BoundUnaryOperator(Kind kind, Types::Type operand_type)
            :m_kind(kind), m_operandType(operand_type), 
            m_returnType(getReturnType(kind, operand_type))
        {}

        BoundUnaryOperator(Kind kind, Types::Type operand_type, Types::Type return_type)
            :m_kind(kind), m_operandType(operand_type), m_returnType(return_type)
        {}

        inline Kind getKind() const { return m_kind; }
        inline Types::Type getOperandType() const { return m_operandType; }
        inline Types::Type getReturnType() const { return m_returnType; }

        static std::string kindToString(Kind kind) 
        {
            switch(kind)
            {
            case Kind::Invalid: return "Invalid Unary Operator";
            case Kind::UnaryPlus: return "Unary Plus Operator";
            case Kind::UnaryMinus: return "Unary Negation Operator";
            case Kind::LogicalNot: return "Logical NOT Operator";
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(BoundUnaryOperator::Kind);
            }
        }
    private:
        static Types::Type getReturnType(Kind operator_kind, Types::Type operand_type)
        {
            switch(operator_kind)
            {
            case Kind::UnaryPlus:
                if(Types::isNumeric(operand_type))
                    return operand_type;
                else if(operand_type == Types::Type::_char)
                    return Types::Type::i32;
            case Kind::UnaryMinus:
                if(Types::isSigned(operand_type) || Types::isFloating(operand_type))
                    return operand_type;
                else return Types::Type::Invalid;
            case Kind::LogicalNot:
                if(Types::isNumeric(operand_type) || operand_type == Types::Type::_bool)
                    return Types::Type::_bool;
                else return Types::Type::Invalid;
            default: return Types::Type::Invalid;
            }
        }

        Kind m_kind;
        Types::Type m_operandType, m_returnType;
    };

    class BoundUnaryExpression final : public BoundExpression
    {
    public:
        BoundUnaryExpression(BoundUnaryOperator* _operator, BoundExpression* operand)
            :BoundExpression(_operator->getReturnType()), m_operator(_operator), m_operand(operand)
        {}  

        inline const BoundUnaryOperator* getOperator() const { return m_operator; }
        inline const BoundExpression* getOperand() const { return m_operand; }
    private:
        BoundUnaryOperator* m_operator;
        BoundExpression* m_operand;
    };
}