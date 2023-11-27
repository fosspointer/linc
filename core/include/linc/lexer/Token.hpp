#pragma once
#include <optional>
#include <string>
#include <linc/system/Exception.hpp>
#include <linc/system/Logger.hpp>

namespace linc
{
    struct Token final
    {
    public:
        enum class Type
        {
            // For error handling (the tokenization won't stop on invalid syntax for syntax-highlighting purposes and more...)
            InvalidToken,

            // For convenience
            EndOfFile,
            
            // Generic keywords
            KeywordReturn, KeywordFunction, KeywordIf, KeywordElseIf, KeywordElse, KeywordWhile, KeywordTrue, KeywordFalse,
            
            // Symbols
            ParenthesisLeft, ParenthesisRight, SquareLeft, SquareRight, BraceLeft, BraceRight, ReturnType, Comma,
            
            // Arithmetic operators
            OperatorPlus, OperatorMinus, OperatorAsterisk, OperatorSlash, OperatorPercent,
            OperatorIncrement, OperatorDecrement,
            
            // Assignment 
            OperatorAssignment, 

            // Arithmetic Asignment operators
            OperatorAsignmentAddition, OperatorAsignmentSubstraction, OperatorAssignmentMultiplication, 
            OperatorAssignmentDivision, OperatorAssignmentModulo,
            
            // Comparison operators
            OperatorEquals, OperatorNotEquals, OperatorGreater, OperatorLess, OperatorGreaterEqual, OperatorLessEqual,

            // Logical operators
            OperatorLogicalAnd, OperatorLogicalOr, OperatorLogicalNot,

            // Bitwise operators
            OperatorBitwiseAnd, OperatorBitwiseOr, OperatorBitwiseXor, OperatorBitwiseNot, OperatorBitwiseShiftLeft, OperatorBitwiseShiftRight,
            
            // Integer literal
            I8Literal, I16Literal, I32Literal, I64Literal, U8Literal, U16Literal, U32Literal, U64Literal,
            
            // Floating-point literal
            F32Literal, F64Literal,
            
            // String literal
            StringLiteral,
            
            // Character literal
            CharacterLiteral,

            // Identifier
            Identifier, 
        };

        static std::string typeToString(Type type)
        {
            switch(type)
            {
            case Type::InvalidToken: return "Invalid Token";
            case Type::EndOfFile: return "End of File";
            case Type::KeywordReturn: return "Return Keyword";
            case Type::KeywordFunction: return "Function Keyword";
            case Type::KeywordIf: return "If Keyword";
            case Type::KeywordElseIf: return "Else If Keyword";
            case Type::KeywordElse: return "Else Keyword";
            case Type::KeywordWhile: return "While Keyword";
            case Type::KeywordTrue: return "True Keyword Literal";
            case Type::KeywordFalse: return "False Keyword Literal";
            case Type::ParenthesisLeft: return "Opening Parenthesis";
            case Type::ParenthesisRight: return "Closing Parenthesis";
            case Type::SquareLeft: return "Opening Square Bracket";
            case Type::SquareRight: return "Closing Square Bracket";
            case Type::BraceLeft: return "Opening Curly Brace";
            case Type::BraceRight: return "Closing Curly Brace";
            case Type::ReturnType: return "Return Type";
            case Type::Comma: return "Comma";
            case Type::OperatorPlus: return "Plus Operator";
            case Type::OperatorMinus: return "Minus Operator";
            case Type::OperatorAsterisk: return "Asterisk Operator";
            case Type::OperatorSlash: return "Slash Operator";
            case Type::OperatorPercent: return "Percent Operator";
            case Type::OperatorIncrement: return "Increment Operator";
            case Type::OperatorDecrement: return "Decrement Operator";
            case Type::OperatorAssignment: return "Assignment Operator";
            case Type::OperatorAsignmentAddition: return "Addition Asignment Operator";
            case Type::OperatorAsignmentSubstraction: return "Substraction Asignment Operator";
            case Type::OperatorAssignmentMultiplication: return "Multiplication Assignment Operator";
            case Type::OperatorAssignmentDivision: return "Division Assignment Operator";
            case Type::OperatorAssignmentModulo: return "Modulo Assignment Operator";
            case Type::OperatorEquals: return "Equals Operator";
            case Type::OperatorNotEquals: return "Not Equals Operator";
            case Type::OperatorGreater: return "Greater than Operator";
            case Type::OperatorLess: return "Less than Operator";
            case Type::OperatorGreaterEqual: return "Greater or Equal Operator";
            case Type::OperatorLessEqual: return "Less or Equal Operator";
            case Type::OperatorLogicalAnd: return "Logical And Operator";
            case Type::OperatorLogicalOr: return "Logical Or Operator";
            case Type::OperatorLogicalNot: return "Logical Not Operator";
            case Type::OperatorBitwiseAnd: return "Bitwise And Operator";
            case Type::OperatorBitwiseOr: return "Bitwise Or Operator";
            case Type::OperatorBitwiseXor: return "Bitwise Xor Operator";
            case Type::OperatorBitwiseNot: return "Bitwise Not Operator";
            case Type::OperatorBitwiseShiftLeft: return "Bitwise Shift Left Operator";
            case Type::OperatorBitwiseShiftRight: return "Bitwise Shift Right Operator";
            case Type::I8Literal: return "I8 Literal";
            case Type::I16Literal: return "I16 Literal";
            case Type::I32Literal: return "I32 Literal";
            case Type::I64Literal: return "I64 Literal";
            case Type::U8Literal: return "U8 Literal";
            case Type::U16Literal: return "U16 Literal";
            case Type::U32Literal: return "U32 Literal";
            case Type::U64Literal: return "U64 Literal";
            case Type::F32Literal: return "F32 Literal";
            case Type::F64Literal: return "F64 Literal";
            case Type::CharacterLiteral: return "Character Literal";
            case Type::StringLiteral: return "String Literal";
            case Type::Identifier: return "Identifier";
            default: 
                return Logger::format("????: $", (int)type);
                throw LINC_EXCEPTION_OUT_OF_BOUNDS(Token::Type);
            }
        }

        [[nodiscard]] bool isValid() const
        {
            return type != Type::InvalidToken;
        }

        [[nodiscard]] bool isLiteral() const
        {
            switch(type)
            {
            case Type::I8Literal:
            case Type::I16Literal:
            case Type::I32Literal:
            case Type::I64Literal:
            case Type::U8Literal:
            case Type::U16Literal:
            case Type::U32Literal:
            case Type::U64Literal:
            case Type::F32Literal:
            case Type::F64Literal:
            case Type::CharacterLiteral:
            case Type::StringLiteral:
            case Type::KeywordTrue:
            case Type::KeywordFalse:
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isKeyword() const
        {
            switch(type)
            {
            case Type::KeywordReturn:
            case Type::KeywordFunction:
            case Type::KeywordIf:
            case Type::KeywordElseIf:
            case Type::KeywordElse:
            case Type::KeywordWhile:
            case Type::KeywordTrue:
            case Type::KeywordFalse:
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isSymbol() const
        {
            switch(type)
            {
            case Type::ParenthesisLeft:
            case Type::ParenthesisRight:
            case Type::SquareLeft:
            case Type::SquareRight:
            case Type::BraceLeft:
            case Type::BraceRight:
            case Type::ReturnType:
            case Type::Comma:
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isIdentifier() const
        {
            return type == Type::Identifier;
        }

        [[nodiscard]] bool isEndOfFile() const
        {
            return type == Type::EndOfFile;
        }

        [[nodiscard]] bool isAssignmentOperator() const
        {
            return type == Type::OperatorAssignment;
        }

        [[nodiscard]] bool isArithmeticOperator() const
        {
            switch(type)
            {
                case Type::OperatorPlus:
                case Type::OperatorMinus:
                case Type::OperatorAsterisk:
                case Type::OperatorSlash:
                case Type::OperatorPercent:
                case Type::OperatorIncrement:
                case Type::OperatorDecrement:
                    return true;
                default: return false;
            }
        }

        [[nodiscard]] bool isArithmeticAssignmentOperator() const
        {
            switch(type)
            {
            case Type::OperatorAsignmentAddition:
            case Type::OperatorAsignmentSubstraction:
            case Type::OperatorAssignmentMultiplication: 
            case Type::OperatorAssignmentDivision:
            case Type::OperatorAssignmentModulo:
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isComparisonOperator() const
        {
            switch(type)
            {
            case Type::OperatorEquals:
            case Type::OperatorNotEquals:
            case Type::OperatorGreater:
            case Type::OperatorLess:
            case Type::OperatorGreaterEqual:
            case Type::OperatorLessEqual:
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isLogicalOperator() const
        {
            switch(type)
            {
            case Type::OperatorLogicalAnd: 
            case Type::OperatorLogicalOr: 
            case Type::OperatorLogicalNot: 
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isBitwiseOperator() const
        {
            switch(type)
            {
            case Type::OperatorBitwiseAnd:
            case Type::OperatorBitwiseOr:
            case Type::OperatorBitwiseXor:
            case Type::OperatorBitwiseNot:
            case Type::OperatorBitwiseShiftLeft:
            case Type::OperatorBitwiseShiftRight:
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isOperator() const
        {
            return isBinaryOperator() || isUnaryOperator();
        }

        [[nodiscard]] bool isBinaryOperator() const
        {
            switch(type)
            {
            case Type::OperatorPlus:
            case Type::OperatorMinus:
            case Type::OperatorAsterisk:
            case Type::OperatorSlash:
            case Type::OperatorPercent:
            case Type::OperatorAssignment:
            case Type::OperatorAsignmentAddition:
            case Type::OperatorAsignmentSubstraction:
            case Type::OperatorAssignmentMultiplication:
            case Type::OperatorAssignmentDivision:
            case Type::OperatorAssignmentModulo:
            case Type::OperatorEquals:
            case Type::OperatorNotEquals:
            case Type::OperatorGreater:
            case Type::OperatorLess:
            case Type::OperatorGreaterEqual:
            case Type::OperatorLessEqual:
            case Type::OperatorLogicalAnd:
            case Type::OperatorLogicalOr:
            case Type::OperatorBitwiseAnd:
            case Type::OperatorBitwiseOr:
            case Type::OperatorBitwiseXor:
            case Type::OperatorBitwiseShiftLeft:
            case Type::OperatorBitwiseShiftRight:
                return true;
            default: return false;
            }
        }

        [[nodiscard]] bool isUnaryOperator() const
        {
            switch(type)
            {
            case Type::OperatorPlus:
            case Type::OperatorMinus:
            case Type::OperatorIncrement:
            case Type::OperatorDecrement:
            case Type::OperatorLogicalNot:
            case Type::OperatorBitwiseNot:
                return true;
            default: return false;
            }
        }

        Type type;
        std::optional<std::string> value;
        size_t lineNumber;
    };
}