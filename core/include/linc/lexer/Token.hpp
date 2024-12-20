#pragma once
#include <linc/system/Exception.hpp>
#include <linc/system/Logger.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Aggregate structure used by the lexer to abstract from raw code, such that the parser does not have to deal code on an individual
    /// character basis.
    struct Token final
    {
    public:
        /// @brief Struct holding data useful for error handling and logging
        struct Info final 
        {
            std::string file;
            std::size_t line, characterStart, characterEnd;

            bool operator==(const Info& other) const = default;
            std::string toString() const { return file + ':' + std::to_string(line); }
        };

        /// @brief The type of a Token
        enum class Type
        {
            // Invalid token
            InvalidToken,

            // End of File token
            EndOfFile,
            
            // Keywords
            KeywordReturn, KeywordFunction, KeywordIf, KeywordElse, KeywordWhile, KeywordTrue, KeywordFalse, KeywordMutability, KeywordFinally,
            KeywordAs, KeywordFor, KeywordIn, KeywordExternal, KeywordBreak, KeywordContinue, KeywordStructure, KeywordMatch,
            KeywordEnumeration,

            // Symbols
            ParenthesisLeft, ParenthesisRight, SquareLeft, SquareRight, BraceLeft, BraceRight, Colon, Comma, Tilde, Dot, PreprocessorSpecifier,
            GlueSpecifier, ColonEquals, Terminator, Arrow, DoubleColon,
            
            // Arithmetic operators
            OperatorPlus, OperatorMinus, OperatorAsterisk, OperatorSlash, OperatorPercent,
            OperatorIncrement, OperatorDecrement,
            
            // Stringify operator
            OperatorStringify,

            // Assignment operator
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
            
            // Signed integer literals
            I8Literal, I16Literal, I32Literal, I64Literal,
            
            // Unsigned integer literals
            U8Literal, U16Literal, U32Literal, U64Literal,
            
            // Floating-point literals
            F32Literal, F64Literal,
            
            // String literal
            StringLiteral,
            
            // Character literal
            CharacterLiteral,

            // Identifier
            Identifier, 
        };

        /// @brief Enumeration of supported number system bases.
        enum class NumberBase: unsigned char
        {
            Decimal, Hexadecimal, Binary
        };
        
        /// @brief Convert a given number base to its corresponding integer (in decimal).
        /// @param base The base enumerator.
        /// @return Unsigned char (integer) corresponding to the number base.
        static unsigned char baseToInt(Token::NumberBase base);
        
        /// @brief Convert a given token type to a string, essential for error handling/debugging/IO.
        /// @param type The type of the token, to be converted to string.
        /// @return String corresponding to the given token type.
        static std::string typeToString(Type type);
        
        /// @brief Check whether the current token is valid- not of the 'InvalidToken' type.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isValid() const;

        /// @brief Check whether the current token is a literal. Literals include character literals (e.g. 'c', 60c),
        /// string literals (e.g. "hello world"), integer literals (e.g. 10u8, 10i16), boolean literals(true, false, 1b).
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isLiteral() const;

        /// @brief Check whether the current token is a keyword. Keywords are reserved character sequences that follow the same rules as identifiers,
        /// but serve a unique purpose in the language (e.g. if, true, return, while).
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isKeyword() const;

        /// @brief Check whether the current token is an identifier. Identifiers are non-reserved sequences of alphanumeric(plus '_') characters
        /// used to 'identify', as the name suggests, structures like functions and variables. An identifier cannot, however, have a digit as its
        /// first character.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isIdentifier() const;

        /// @brief Check whether the current token is the end of a file. the current token has been conventionally used to ease the parsing process, and is thus,
        /// also used int his project. EOF tokens are only produced at the end of files by the lexer. This means that no sequence of characters can
        /// 'emit' and end of File token as its output.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isEndOfFile() const;

        /// @brief Check whether the current token is a bracket. Brackets are symbols that group together expressions and may declare additional syntax.
        /// For example, parentheses are used both for 'redirecting' the precedence of an expression, but also in cases like function declarations
        /// and function calls, where they specify the start and end of the function's argument list. Braces group statements and produce an output expression.
        /// Consequently, they are used for both grouping and introducing new syntax (the block expression in this case).
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isBracket() const;

        /// @brief Check whether the current token is an operator. Operators are special characters that return the result of an operation that has been
        /// 'acted' upon a value. Operators are split into two primary categories: unary operators (those who modify a singular value, e.g. the unary
        /// negation operator), and binary operators (those who modify two values, e.g. the basic arithmetic operators of addition, subtraction, 
        /// multiplication and division).
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isOperator() const;

        /// @brief Check whether the current token is a symbol. Symbols are all sequences of printable ASCII characters that are not alphanumeric
        /// (including '_'), and do not correspond to a valid operator or bracket.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isSymbol() const;

        /// @brief Check whether the current token is an assignment operator. Assignment operators include the 'main assignment operator' —typically used
        /// to assign a value to a given variable—, as well as all arithmetic assignment operators, which are equivelant to assignments of arithmetic
        /// binary operations to a variable, where the first operand is the same as the variable.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isAssignmentOperator() const;
        
        /// @brief Check whether the current token is an arithmetic operator. Arithmetic operators are those where all operands are numbers, and where the
        /// return value of the operation is also a number (e.g. addition, multiplication, negation, increment). Note: the arithmetic assignment
        /// operators are NOT arithmetic operators, since the assignment ones mutate a variable, and thus, cannot be applied to all number values
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isArithmeticOperator() const;

        /// @brief Check whether the current token is an arithmetic assignment operator. Arithmetic assignment operators are equivelant to their
        /// corresponding binary arithmetic operators, with the first operand being a variable, which the result of the operation is also
        /// assigned to.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isArithmeticAssignmentOperator() const;

        /// @brief Check whether the current token is a comparison operator. Comparison operators are binary operators where the output is a boolean
        /// (e.g. the greater-than operator, the equality operator). Note: these operators operate solely on numbers, with the exception of
        /// the equality and inequality operators, which operate on any given type.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isComparisonOperator() const;

        /// @brief Check whether the current token is a logical operator. Logical operators are operators where both the operands and the return value are
        /// of boolean type, corresponding to basic statement logical operators in math (and, or, not).
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isLogicalOperator() const;

        /// @brief Check whether the current token is a bitwise operator. Bitwise operators are those where both the operands are integrals, and where
        /// an operation is applied to each of their individual bits. The most common bitwise operators correspond to the logical ones, but on a
        /// bit-wise level (bitwise and, bitwise or, bitwise not). Additional ones include bitwise xor, bitshift-left, bitshift-right, etc...
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isBitwiseOperator() const;

        /// @brief Check whether the current token is a binary operator. Binary operators are those that are applied to exactly two operands.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isBinaryOperator() const;

        /// @brief Check whether the current token is a unary operator. Unary operators are those that are applied to only a singular operand.
        /// @return Boolean corresponding to the result of the test.
        [[nodiscard]] bool isUnaryOperator() const;

        /// @brief Return a string representation of the current token, or the literal code representation of it for operators and brackets.
        [[nodiscard]] std::string getDescriptor() const;

        Type type;
        std::optional<std::string> value;
        std::optional<NumberBase> numberBase;
        Info info;
    };
}
