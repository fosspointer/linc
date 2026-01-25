#include "TestingException.hpp"
#include <cstdio>
#include <linc/system/Logger.hpp>
#include <linc/system/Containers.hpp>

namespace testing
{
    TestingException::TestingException(Kind kind, std::source_location location)
        :m_kind(kind), m_location(location)
    {}

    TestingException::TestingException(Kind kind, std::string_view message, std::source_location location)
        :m_kind(kind), m_location(location), m_message(message)
    {}

    std::string_view TestingException::kindToString(Kind kind)
    {
        switch(kind)
        {
        case Kind::AssertionFailed: return "Assertion Failed";
        case Kind::UnexpectedValue: return "Unexpected Value";
        case Kind::UnexpectedException: return "Unexpected Exception Thrown";
        case Kind::InvalidExceptionThrown: return "Invalid Exception Thrown";
        case Kind::ShouldHaveThrownException: return "Test Exited Without Throw";
        case Kind::TimedOut: return "Test Timed Out";
        case Kind::UnmatchedSnapshot: return "Unmatched Snapshot";
        default: throw std::runtime_error("TestingException::Kind enum out of bounds");
        }
    }

    linc::String TestingException::info() const
    {
        auto kind_string = TestingException::kindToString(m_kind);
        if(m_message.has_value())
            return linc::Logger::format("[$] $::$::$ (in `$`) :: Message: $", kind_string, m_location.file_name(), m_location.line(), m_location.column(), m_location.function_name(), m_message.value());
        else
            return linc::Logger::format("[$] $::$::$ (in `$`)", kind_string, m_location.file_name(), m_location.line(), m_location.column(), m_location.function_name());

    }
}
