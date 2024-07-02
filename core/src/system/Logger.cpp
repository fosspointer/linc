#include <linc/system/Logger.hpp>
#include <linc/system/Colors.hpp>
#ifdef LINC_LINUX
#include <unistd.h>
#include <termios.h>
#endif

static void reprint_line(const std::string& line, const std::string& prompt, std::size_t position)
{
    std::fputs(std::string('\r' + prompt + "\x1B[0K" + line).c_str(), stdout);
    
    if(position < line.length())
        std::fputs(std::string("\x1B[" + std::to_string(line.length() - position) + 'D').c_str(), stdout);
}

namespace linc
{
    std::string Logger::s_logFormat = "[$] $";
    
    std::string Logger::logTypeToString(Type type)
    {
        static const auto log_type_format{"\e[1;$m$\e[0m"};
        switch(type)
        {
        case Type::Debug:   return Logger::format(log_type_format, 36, "DEBUG");
        case Type::Info:    return Logger::format(log_type_format, 34, "INFO");
        case Type::Warning: return Logger::format(log_type_format, 33, "WARNING");
        case Type::Error:   return Logger::format(log_type_format, 31, "ERROR");
        default:
            throw LINC_EXCEPTION_OUT_OF_BOUNDS(Logger::Type);
        }
    }

    void Logger::println()
    {
        std::fputc('\n', stdout);
    }

    std::string Logger::read(const std::string& prompt)
    {
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
                reprint_line(result, prompt, index);
                continue;
            }
            else if(character == 12)
            {
                std::fputs("\033c", stdout);
                reprint_line(result, prompt, index);
                continue;
            }
            else if(character == 1)
            {
                index = 0ul;
                reprint_line(result, prompt, index);
                continue;
            }
            else if(character == '\033')
            {
                std::getchar();

                switch (std::getchar())
                {
                case 'D': // Left arrow
                    if(index > 0ul)
                    {
                        index--;
                        std::fputs("\x1B[D", stdout);
                    }
                    else std::fputc('\a', stdout);
                    break;
                case 'C': // Right arrow
                    if(index < result.length())
                    {
                        index++;
                        std::fputs("\x1B[C", stdout);
                    }
                    else std::fputc('\a', stdout);
                    break;
                case '3': // Delete key
                    if (std::getchar() == '~') // Delete key
                        if(index < result.length())
                        {
                            result.erase(index, 1ul);
                            reprint_line(result, prompt, index);
                        }
                        else std::fputc('\a', stdout);
                    break;
                }
                continue;
            }
            else if(std::isprint(character))
            {
                result.insert(index, 1ul, character);
                index++;
                reprint_line(result, prompt, index);
                continue;
            }
            
            std::fputc('\a', stdout);
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        return result;
    }

    std::string Logger::format(const std::string& str, const std::vector<Printable>& _args)
    {
        std::vector<Printable> args{_args};

        std::string output{};

        bool lexical_bool{true};
        size_t precision{6};

        for(std::string::size_type i = 0ul; i < str.size(); ++i)
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
                        while (str[++i] == ':')
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
                            case '$': i--; break;
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
}