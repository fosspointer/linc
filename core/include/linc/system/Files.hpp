#pragma once
#include <linc/Include.hpp>

namespace linc
{
    class Files final 
    {
    public:
        static std::string toAbsolute(const std::string& filepath_string);
        static std::fstream* load(const std::string& filepath_string, bool write);
        static std::string read(const std::string& filepath_string);
        static bool exists(const std::string& filepath_string);
        static void write(const std::string& filepath_string, const std::string& contents);
    private:
        static std::unordered_map<std::filesystem::path, std::fstream*> s_fileMap;
    };
}