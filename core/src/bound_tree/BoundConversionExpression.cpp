#include <linc/bound_tree/BoundConversionExpression.hpp>

namespace linc
{
    BoundConversion::BoundConversion(Types::type initial_type, Types::type target_type)
        :m_initialType(initial_type), m_returnType(getReturnType(initial_type, target_type))
    {}

    std::unique_ptr<const BoundConversion> BoundConversion::clone() const
    {
        return std::make_unique<const BoundConversion>(m_initialType, m_returnType);
    }

    Types::type BoundConversion::getReturnType(Types::type initial_type, Types::type target_type)
    {
        if(initial_type.kind != Types::type::Kind::Primitive || target_type.kind != Types::type::Kind::Primitive)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                .message = "Cannot convert to/from non-primitive type."
            });
            return Types::invalidType;
        }

        else if(initial_type.isMutable != target_type.isMutable)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                .message = "Cannot convert between mutable and non-mutable types."
            });
            return Types::invalidType;
        }

        if(Types::isNumeric(initial_type.primitive) && Types::isNumeric(target_type.primitive))
            return target_type;
        
        else if(Types::isNumeric(initial_type.primitive) && target_type.primitive == Types::Kind::_char)
            return target_type; 
            
        else if(initial_type.primitive == Types::Kind::_char && Types::isNumeric(target_type.primitive))
            return target_type;

        else if(initial_type.primitive == Types::Kind::u8 && target_type.primitive == Types::Kind::_char)
            return target_type;

        Reporting::push(Reporting::Report{
                .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                .message = "Type conversion was given incompatible types."
            });
        return Types::invalidType;
    }

    BoundConversionExpression::BoundConversionExpression(std::unique_ptr<const BoundExpression> expression,
        std::unique_ptr<const BoundConversion> conversion)
        :BoundExpression(conversion->getReturnType()), m_expression(std::move(expression)), m_conversion(std::move(conversion))
    {}
        
    std::unique_ptr<const BoundExpression> BoundConversionExpression::clone() const
    {
        return std::make_unique<const BoundConversionExpression>(m_expression->clone(), m_conversion->clone());
    }

    std::string BoundConversionExpression::toStringInner() const
    {
        return "Conversion Expression";
    }
}