#include <linc/system/Numbers.hpp>

namespace linc
{
    std::size_t Numbers::getBaseByDescriptor(char descriptor)
    {
        switch(descriptor)
        {
        case 'b': return 2ul;
        case 'o': return 8ul;
        case 'd': return 10ul;
        case 'x': return 16ul;
        case 't': return 36ul;
        default: return 10ul;
        }
    }
}
