#include <linc/system/Exception.hpp>
#include <linc/system/Colors.hpp>
#include <linc/system/Logger.hpp>

#if defined(__GNUC__) || defined(__clang__)
std::string demangle(const char* name)
{
    auto status{-1};
    std::unique_ptr<char[], void(*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };
    return (status == 0 && res)? res.get(): name;
}
#endif

namespace linc
{
    Exception::Exception(std::string_view file, int line, std::string_view function, const std::string& message) noexcept
        :m_file(file), m_function(function), m_message(message), m_line(line)
    {}

    Exception::Exception(std::string_view file, int line, std::string_view function, std::string_view message) noexcept
        :m_file(file), m_function(function), m_message(message), m_line(line)
    {}

    std::string Exception::info() const noexcept
    {
        std::ostringstream stream; stream
        #ifdef LINC_DEBUG
        << m_file << ':' << m_line << ' '
        #endif
        << "-> exception thrown in function " << m_function << ". Error message: \"" << m_message << "\"";
        return stream.str();
    }

    std::string Exception::variableExceptionMessage(const Printable& variable_value, std::string_view variable_name, std::string_view variable_type, std::string_view message)
    {
        return Logger::format("$ $:#5$::$:$:#4 (=$:#6$:$:#4)", message, variable_name, variable_type, variable_value, Colors::pop(), Colors::push(Colors::Color::Blue), Colors::push(Colors::Color::Cyan));
    }
}