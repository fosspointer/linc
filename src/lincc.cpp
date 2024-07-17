#include <linc/Preprocessor.hpp>
#include <linc/System.hpp>
#include <linc/Lexer.hpp>
#include <linc/Tree.hpp>
#include <linc/Parser.hpp>
#include <linc/BoundTree.hpp> 
#include <linc/Binder.hpp>
#include <linc/Generator.hpp>
#include <linc/generator/Optimizer.hpp>
#include "Arguments.hpp"
#ifdef LINC_WINDOWS
#include "Windows.hpp"
#endif

#define LINC_EXIT_SUCCESS 0
#define LINC_EXIT_FAILURE_LINC_EXCEPTION 1
#define LINC_EXIT_FAILURE_STANDARD_EXCEPTION 2
#define LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION 3
#define LINC_EXIT_COMPILATION_FAILURE 4

static std::string filepath_to_directory(const std::string& str)
{
    return str.substr(0ul, str.find_last_of('/'));
}

static std::string cut_filename(std::string str)
{
    auto find = str.find_last_of('/');
    if(find == std::string::npos)
        return str.substr(0ul, str.find_last_of('.'));

    str = str.substr(find + 1);
    return str.substr(0ul, str.find_last_of('.'));
}

static auto compile_code(const std::string& raw_code, const std::string& filepath, std::vector<std::string> include_directories, bool optimization)
{
    auto code = linc::Code::toSource(raw_code, filepath);

    linc::Lexer lexer(code);
    lexer.appendIncludeDirectories(std::move(include_directories));
    auto tokens = lexer();

    linc::Preprocessor preprocessor(tokens, filepath);
    auto processed_code = preprocessor();

    linc::Parser parser;
    parser.set(processed_code, filepath);
    linc::Binder binder;
    
    auto program = parser();
    auto bound_program = binder.bind(&program);
    if(optimization)
        bound_program = linc::Optimizer::optimizeProgram(bound_program);
    
    if(!linc::Reporting::hasError())
        return linc::Generator::operator()(&bound_program, linc::Target{
            .architecture = linc::Target::Architecture::AMD64,
            .platform = linc::Target::Platform::Unix
        });
    else return std::pair<std::string, bool>({}, {});
}

int main(int argument_count, const char** arguments)
try 
{
#ifdef LINC_WINDOWS
    linc::Windows::enableAnsi();
#endif
    const static auto option_include = 'i', option_output = 'o', option_version = 'v', option_optimization = 'O';
    const static auto internal_source{LINC_INSTALL_PATH "/include/stdinternal"};

    Arguments argument_handler(argument_count, arguments, std::unordered_map<char, Arguments::Option>{
        std::pair(option_include, Arguments::Option{.description = "Specify a custom include path."}),
        std::pair(option_output, Arguments::Option{.description = "Manually name the resulting output symbol."}),
        std::pair(option_optimization, Arguments::Option{.description = "Use optimization.", .flag = true}),
        std::pair(option_version, Arguments::Option{.description = "Display the current Linc version in use.", .flag = true}),
    }, std::vector<std::pair<std::string, char>>{
        std::pair("--include", option_include),
        std::pair("--output", option_output),
        std::pair("--version", option_version),
        std::pair("--optimization", option_optimization),
    });

    if(!linc::Reporting::getReports().empty())
        return linc::Reporting::hasError()? LINC_EXIT_COMPILATION_FAILURE: LINC_EXIT_SUCCESS;
    else if(!argument_handler.get(option_version).empty())
    {
        linc::Logger::log(linc::Logger::Type::Info, "Linc version $", LINC_VERSION);
        return LINC_EXIT_SUCCESS;
    }

    auto files = argument_handler.getDefaults();
    auto output = argument_handler.get(option_output);
    auto optimization = !argument_handler.get(option_optimization).empty(); 

    bool found_entry_point{false};
    std::string binary_name;

    if(output.size() > 1ul)
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = "Cannot provide more than one object or binary output name."
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }
    else if(!output.empty()) binary_name = output.at(0ul);
    
    if(files.empty())
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = "No input file(s) provided."
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }

    std::vector<std::pair<std::string, bool>> code;
    std::string linker_command{linc::Logger::format("ld $.o ", cut_filename(internal_source))};

    std::system(linc::Logger::format("nasm -felf64 $.s -o $.o", internal_source, cut_filename(internal_source)).c_str());
    for(const auto& file: files)
    {
        if(!linc::Files::exists(file))
        {
            linc::Reporting::push(linc::Reporting::Report{
                .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
                .message = linc::Logger::format("Could not read file `$`", file)
            });
            return LINC_EXIT_COMPILATION_FAILURE;
        }
        auto filepath = linc::Files::toAbsolute(file);
        auto raw_code = linc::Files::read(file);
        auto [assembly, file_main] = compile_code(raw_code, filepath, argument_handler.get(option_include), optimization);
        
        if(assembly.empty())
            return LINC_EXIT_COMPILATION_FAILURE;

        if(file_main)
        {
            if(found_entry_point)
            {
                linc::Reporting::push(linc::Reporting::Report{
                    .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
                    .message = linc::Logger::format("Redefinition of entry-point function main between different files.", file)
                });
                return LINC_EXIT_COMPILATION_FAILURE;
            }
            else found_entry_point = true;
            
            if(binary_name.empty()) binary_name = cut_filename(file);
        }

        auto binary_path = cut_filename(filepath);
        linc::Files::write(linc::Logger::format("$.s", binary_path), assembly);
        std::system(linc::Logger::format("nasm -felf64 $:#0.s -o $.o", binary_path).c_str());
        linc::Logger::append(linker_command, "$.o ", binary_path);
    }

    if(!found_entry_point)
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Info, .stage = linc::Reporting::Stage::Environment,
            .message = "No entry point was given, only object files created!"
        });
        return LINC_EXIT_SUCCESS;
    }

    linc::Logger::append(linker_command, "-o $", binary_name);
    return std::system(linker_command.c_str());
}
catch(const linc::Exception& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "[LINC EXCEPTION] $", e.info());

    return LINC_EXIT_FAILURE_LINC_EXCEPTION;
}
catch(const std::exception& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "[STANDARD EXCEPTION] $", e.what());
    return LINC_EXIT_FAILURE_STANDARD_EXCEPTION;
}
catch(...)
{
    linc::Logger::log(linc::Logger::Type::Error, "[UNKNOWN EXCPETION]");
    return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
}