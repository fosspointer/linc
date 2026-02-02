#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <cstdio>
#include <linc/lexer/Token.hpp>
#include <linc/system/Containers.hpp>
#include <linc/system/Files.hpp>

namespace linc
{
    class Code final
    {
    public:
        [[nodiscard]] static inline std::pair<std::size_t, std::optional<std::string>> toSource(const std::string& filepath)
        {
            std::string result;
            std::FILE* file = std::fopen(filepath.c_str(), "r");

            if(!file)
                return std::make_pair(-1ul, std::nullopt);

            for(char buffer[1024ul]; std::fgets(buffer, sizeof buffer, file);)
                result.append(buffer);

            std::fclose(file);
            
            auto file_index = Files::computeFileIndex(filepath);
            return std::make_pair(file_index, result);
        }
    };
}
