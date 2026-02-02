#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace linc
{
    class Files final
    {
    public:
        Files() = delete;
        [[nodiscard]] static inline std::vector<std::filesystem::path>::const_iterator begin() { return s_filepaths.begin(); }
        [[nodiscard]] static inline std::vector<std::filesystem::path>::const_iterator end() { return s_filepaths.end(); }
        static std::size_t computeFileIndex(std::string_view filepath);
        static inline std::string filepathAtIndex(std::size_t index)
        {
            return s_filepaths.at(index);
        }
    private:
        static std::vector<std::filesystem::path> s_filepaths;
    };
}
