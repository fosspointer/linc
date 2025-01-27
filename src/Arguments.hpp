#pragma once
#include <linc/Include.hpp>

class Arguments final 
{
public:
    struct Option
    {
        std::string description;
        bool flag{false};
    };

    struct Argument
    {
        int index;
        std::optional<std::string> value;
    };

    Arguments(int argument_count, const char** arguments, std::unordered_map<char, Option> options,
        std::vector<std::pair<std::string, char>> option_names)
        :m_program(arguments[0ul]), m_options(std::move(options)), m_optionNames(option_names)
    {
        const_cast<std::vector<std::pair<std::string, char>>&>(m_optionNames).push_back(std::pair("--help", 'h'));

        bool default_arguments{false};
        for(int i{1}; i < argument_count; ++i)
        {
            const auto full_argument = std::string{arguments[i]};
            const auto argument = full_argument.length() == 2ul && full_argument[0ul] == '-'? full_argument[1ul]: findOptionShortName(full_argument);
            
            if(default_arguments || !argument)
            {
                m_defaultArguments.push_back(Argument {.index = i, .value = full_argument});
                default_arguments = true;
                continue;
            }
            else if(argument == 'h')
            {
                printUsage();
                linc::Reporting::push(linc::Reporting::Report{
                    .type = linc::Reporting::Type::Info, .stage = linc::Reporting::Stage::Environment,
                    .message = "Displayed help menu."
                }, false);
                break;
            }

            auto find = m_options.find(argument);

            if(find == m_options.end())
            {
                linc::Reporting::push(linc::Reporting::Report{
                    .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
                    .message = linc::Logger::format("Unrecognized option `$`. Use --help for more info.", argument)
                });
                break;
            }

            else if(find->second.flag)
            {
                m_namedArguments.insert(std::pair(argument, std::nullopt));
                continue;
            }

            else if(i >= argument_count - 1)
            {
                linc::Reporting::push(linc::Reporting::Report{
                    .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
                    .message = linc::Logger::format("Specified option `$` but didn't provide a value.", full_argument)
                });
                break;
            }

            auto value = arguments[i + 1];
            m_namedArguments.insert(std::pair(argument, Argument{.index = i, .value = value}));
            ++i;
        }
    }

    void printUsage()
    {
        linc::Logger::println("$ [Usage]\n--help (-h): Display this menu.", m_program);
            
        for(const auto& option: m_options)
            linc::Logger::println("$ (-$): $", findOptionFullName(option.first), option.first, option.second.description);
    }

    inline std::vector<std::string> get(char short_name) const
    {
        std::vector<std::string> result;
        auto lookup = m_namedArguments.equal_range(short_name);
        result.reserve(std::distance(lookup.first, lookup.second));

        for(auto iterator = lookup.first; iterator != lookup.second; ++iterator)
        {
            result.push_back(iterator->second->value.value_or(std::string{}));
            if(result.back().empty()) break;
        }

        return result;
    }

    inline int getFirstDefaultIndex() const
    {
        return m_defaultArguments.empty()? -1: m_defaultArguments[0ul].index;
    }

    inline const std::vector<Argument>& getDefaults() const { return m_defaultArguments; }
    inline std::string findOptionFullName(char short_name) const
    {
        for(const auto& item: m_optionNames)
            if(item.second == short_name)
                return item.first;
        
        return std::string{};
    }

    inline char findOptionShortName(const std::string& full_name) const
    {
        for(const auto& item: m_optionNames)
            if(item.first == full_name)
                return item.second;
        
        return '\0';
    }
private:
    const std::string m_program;
    const std::unordered_map<char, Option> m_options;
    const std::vector<std::pair<std::string, char>> m_optionNames;
    std::unordered_multimap<char, std::optional<Argument>> m_namedArguments;
    std::vector<Argument> m_defaultArguments;
};
