#include <linc/system/Logger.hpp>
#include <linc/system/Colors.hpp>
#include <linc/system/Exception.hpp>
#ifdef LINC_LINUX
#include <unistd.h>
#include <termios.h>
#endif

static void reprintLine(const std::string& line, std::string_view prompt, std::size_t position)
{
    std::fputs(std::string('\r' + std::string{prompt} + "\x1B[0K" + line).c_str(), stdout);
    
    if(position < line.length())
        std::fputs(std::string("\x1B[" + std::to_string(line.length() - position) + 'D').c_str(), stdout);
}

namespace linc
{
    std::string Logger::s_logFormat = "$ $";
    
    std::string Logger::logTypeToString(Type type)
    {
        static const auto log_type_format{"\x1B[1;$m$\x1B[0m"};
        switch(type)
        {
        case Type::Debug:   return Logger::format(log_type_format, 36, "DEBUG");
        case Type::Info:    return Logger::format(log_type_format, 34, "INFO");
        case Type::Warning: return Logger::format(log_type_format, 33, "WARNING");
        case Type::Error:   return Logger::format(log_type_format, 31, "ERROR");
        default:
            throw LINC_EXCEPTION_OUT_OF_BOUNDS(type);
        }
    }

    void Logger::println()
    {
        std::fputc('\n', stdout);
    }

    std::string Logger::read(std::string_view prompt)
    {
    #ifdef LINC_LINUX
        static constexpr auto csi_back{'D'}, csi_forward{'C'}; 
        static constexpr auto csi_delete{"3~"}, csi_control{";5"};
        std::fputs(std::string(prompt).c_str(), stdout);

        struct termios old_term, new_term;
        tcgetattr(STDIN_FILENO, &old_term);
        new_term = old_term;
        new_term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

        std::string result;
        std::string::size_type index{};

        while(true)
        {
            char character = std::getchar();
            if(character == '\n')
            {
                std::fputc('\n', stdout); break;
            }

            if(character == 127ul || character == '\b')
            {
                if(index == 0ul)
                {
                    std::fputc('\a', stdout);
                    continue;
                }

                result.erase(index -1ul, 1ul);
                index--;
                reprintLine(result, prompt, index);
                continue;
            }
            else if(character == 12)
            {
                std::fputs("\033c", stdout);
                reprintLine(result, prompt, index);
                continue;
            }
            else if(character == 1)
            {
                index = 0ul;
                reprintLine(result, prompt, index);
                continue;
            }
            else if(character == '\033')
            {
                std::getchar();
                switch(std::getchar())
                {
                case csi_back: // Left arrow
                    if(index != 0ul)
                    {
                        index--;
                        std::fputs((std::string("\x1B[") + csi_back).c_str(), stdout);
                    }
                    else std::fputc('\a', stdout);
                    break;
                case csi_forward: // Right arrow
                    if(index < result.length())
                    {
                        index++;
                        std::fputs((std::string("\x1B[") + csi_forward).c_str(), stdout);
                    }
                    else std::fputc('\a', stdout);
                    break;
                case csi_delete[0ul]: // Delete key
                {
                    auto character = std::getchar();
                    if(character == csi_delete[1ul]) // Delete key
                    {
                        if(index < result.length())
                        {
                            result.erase(index, 1ul);
                            reprintLine(result, prompt, index);
                        }
                        else std::fputc('\a', stdout);
                    }
                    else if(character == csi_control[0ul] && std::getchar() == csi_control[1ul]
                        && std::getchar() == csi_delete[1ul])
                    {
                        auto delete_index = index;
                        while(delete_index < result.size() && result[delete_index] == ' ') ++delete_index;
                        while(delete_index < result.size() && result[delete_index] != ' ') ++delete_index;
                        result.erase(index, delete_index);
                        reprintLine(result, prompt, index);
                    }
                    else std::fputc('\a', stdout);
                    break;
                }
                case '1':
                {
                    if(std::getchar() != csi_control[0ul] || std::getchar() != csi_control[1ul]) break;
                    auto character = std::getchar();
                    if(character == csi_back)
                    {
                        while(index != 0ul && result[index - 1ul] == ' ') { --index; std::fputs((std::string("\x1B[") + csi_back).c_str(), stdout); }
                        while(index != 0ul && result[index - 1ul] != ' ') { --index; std::fputs((std::string("\x1B[") + csi_back).c_str(), stdout); }
                    }
                    else if(character == csi_forward)
                    {
                        while(index < result.size() && result[index] == ' ') { ++index; std::fputs((std::string("\x1B[") + csi_forward).c_str(), stdout); }
                        while(index < result.size() && result[index] != ' ') { ++index; std::fputs((std::string("\x1B[") + csi_forward).c_str(), stdout); }
                    }
                    else std::fputc('\a', stdout);
                    break;
                }
                }
                continue;
            }
            else if(std::isprint(character))
            {
                result.insert(index, 1ul, character);
                index++;
                reprintLine(result, prompt, index);
                continue;
            }
            
            std::fputc('\a', stdout);
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        return result;
    #else
        std::cout << prompt;
        std::string result;
        std::getline(std::cin, result);
        return result;
    #endif
    }

    std::string Logger::format(std::string_view str, const std::vector<Printable>& _args)
    {
        std::vector<Printable> args{_args};

        std::string output{};
        output.reserve(2ul * str.size());

        bool lexical_bool{true};
        size_t precision{6ul};

        for(std::string::size_type i{0ul}; i < str.size(); ++i)
        {
            if(str[i] == '$')
            {
                if(i + 1ul < str.size() && str[i + 1ul] == '$')
                {
                    output.push_back('$');
                    ++i;
                }    
                else
                {
                    [&]()
                    {
                        while(str[++i] == ':')
                        {
                            switch(str[++i])
                            {
                            case 'n': lexical_bool = 0; break;
                            case 'l': lexical_bool = 1; break;
                            case 'p':
                            {
                                std::string buffer;
                                while(i + 1ul < str.size() && std::isdigit(str[++i]))
                                    buffer.push_back(str[i]);

                                if(buffer.empty())
                                    throw LINC_EXCEPTION_INVALID_INPUT("Precision specifier expects unsigned integral argument.");
                                precision = std::stoull(buffer);
                                --i;
                                break;
                            }
                            case '#':
                            {
                                std::string buffer;
                                while(i < str.size() && std::isdigit(str[++i]))
                                    buffer.push_back(str[i]);

                                if(buffer.empty())
                                    throw LINC_EXCEPTION_INVALID_INPUT("Argument index expects unsigned integral argument.");

                                if(args.empty())
                                    args.push_back(_args.at(std::stoull(buffer)));
                                else args.insert(args.begin(), _args.at(std::stoull(buffer)));

                                --i;
                                break;
                            }
                            case '$': --i; break;
                            case ':': return;
                            default:
                                throw LINC_EXCEPTION_INVALID_INPUT("Unrecognized formatting option.");
                            }
                        }
                    }();
                    --i;
                    auto printable = args[0ul];
                    args.erase(args.begin());
                    appendPrintable(output, printable, lexical_bool, precision);
                }
            }
            else output.push_back(str[i]);
        }

        return output;
    }

    FILE* Logger::getLogTypeFile(Type type)
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
            throw LINC_EXCEPTION_OUT_OF_BOUNDS(type);
        }
    }

    void Logger::appendPrintable(std::string& output, class Printable& printable, bool lexical_bool, std::size_t precision)
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
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(printable.getType());
        }
    }
}