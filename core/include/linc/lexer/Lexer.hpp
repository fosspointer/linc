#pragma once
#include <linc/system/Code.hpp>
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Lexer class, the purpose of which is to take raw source code as its input and produce a list of Tokens that correspond to it as its output.
    class Lexer final
    {
    public:
        /// @brief Initialize a lexer object.
        /// @param source_code The actual source code to be tokenized.
        /// @param initialize_source Whether to initialize the source code representation used for reporting errors. To be used for files that directly
        /// go through language analysis, i.e. not via include directives.
        explicit Lexer(const Code::Source& source_code, bool initialize_source);
        
        /// @brief Process the source code given to the lexer and output its tokenized form.
        /// @return The list of tokens that correspond to the original source code.
        std::vector<Token> operator()() const;

        /// @brief Append a list of custom directories to the include paths list.
        /// @param new_directories The new paths to be appended.
        inline void appendIncludeDirectories(const std::vector<std::string>& new_directories) const
        {
            m_includeDirectories.insert(m_includeDirectories.end(), new_directories.begin(), new_directories.end());
        }

        /// @brief Determine whether a character is a digit for a given number system base.
        /// @return Boolean corresponding to the result of the test.
        inline static bool isDigit(char c, Token::NumberBase base)
        {
            switch(base)
            {
            case Token::NumberBase::Decimal: return std::isdigit(c);
            case Token::NumberBase::Hexadecimal: return std::string("01234567890ABCDEF").contains(c);
            case Token::NumberBase::Binary: return c == '0' || c == '1';
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(base);
            }
        }
    private:        
        /// @brief Internal utility method to check whether the next sequence of characters is whitespace. If it is, ignore it and return true
        /// (whitespace is non-significant in linc). If not, return false to let other methods tokenize it.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeSpace() const;

        /// @brief Internal utility method to check whether the next sequence of characters is a comment. If it is, ignore it and return true
        /// (comments are ignored). If not, return false to let other methods tokenize it.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeComments() const;
        
        
        /// @brief Internal utility method to check whether the next sequence of characters is a literal. If it is, tokenize it, returning true.
        /// Otherwise, return false to let other methods tokenize it.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeLiterals(std::vector<Token>& tokens, std::string& value_buffer) const;

        /// @brief Internal utility method to check whether the next sequence of characters is a number literal. if it is, tokenize it, returning true.
        /// Otherwise, return false to let other methods tokenize it.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeLiteralNumber(std::vector<Token>& tokens, std::string& value_buffer) const;

        /// @brief Internal utility method to check whether the next sequence of characters is a string literal. if it is, tokenize it, returning true.
        /// Otherwise, return false to let other methods tokenize it.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeLiteralString(std::vector<Token>& tokens, std::string& value_buffer) const;

        /// @brief Internal utility method to check whether the next sequence of characters is a valid include path directory (used for include directives).
        /// If it is, tokenize it, returning true. Otherwise, return false to let other methods tokenize it.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeLiteralIncludePath(std::vector<Token>& tokens, std::string& value_buffer) const;


        /// @brief Internal utility method to check whether the next sequence of characters is a character literal. if it is, tokenize it, returning true.
        /// Otherwise, return false to let other methods tokenize it.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeLiteralCharacter(std::vector<Token>& tokens, std::string& value_buffer) const;

        /// @brief Internal utility method to check whether the next sequence of characters is a 'word'. if it is, tokenize it, returning true.
        /// Otherwise, return false to let other methods tokenize it. Word, in this case, refers to keyword and identifier tokens.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeWords(std::vector<Token>& tokens, std::string& value_buffer) const;

        /// @brief Internal utility method to check whether the next sequence of characters is a bracket-like. if it is, tokenize it, returning true.
        /// Otherwise, return false to let other methods tokenize it. Bracket-like refers to parenthesis, square-brackets and curly-braces.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        /// @return Boolean corresponding to whether whitespace was found.
        [[nodiscard]] bool tokenizeBrackets(std::vector<Token>& tokens, std::string& value_buffer) const;
        
        /// @brief Internal utility method to check whether the next sequence of characters is a number literal. if it is, tokenize it.
        /// Does not return anything since it's the last method of the chain to be called, and as such, appends an invalid token if the check fails.
        /// @param tokens The list of tokens to append the new literal token to.
        /// @param value_buffer A buffer used to determine some tokens and their value.
        void tokenizeOperators(std::vector<Token>& tokens, std::string& value_buffer) const;

        /// @brief Get the character that is offseted by as many characters as specified by offset.
        /// @param offset The offset count.
        /// @return Optionally returns the requested character if that exists, otherwise returning nullopt.
        [[nodiscard]] inline std::optional<Code::Character> peek(std::string::size_type offset = 0ul) const
        {
            return Code::peek(m_sourceCode, m_characterIndex, m_lineIndex, offset);
        }

        /// @brief Return the current character, and increment the index pointer by one.
        /// @return The character that was consumed.
        inline Code::Character consume() const
        {
            return Code::consume(m_sourceCode, m_characterIndex, m_lineIndex);
        }

        /// @brief Check whether a given character represents a valid symbol in Linc. 
        [[nodiscard]] static bool isSymbol(char c);

        /// @brief Utility method used to determine the type of a number literal (between floating point and integral).
        [[nodiscard]] static bool digitHandle(char c, char next, size_t* decimal_count, Token::NumberBase base);

        /// @brief Check if a given character is valid within a 'stem' of a number literal (i.e. it is either a digit or a decimal-point).
        [[nodiscard]] static bool digitPeek(char c, char next, Token::NumberBase base);

        /// @brief  Check whether a given string view contains at least one digit.
        [[nodiscard]] static bool hasDigit(std::string_view str, Token::NumberBase base);

        const Code::Source m_sourceCode;
        mutable std::string::size_type m_characterIndex{}, m_lineIndex{};
        mutable std::vector<std::string> m_includeDirectories{"/usr/include", "/usr/local/include", LINC_INSTALL_PATH "/include"};
    };
}
