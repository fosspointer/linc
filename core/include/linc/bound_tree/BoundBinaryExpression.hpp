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
            Equals, NotEquals, Greater, Less, GreaterEqual, LessEqual, Assignment
        };

        BoundBinaryOperator(Kind kind, Types::type left_type, Types::type right_type);

        [[nodiscard]] inline Kind getKind() const { return m_kind; }
        [[nodiscard]] inline const Types::type& getLeftType() const { return m_leftType; }
        [[nodiscard]] inline const Types::type& getRightType() const { return m_rightType; }
        [[nodiscard]] inline const Types::type& getReturnType() const { return m_returnType; }

        std::unique_ptr<const BoundBinaryOperator> cloneConst() const;

        static std::string kindToString(Kind kind);
    private:
        static Types::type getReturnType(Kind kind, Types::type left_type, Types::type right_type);

        const Kind m_kind;
        const Types::type m_leftType, m_rightType, m_returnType;
    };

    class BoundBinaryExpression final : public BoundExpression
    {
    public:
        BoundBinaryExpression(std::unique_ptr<const BoundBinaryOperator> _operator, std::unique_ptr<const BoundExpression> left,
            std::unique_ptr<const BoundExpression> right);

        [[nodiscard]] inline const BoundBinaryOperator* const getOperator() const { return m_operator.get(); }
        [[nodiscard]] inline const BoundExpression* const getLeft() const { return m_left.get(); }
        [[nodiscard]] inline const BoundExpression* const getRight() const { return m_right.get(); }

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        
        const std::unique_ptr<const BoundBinaryOperator> m_operator;
        const std::unique_ptr<const BoundExpression> m_left, m_right;
    };
}