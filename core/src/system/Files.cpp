#include <linc/system/Files.hpp>

namespace linc
{
    std::unordered_map<std::filesystem::path, std::fstream*> Files::s_fileMap;

    std::fstream* Files::load(const std::string& filepath_string)
    {
        std::filesystem::path filepath = std::filesystem::absolute(filepath_string);
        auto find = s_fileMap.find(filepath);

        if(find != s_fileMap.end())
            return find->second;
        else
        {
            auto* file = new std::fstream(filepath);
            s_fileMap[filepath] = file;
            return file;
        }
    }
    
    std::string Files::read(const std::string& filepath_string)
    {
        const auto* file = load(filepath_string);
        std::ostringstream stream;
        stream << file->rdbuf();
        return stream.str();
    }
    
    void Files::write(const std::string& filepath_string, const std::string& contents)
    {
        auto* file = load(filepath_string);
        (*file) << contents;
    }
}