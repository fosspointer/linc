#include <linc/lexer/Lexer.hpp>
#include <linc/system/Logger.hpp>

namespace linc
{
    Token Lexer::makeTokenFromValue(Token::Kind kind, std::size_t character_start, std::size_t character_end, std::size_t file, std::size_t line)
    {
        return Token{.kind = kind, .info = Token::Info{.file = file, .line = line, .characterStart = character_start, .characterEnd = character_end},
            .value = m_sourceCode.at(line).text.substr(character_start, character_end - character_start + 1ul)
        };
    }

    std::vector<Token> Lexer::operator()()
    {
        m_tokens.clear();

        ignoreShebang();
        while(peek().has_value())
        {
            if(ignoreSpace());
            else if(ignoreComments());
            else if(tokenizeIdentifier());
            else consume();
        }

        return m_tokens;
    }

    bool Lexer::ignoreSpace()
    {
        bool ignored{false};
        while(std::isspace(*peek()))
        {
            consume();
            ignored = true;

            if(!peek())
                break;
        }
        return ignored;
    }

    bool Lexer::ignoreComments()
    {
        if(!peek(1ul) || *peek() != '/' || *peek() != '/')
            return false;

        m_characterIndex = {};
        ++m_line;
        return true;
    }

    void Lexer::ignoreShebang()
    {
        ignoreSpace();
        if(!peek(1ul) || *peek() != '#' || *peek() != '!')
            return;

        m_characterIndex = {};
        ++m_line;
    }

    bool Lexer::tokenizeIdentifier()
    {
        if(!(std::isalpha(*peek())) && *peek() != '_')
            return false;

        auto startIdentifier = consume();
        Code::Character endIdentifier = startIdentifier;

        while(peek().has_value() && peek()->line == startIdentifier.line && (std::isalnum(peek().value()) || peek().value() == '_'))
            endIdentifier = consume();

        m_tokens.push_back(Token{.kind = Token::Kind::Identifier, .value = std::string_view{
            m_sourceCode.at(startIdentifier.line - 1).text.substr(startIdentifier.index, endIdentifier.index - startIdentifier.index + 1)}});
        return true;
    }
}
