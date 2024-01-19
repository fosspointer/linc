#include <linc/lexer/Token.hpp>

namespace linc
{
    std::string Token::typeToString(Type type)
    {
        switch(type)
        {
        case Type::InvalidToken: return "Invalid Token";
        case Type::EndOfFile: return "End of File";
        case Type::KeywordReturn: return "Return Keyword";
        case Type::KeywordFunction: return "Function Keyword";
        case Type::KeywordIf: return "If Keyword";
        case Type::KeywordElse: return "Else Keyword";
        case Type::KeywordWhile: return "While Keyword";
        case Type::KeywordTrue: return "True Keyword Literal";
        case Type::KeywordFalse: return "False Keyword Literal";
        case Type::KeywordMutability: return "Mutability Keyword";
        case Type::ParenthesisLeft: return "Opening Parenthesis";
        case Type::ParenthesisRight: return "Closing Parenthesis";
        case Type::SquareLeft: return "Opening Square Bracket";
        case Type::SquareRight: return "Closing Square Bracket";
        case Type::BraceLeft: return "Opening Curly Brace";
        case Type::BraceRight: return "Closing Curly Brace";
        case Type::OperatorType: return "Type Operator";
        case Type::Comma: return "Comma";
        case Type::OperatorPlus: return "Plus Operator";
        case Type::OperatorMinus: return "Minus Operator";
        case Type::OperatorAsterisk: return "Asterisk Operator";
        case Type::OperatorSlash: return "Slash Operator";
        case Type::OperatorPercent: return "Percent Operator";
        case Type::OperatorIncrement: return "Increment Operator";
        case Type::OperatorDecrement: return "Decrement Operator";
        case Type::OperatorStringify: return "Stringify Operator";
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
            return Logger::format("<Unknown Token Type>: $", (int)type);
            throw LINC_EXCEPTION_OUT_OF_BOUNDS(Token::Type);
        }
    }

    bool Token::isValid() const
    {
        return type != Type::InvalidToken;
    }

    bool Token::isLiteral() const 
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

    bool Token::isKeyword() const 
    {
        switch(type)
        {
        case Type::KeywordReturn:
        case Type::KeywordFunction:
        case Type::KeywordIf:
        case Type::KeywordElse:
        case Type::KeywordWhile:
        case Type::KeywordTrue:
        case Type::KeywordFalse:
        case Type::KeywordMutability:
            return true;
        default: return false;
        }
    }

    bool Token::isSymbol() const
    {
        switch(type)
        {
        case Type::ParenthesisLeft:
        case Type::ParenthesisRight:
        case Type::SquareLeft:
        case Type::SquareRight:
        case Type::BraceLeft:
        case Type::BraceRight:
        case Type::OperatorType:
        case Type::Comma:
            return true;
        default: return false;
        }
    }

    bool Token::isIdentifier() const 
    {
        return type == Type::Identifier;
    }

    bool Token::isEndOfFile() const
    {
        return type == Type::EndOfFile;
    }

    bool Token::isAssignmentOperator() const
    {
        return type == Type::OperatorAssignment;
    }

    bool Token::isArithmeticOperator() const
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

    bool Token::isArithmeticAssignmentOperator() const
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

    bool Token::isComparisonOperator() const
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

    bool Token::isLogicalOperator() const
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

    bool Token::isBitwiseOperator() const
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

    bool Token::isOperator() const
    {
        return isBinaryOperator() || isUnaryOperator();
    }

    bool Token::isBinaryOperator() const
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

    bool Token::isUnaryOperator() const
    {
        switch(type)
        {
        case Type::OperatorStringify:
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
}