#pragma once
#include <unordered_map>
#include <filesystem>
#include <fstream>

namespace linc
{
    class Files final 
    {
    public:
        static std::fstream* load(const std::string& filepath_string);
        static std::string read(const std::string& filepath_string);
        static void write(const std::string& filepath_string, const std::string& contents);
    private:
        static std::unordered_map<std::filesystem::path, std::fstream*> s_fileMap;
    };
}