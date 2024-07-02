#include <linc/system/Files.hpp>

namespace linc
{
    std::unordered_map<std::filesystem::path, std::fstream*> Files::s_fileMap;

    std::string Files::toAbsolute(const std::string& filepath_string)
    {
        return std::filesystem::weakly_canonical(filepath_string).string();
    }

    std::fstream* Files::load(const std::string& filepath_string, bool write = false)
    {
        std::filesystem::path filepath = std::filesystem::absolute(filepath_string);
        auto find = s_fileMap.find(filepath);
        auto mode = write? std::ios::in | std::ios::out: std::ios::in;

        if(find != s_fileMap.end())
        {
            if(!find->second->is_open())
                find->second->open(find->first, mode);
            return find->second;
        }
        else
        {
            auto* file = new std::fstream(filepath, mode);
            s_fileMap[filepath] = file;
            return file;
        }
    }
    
    std::string Files::read(const std::string& filepath_string)
    {
        auto* file = load(filepath_string);
        std::ostringstream stream;
        stream << file->rdbuf();
        file->close();
        return std::move(stream.str());
    }

    bool Files::exists(const std::string& filepath_string)
    {
        return std::ifstream(toAbsolute(filepath_string)).good();
    }
    
    void Files::write(const std::string& filepath_string, const std::string& contents)
    {
        auto* file = load(filepath_string, true);
        (*file) << contents;
        file->close();
    }
}