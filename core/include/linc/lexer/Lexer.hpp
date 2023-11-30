#pragma once
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <linc/lexer/Token.hpp>

namespace linc
{
    /// @brief Lexer class, the purpose of which is to take raw source code as its input and produce a list of Tokens that correspond to it as its output.
    class Lexer final
    {
    public:
        /// @brief Initialize a lexer object.
        /// @param source_code The actual source code to be tokenized.
        explicit Lexer(const std::string& source_code);
        
        /// @brief Process the source code given to the lexer and output its tokenized form.
        /// @return The list of tokens that correspond to the original source code.
        std::vector<Token> operator()() const;
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
        [[nodiscard]] inline std::optional<char> peek(std::string::size_type offset) const
        {
            if(m_index + offset > m_sourceCode.size() - 1)
                return std::nullopt; 
            return m_sourceCode[m_index + offset];
        }

        /// @brief Get the current character.
        /// @return Optionally returns the current character if it exists, otherwise returning nullopt.
        [[nodiscard]] inline std::optional<char> peek() const
        {
            if(m_index >= m_sourceCode.size())
                return std::nullopt; 
            return m_sourceCode[m_index];
        }

        /// @brief Return the current character, and increment the index pointer by one.
        /// @return The character that was consumed.
        inline char consume() const
        {
            return m_sourceCode[m_index++];
        }

        const std::string m_sourceCode;
        mutable std::string::size_type m_index{};
        mutable size_t m_lineNumber{0};
        mutable std::string m_lineBuffer;
    };
}