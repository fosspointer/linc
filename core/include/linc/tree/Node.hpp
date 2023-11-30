#pragma once
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
        inline const std::vector<Token>& getTokens() const { return m_info.tokenList; }
        inline std::vector<Token>& getTokens() { return m_info.tokenList; }
        inline bool isValid() const { return m_info.isValid; }
        inline const NodeInfo& getInfo() const { return m_info; } 

        void setLineNumber(size_t line_number){ m_info.lineNumber = line_number; }
        void setTokens(std::vector<Token> tokens){ m_info.tokenList = std::move(tokens); }
        void setValid(bool option){ m_info.isValid = option; }

        void addToken(const Token& token){ m_info.tokenList.push_back(token); }
        void addTokens(const std::vector<Token>& tokens){ m_info.tokenList.insert(m_info.tokenList.end(), tokens.begin(), tokens.end()); }
    private:
        NodeInfo m_info;
    };
}