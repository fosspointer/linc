#include <linc/bound_tree/BoundConversionExpression.hpp>

namespace linc
{
    BoundConversion::BoundConversion(Types::type initial_type, Types::type target_type)
        :m_initialType(initial_type), m_returnType(getReturnType(initial_type, target_type))
    {}

    std::unique_ptr<const BoundConversion> BoundConversion::cloneConst() const
    {
        return std::make_unique<const BoundConversion>(m_initialType, m_returnType);
    }

    Types::type BoundConversion::getReturnType(Types::type initial_type, Types::type target_type)
    {
        if(Types::isNumeric(initial_type.kind) && Types::isNumeric(target_type.kind))
            return target_type;
        
        else if(initial_type.kind == Types::Kind::_char && Types::isNumeric(target_type.kind))
            return target_type;

        else return Types::invalidType;
    }

    BoundConversionExpression::BoundConversionExpression(std::unique_ptr<const BoundExpression> expression,
        std::unique_ptr<const BoundConversion> conversion)
        :BoundExpression(conversion->getReturnType()), m_expression(std::move(expression)), m_conversion(std::move(conversion))
    {}
        
    std::unique_ptr<const BoundExpression> BoundConversionExpression::cloneConst() const
    {
        return std::make_unique<const BoundConversionExpression>(m_expression->cloneConst(), m_conversion->cloneConst());
    }

    std::string BoundConversionExpression::toStringInner() const
    {
        return "Bound Conversion Expression";
    }
}