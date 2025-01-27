#pragma once
#include <linc/Include.hpp>
#include <linc/system/Colors.hpp>
#include <linc/system/Printable.hpp>

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
        static std::string format(std::string_view str, const std::vector<Printable>& _args);

        /// @brief String formatting utility method, similar to std::format.
        /// @param str The string to be formatted.
        /// @param ...args The variadic argument list to format with.
        /// @return The resulting string.
        template <typename... Args>
        static inline std::string format(std::string_view str, Args... args)
        {
            return format(str, std::vector<Printable>{args...});
        }

        /// @brief Print single new-line character. 
        static void println();

        /// @brief Format a string and print it to stdout.
        /// @param str The string to be formatted.
        /// @param ...args The variadic argument list to format with.
        template <typename... Args>
        inline static void print(std::string_view str, Args... args)
        {
            fputs(format(Colors::toANSI(Colors::getCurrentColor()) + std::string{str}, args...).c_str(), stdout);
        }

        /// @brief Read string from stdin until return key entered.
        /// @return The resulting string, as read from stdin.
        static std::string read(std::string_view prompt = "", bool is_shell = false);

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
        inline static void println(std::string_view str, Args... args)
        {
            fputs(format(Colors::toANSI(Colors::getCurrentColor()) + std::string{str} + '\n', args...).c_str(), stdout);
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
        static FILE* getLogTypeFile(Type type);

        /// @brief Internal utility method to convert a printable value to string and append it, according to its type. 
        /// @param output The string to append the output to.
        /// @param printable The printable value to test.
        /// @param lexical_bool Flag corresponding to whether bools are represented using the keywords 'true' and 'false', instead of 1 and 0 respectively.
        /// @param precision The number of decimal digits to allow precision for.
        static void appendPrintable(std::string& output, class Printable& printable, bool lexical_bool, std::size_t precision);
        static std::string s_logFormat;
    };
}