#pragma once
#include <linc/system/Exception.hpp>
#include <linc/system/Printable.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Input/Output utility static class.
    class Logger final
    {
    public:
        Logger() = delete;

        /// @brief Enumeration used when logging.
        enum class Type: char
        {
            Info, Warning, Error, Debug
        };

        /// @brief Convert logging type enumerator to string (used for printing). 
        /// @param type The enumerator to convert.
        /// @return The resulting string.
        static std::string logTypeToString(Type type);

        /// @brief String formatting utility method, similar to std::format.
        /// @param str The string to be formatted.
        /// @param args The list of arguments to format with.
        /// @return The resulting string.
        static std::string format(const std::string& str, const std::vector<Printable>& _args);

        /// @brief String formatting utility method, similar to std::format.
        /// @param str The string to be formatted.
        /// @param ...args The variadic argument list to format with.
        /// @return The resulting string.
        template <typename... Args>
        static inline std::string format(const std::string& str, Args... args)
        {
            return format(str, std::vector<Printable>{args...});
        }

        /// @brief Print single new-line character. 
        static void println();

        /// @brief Format a string and print it to stdout.
        /// @param str The string to be formatted.
        /// @param ...args The variadic argument list to format with.
        template <typename... Args>
        inline static void print(const std::string& str, Args... args)
        {
            fputs(format(Colors::toANSI(Colors::getCurrentColor()) + str, args...).c_str(), stdout);
        }

        /// @brief Read string from stdin until return key entered.
        /// @return The resulting string, as read from stdin.
        static std::string read(const std::string& prompt = "");

        /// @brief Format a string and append it to the output string. 
        /// @param output_string The output string.
        /// @param str The string to be formatted.
        /// @param ...args The variadic argument list to format with.
        template <typename... Args>
        inline static void append(std::string& output_string, const std::string& str, Args... args)
        {
            output_string.append(format(str, args...));
        }

        /// @brief Format a string, append new-line, then print it to stdout. 
        /// @param str The string to be formatted.
        /// @param ...args The variadic argument list to format with.
        template <typename... Args>
        inline static void println(const std::string& str, Args... args)
        {
            fputs(format(Colors::toANSI(Colors::getCurrentColor()) + str + '\n', args...).c_str(), stdout);
        }

        /// @brief Prints a new-line character to stdout.
        /// @tparam ...Args Placeholder template argument (required for function overload).
        template <typename... Args>
        inline static void println()
        {
            fputc('\n', stdout);
        }

        /// @brief Log a formatted message to stdout, according to its type.
        /// @param type The type of message to log (e.g. warning, error).
        /// @param str The string to be formatted.
        /// @param ...args The variadic argument list to format with.
        template <typename... Args>
        static void log(Type type, const std::string& str, Args... args)
        {
            #ifdef LINC_RELEASE
                if(type == Type::Debug)
                    return;
            #endif
            std::string type_string = logTypeToString(type);
            std::fputs(format(s_logFormat + '\n', type_string, format(str, args...)).c_str(), getLogTypeFile(type));
        }
    private:

        inline static FILE* getLogTypeFile(Type type)
        {
            switch(type)
            {
            case Type::Info:
            case Type::Debug:
                return stdout;
            case Type::Warning:
            case Type::Error:
                return stderr;
            default:
                throw LINC_EXCEPTION_OUT_OF_BOUNDS(Logger::Type);
            }
        }

        /// @brief Internal utility method to convert a printable value to string and append it, according to its type. 
        /// @param output The string to append the output to.
        /// @param printable The printable value to test.
        /// @param lexical_bool Flag corresponding to whether bools are represented using the keywords 'true' and 'false', instead of 1 and 0 respectively.
        /// @param precision The number of decimal digits to allow precision for.
        inline static void appendPrintable(std::string& output, Printable& printable, bool lexical_bool, size_t precision)
        {
            switch(printable.getType())
            {
            case Printable::Type::String: output += printable.getString(); break;
            case Printable::Type::SignedIntegral: output += printable.signedToString(); break;
            case Printable::Type::UnsignedIntegral: output += printable.unsignedToString(); break;
            case Printable::Type::Floating: output += printable.floatingToString(precision); break;
            case Printable::Type::Nullptr: output += printable.nullptrToString(); break;
            case Printable::Type::Boolean: output += printable.booleanToString(lexical_bool); break;
            case Printable::Type::Character: output += printable.characterToString(); break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Printable::Type);
            }
        }
        static std::string s_logFormat;
    };
}