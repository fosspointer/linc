#include <linc/bound_tree/BoundBinaryExpression.hpp>

namespace linc
{
    BoundBinaryOperator::BoundBinaryOperator(Kind kind, Types::type left_type, Types::type right_type)
        :m_kind(kind), m_leftType(left_type), m_rightType(right_type), 
        m_returnType(getReturnType(kind, left_type, right_type))
    {}

    std::unique_ptr<const BoundBinaryOperator> BoundBinaryOperator::clone() const 
    {
        return std::make_unique<const BoundBinaryOperator>(m_kind, m_leftType, m_rightType);
    }

    std::string BoundBinaryOperator::kindToString(Kind kind) 
    {
        switch(kind)
        {
        case Kind::Invalid: return "Invalid Binary Operator";
        case Kind::Addition: return "Arithmetic Addition Operator";
        case Kind::Subtraction: return "Arithmetic Subtraction Operator";
        case Kind::Multiplication: return "Arithmetic Multiplication Operator";
        case Kind::Division: return "Arithmetic Division Operator";
        case Kind::Modulo: return "Modulo Operator";
        case Kind::LogicalAnd: return "Logical AND Operator";
        case Kind::LogicalOr: return "Logical OR Operator";
        case Kind::Equals: return "Relational Equality Operator";
        case Kind::NotEquals: return "Relational Inequality Operator";
        case Kind::Greater: return "Relational Greater-Than Operator";
        case Kind::Less: return "Relational Less-Than Operator";
        case Kind::GreaterEqual: return "Relational Greater-or-Equal Operator";
        case Kind::LessEqual: return "Relational Less-or-Equal Operator";
        case Kind::Assignment: return "Assignment Operator";
        case Kind::AdditionAssignment: return "Addition Assignment Operator";
        case Kind::SubtractionAssignment: return "Subtraction Assignment Operator";
        case Kind::MultiplicationAssignment: return "Multiplication Assignment Operator";
        case Kind::DivisionAssignment: return "Division Assignment Operator";
        case Kind::ModuloAssignment: return "Modulo Assignment Operator";
        case Kind::BitwiseAnd: return "Bitwise AND Operator";
        case Kind::BitwiseOr: return "Bitwise OR Operator";
        case Kind::BitwiseXor: return "Bitwise XOR Operator";
        case Kind::BitwiseShiftLeft: return "Left-Shift Operator";
        case Kind::BitwiseShiftRight: return "Right-Shift Operator";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
        }
    }

    Types::type BoundBinaryOperator::getReturnType(Kind kind, Types::type left_type, Types::type right_type)
    {
        if(left_type.kind != right_type.kind)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("Cannot call operator '$' on incompatible operand pair. ('$' and '$')",
                    kindToString(kind), left_type.toString(), right_type.toString())
            });
            return Types::invalidType;
        }
        else if(kind == Kind::Equals || kind == Kind::NotEquals)
            return left_type.isCompatible(right_type)? Types::fromKind(Types::Kind::_bool): Types::invalidType;
        else if(kind == Kind::Assignment)
            return left_type.isMutable && left_type.isAssignableTo(right_type)? left_type: Types::invalidType;
        else if(left_type.kind == Types::type::Kind::Array)
        {
            switch (kind)
            {
            case Kind::Addition:
                if(left_type.array.baseType->isCompatible(*right_type.array.baseType))
                    return Types::type(Types::type::Array{
                        .baseType = std::make_unique<const Types::type>(*left_type.array.baseType),
                        .count = left_type.array.count && right_type.array.count? std::make_optional(*left_type.array.count + *right_type.array.count): std::nullopt
                    });
                else return Types::invalidType;
            case Kind::AdditionAssignment:
                if(left_type.array.baseType->isAssignableTo(*right_type.array.baseType) && left_type.isMutable && !left_type.array.count.has_value())
                    return Types::type(Types::type::Array{
                        .baseType = std::make_unique<const Types::type>(*left_type.array.baseType),
                        .count = left_type.array.count && right_type.array.count? std::make_optional(*left_type.array.count + *right_type.array.count): std::nullopt
                    });
            default: return Types::invalidType;
            }
        }
        else if(left_type.kind == Types::type::Kind::Structure)
            return Types::invalidType;

        switch(kind)
        {
        case Kind::Addition:
            if((left_type.primitive == Types::Kind::_char || left_type.primitive == Types::Kind::string)
            && (right_type.primitive == Types::Kind::_char || right_type.primitive == Types::Kind::string))
                return Types::fromKind(Types::Kind::string);
            else if(Types::isNumeric(left_type.primitive) && left_type.primitive == right_type.primitive)
                return Types::fromKind(right_type.primitive);
            else return Types::invalidType;
        case Kind::Subtraction:
        case Kind::Multiplication:
        case Kind::Division:
        case Kind::Modulo:
            if(Types::isNumeric(left_type.primitive) && left_type.primitive == right_type.primitive)
                return Types::fromKind(right_type.primitive);
            else return Types::invalidType;
        case Kind::LogicalAnd:
        case Kind::LogicalOr:
            if(left_type.primitive == Types::Kind::_bool && right_type.primitive == Types::Kind::_bool)
                return Types::fromKind(Types::Kind::_bool);
            else return Types::invalidType;
        case Kind::Greater:
        case Kind::GreaterEqual:
        case Kind::Less:
        case Kind::LessEqual:
            if(Types::isNumeric(left_type.primitive) && left_type.primitive == right_type.primitive)
                return Types::fromKind(Types::Kind::_bool);
            else return Types::invalidType;
        case Kind::Assignment:
            if(!left_type.isMutable)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                    .message = "Cannot use assignment operator on non-mutable operand."
                });
                return Types::invalidType;
            }
            else if(left_type.primitive == right_type.primitive)
                return left_type;
            else return Types::invalidType;
        case Kind::AdditionAssignment:
            if((left_type.primitive == Types::Kind::_char || left_type.primitive == Types::Kind::string)
            && (right_type.primitive == Types::Kind::_char || right_type.primitive == Types::Kind::string) && left_type.isMutable)
                return Types::type(Types::Kind::string, true);
        case Kind::SubtractionAssignment:
        case Kind::MultiplicationAssignment:
        case Kind::DivisionAssignment:
        case Kind::ModuloAssignment:
            if(!left_type.isMutable)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Info, .stage = Reporting::Stage::ABT,
                    .message = "Cannot use arithmetic assignment operator on non-mutable operand."
                });
                return Types::invalidType;
            }

            if(!Types::isNumeric(left_type.primitive) || left_type.primitive != right_type.primitive)
                return Types::invalidType;
            else return Types::type(left_type.primitive, true);
        case Kind::BitwiseAnd:
        case Kind::BitwiseOr:
        case Kind::BitwiseXor:
            if(left_type.primitive == Types::Kind::_bool && right_type.primitive == Types::Kind::_bool)
                return Types::fromKind(Types::Kind::_bool);
            
            else if(Types::isIntegral(left_type.primitive) && left_type.primitive == right_type.primitive)
                return Types::fromKind(left_type.primitive);

            else return Types::invalidType;
        case Kind::BitwiseShiftLeft:
        case Kind::BitwiseShiftRight:
            if(Types::isIntegral(left_type.primitive) && right_type.primitive == Types::Kind::u8)
                return Types::fromKind(left_type.primitive);
            else return Types::invalidType;
        default: return Types::invalidType;
        }
    }

    BoundBinaryExpression::BoundBinaryExpression(std::unique_ptr<const BoundBinaryOperator> _operator, std::unique_ptr<const BoundExpression> left,
        std::unique_ptr<const BoundExpression> right)
        :BoundExpression(_operator->getReturnType()), m_operator(std::move(_operator)), m_left(std::move(left)), m_right(std::move(right))
    {}

    std::unique_ptr<const BoundExpression> BoundBinaryExpression::clone() const
    {
        return std::make_unique<const BoundBinaryExpression>(m_operator->clone(), m_left->clone(), m_right->clone());
    }

    std::string BoundBinaryExpression::toStringInner() const
    {
        return linc::Logger::format("Binary Expression (@$:$:$)", Colors::toANSI(Colors::Color::Green),
            BoundBinaryOperator::kindToString(m_operator->getKind()), Colors::toANSI(Colors::getCurrentColor()));
    }
}
