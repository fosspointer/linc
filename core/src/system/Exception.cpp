#include <linc/system/Exception.hpp>
#include <sstream>

namespace linc
{
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
}