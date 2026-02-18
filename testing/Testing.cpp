#include "Testing.hpp"
#include <linc/system/Logger.hpp>
#include <filesystem>

#define TESTING_EXIT_SUCCESS 0
#define TESTING_EXIT_FAILURE 1

namespace testing
{
    void Testing::assertCondition(bool expression_result, TestingException::Kind kind, const std::optional<std::string>& message, std::source_location caller_location)
    {
        if(!expression_result)
            throw TestingException(kind, message, caller_location);
    }

    void Testing::assertSnapshot(std::string_view expression_result, std::string_view file_identifier, const std::optional<std::string>& message, std::source_location caller_location)
    {
        auto snapshot_path = linc::Logger::format("$/$", LINC_TESTING_SNAPSHOT_PATH, file_identifier);

        bool snapshot_exists = std::filesystem::exists(snapshot_path);
        std::FILE* snapshot;

        if(!snapshot_exists)
        {
            snapshot = std::fopen(snapshot_path.c_str(), "w");
            if(!snapshot)
            {
                auto message = linc::Logger::format("Could not create snapshot file, `$`. Make sure its directory exists and that this executable has proper write permissions to it.", snapshot_path);
                throw TestingException(TestingException::Kind::SystemFailure, message, std::source_location::current());
            }

            std::fwrite(expression_result.data(), 1ul, expression_result.length(), snapshot);
            std::fclose(snapshot);
            return;
        }
        
        std::string contents;
        snapshot = std::fopen(snapshot_path.c_str(), "r");

        if(!snapshot)
        {
            auto message = linc::Logger::format("Could not read snapshot file, `$`. Make sure that this executable has read access to it.", snapshot_path);
            throw TestingException(TestingException::Kind::SystemFailure, message, std::source_location::current());
        }

        for(char buffer[1024ul]; std::fgets(buffer, sizeof buffer, snapshot) != nullptr;)
            contents.append(buffer);

        std::fclose(snapshot);
        Testing::assertCondition(contents == expression_result, TestingException::Kind::UnmatchedSnapshot, message, caller_location);
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
