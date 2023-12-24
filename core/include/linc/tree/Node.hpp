#pragma once
#include <memory>
#include <linc/lexer/Token.hpp>
#include <linc/tree/NodeInfo.hpp>

namespace linc
{
    class Node
    {
    public:
        Node(const NodeInfo& info)
            :m_info(info)
        {}

        virtual ~Node() = default;
        virtual std::vector<const Node*> getChildren() const = 0;

        inline size_t getLineNumber() const { return m_info.lineNumber; }
        inline std::vector<Token>& getTokens() const { return m_info.tokenList; }
        inline const NodeInfo& getInfo() const { return m_info; } 

        void setLineNumber(size_t line_number) const { m_info.lineNumber = line_number; }
        void setTokens(std::vector<Token> tokens) const { m_info.tokenList = std::move(tokens); }
    
        void addToken(const Token& token) const { m_info.tokenList.push_back(token); }
        void addTokens(const std::vector<Token>& tokens) const { m_info.tokenList.insert(m_info.tokenList.end(), tokens.begin(), tokens.end()); }
        
        std::unique_ptr<const Node> clone_const() const;
    private:
        mutable NodeInfo m_info;
    };
}