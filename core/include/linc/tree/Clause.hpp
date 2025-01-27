#pragma once
#include <linc/tree/NodeInfo.hpp>
#include <linc/lexer/Token.hpp>

namespace linc
{
    template <typename IMPL>
    class Clause
    {
    public:
        Clause(const NodeInfo& info)
            :m_info(info)
        {}

        Clause(const Token::Info& token_info)
            :m_info(NodeInfo{.tokenList = std::vector<Token>{}, .info = token_info})
        {}

        virtual ~Clause() = default;
        [[nodiscard]] inline const Token::Info& getTokenInfo() const { return m_info.info; }
        [[nodiscard]] inline const std::vector<Token>& getTokens() const { return m_info.tokenList; }
        [[nodiscard]] inline const NodeInfo& getInfo() const { return m_info; }
        [[nodiscard]] inline std::string getInfoString() const { return m_info.info.file + ":" + std::to_string(m_info.info.line); }

        inline void setLineNumber(size_t line_number) const { m_info.info.line = line_number; }
        inline void setTokens(std::vector<Token> tokens) const { m_info.tokenList = std::move(tokens); }
    
        inline void addToken(const Token& token) const { m_info.tokenList.push_back(token); }
        inline void addTokens(const std::vector<Token>& tokens) const { m_info.tokenList.insert(m_info.tokenList.end(), tokens.begin(), tokens.end()); }
        
        virtual std::unique_ptr<const IMPL> clone() const = 0;
    private:
        mutable NodeInfo m_info;
    };
}