#pragma once
#include <optional>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <linc/lexer/Token.hpp>
#include <linc/system/Types.hpp>

namespace linc
{
    class Operators final
    {
    public:
        using OperatorMap = std::unordered_map<std::string, Token::Type>;
        using BinaryPrecedenceMap = std::unordered_map<Token::Type, std::uint16_t>;
        using UnaryPrecedenceMap = std::unordered_map<Token::Type, std::uint16_t>;
        
        static Token::Type get(const std::string& operator_string);
        static uint16_t getBinaryPrecedence(Token::Type operator_token_type);
        static uint16_t getUnaryPrecedence(Token::Type operator_token_type);
    private:
        static const OperatorMap s_OperatorMap;
        static const UnaryPrecedenceMap s_UnaryOperatorPrecedenceMap;
        static const BinaryPrecedenceMap s_BinaryOperatorPrecedenceMap;
    };
}