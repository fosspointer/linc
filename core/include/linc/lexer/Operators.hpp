#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Operators final
    {
    public:
        enum class Associativity: bool {
            Left, Right
        };

        using OperatorMap = std::unordered_map<std::string, Token::Type>;
        using OperatorPrecedenceMap = std::unordered_map<Token::Type, std::uint16_t>;
        using OperatorAssociativityMap = std::unordered_map<Token::Type, Associativity>;
        
        static Token::Type get(const std::string& operator_string);
        static Associativity getAssociativity(Token::Type operator_token_type);
        static uint16_t getBinaryPrecedence(Token::Type operator_token_type);
        static uint16_t getUnaryPrecedence(Token::Type operator_token_type);
    private:
        static const OperatorMap s_operatorMap;
        static const OperatorAssociativityMap s_operatorAssociativityMap;
        static const OperatorPrecedenceMap s_unaryOperatorPrecedenceMap, s_binaryOperatorPrecedenceMap;
    };
}