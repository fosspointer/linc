#pragma once
#include <linc/Include.hpp>

namespace linc
{
    class StringStack final
    {
    public:
        inline void push(std::string_view symbol)
        {
            m_symbols.push(std::string{symbol});
        }

        inline void pop()
        {
            m_symbols.pop();
        }

        [[nodiscard]] std::string get(std::string_view name) const
        {
            std::stack<std::string> symbols_copy{m_symbols};
            while(!symbols_copy.empty())
            {
                if(symbols_copy.top() == name) return symbols_copy.top();
                symbols_copy.pop();
            }
            throw LINC_EXCEPTION_ILLEGAL_STATE(name);
        }

        inline std::stack<std::string>::size_type getSize() const { return m_symbols.size(); }

        std::string find(std::string_view name)
        {
            std::stack<std::string> symbols_copy{m_symbols};
            while(!symbols_copy.empty())
            {
                if(symbols_copy.top() == name) return symbols_copy.top();
                symbols_copy.pop();
            }
            return std::string{};
        }

    private:
        std::stack<std::string> m_symbols;
    };
}