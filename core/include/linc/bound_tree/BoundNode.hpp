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
        BoundNode(const Types::type& type, const Token::Info& info = Token::Info{.file = {}, .line = {}})
            :m_type(type), m_info(info)
        {}
        virtual ~BoundNode() = default;
        virtual std::vector<const BoundNode*> getChildren() const { return std::vector<const BoundNode*>{}; }

        [[nodiscard]] inline std::string toString() const
        {
            std::string result{Colors::push(Colors::Color::Yellow)};
            result.append(toStringInner());
            result.append(Colors::pop());
            Logger::append(result, " (:$)", PrimitiveValue(m_type));
            return result;
        }
        
        [[nodiscard]] inline Types::type getType() const { return m_type; }
    protected:
        virtual std::string toStringInner() const = 0;
    private:
        const Types::type m_type;
        const Token::Info m_info;
    };
}