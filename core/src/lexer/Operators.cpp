#include <linc/lexer/Operators.hpp>
#include <linc/system/Reporting.hpp>

#define LINC_OPERATOR_MAP_PAIR(first, second) std::pair<std::string, linc::Token::Type>(first, second)
#define LINC_OPERATOR_PRECEDENCE_MAP_PAIR(first, second) std::pair<Token::Type, uint16_t>(first, second)

struct UnaryOperator
{
    linc::Token::Type operatorToken;
    linc::Types::Type operandType;
    linc::Types::Type returnType;
};

struct BinaryOperator
{
    linc::Token::Type operatorToken;
    linc::Types::Type leftType;
    linc::Types::Type rightType;
    linc::Types::Type returnType;
};

namespace linc
{
    const Operators::BinaryPrecedenceMap Operators::s_BinaryOperatorPrecedenceMap = {
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalAnd, 2),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalOr, 1),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorEquals, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorNotEquals, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorGreater, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorGreaterEqual, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLess, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLessEqual, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorPlus, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorMinus, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAsterisk, 5),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorSlash, 5),
    };

    const Operators::UnaryPrecedenceMap Operators::s_UnaryOperatorPrecedenceMap = {
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorStringify, 6),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorIncrement, 6),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorDecrement, 6),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorMinus, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorPlus, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalNot, 7),
    };

    const Operators::OperatorMap Operators::s_OperatorMap = {
        LINC_OPERATOR_MAP_PAIR(":", Token::Type::ReturnType),
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
        auto find = s_OperatorMap.find(operator_string);
        if(find != s_OperatorMap.end())
            return find->second;

        Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                        .message = linc::Logger::format("Expected operator, found invalid character sequence '$'", operator_string)});
        
        return Token::Type::InvalidToken;
    }

    uint16_t Operators::getBinaryPrecedence(Token::Type operator_token_type)
    {
        auto find = s_BinaryOperatorPrecedenceMap.find(operator_token_type);
            if(find != s_BinaryOperatorPrecedenceMap.end())
                return find->second;

            Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
                            .message = linc::Logger::format("Cannot determine the precedence of token-type '$', as it is not a valid binary operator", 
                        Token::typeToString(operator_token_type))});
            
            return 0;
    }

    uint16_t Operators::getUnaryPrecedence(Token::Type operator_token_type)
    {
        auto find = s_UnaryOperatorPrecedenceMap.find(operator_token_type);
            if(find != s_UnaryOperatorPrecedenceMap.end())
                return find->second;

            Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
                            .message = linc::Logger::format("Cannot determine the precedence of token-type '$', as it is not a valid binary operator",
                        Token::typeToString(operator_token_type))});
            
            return 0;
    }
}