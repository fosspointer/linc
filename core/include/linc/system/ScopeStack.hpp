#pragma once

namespace linc
{
    template <typename SYMBOL_TYPE>
    class ScopeStack final
    {
    public:
        inline void beginScope()
        {
            m_symbols.push(std::unordered_map<std::string, SYMBOL_TYPE>{});
        }

        inline void endScope()
        {
            m_symbols.pop();
        }

        [[nodiscard]] SYMBOL_TYPE get(std::string_view name) const
        {
            std::stack<std::unordered_map<std::string, SYMBOL_TYPE>> symbols_copy{m_symbols};
            while(!symbols_copy.empty())
            {
                auto find = symbols_copy.top().find(std::string{name});
                if(find != symbols_copy.top().end()) return find->second;
                else symbols_copy.pop();
            }

            throw LINC_EXCEPTION_INVALID_INPUT("Expected to find undefined variable (in codegen representation).");
        }

        inline std::stack<std::unordered_map<std::string, SYMBOL_TYPE>>::size_type getScopeSize() const { return m_symbols.size(); }

        std::optional<SYMBOL_TYPE> find(std::string_view name)
        {
            std::stack<std::unordered_map<std::string, SYMBOL_TYPE>> symbols_copy{m_symbols};
            while(!symbols_copy.empty())
            {
                auto find = symbols_copy.top().find(std::string{name});
                if(find != symbols_copy.top().end()) return find->second;
                else symbols_copy.pop();
            }
            return std::nullopt;
        }

        void append(std::string_view name, const SYMBOL_TYPE& variable)
        {
            m_symbols.top().insert(std::pair<std::string, SYMBOL_TYPE>(std::string{name}, variable));
        }
    private:
        std::stack<std::unordered_map<std::string, SYMBOL_TYPE>> m_symbols;
    };
}