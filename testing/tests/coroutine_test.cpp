#include "../Testing.hpp"
#include <generator>

std::generator<int> range(std::size_t start, std::size_t end)
{
    for(std::size_t i = start; i < end; ++i)
        co_yield i;
}

DEFINE_TEST(Coroutine, "A simple test for std coroutine generator")
{
    for(int i : range(0, 10))
        linc::Logger::println("meow $", i);
}

TESTING_MAIN
