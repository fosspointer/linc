#include "Testing.hpp"
#include <linc/system/Logger.hpp>

#define TESTING_EXIT_SUCCESS 0
#define TESTING_EXIT_FAILURE 1

namespace testing
{
    void Testing::assertCondition(bool expression_result, TestingException::Kind kind, std::source_location caller_location)
    {
        if(!expression_result)
            throw TestingException(kind, caller_location);
    }

    int Testing::runTests()
    {
        std::size_t total_tests = tests().size();
        std::size_t tests_passed{};

        for(const auto& test : tests())
        {
            try
            {
                auto name = test.second.name;
                auto description = test.second.description;

                linc::Logger::log(linc::Logger::Level::Info, "Running test: $ ($)", name, description);
                test.second.function();
                linc::Logger::log(linc::Logger::Level::Info, "System :: Test passed", name, description);
                ++tests_passed;
            }
            catch(const TestingException& test_exception)
            {
                auto info = std::string{test_exception.info()};
                linc::Logger::log(linc::Logger::Level::Error, "System :: $ :: Test failed", info);
            }
            catch(...)
            {
                linc::Logger::log(linc::Logger::Level::Critical, "System :: Test threw exception in non assertion context.");
                return TESTING_EXIT_FAILURE;
            }
        }

        auto test_success = total_tests == tests_passed;
        if(test_success)
            linc::Colors::push(linc::Colors::Color::Green);
        else
            linc::Colors::push(linc::Colors::Color::Red);

        linc::Logger::log(linc::Logger::Level::Info, "System :: $/$ tests passed.$:!:-", tests_passed, tests().size());

        return total_tests == tests_passed? TESTING_EXIT_SUCCESS: TESTING_EXIT_FAILURE;
    }
}
