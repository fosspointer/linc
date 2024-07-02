#include <linc/lexer/Operators.hpp>
#include <linc/system/Reporting.hpp>

#define LINC_OPERATOR_MAP_PAIR(first, second) std::pair<std::string, linc::Token::Type>(first, second)
#define LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(first, second) std::pair<linc::Token::Type, linc::Operators::Associativity>(first, second)
#define LINC_OPERATOR_PRECEDENCE_MAP_PAIR(first, second) std::pair<linc::Token::Type, std::uint16_t>(first, second)

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
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAsignmentAddition, 1),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAsignmentSubstraction, 1),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAssignmentMultiplication, 1),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAssignmentDivision, 1),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAssignmentModulo, 1),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalOr, 2),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalAnd, 3),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorBitwiseOr, 4),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorBitwiseXor, 5),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorBitwiseAnd, 6),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorEquals, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorNotEquals, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorGreater, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorGreaterEqual, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLess, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLessEqual, 7),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorBitwiseShiftLeft, 8),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorBitwiseShiftRight, 8),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorPlus, 9),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorMinus, 9),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorAsterisk, 10),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorSlash, 10),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorPercent, 10)
    };

    const Operators::OperatorAssociativityMap Operators::s_operatorAssociativityMap = {
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorAssignment, Operators::Associativity::Right),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorAsignmentAddition, Operators::Associativity::Right),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorAsignmentSubstraction, Operators::Associativity::Right),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorAssignmentMultiplication, Operators::Associativity::Right),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorAssignmentDivision, Operators::Associativity::Right),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorAssignmentModulo, Operators::Associativity::Right),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorLogicalOr, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorLogicalAnd, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorBitwiseOr, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorBitwiseXor, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorBitwiseAnd, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorEquals, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorNotEquals, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorGreater, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorGreaterEqual, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorLess, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorLessEqual, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorBitwiseShiftLeft, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorBitwiseShiftRight, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorPlus, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorMinus, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorAsterisk, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorSlash, Operators::Associativity::Left),
        LINC_OPERATOR_ASSOCIATIVITY_MAP_PAIR(Token::Type::OperatorPercent, Operators::Associativity::Left)
    };

    const Operators::OperatorPrecedenceMap Operators::s_unaryOperatorPrecedenceMap = {
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorStringify, 11),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorBitwiseNot, 11),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorIncrement, 11),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorDecrement, 11),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorMinus, 12),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorPlus, 12),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::OperatorLogicalNot, 12),
        LINC_OPERATOR_PRECEDENCE_MAP_PAIR(Token::Type::Colon, 13)
    };

    const Operators::OperatorMap Operators::s_operatorMap = {
        LINC_OPERATOR_MAP_PAIR("~", Token::Type::Tilde),
        LINC_OPERATOR_MAP_PAIR(":", Token::Type::Colon),
        LINC_OPERATOR_MAP_PAIR(".", Token::Type::Dot),
        LINC_OPERATOR_MAP_PAIR(",", Token::Type::Comma),
        LINC_OPERATOR_MAP_PAIR("#", Token::Type::PreprocessorSpecifier),
        LINC_OPERATOR_MAP_PAIR("##", Token::Type::GlueSpecifier),
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
        LINC_OPERATOR_MAP_PAIR("!!", Token::Type::OperatorBitwiseNot)
    };

    Token::Type Operators::get(const std::string& operator_string)
    {
        auto find = s_operatorMap.find(operator_string);
        
        if(find != s_operatorMap.end())
            return find->second;

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
            .message = linc::Logger::format("Expected operator, found invalid character sequence '$'.", operator_string)});
        
        return Token::Type::InvalidToken;
    }

    auto Operators::getAssociativity(Token::Type operator_token_type) -> Associativity {
        auto find = s_operatorAssociativityMap.find(operator_token_type);

        if(find != s_operatorAssociativityMap.end())
            return find->second;

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Info, .stage = Reporting::Stage::Parser,
            .message = linc::Logger::format("Cannot determine the associativity of token-type '$', as it is not a valid binary operator.", 
                Token::typeToString(operator_token_type))});
        
        return Associativity::Left;
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
