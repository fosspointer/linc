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

#define LINC_ASSEMBLER "nasm"
#define LINC_LINKER "ld"
#define LINC_EXIT_SUCCESS 0
#define LINC_EXIT_FAILURE_LINC_EXCEPTION 1
#define LINC_EXIT_FAILURE_STANDARD_EXCEPTION 2
#define LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION 3
#define LINC_EXIT_COMPILATION_FAILURE 4

static std::string getFilename(std::string_view str)
{
    return std::filesystem::path(str).stem().string();
}

static inline std::string getPath(std::string_view str)
{
    return std::filesystem::path(str).parent_path().string();
}

static inline bool executableExists(std::string_view executable_name)
{
    const static std::string path = []()
    {
        auto path = std::getenv("PATH");
        return path? std::string{path}: std::string{};
    }();
    std::string::size_type start{}, end{};

    while(std::string::npos != (end = path.find(':', start)))
    {
        auto directory = path.substr(start, end - start);
        auto executable_test = std::filesystem::path(directory) / executable_name;

        if(std::filesystem::exists(executable_test)
        && std::filesystem::is_regular_file(executable_test)
        && std::filesystem::perms::none != (std::filesystem::status(executable_test).permissions()
            & (std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec)))
            return true;

        start = end + 1ul;
    }
    return false;
}

static auto compileCode(const std::string& raw_code, const std::string& filepath, std::vector<std::string> include_directories, bool optimization)
{
    auto code = linc::Code::toSource(raw_code, filepath);

    linc::Lexer lexer(code, true);
    lexer.appendIncludeDirectories(std::move(include_directories));
    auto tokens = lexer();

    linc::Preprocessor preprocessor(tokens, filepath);
    auto processed_code = preprocessor();

    linc::Parser parser;
    parser.set(processed_code, filepath);
    linc::Binder binder;
    
    auto program = parser();
    auto bound_program = binder.bindProgram(&program);
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
    const static auto option_include = 'i', option_output = 'o', option_version = 'v', option_optimization = 'O', option_compile_only = 'c', option_notice = 'C';
    constexpr const char* notice = 
        #include "notice"
    ;
    linc::Logger::log(linc::Logger::Type::Warning, "Note that the linc compiler is still in an experimental stage and does not adequately support linc syntax");

    Arguments argument_handler(argument_count, arguments, std::unordered_map<char, Arguments::Option>{
        std::pair(option_version, Arguments::Option{.description = "Display the current Linc version in use.", .flag = true}),
        std::pair(option_include, Arguments::Option{.description = "Specify a custom include path."}),
        std::pair(option_output, Arguments::Option{.description = "Specify the output symbol."}),
        std::pair(option_optimization, Arguments::Option{.description = "Use optimization.", .flag = true}),
        std::pair(option_compile_only, Arguments::Option{.description = "Compile to object file(s) only; do not link.", .flag = true}),
        std::pair(option_notice, Arguments::Option{.description = "Display the legal notice.", .flag = true}),
    }, std::vector<std::pair<std::string, char>>{
        std::pair("--include", option_include),
        std::pair("--output", option_output),
        std::pair("--version", option_version),
        std::pair("--optimization", option_optimization),
        std::pair("--compile-only", option_compile_only),
        std::pair("--notice", option_notice),
    });

    if(!linc::Reporting::getReports().empty())
        return linc::Reporting::hasError()? LINC_EXIT_COMPILATION_FAILURE: LINC_EXIT_SUCCESS;
    else if(!argument_handler.get(option_notice).empty())
    {
        linc::Logger::log(linc::Logger::Type::Info, "$", notice);
        return LINC_EXIT_SUCCESS;
    }
    else if(!argument_handler.get(option_version).empty())
    {
        linc::Logger::log(linc::Logger::Type::Info, "Linc version $", LINC_VERSION);
        return LINC_EXIT_SUCCESS;
    }
    else if(!executableExists(LINC_ASSEMBLER))
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = linc::Logger::format("Assembler executable `$` not found in PATH.", LINC_ASSEMBLER)
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }
    else if(!argument_handler.get(option_compile_only).empty() && !executableExists(LINC_LINKER))
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = linc::Logger::format("Linker executable `$` not found in PATH.", LINC_LINKER)
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }

    auto files = argument_handler.getDefaults();
    auto output = argument_handler.get(option_output);
    auto optimization = !argument_handler.get(option_optimization).empty(); 

    bool found_entry_point{false};
    std::string binary_filename;

    if(output.size() > 1ul)
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = "Cannot provide more than one object or binary output name."
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }
    else if(!output.empty()) binary_filename = output.at(0ul);
    
    if(files.empty())
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = "No input file(s) provided."
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }

    std::vector<std::pair<std::string, bool>> code;
    std::string linker_command{linc::Logger::format("LD_LIBRARY_PATH=$/lib $ ", LINC_INSTALL_PATH, LINC_LINKER)};

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
        auto raw_code = linc::Files::read(file);
        auto build_directory = getPath(binary_filename);
        auto [assembly, file_main] = compileCode(raw_code, linc::Files::toAbsolute(file), argument_handler.get(option_include), optimization);
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
            
            if(binary_filename.empty()) binary_filename = getFilename(file);
        }

        if(assembly.empty())
            return LINC_EXIT_COMPILATION_FAILURE;

        auto stem = build_directory.empty()? std::filesystem::current_path(): std::filesystem::path(build_directory);
        auto filepath = stem / getFilename(file);

        linc::Files::write(linc::Logger::format("$.asm", filepath), assembly);
        std::system(linc::Logger::format("$ -felf64 $:#1.asm -o $.o", LINC_ASSEMBLER, filepath).c_str());
        linc::Logger::append(linker_command, "$.o ", filepath);
    }

    if(!argument_handler.get(option_compile_only).empty())
        return LINC_EXIT_SUCCESS;
    else if(!found_entry_point)
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = "No entry point provided; linking aborted."
        });
        return LINC_EXIT_SUCCESS;
    }

    linc::Logger::append(linker_command, "-o $ -llinc -dynamic-linker /lib64/ld-linux-x86-64.so.2", binary_filename);
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
    linc::Logger::log(linc::Logger::Type::Error, "[UNKNOWN EXCEPTION]");
    return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
}