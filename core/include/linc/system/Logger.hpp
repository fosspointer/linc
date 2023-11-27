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

        static std::string logTypeToString(Type type)
        {
            switch(type)
            {
            case Type::Debug:   return "\e[1;36mDEBUG\e[0m"; 
            case Type::Info:    return "\e[1;34mINFO\e[0m";
            case Type::Warning: return "\e[1;33mWARNING\e[0m"; 
            case Type::Error:   return "\e[1;31mERROR\e[0m"; 
            default:
                throw LINC_EXCEPTION_OUT_OF_BOUNDS(Logger::Type);
            }
        }

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

        static void println()
        {
            fputs("\n", stdout);
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
        static std::string format(const std::string& str, Args... _args)
        {
            std::string output;
            std::vector<Printable> args{_args...};

            bool lexical_bool{true};
            size_t precision{6};

            for(std::string::size_type i = 0; i < str.size(); i++)
            {
                if(str[i] == '$')
                {
                    if(i + 1 < str.size() && str[i + 1] == '$')
                    {
                        output.push_back('$');
                        ++i;
                    }    
                    else
                    {
                        [&]()
                        {
                            while (str[++i] == ':')
                            {
                                switch(str[++i])
                                {
                                case 'n': lexical_bool = 0; break;
                                case 'l': lexical_bool = 1; break;
                                case 'p':
                                    {
                                        std::string buf;
                                        while(i + 1 < str.size() && std::isdigit(str[++i]))
                                        {
                                            buf.push_back(str[i]);
                                        }

                                        if(buf == "")
                                            throw LINC_EXCEPTION_INVALID_INPUT("Precision specifier expects unsigned integral argument");
                                        precision = std::stoull(buf);
                                        break;
                                    }
                                case '$': i--; break;
                                case ':': return;
                                default:
                                    throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized formatting option");
                                }
                            }
                        }();
                        --i;
                        auto printable = args[0];
                        args.erase(args.begin());
                        appendPrintable(output, printable, lexical_bool, precision);
                    }
                }
                else output.push_back(str[i]);
            }

            return output;
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
            case Printable::Type::TypedValue: output += printable.typedValueToString(lexical_bool, precision); break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Printable::Type);
            }
        }
        static std::string s_logFormat;
    };
}