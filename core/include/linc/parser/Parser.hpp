#pragma once
#include <linc/parser/Program.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Class responsible for the parsing stage of compilation. Parses a list of tokens into AST nodes.
    class Parser final 
    {
    public:
        /// @brief A symbol generic definition representation.
        struct Definition
        {
            enum class Kind: Types::u8
            {
                Function, Variable, External, Typename
            } kind;
            std::string identifier;
        };

        using TokenList = std::vector<Token>;
        using TokenSize = TokenList::size_type;

        /// @brief Constructor: begin a scope and append all internal symbols as valid definitions.
        Parser();
        
        /// @brief Parse the current list of tokens as a program.
        /// @return The resulting program.
        Program operator()() const;

        /// @brief Set the current list of tokens, as well as their corresponding filepath.
        void set(std::vector<Token> tokens, std::string_view filepath);

        /// @brief Parse the following tokens as a list of delimeted types (or nothing if a right parenthesis token is found).
        std::vector<class DelimitedType> parseDelimitedTypeList() const;

        /// @brief Parse the following tokens as an AST statement.
        std::unique_ptr<const class Statement> parseStatement() const;

        /// @brief Parse the following tokens as an AST declaration.
        std::unique_ptr<const class Declaration> parseDeclaration() const;

        /// @brief Parse the following tokens as an AST variant node (i.e. either a statement or an expression).
        std::unique_ptr<const class Node> parseVariant() const;
        
        /// @brief Parse the following tokens as an AST variable declaration.
        std::unique_ptr<const class VariableDeclaration> parseVariableDeclaration() const;

        /// @brief Parse the following tokens as a direct variable declaration (implicit typing). 
        std::unique_ptr<const class DirectVariableDeclaration> parseDirectVariableDeclaration() const;
        
        /// @brief Parse the following tokens as an AST function declaration.
        std::unique_ptr<const class FunctionDeclaration> parseFunctionDeclaration() const;
        
        /// @brief Parse the following tokens as an AST external declaration.
        std::unique_ptr<const class ExternalDeclaration> parseExternalDeclaration() const;
        
        /// @brief Parse the following tokens as an AST structure declaration.
        std::unique_ptr<const class StructureDeclaration> parseStructureDeclaration() const;

        /// @brief Parse the following tokens as an AST modifier expression.
        std::unique_ptr<const class Expression> parseModifierExpression() const;
        
        /// @brief Parse the following tokens as an AST expression.
        std::unique_ptr<const class Expression> parseExpression(uint16_t parent_precedence = 0) const;
        
        /// @brief Parse the following tokens as an AST primary expression.
        std::unique_ptr<const class Expression> parsePrimaryExpression() const;

        /// @brief Parse the following tokens as an AST array initializer expression.
        std::unique_ptr<const class ArrayInitializerExpression> parseArrayInitializerExpression() const;

        /// @brief Parse the following tokens as an AST block expression.
        std::unique_ptr<const class BlockExpression> parseBlockExpression() const;

        /// @brief Parse the following tokens as an AST parenthesis expression.
        std::unique_ptr<const class ParenthesisExpression> parseParenthesisExpression() const;

        /// @brief Parse the following tokens as an AST if(/else) expression.
        std::unique_ptr<const class IfExpression> parseIfExpression() const;

        /// @brief Parse the following tokens as an AST for expression.
        std::unique_ptr<const class ForExpression> parseForExpression() const;

        /// @brief Parse the following tokens as an AST while(/else/finally) expression.
        std::unique_ptr<const class WhileExpression> parseWhileExpression() const;

        /// @brief Parse the following tokens as an AST conversion expression.
        std::unique_ptr<const class ConversionExpression> parseConversionExpression() const;

        /// @brief Parse the following tokens as an AST conversion expression.
        std::unique_ptr<const class StructureInitializerExpression> parseStructureInitializerExpression() const;
        
        /// @brief Parse the following tokens as an AST literal expression.
        std::unique_ptr<const class LiteralExpression> parseLiteralExpression() const;
        
        /// @brief Parse the following tokens as an AST identifier expression.
        std::unique_ptr<const class IdentifierExpression> parseIdentifierExpression(bool type_inclusive = false) const;

        /// @brief Parse the following tokens as an AST function call expression.
        std::unique_ptr<const class CallExpression> parseCallExpression() const;
        
        /// @brief Parse the following tokens as an AST type expression.
        std::unique_ptr<const class TypeExpression> parseTypeExpression() const;

        /// @brief Require the next token to be an EOF token (used when appropriate).
        inline auto parseEndOfFile() const { return match(Token::Type::EndOfFile); }
    private:
        /// @brief Begin a new scope for all definitions.
        void beginScope() const
        {
            m_definitions.push(m_definitions.empty()? std::vector<Definition>{}: m_definitions.top());
        }
        
        /// @brief End the current scope for all definitions.
        void endScope() const
        {
            m_definitions.pop();
        }

        /// @brief Check whether a token is a valid identifier that corresponds to a type. 
        [[nodiscard]] bool isTypeIdentifier(const Token& token) const
        {
            if(token.type != Token::Type::Identifier || !token.value)
                return false;

            auto kind = Types::kindFromUserString(*token.value);
            
            if(kind != Types::Kind::invalid)
                return true;

            return isValidStructure(*token.value);
        }

        /// @brief Check whether a given identifier has been defined as a valid structure.
        [[nodiscard]] inline bool isValidStructure(const std::string& name) const
        {
            for(const auto& definition: m_definitions.top())
                if(definition.kind == Definition::Kind::Typename && name == definition.identifier)
                    return true;

            return false;
        }

        /// @brief Find an optional definition from an identifier.
        [[nodiscard]] std::optional<Definition::Kind> findDefinition(const std::string& name) const
        {
            for(const auto& definition: m_definitions.top())
                if(name == definition.identifier)
                    return definition.kind;

            return std::nullopt;
        }

        /// @brief Peek the token at a specified offset.
        /// @return The token at the specified offset, if it exists. Otherwise, nullopt.
        [[nodiscard]] inline std::optional<Token> peek(TokenSize offset) const
        {
            if(m_index + offset > m_tokens.size() - 1ul)
                return std::nullopt; 
            return m_tokens[m_index + offset];
        }

        /// @brief Peek the next token.
        /// @return The token, if it exists. Otherwise, nullopt.
        [[nodiscard]] inline std::optional<Token> peek() const
        {
            if(m_index > m_tokens.size() - 1ul)
                return std::nullopt; 
            return m_tokens[m_index];
        }

        /// @brief Consume and return the next token.
        /// @return The consumed token, if it exists. Otherwise, an EOF.
        [[nodiscard]] inline Token consume() const
        {
            Token::Info info = peekInfo();

            if(m_index + 1ul >= m_tokens.size())
                return Token{.type = Token::Type::EndOfFile, .info = info};
            return m_tokens[m_index++];
        }

        /// @brief Require that the next token be of the specified token-type. If it's not, report an error.
        /// @return The following token, if it matches the given token-type. Otherwise, a dummy token that satisfies the type.
        [[nodiscard]] inline Token match(Token::Type type) const
        {
            Token::Info info = peekInfo();

            if(peek() && peek()->type == type)
                return consume();

            auto token_type = peek()? peek()->type: Token::Type::InvalidToken;
            
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = TextSpan::fromTokenInfo(info),
                .message = Logger::format("$::$ Expected token of type '$', got '$'.", info.file, info.line, Token::typeToString(type),
                    Token::typeToString(token_type))}, !m_matchFailed);

            m_matchFailed = true;
            
            return Token{.type = type, .info = info};
        }
        
        /// @brief Get most appropriate/last available token information (character index, file, line).
        [[nodiscard]] inline Token::Info peekInfo() const
        {
            return m_index < m_tokens.size()? m_tokens[m_index].info: m_tokens.back().info;
        }

        TokenList m_tokens;
        std::string m_filepath;
        mutable std::stack<std::vector<Definition>> m_definitions; 
        mutable bool m_matchFailed{};
        mutable TokenSize m_index{0};
    };
}