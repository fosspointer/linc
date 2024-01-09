#pragma once
#include <string>
#include <optional>
#include <cstdio>
#include <vector>
#include <linc/system/Exception.hpp>
#include <linc/system/Printable.hpp>

namespace linc
{
    class Logger final
    {
    public:
        enum class Type: char
        {
            Info, Warning, Error, Debug
        };

        static std::string logTypeToString(Type type);
        static std::string format(const std::string& str, std::vector<Printable> args);
        static void println();

        template <typename... Args>
        static void print(const std::string& str, Args... args)
        {
            fputs(format(str, args...).c_str(), stdout);
        }

        template <typename... Args>
        static void println(const std::string& str, Args... args)
        {
            puts(format(str, args...).c_str());
        }

        template <typename... Args>
        static void log(Type type, const std::string& str, Args... args)
        {
            #ifdef LINC_RELEASE
                if(type == Type::Debug)
                    return;
            #endif
            std::string type_string = logTypeToString(type);
            puts(format(s_logFormat, type_string, format(str, args...)).c_str());
        }

        template <typename... Args>
        static std::string format(const std::string& str, Args... args)
        {
            return format(str, std::vector<Printable>{args...});
        }
    private:
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
            case Printable::Type::TypedValue: output += printable.typedValueToString(); break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Printable::Type);
            }
        }
        static std::string s_logFormat;
    };
}