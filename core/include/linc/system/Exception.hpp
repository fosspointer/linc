#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <linc/system/Formattable.hpp>
#include <linc/system/Containers.hpp>

#ifdef LINC_DEBUG
    #define LINC_EXCEPTION_FILE __FILE__
    #define LINC_EXCEPTION_LINE 0
#else
    #define LINC_EXCEPTION_FILE ""
    #define LINC_EXCEPTION_LINE 0
#endif

#define LINC_ASSERT_VALID_STATE(condition, variable) if(!(condition)) throw LINC_EXCEPTION_INVALID_STATE(variable);
#define LINC_ASSERT_VALID_VALUE(condition, variable) if(!(condition)) throw LINC_EXCEPTION_INVALID_STATE(variable);
#define LINC_ASSERT(condition, message) if(!(condition)) throw LINC_EXCEPTION(linc::Logger::format("Assertion failed: $", message));
#define LINC_EXCEPTION(message) linc::Exception(LINC_EXCEPTION_FILE, LINC_EXCEPTION_LINE, __FUNCTION__, std::string{message})
#define LINC_EXCEPTION_INVALID_STATE(variable) LINC_EXCEPTION_VARIABLE(variable, "Unexpected state for symbol:") 
#define LINC_EXCEPTION_INVALID_VALUE(variable) LINC_EXCEPTION_VARIABLE(variable, "Invalid value for symbol:")
#define LINC_EXCEPTION_OUT_OF_BOUNDS(variable) LINC_EXCEPTION_VARIABLE(variable, "Symbol out of bounds:")
#define LINC_EXCEPTION_VARIABLE(variable, message) linc::Exception(LINC_EXCEPTION_FILE, LINC_EXCEPTION_LINE, __FUNCTION__, linc::Exception::variableExceptionMessage(linc::Exception::formattableFromSymbol(variable), #variable, LINC_EXCEPTION_TYPEOF(variable), message))

#if defined(__GNUC__) || defined(__clang__)
    #include <cxxabi.h>
    #include <typeinfo>
    #define LINC_EXCEPTION_TYPEOF(value) demangle(typeid(decltype(value)).name())
    std::string demangle(const char* name);
#else
    #define LINC_EXCEPTION_TYPEOF(value) typeid(decltype(value)).name()
#endif

namespace linc
{
    class Exception final
    {
    public:
        Exception(std::string_view file, uint16_t line, std::string_view function, const String& message) noexcept;
        String info() const noexcept;

        static String variableExceptionMessage(const Formattable& value, std::string_view variable_name, std::string_view variable_type, std::string_view message);

        template <typename T>
        static Formattable formattableFromSymbol(const T& value)
        {
            if constexpr(std::is_constructible<linc::Formattable, T>::value)
            {
                return Formattable{value};
            }
            else
            {
                static Formattable value_void("{}");
                return value_void;
            }
        }
    private:
        std::string_view m_file, m_function;
        std::uint16_t m_line;
        std::string m_message;
    };
}
