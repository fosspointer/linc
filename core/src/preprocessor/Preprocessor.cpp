#include <linc/preprocessor/Preprocessor.hpp>

namespace linc
{
    std::unordered_set<std::string> Preprocessor::s_guardedFiles;
    const std::unordered_set<std::string> Preprocessor::s_attributes{"deprecated", "entry", "noreturn"};
}