#include <linc/system/Exception.hpp>
#include <linc/system/Logger.hpp>
#include <linc/system/Containers.hpp>
#include <memory>

#if defined(__GNUC__) || defined(__clang__)
std::string demangle(const char* name)
{
    int status{-1};
    std::unique_ptr<char[], void(*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };
    return (status == 0 && res)? res.get(): name;
}
#endif

namespace linc
{
    Exception::Exception(std::string_view file, uint16_t line, std::string_view function, const std::string& message) noexcept
        :m_file(file), m_function(function), m_line(line), m_message(message)
    {}

    std::string Exception::info() const noexcept
    {
    #ifdef LINC_DEBUG
        return Logger::format("$::$ -> exception thrown in function $. Error message: `$.`.\n", m_file, m_line, m_function, m_message);
    #else
        return Logger::format("Exception thrown in function $. This is probably not intended; please contact the developer of this software to fix it.", m_function);
    #endif
    }

    std::string Exception::variableExceptionMessage(const Formattable &value, std::string_view variable_name, std::string_view variable_type, std::string_view message)
    {
        return Logger::format("$ $:+r::$:$:!:- (=$:+c$:!:-)", message, variable_name, variable_type, value);
    }
}
