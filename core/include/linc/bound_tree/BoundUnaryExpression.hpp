#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundUnaryOperator final 
    {
    public:
        enum class Kind: char
        {
            Invalid, UnaryPlus, UnaryMinus, LogicalNot, Stringify, Increment, Decrement, Typeof
        };

        BoundUnaryOperator(Kind kind, Types::type operand_type);
        BoundUnaryOperator(Kind kind, Types::type operand_type, Types::type return_type);

        [[nodiscard]] inline Kind getKind() const { return m_kind; }
        [[nodiscard]] inline Types::type getOperandType() const { return m_operandType; }
        [[nodiscard]] inline Types::type getReturnType() const { return m_returnType; }

        std::unique_ptr<const BoundUnaryOperator> cloneConst() const;
        static std::string kindToString(Kind kind);
    private:
        static Types::type getReturnType(Kind operator_kind, Types::type operand_type);

        const Kind m_kind;
        const Types::type m_operandType, m_returnType;
    };

    class BoundUnaryExpression final : public BoundExpression
    {
    public:
        BoundUnaryExpression(std::unique_ptr<const BoundUnaryOperator> _operator, std::unique_ptr<const BoundExpression> operand);

        [[nodiscard]] inline const BoundUnaryOperator* const getOperator() const { return m_operator.get(); }
        [[nodiscard]] inline const BoundExpression* const getOperand() const { return m_operand.get(); }

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        
        const std::unique_ptr<const BoundUnaryOperator> m_operator;
        const std::unique_ptr<const BoundExpression> m_operand;
    };
}