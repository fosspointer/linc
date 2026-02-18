#include "../Testing.hpp"
#include <thread>

DEFINE_TEST(MyTest, "Example test")
{
    TESTING_ASSERT(1 + 1 == 2);
}

DEFINE_TEST(ThrowStdException, "Test that std exception are correctly caught")
{
    auto willThrow = []()
    {
        std::string test_variable{};
        throw LINC_EXCEPTION_INVALID_STATE(test_variable);
    };
    TESTING_ASSERT_THROW(willThrow(), linc::Exception);
    TESTING_ASSERT_THROW_MESSAGE(willThrow(), linc::Exception, "Function willThrow() should have thrown a linc::Exception exception.");
}

DEFINE_TEST(ThrowLincException, "Test that linc exceptions are correctly caught")
{
    auto willThrow = []()
    {
        throw std::runtime_error("Test std exception throw");
    };
    TESTING_ASSERT_THROW(willThrow(), std::exception);
}

DEFINE_TEST(SnapshotTest, "Simple test to see if snapshots are working")
{
    auto evaluation = "Example string";
    // TESTING_SNAPSHOT(evaluation, test_file);
    TESTING_SNAPSHOT_MESSAGE(evaluation, test_file, "The given example snapshot did not match the snapshot file found");
}

TESTING_MAIN
