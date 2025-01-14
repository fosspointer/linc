#pragma once
#include <linc/system/Printable.hpp>
#include <linc/Include.hpp>

#ifdef LINC_DEBUG
    #define LINC_EXCEPTION_FILE __FILE__
    #define LINC_EXCEPTION_LINE __LINE__
#else
    #define LINC_EXCEPTION_FILE std::string{}
    #define LINC_EXCEPTION_LINE 0
#endif

#define LINC_EXCEPTION(message) linc::Exception(LINC_EXCEPTION_FILE, LINC_EXCEPTION_LINE, __FUNCTION__, std::string{message})
#define LINC_EXCEPTION_INVALID_INPUT(message) linc::Exception(LINC_EXCEPTION_FILE, LINC_EXCEPTION_LINE, __FUNCTION__, std::string{"Invalid input: "} + message + '.')
#define LINC_EXCEPTION_OUT_OF_BOUNDS(variable) LINC_EXCEPTION_VARIABLE(variable, "Enumerator out of bounds:")
#define LINC_EXCEPTION_ILLEGAL_NULL(variable) LINC_EXCEPTION_VARIABLE(variable, "Unexpected nullptr value for symbol:")
#define LINC_EXCEPTION_ILLEGAL_VALUE(variable) LINC_EXCEPTION_VARIABLE(variable, "Illegal value or value range given for symbol:")
#define LINC_EXCEPTION_ILLEGAL_STATE(variable) LINC_EXCEPTION_VARIABLE(variable, "Illegal or unexpected state found for symbol:")
#define LINC_EXCEPTION_VARIABLE(variable, message) linc::Exception(LINC_EXCEPTION_FILE, LINC_EXCEPTION_LINE, __FUNCTION__, linc::Exception::variableExceptionMessage(linc::Exception::variableToPrintable(variable), #variable, LINC_EXCEPTION_TYPEOF(variable), message))

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
        Exception(std::string_view file, int line, std::string_view function, const std::string& message) noexcept;
        Exception(std::string_view file, int line, std::string_view function, std::string_view message) noexcept;
        std::string info() const noexcept;

        inline friend std::ostream& operator<<(std::ostream& output_stream, const Exception& exception)
        {
            output_stream << exception.info();
            return output_stream;
        }

        static std::string variableExceptionMessage(const Printable& variable_value, std::string_view variable_name, std::string_view variable_type, std::string_view message);
        
        template <typename T>
        static Printable variableToPrintable(const T& value)
        {
            if constexpr(std::is_constructible<linc::Printable, T>::value) 
            {
                return Printable{value};
            }
            else
            {
                static Printable value_void{"{}"};
                return value_void;
            }
        }
    private:
        std::string_view m_file, m_function;
        std::string m_message;
        std::size_t m_line;
    };
}