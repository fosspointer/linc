#pragma once
#include <linc/system/Exception.hpp>
#include <linc/Include.hpp>

namespace linc
{
    template <typename SYMBOL_TYPE>
    class ScopeStack final
    {
    public:
        ScopeStack() { beginScope(); }

        inline void beginScope()
        {
            m_symbols.push_back(std::unordered_map<std::string, SYMBOL_TYPE>{});
        }

        inline void endScope()
        {
            m_symbols.pop_back();
        }

        [[nodiscard]] inline decltype(auto) top(this auto& self) { return self.m_symbols.back(); }
        [[nodiscard]] decltype(auto) get(this auto& self, const std::string& name)
        {
            for(auto it = self.m_symbols.rbegin(); it != self.m_symbols.rend(); ++it)
            {
                auto find = it->find(name);
                if(find != it->end()) return &find->second;
            }
            throw LINC_EXCEPTION_ILLEGAL_STATE(name);
        }

        [[nodiscard]] decltype(auto) findTop(this auto& self, const std::string& name)
        {
            auto find = self.m_symbols.back().find(name);
            if(find != self.m_symbols.back().end()) return &find->second;
            return static_cast<decltype(&find->second)>(nullptr);
        }

        [[nodiscard]] decltype(auto) find(this auto& self, const std::string& name)
        {
            using SymbolPointerType = std::conditional_t<
                std::is_const_v<std::remove_reference_t<decltype(self)>>,
                const SYMBOL_TYPE*,
                SYMBOL_TYPE*>;

            for(auto it = self.m_symbols.rbegin(); it != self.m_symbols.rend(); ++it)
            {
                auto find = it->find(name);
                if(find != it->end()) return &find->second;
            }
            return static_cast<SymbolPointerType>(nullptr);
        }

        inline void removeTop(const std::string& name)
        {
            m_symbols.back().erase(name);
        }

        [[nodiscard]] inline std::stack<std::unordered_map<std::string, SYMBOL_TYPE>>::size_type getScopeSize() const { return m_symbols.size(); }
        [[nodiscard]] std::vector<std::pair<std::string, SYMBOL_TYPE>> getNamedSymbols() const
        {
            std::unordered_set<std::string> seen_names;
            std::vector<std::pair<std::string, SYMBOL_TYPE>> symbol_list;

            for(auto it = m_symbols.rbegin(); it != m_symbols.rend(); ++it)
            {
                for(const auto& [key, value]: *it)
                {
                    if(seen_names.find(key) == seen_names.end())
                        symbol_list.push_back(std::pair<std::string, SYMBOL_TYPE>(key, value));
                }
            }

            return symbol_list;
        }
        [[nodiscard]] std::vector<const SYMBOL_TYPE*> getSymbols() const
        {
            std::unordered_set<std::string> seen_names;
            std::vector<const SYMBOL_TYPE*> symbol_list;

            for(auto it = m_symbols.rbegin(); it != m_symbols.rend(); ++it)
            {
                for(const auto& [key, value]: *it)
                {
                    if(seen_names.find(key) == seen_names.end())
                        symbol_list.push_back(&value);
                }
            }

            return symbol_list;
        }

        void append(std::string_view name, SYMBOL_TYPE symbol)
        {
            m_symbols.back().insert(std::pair<std::string, SYMBOL_TYPE>(std::string{name}, std::move(symbol)));
        }

        void update(std::string_view name, SYMBOL_TYPE symbol)
        {
            m_symbols.back()[std::string{name}] = std::move(symbol);
        }
    private:
        std::deque<std::unordered_map<std::string, SYMBOL_TYPE>> m_symbols;
    };
}