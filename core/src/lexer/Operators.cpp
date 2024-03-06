#include <linc/lexer/Operators.hpp>
#include <linc/system/Reporting.hpp>

#define LINC_OPERATOR_MAP_PAIR(first, second) std::pair<std::string, linc::Token::Type>(first, second)
#define LINC_OPERATOR_PRECEDENCE_MAP_PAIR(first, second) std::pair<Token::Type, std::uint16_t>(first, second)

struct UnaryOperator
{
    linc::Token::Type operatorToken;
    linc::Types::Kind operandType;
    linc::Types::Kind returnType;
};

struct BinaryOperator
{
    linc::Token::Type operatorToken;
    linc::Types::Kind leftType;
    linc::Types::Kind rightType;
    linc::Types::Kind returnType;
};

namespace linc
{
    const Operators::OperatorPrecedenceMap Operators::s_binaryOperatorPrecedenceMap = {
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAssignment, 1),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalOr, 2),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalAnd, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorEquals, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorNotEquals, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorGreater, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorGreaterEqual, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLess, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLessEqual, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorPlus, 5),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorMinus, 5),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAsterisk, 6),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorSlash, 6),
    };

    const Operators::OperatorPrecedenceMap Operators::s_unaryOperatorPrecedenceMap = {
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorStringify, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorIncrement, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorDecrement, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorMinus, 8),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorPlus, 8),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalNot, 8),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::TypeSpecifier, 9),
    };

    const Operators::OperatorMap Operators::s_operatorMap = {
        LINC_OPERATOR_MAP_PAIR(":", Token::Type::TypeSpecifier),
        LINC_OPERATOR_MAP_PAIR("+", Token::Type::OperatorPlus),
        LINC_OPERATOR_MAP_PAIR("++", Token::Type::OperatorIncrement),
        LINC_OPERATOR_MAP_PAIR("--", Token::Type::OperatorDecrement),
        LINC_OPERATOR_MAP_PAIR("-", Token::Type::OperatorMinus),
        LINC_OPERATOR_MAP_PAIR("*", Token::Type::OperatorAsterisk),
        LINC_OPERATOR_MAP_PAIR("/", Token::Type::OperatorSlash),
        LINC_OPERATOR_MAP_PAIR("%", Token::Type::OperatorPercent),
        LINC_OPERATOR_MAP_PAIR("+=", Token::Type::OperatorAsignmentAddition),
        LINC_OPERATOR_MAP_PAIR("-=", Token::Type::OperatorAsignmentSubstraction),
        LINC_OPERATOR_MAP_PAIR("*=", Token::Type::OperatorAssignmentMultiplication),
        LINC_OPERATOR_MAP_PAIR("/=", Token::Type::OperatorAssignmentDivision),
        LINC_OPERATOR_MAP_PAIR("%=", Token::Type::OperatorAssignmentModulo),
        LINC_OPERATOR_MAP_PAIR("@", Token::Type::OperatorStringify),
        LINC_OPERATOR_MAP_PAIR("=", Token::Type::OperatorAssignment),
        LINC_OPERATOR_MAP_PAIR("==", Token::Type::OperatorEquals),
        LINC_OPERATOR_MAP_PAIR("!=", Token::Type::OperatorNotEquals),
        LINC_OPERATOR_MAP_PAIR(">", Token::Type::OperatorGreater),
        LINC_OPERATOR_MAP_PAIR("<", Token::Type::OperatorLess),
        LINC_OPERATOR_MAP_PAIR(">=", Token::Type::OperatorGreaterEqual),
        LINC_OPERATOR_MAP_PAIR("<=", Token::Type::OperatorLessEqual),
        LINC_OPERATOR_MAP_PAIR("&&", Token::Type::OperatorLogicalAnd),
        LINC_OPERATOR_MAP_PAIR("!", Token::Type::OperatorLogicalNot),
        LINC_OPERATOR_MAP_PAIR("||", Token::Type::OperatorLogicalOr),
        LINC_OPERATOR_MAP_PAIR("<<", Token::Type::OperatorBitwiseShiftLeft),
        LINC_OPERATOR_MAP_PAIR(">>", Token::Type::OperatorBitwiseShiftRight),
        LINC_OPERATOR_MAP_PAIR("&", Token::Type::OperatorBitwiseAnd),
        LINC_OPERATOR_MAP_PAIR("|", Token::Type::OperatorBitwiseOr),
        LINC_OPERATOR_MAP_PAIR("^", Token::Type::OperatorBitwiseXor),
        LINC_OPERATOR_MAP_PAIR("~", Token::Type::OperatorBitwiseNot),
    };

    Token::Type Operators::get(const std::string& operator_string)
    {
        auto find = s_operatorMap.find(operator_string);
        
        if(find != s_operatorMap.end())
            return find->second;

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
            .message = linc::Logger::format("Expected operator, found invalid character sequence '$'", operator_string)});
        
        return Token::Type::InvalidToken;
    }

    uint16_t Operators::getBinaryPrecedence(Token::Type operator_token_type)
    {
        auto find = s_binaryOperatorPrecedenceMap.find(operator_token_type);

        if(find != s_binaryOperatorPrecedenceMap.end())
            return find->second;

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
            .message = linc::Logger::format("Cannot determine the precedence of token-type '$', as it is not a valid binary operator.", 
                Token::typeToString(operator_token_type))});
        
        return 0;
    }

    uint16_t Operators::getUnaryPrecedence(Token::Type operator_token_type)
    {
        auto find = s_unaryOperatorPrecedenceMap.find(operator_token_type);

        if(find != s_unaryOperatorPrecedenceMap.end())
            return find->second;

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
            .message = linc::Logger::format("Cannot determine the precedence of token-type '$', as it is not a valid binary operator.",
                Token::typeToString(operator_token_type))});
        
        return 0;
    }
}