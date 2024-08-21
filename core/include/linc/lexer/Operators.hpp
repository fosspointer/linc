#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Token representation of operators, operator precedence and associativity mapping.
    class Operators final
    {
    public:
        enum class Associativity: bool {
            Left, Right
        };

        using OperatorMap = std::unordered_map<std::string, Token::Type>;
        using OperatorPrecedenceMap = std::unordered_map<Token::Type, std::uint16_t>;
        using OperatorAssociativityMap = std::unordered_map<Token::Type, Associativity>;
        
        /// @brief Get the corresponding token-type representation of an operator in string representation (or invalid). 
        static Token::Type get(const std::string& operator_string, const Token::Info& token_info);

        /// @brief Get the associativity of a specified operator token type (left or right associative). Otherwise, report an error.
        static Associativity getAssociativity(Token::Type operator_token_type);

        /// @brief Get the precedence of a specified binary operator token type.
        static uint16_t getBinaryPrecedence(Token::Type operator_token_type);

        /// @brief Get the precedence of a specified unary operator token type.
        static uint16_t getUnaryPrecedence(Token::Type operator_token_type);
    private:
        static const OperatorMap s_operatorMap;
        static const OperatorAssociativityMap s_operatorAssociativityMap;
        static const OperatorPrecedenceMap s_unaryOperatorPrecedenceMap, s_binaryOperatorPrecedenceMap;
    };
}