#pragma once
#include <string>
#include <system_error>
#include <vector>

namespace linc
{
    class Files final
    {
    public:
        Files() = delete;
        [[nodiscard]] static inline std::vector<std::string>::const_iterator begin() { return s_filepaths.begin(); }
        [[nodiscard]] static inline std::vector<std::string>::const_iterator end() { return s_filepaths.end(); }
        [[nodiscard]] static inline const std::string& get(std::vector<std::string>::size_type index) { return s_filepaths.at(index); }
        static inline void push(const std::string& filepath) { s_filepaths.push_back(filepath); }
    private:
        static std::vector<std::string> s_filepaths;
    };
}
