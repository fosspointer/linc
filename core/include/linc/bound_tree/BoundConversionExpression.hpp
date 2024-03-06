#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundConversion final 
    {
    public:
        BoundConversion(Types::type initial_type, Types::type target_type);

        std::unique_ptr<const BoundConversion> cloneConst() const;

        [[nodiscard]] inline const Types::type& getInitialType() const { return m_initialType; }
        [[nodiscard]] inline const Types::type& getReturnType() const { return m_returnType; }
    private:
        static Types::type getReturnType(Types::type initial_type, Types::type target_type);
        Types::type m_initialType, m_returnType;
    };

    class BoundConversionExpression final : public BoundExpression
    {
    public:
        BoundConversionExpression(std::unique_ptr<const BoundExpression> expression, std::unique_ptr<const BoundConversion> conversion);

        [[nodiscard]] inline const BoundExpression* const getExpression() const { return m_expression.get(); }    
        [[nodiscard]] inline const BoundConversion* const getConversion() const { return m_conversion.get(); }    
        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        std::unique_ptr<const BoundExpression> m_expression;
        std::unique_ptr<const BoundConversion> m_conversion;
    };
}