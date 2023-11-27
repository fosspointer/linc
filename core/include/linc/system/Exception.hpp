#pragma once
#include <string_view>
#include <iostream>

#define LINC_EXCEPTION(message) linc::Exception(__FILE__, __LINE__, __FUNCTION__, message)
#define LINC_EXCEPTION_INVALID_INPUT(message) linc::Exception(__FILE__, __LINE__, __FUNCTION__, "Invalid input: " message)
#define LINC_EXCEPTION_OUT_OF_BOUNDS(type) linc::Exception(__FILE__, __LINE__, __FUNCTION__, "Type '" #type "' out of bounds")

namespace linc
{
    class Exception final
    {
    public:
        Exception(std::string_view file, int line, std::string_view function, std::string_view message) noexcept;
        std::string info() const noexcept;

        inline friend std::ostream& operator<<(std::ostream& output_stream, const Exception& exception)
        {
            output_stream << exception.info();
            return output_stream;
        }
    private:
        std::string_view m_file, m_function, m_message;
        size_t m_line;
    };
}