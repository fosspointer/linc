#include <linc/system/Files.hpp>

namespace linc
{
    std::vector<std::filesystem::path> Files::s_filepaths;

    std::size_t Files::computeFileIndex(std::string_view filepath)
    {
        auto canonical_path = std::filesystem::weakly_canonical(std::filesystem::path(filepath));
        for(std::size_t i{0ul}; i < s_filepaths.size(); ++i)
            if(s_filepaths[i] == filepath)
                return i;

        s_filepaths.push_back(canonical_path);
        return s_filepaths.size() - 1ul;
    }
}
