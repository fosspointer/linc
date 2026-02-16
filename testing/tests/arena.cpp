#include <Testing.hpp>
#include <linc/system/Arena.hpp>
#include <linc/system/Containers.hpp>

DEFINE_TEST(ArenaAllocate, "Testing if the arena allocator works properly with multiple blocks.")
{
    struct BigStructure {
        float stuff [100] = {0};
    };

    linc::Vector<BigStructure> vec;

    try
    {
        for(int i = 0; i < 70; ++i)
            vec.push_back(BigStructure{});
    }
    catch(const linc::Exception& exception)
    {
        linc::Logger::println("exception info: $", exception.info());
        TESTING_ASSERT(false);
    }
    catch(const std::exception& exception)
    {
        linc::Logger::println("exception info: $", exception.what());
        TESTING_ASSERT(false);
    }
}

DEFINE_TEST(ArenaDeallocate, "Testing if the arena deallocation optimization works")
{
}

TESTING_MAIN
