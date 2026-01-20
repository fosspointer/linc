#pragma once
#include <unordered_map>
#include <chrono>
#include <linc/system/Formattable.hpp>
#include <linc/system/Colors.hpp>
#include <linc/system/Containers.hpp>

namespace linc
{
    class Logger final
    {
    public:
        Logger() = delete;
        enum class Level : std::uint_least8_t {
            Debug, Info, Verbose, Warning, Error, Critical
        };

        static std::string_view levelToString(Level level)
        {
            switch(level)
            {
            case Level::Debug: return "Debug";
            case Level::Info: return "Info";
            case Level::Verbose: return "Verbose";
            case Level::Warning: return "Warning";
            case Level::Error: return "Error";
            case Level::Critical: return "Critical";
            default: throw std::runtime_error("Log level enum out of range.");
            }
        }

        static void init()
        {
           s_initTime = std::chrono::system_clock::now(); 
        }

        template<typename... Args>
        static void log(Level kind, std::string_view _format, Args&&... args)
        {
            auto time_elapsed = std::chrono::system_clock::now() - s_initTime;
            auto milliseconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed).count();
            println("[$:p6s: $] $", milliseconds_elapsed / 1000.0f, levelToString(kind), format(_format, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static inline void print(std::string_view format, Args&&... args)
        {
            formatSinkImplementation(Format::printSink, format, Vector<Formattable>{std::forward<Args>(args)...}); 
        }

        template<typename... Args>
        static inline void println(std::string_view format, Args&&... args)
        {
            formatSinkImplementation(Format::printSink, format, Vector<Formattable>{std::forward<Args>(args)...}); 
            std::fputc('\n', stdout);
        }

        template<typename... Args>
        static inline String format(std::string_view format, Args&&... args)
        {
            return formatSinkImplementation(Format::stringSink, format, Vector<Formattable>{std::forward<Args>(args)...}); 
        }

        template<typename Sink, typename... Args>
        static inline Format::Appender<Sink>::SinkReturnType formatSink(Sink&& sink, std::string_view format, Args&&... args)
        {
            return formatSinkImplementation(std::forward<Sink>(sink), format, Vector<Formattable>{std::forward<Args>(args)...});
        }

        template <typename Sink>
        static Format::Appender<Sink>::SinkReturnType formatSinkImplementation(Sink&& sink, std::string_view _format, Vector<Formattable> args)
        {
            Format::Appender<Sink&&> appender(std::forward<Sink>(sink));
            if constexpr(std::is_same_v<typename Format::Appender<Sink>::SinkReturnType, String>)
                appender.getAggregate().reserve(_format.size() + args.size() * 5ul);

            static FormatOptions options;
            std::size_t current_argument{0ul};
            for(std::string::size_type i{0ul}; i < _format.size(); ++i)
            {
                bool append_argument{true};
                if(_format[i] == '$')
                {
                    if(i + 1ul < _format.size() && _format[i + 1ul] == '$')
                    {
                        appender.append('$');
                        ++i;
                    }
                    else
                    {
                        [&]()
                        {
                            while(++i < _format.size() && _format[i] == ':')
                            {
                                static char buffer[21ul];
                                switch(_format[++i])
                                {
                                case '!': append_argument = false; break;
                                case 'n': options.lexicalBool = false; break;
                                case 'l': options.lexicalBool = true; break;
                                case 'p':
                                {
                                    std::size_t buffer_index{0ul};
                                    while(i + 1ul < _format.size() && std::isdigit(_format[++i])) buffer[buffer_index++] = _format[i];
                                    buffer[buffer_index++] = '\0';

                                    if(buffer_index == 0ul)
                                        throw std::runtime_error("Precision specifier expects unsigned integral argument");
                                    options.floatingPrecision = std::stoull(buffer);
                                    --i;
                                    break;
                                }
                                case '0':
                                {
                                    static const std::unordered_map<char, std::uint8_t> bases{
                                        std::pair('b', 2u),
                                        std::pair('d', 10u),
                                        std::pair('x', 16u),
                                        std::pair('o', 8u)
                                    };
                                    auto option = _format[++i];
                                    options.numericBase = bases.at(option);
                                    break;
                                }
                                case '+':
                                { 
                                    static const std::unordered_map<char, Colors::Color> hues{
                                        std::pair('k', Colors::Black),
                                        std::pair('r', Colors::Red),
                                        std::pair('g', Colors::Green),
                                        std::pair('y', Colors::Yellow),
                                        std::pair('b', Colors::Blue),
                                        std::pair('p', Colors::Purple),
                                        std::pair('c', Colors::Cyan),
                                        std::pair('w', Colors::White),
                                    };

                                    auto option = _format[++i];
                                    Colors::Color color = {};
                                    if(option == 'x')
                                    {
                                        appender.append(Colors::push(Colors::Reset));
                                        break;
                                    }
                                    if(option == 'B')
                                    {
                                        color = color | Colors::Bold;
                                        option = _format[++i];
                                    }
                                    if(option == 'U')
                                    {
                                        color = color | Colors::Underline;
                                        option = _format[++i];
                                    }
                                    if(option == '@')
                                    {
                                        color = color | Colors::Background;
                                        option = _format[++i];
                                    }
                                    color = color | hues.at(option);
                                    appender.append(Colors::push(color));
                                    break;
                                }
                                case '-':
                                {
                                    appender.append(Colors::pop());
                                    break;
                                }
                                case '#':
                                {
                                    std::size_t buffer_index{0ul};
                                    while(++i < _format.size() && std::isdigit(_format[i])) buffer[buffer_index++] = _format[i];
                                    buffer[buffer_index++] = '\0';

                                    if(buffer_index == 0ul)
                                        throw std::runtime_error("Argument index expects unsigned integral argument.");

                                    append_argument = false;
                                    auto argument_index = std::stoul(buffer);
                                    appender.append(args[argument_index].formatSink(Format::identitySink, options));
                                    return;
                                }
                                case '$': --i; break;
                                case ':': return;
                                default:
                                    throw std::runtime_error("Unrecognized formatting option");
                                }
                            }
                        }();
                        --i;
                        if(append_argument) [[likely]]
                            appender.append(args[current_argument++].formatSink(Format::identitySink, options));
                    }
                }
                else appender.append(_format[i]);
            }

            return appender.getAggregate();
        }
    private:
        static std::chrono::system_clock::time_point s_initTime;
    };
}
