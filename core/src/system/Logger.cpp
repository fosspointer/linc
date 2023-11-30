#include <linc/system/Logger.hpp>

namespace linc
{
    std::string Logger::s_logFormat = "[$] $";

    std::string Logger::logTypeToString(Type type)
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

    void Logger::println()
    {
        fputs("\n", stdout);
    }

    std::string Logger::format(const std::string& str, std::vector<Printable> args)
    {
        std::string output;

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
}