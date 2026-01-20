#include <cstring>
#include <linc/system/Logger.hpp>

int main(int argument_count, const char** arguments)
{
    if(argument_count < 2)
    {
        std::fputs("Invalid arguments given to print. Need format argument.", stderr);
        return -1;
    }
    std::string format_string = arguments[1ul];

    for(std::size_t i{0ul}; i < format_string.size(); ++i)
        if(format_string[i] == '\\')
        {
            format_string.erase(i, 1ul);
            switch(format_string[i])
            {
            case 'a': format_string[i] = '\a'; break;
            case 'b': format_string[i] = '\b'; break;
            case 'e': format_string[i] = '\x1B'; break;
            case 'f': format_string[i] = '\f'; break;
            case 'n': format_string[i] = '\n'; break;
            case 'r': format_string[i] = '\r'; break;
            case 't': format_string[i] = '\t'; break;
            case 'v': format_string[i] = '\v'; break;
            case '\\': format_string[i] = '\\'; break;
            case '\'': format_string[i] = '\''; break;
            case '"': format_string[i] = '"'; break;
            case '$': format_string[i] = '$'; break;
            }
        }

    linc::Vector<linc::Formattable> argument_views;
    argument_views.reserve(argument_count - 2ul);

    for(int i{2}; i < argument_count; ++i)
        argument_views.push_back(arguments[static_cast<std::size_t>(i)]);

    linc::Logger::formatSinkImplementation(linc::Format::printSink, format_string, std::move(argument_views));
}
