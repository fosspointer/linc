#pragma once
#include <linc/system/Internals.hpp>
#include <linc/parser/Program.hpp>
#include <linc/lexer/Operators.hpp>
#include <linc/Tree.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Parser final 
    {
    public:
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

        inline Parser()
        {
            beginScope();
            m_definitions.top().reserve(Internals::get().size());
            
            for(const auto& internal: Internals::get())
                m_definitions.top().push_back(Definition{Definition::Kind::External, internal.name});
        }
        
        Program operator()() const;
        void set(std::vector<Token> tokens, std::string_view filepath);

        std::unique_ptr<const Statement> parseStatement() const;
        std::unique_ptr<const Declaration> parseDeclaration() const;
        std::unique_ptr<const VariableDeclaration> parseVariableDeclaration() const;
        std::unique_ptr<const FunctionDeclaration> parseFunctionDeclaration() const;
        std::unique_ptr<const ExternalDeclaration> parseExternalDeclaration() const;
        std::unique_ptr<const StructureDeclaration> parseStructureDeclaration() const;

        std::unique_ptr<const Expression> parseModifierExpression() const;
        std::unique_ptr<const Expression> parseExpression(uint16_t parent_precedence = 0) const;
        std::unique_ptr<const Expression> parsePrimaryExpression() const;
        std::unique_ptr<const LiteralExpression> parseLiteralExpression() const;
        std::unique_ptr<const IdentifierExpression> parseIdentifierExpression() const;
        std::unique_ptr<const TypeExpression> parseTypeExpression() const;

        inline auto parseEndOfFile() const { return match(Token::Type::EndOfFile); }
    private:
        void beginScope() const
        {
            m_definitions.push(m_definitions.empty()? std::vector<Definition>{}: m_definitions.top());
        }
        
        void endScope() const
        {
            m_definitions.pop();
        }

        [[nodiscard]] bool isTypeIdentifier(const Token& token) const
        {
            if(token.type != Token::Type::Identifier || !token.value)
                return false;

            auto kind = Types::kindFromUserString(*token.value);
            
            if(kind != Types::Kind::invalid)
                return true;

            return isValidStructure(*token.value);
        }

        [[nodiscard]] inline bool isValidStructure(const std::string& name) const
        {
            for(const auto& definition: m_definitions.top())
                if(definition.kind == Definition::Kind::Typename && name == definition.identifier)
                    return true;

            return false;
        }

        [[nodiscard]] std::optional<Definition::Kind> findDefinition(const std::string& name) const
        {
            for(const auto& definition: m_definitions.top())
                if(name == definition.identifier)
                    return definition.kind;

            return std::nullopt;
        }

        [[nodiscard]] inline std::optional<Token> peek(TokenSize offset) const
        {
            if(m_index + offset > m_tokens.size() - 1ul)
                return std::nullopt; 
            return m_tokens[m_index + offset];
        }

        [[nodiscard]] inline std::optional<Token> peek() const
        {
            if(m_index > m_tokens.size() - 1ul)
                return std::nullopt; 
            return m_tokens[m_index];
        }

        [[nodiscard]] inline Token consume() const
        {
            Token::Info info = getLastAvailableInfo();

            if(m_index + 1ul >= m_tokens.size())
                return Token{.type = Token::Type::EndOfFile, .info = info};
            return m_tokens[m_index++];
        }

        [[nodiscard]] inline Token match(Token::Type type) const
        {
            Token::Info info = getLastAvailableInfo();

            if(peek() && peek()->type == type)
                return consume();

            auto token_type = peek()? peek()->type: Token::Type::InvalidToken;

            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Parser,
                .span = peek() && !peek()->value? TextSpan{.lineIndex = -1ul}: TextSpan{.lineIndex = info.line - 1ul,
                    .spanStart = info.characterIndex, .spanEnd = info.characterIndex + peek()->value.value().length()},
                .message = Logger::format("$::$ Expected token of type '$', got '$'.", info.file, info.line, Token::typeToString(type),
                    Token::typeToString(token_type))}, !m_matchFailed);

            m_matchFailed = true;
            
            return Token{.type = type, .info = info};
        }
        
        [[nodiscard]] inline Token::Info getLastAvailableInfo() const
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