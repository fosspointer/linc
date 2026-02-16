#pragma once
#include <unordered_map>
#include <string_view>
#include "TestingException.hpp"
#include <linc/system/Exception.hpp>
#include <linc/system/Logger.hpp>

#define TESTING_EXIT_SUCCESS 0
#define TESTING_EXIT_FAILURE 1

#define TESTING_ASSERT(expression) \
    do \
    {  \
        auto location = std::source_location::current(); \
        try \
        { \
            testing::Testing::assertCondition(!!(expression), testing::TestingException::Kind::AssertionFailed); \
        } \
        catch(const testing::TestingException& exception) \
        { \
            throw exception; \
        } \
        catch(const std::exception& exception) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::UnexpectedException, exception.what(), location); \
        } \
        catch(const linc::Exception& exception) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::UnexpectedException, exception.info(), location); \
        } \
        catch(...) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::UnexpectedException, location); \
        } \
    } \
    while(false)

#define TESTING_ASSERT_THROW(expression, exception_type) \
    do \
    { \
        auto location = std::source_location::current(); \
        try \
        { \
            expression; \
        } \
        catch(const exception_type& exception) \
        { \
            return; \
        } \
        catch(const std::exception& exception) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::InvalidExceptionThrown, exception.what(), location); \
        } \
        catch(const linc::Exception& exception) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::InvalidExceptionThrown, exception.info(), location); \
        } \
        catch(...) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::InvalidExceptionThrown, location); \
        } \
        throw testing::TestingException(testing::TestingException::Kind::ShouldHaveThrownException, location); \
    } \
    while(false)

#define TESTING_SNAPSHOT(expression, fileIdentifier) \
    do \
    { \
        auto location = std::source_location::current(); \
        try \
        { \
            auto result = expression; \
            testing::Testing::assertSnapshot(result, #fileIdentifier, location); \
        } \
        catch(const testing::TestingException& exception) \
        { \
            throw exception; \
        } \
        catch(const std::exception& exception) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::UnexpectedException, exception.what(), location); \
        } \
        catch(const linc::Exception& exception) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::UnexpectedException, exception.info(), location); \
        } \
        catch(...) \
        { \
            throw testing::TestingException(testing::TestingException::Kind::UnexpectedException, location); \
        } \
    } \
    while(false)

#define DEFINE_TEST(test_name, description_value) \
    void test_name(); \
    struct test_name##_TestRegistrator { \
        test_name##_TestRegistrator() { \
            testing::Testing::tests()[#test_name] = testing::Testing::Test{.name = #test_name, .description = description_value, .function = test_name}; \
        } \
    } registrator_##test_name; \
    void test_name() \

#define TESTING_MAIN \
    int main() \
    { \
        linc::Logger::init(); \
        return testing::Testing::runTests(); \
    }

namespace testing
{
    class Testing final
    {
    public:
        Testing() = delete;
        using TestFunctionType = void(*)();
        struct Test
        {
            std::string_view name, description;
            TestFunctionType function; 
        };

        static std::unordered_map<std::string_view, Test>& tests()
        {
            static std::unordered_map<std::string_view, Test> instance;
            return instance;
        }

        static void assertCondition(bool expression_result, TestingException::Kind kind, std::source_location caller_location = std::source_location::current());
        static void assertSnapshot(std::string_view expression_result, std::string_view file_identifier, std::source_location caller_location = std::source_location::current());
        static int runTests();
    };
}
