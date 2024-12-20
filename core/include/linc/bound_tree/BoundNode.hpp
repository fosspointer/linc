#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/PrimitiveValue.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundNode 
    {
    public:
        BoundNode(const Token::Info& info = Token::Info{.file = {}, .line = {}})
            :m_info(info)
        {}
        virtual ~BoundNode() = default;
        virtual std::vector<const BoundNode*> getChildren() const { return std::vector<const BoundNode*>{}; }

        [[nodiscard]] inline const Token::Info& getInfo() const { return m_info; }
        [[nodiscard]] virtual std::string toString() const
        {
            std::string result{Colors::push(Colors::Color::Yellow)};
            result.append(toStringInner());
            result.append(Colors::pop());
            return result;
        }
    protected:
        virtual std::string toStringInner() const = 0;
    private:
        const Token::Info m_info;
    };
}