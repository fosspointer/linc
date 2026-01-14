#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <source_location>

namespace testing
{
    class TestingException
    {
    public:
        enum class Kind : std::uint_least8_t {
            AssertionFailed, UnexpectedValue, UnexpectedException, InvalidExceptionThrown, ShouldHaveThrownException, TimedOut 
        };

        TestingException(Kind kind, std::source_location location);
        TestingException(Kind kind, std::string_view message, std::source_location location);

        std::string info() const;
    private:
        static std::string_view kindToString(Kind kind);
        const Kind m_kind;
        const std::source_location m_location;
        const std::optional<std::string_view> m_message;
    };
}

