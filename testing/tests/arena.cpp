#include <Testing.hpp>
#include <linc/system/Arena.hpp>
#include <linc/system/Containers.hpp>

DEFINE_TEST(ArenaAllocate, "Testing if the arena allocator works properly with multiple blocks.")
{
    struct BigStructure {
        float stuff [100] = {0};
    };

    linc::Vector<BigStructure> vec;

    for(int i = 0; i < 1000; ++i)
        vec.push_back(BigStructure{});
}

DEFINE_TEST(ArenaDeallocate, "Testing if the arena deallocation optimization works")
{
}

TESTING_MAIN
