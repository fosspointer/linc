#include <linc/Preprocessor.hpp>
#include <linc/System.hpp>
#include <linc/Lexer.hpp>
#include <linc/Tree.hpp>
#include <linc/Parser.hpp>
#include <linc/BoundTree.hpp> 
#include <linc/Binder.hpp>
#include <linc/Generator.hpp>
#include "Arguments.hpp"

#ifdef LINC_WINDOWS
#include "Windows.hpp"
#endif

#define LINC_EXIT_SUCCESS 0
#define LINC_EXIT_FAILURE_LINC_EXCEPTION 1
#define LINC_EXIT_FAILURE_STANDARD_EXCEPTION 2
#define LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION 3
#define LINC_EXIT_COMPILATION_FAILURE 4

#ifdef LINC_DEBUG
#define LINC_EXCEPTION_WARNING ""
#else
#define LINC_EXCEPTION_WARNING "This is probaby not intended, please contact the developer of this software to fix it."
#endif

static int evaluate_file(std::string filepath, int argc, const char** argv, Arguments& argument_handler)
{
    filepath = linc::Files::toAbsolute(filepath);

    if(!linc::Files::exists(filepath))
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Preprocessor,
            .message = linc::Logger::format("Specified filepath '$' does not exist!", filepath)
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }
    auto raw_code = linc::Files::read(filepath);
    auto code = linc::Code::toSource(raw_code, filepath);

    linc::Lexer lexer(code);
    lexer.appendIncludeDirectories(argument_handler.get('i'));
    auto tokens = lexer();

    linc::Preprocessor preprocessor(tokens, filepath);
    auto processed_code = preprocessor();

    linc::Parser parser;
    parser.set(processed_code, filepath);
    linc::Binder binder;
    
    auto program = parser();
    auto bound_program = binder.bind(&program);
    bool errors{false};
    
    for(const auto& report: linc::Reporting::getReports())
        if(report.type == linc::Reporting::Type::Error)
            errors = true;

    if(!errors)
    {
        linc::Interpreter interpreter;
        std::vector<linc::ArrayInitializerExpression::Argument> arguments;
        for(std::size_t i = 0ul; i < argc; ++i)
            arguments.push_back(linc::ArrayInitializerExpression::Argument{
                .separator = i == argc - 1ul? std::nullopt: std::make_optional(linc::Token{.type = linc::Token::Type::Comma}),
                .value = std::make_unique<const linc::LiteralExpression>(linc::Token{
                    .type = linc::Token::Type::StringLiteral,
                    .value = argv[i]
                })
            });

        return interpreter.evaluateProgram(&bound_program, binder, std::make_unique<const linc::ArrayInitializerExpression>(
            linc::Token{.type = linc::Token::Type::SquareLeft}, linc::Token{.type = linc::Token::Type::SquareRight}, std::move(arguments)
        ));
    }
    else return LINC_EXIT_COMPILATION_FAILURE;
}

int main(int argument_count, const char** arguments)
try
{
#ifdef LINC_WINDOWS
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if(console == INVALID_HANDLE_VALUE)
    {
        linc::Logger::log(linc::Logger::Type::Error, "Failed to get console handle.");
        return LINC_EXIT_COMPILATION_FAILURE;
    }

    DWORD mode;
    if(!GetConsoleMode(console, &mode))
    {
        linc::Logger::log(linc::Logger::Type::Error, "Failed to get console mode.");
        return LINC_EXIT_COMPILATION_FAILURE;
    }
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if(!SetConsoleMode(console, mode))
    {
        linc::Logger::log(linc::Logger::Type::Error, "Failed to set console handle.");
        return LINC_EXIT_COMPILATION_FAILURE;
    }
#endif
    const static auto option_include = 'i', option_eval = 'e', option_version = 'v';
    
    Arguments argument_handler(argument_count, arguments, std::unordered_map<char, Arguments::Option>{
        std::pair(option_include, Arguments::Option{.description = "Specify a custom include path."}),
        std::pair(option_eval, Arguments::Option{.description = "Evaluate a given statement."}),
        std::pair(option_version, Arguments::Option{.description = "Display the current Linc version in use.", .flag = true})
    }, std::vector<std::pair<std::string, char>>{
        std::pair("--include", option_include),
        std::pair("--eval", option_eval),
        std::pair("--version", option_version)
    });

    if(!linc::Reporting::getReports().empty())
        return linc::Reporting::hasError()? LINC_EXIT_COMPILATION_FAILURE: LINC_EXIT_SUCCESS;
    else if(!argument_handler.get(option_version).empty())
    {
        linc::Logger::log(linc::Logger::Type::Info, "Linc version $", LINC_VERSION);
        return LINC_EXIT_SUCCESS;
    }

    auto files = argument_handler.getDefaults();
    auto evaluate_statements = argument_handler.get(option_eval);

    if(!evaluate_statements.empty())
    {
        for(std::size_t i{0ul}; i < evaluate_statements.size(); ++i)
        {
            const auto path = "constant-input-" + std::to_string(i);
            linc::Binder binder;
            linc::Interpreter interpreter;
            linc::Parser parser;

            auto code = linc::Code::toSource(evaluate_statements[i]);
            linc::Lexer lexer(code);
            linc::Preprocessor preprocessor(lexer(), path);
            parser.set(preprocessor(), path);
            
            const auto statement = parser.parseStatement();
            const auto bound_statement = binder.bindStatement(statement.get());
            linc::Logger::println("$::`$` >> $", linc::PrimitiveValue{i}, 
                linc::Colors::toANSI(linc::Colors::Color::Cyan) + evaluate_statements[i] + linc::Colors::toANSI(linc::Colors::Color::Default),
                interpreter.evaluateStatement(bound_statement.get()));
        }
        return linc::Reporting::getReports().size()? LINC_EXIT_COMPILATION_FAILURE: LINC_EXIT_SUCCESS;
    }

    if(files.size() != 0ul)
        return evaluate_file(files.at(0ul), argument_count, arguments, argument_handler);

    bool show_tree{false}, show_lexer{false}, optimization{false};

    linc::Binder binder;
    linc::Interpreter interpreter;
    linc::Parser parser;
    
    bool success{true};

    auto init = [&]()
    {
        binder.reset();
        interpreter.reset();
        linc::Preprocessor::reset();

        static const auto shell_name = "./include-std";
        static const auto include_code = "#include `std.linc`";
        auto code = linc::Code::toSource(include_code, shell_name);

        linc::Lexer lexer(code);
        lexer.appendIncludeDirectories(argument_handler.get('i'));

        linc::Preprocessor preprocessor(lexer(), shell_name);
        auto tokens = preprocessor();

        parser.set(tokens, shell_name);
        auto tree = parser();
        auto program = binder.bind(&tree);
        auto optimized_program = linc::Optimizer::optimizeProgram(&program);

        for(const auto& declaration: optimized_program->declarations)
            interpreter.evaluateDeclaration(declaration.get());
    };

    init();

    while(true)
    {
        auto prompt = linc::Logger::format("$linc $:$$>$ ", linc::Colors::toANSI(linc::Colors::getCurrentColor()),
            linc::Colors::toANSI(success? linc::Colors::Color::Yellow: linc::Colors::Color::Red), linc::Colors::toANSI(linc::Colors::Color::Default));

        success = true;

        std::string buffer, buffer_tolower;
        buffer = linc::Logger::read(prompt);

        for(char& c: buffer)
            buffer_tolower.push_back(tolower(c));

        if(buffer_tolower.starts_with("/clear"))
        {
            system("clear");
            continue;
        }
        else if(buffer_tolower.starts_with("/tree"))
        {
            show_tree = !show_tree;
            linc::Logger::println("$ node tree display!", show_tree? "Enabled": "Disabled");
            continue;
        }
        else if(buffer_tolower.starts_with("/lexer"))
        {
            show_lexer = !show_lexer;
            linc::Logger::println("$ lexer token display!", show_lexer? "Enabled": "Disabled");
            continue;
        }
        else if(buffer_tolower.starts_with("/opt"))
        {
            optimization = !optimization;
            linc::Logger::println("$ syntax tree optimization!", optimization? "Enabled": "Disabled");
            continue;
        }
        else if(buffer_tolower.starts_with("/reset"))
        {
            init();
            system("clear");
            continue;
        }
        else if(buffer_tolower.starts_with("/symbols"))
        {
            auto& list = binder.getSymbols();
            
            for(size_t i = 0; i < list.size(); i++)
            {
                const auto& symbol = list[i];
                
                linc::Logger::print(linc::Colors::push(linc::Colors::Color::Yellow));

                const auto index = linc::PrimitiveValue(i).toString();

                if(auto variable = dynamic_cast<const linc::BoundVariableDeclaration*>(symbol.get()))
                    linc::Logger::println("[$]: variable $ of type '$'", index, linc::PrimitiveValue(variable->getName()),
                    linc::PrimitiveValue(variable->getActualType()));
                
                else if(auto function = dynamic_cast<const linc::BoundFunctionDeclaration*>(symbol.get()))
                    linc::Logger::println("[$]: function $ with return type '$' (# of args: $)", index,
                        linc::PrimitiveValue(function->getName()), linc::PrimitiveValue(function->getReturnType()),
                        linc::PrimitiveValue(function->getArguments().size()));

                else if(auto external_function = dynamic_cast<const linc::BoundExternalDeclaration*>(symbol.get()))
                    linc::Logger::println("[$]: external function $ with return type '$' (# of args: $)", index,
                        linc::PrimitiveValue(external_function->getName()), linc::PrimitiveValue(external_function->getActualType()->getActualType()),
                        linc::PrimitiveValue(external_function->getArguments().size()));

                else if(auto structure = dynamic_cast<const linc::BoundStructureDeclaration*>(symbol.get()))
                    linc::Logger::println("[$]: structure $ of type $", index, linc::PrimitiveValue(structure->getName()),
                        structure->getActualType());

                linc::Colors::pop();
            }
            
            if(list.empty())
                linc::Logger::println("No symbols have been declared.");

            continue;
        }
        else if(buffer_tolower.starts_with("/file"))
        {
            std::string filename;

            linc::Logger::print("Enter the file to evaluate: ");
            std::getline(std::cin, filename);

            const char** _arguments = new const char*[2ul];
            _arguments[0ul] = arguments[0ul];
            _arguments[1ul] = filename.c_str();

            evaluate_file(filename, 2ul, _arguments, argument_handler);
            delete[] _arguments;
            continue;
        }
        else if(buffer_tolower == "/?" || buffer_tolower == "/help" || buffer_tolower == "?")
        {
            linc::Logger::println("$:#1/clear:$:#0 runs the GNU command of the same name.", linc::Colors::pop(),
                linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/tree:$:#0 toggles the visual representation of the AST.", linc::Colors::pop(),
                linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/symbols:$:#0 displays the list of all declared symbols of the current scope (Variables/Functions).",
                linc::Colors::pop(), linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/reset:$:#0 gets rid of any declared variables.", linc::Colors::pop(),
                linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/lexer:$:#0 toggles the lexer token display.", linc::Colors::pop(),
                linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/file:$:#0 evaluate program from file.", linc::Colors::pop(),
                linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/opt:$:#0 toggles optimization.", linc::Colors::pop(),
                linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/help:$:#0 displays this menu.", linc::Colors::pop(),
                 linc::Colors::push(linc::Colors::Color::Yellow));

            linc::Logger::println("$:#1/q:$:#0 quits the application.", linc::Colors::pop(),
                linc::Colors::push(linc::Colors::Color::Yellow));

            continue;
        }
        else if(buffer == "q" || buffer == "/q")
            return LINC_EXIT_SUCCESS;

        else if(bool curly = buffer.ends_with('{'))
        {
            std::size_t depth{1ul};
            std::string sub_buffer{};
            buffer.push_back('\n');

            for(;;)
            {
                std::string depth_string{};
                
                if(depth == 0ul)
                    break;

                linc::Logger::print("----- > ");
                std::getline(std::cin, sub_buffer);

                auto has_left = sub_buffer.contains('{');
                auto has_right = sub_buffer.contains('}');

                for(std::size_t i = 0ul; i < depth - has_right? 1ul: 0ul; ++i)
                    depth_string += ' ';

                depth += has_left - has_right;

                buffer.append(depth_string + sub_buffer + '\n');
            }
        }

        const auto shell_name = "./shell-input";
        auto code = linc::Code::toSource(buffer, shell_name);
        linc::Lexer lexer(code);
        lexer.appendIncludeDirectories(argument_handler.get('i'));

        linc::Preprocessor preprocessor(lexer(), shell_name);
        auto tokens = preprocessor();

        if(show_lexer)
            for(auto& token: tokens)
                linc::Logger::println("$:#4Token$:#3 {type: $:$:$}",
                    linc::Logger::format("$:#2$:$:#1", linc::Token::typeToString(token.type), 
                        linc::Colors::pop(), linc::Colors::push(token.isValid()? linc::Colors::Color::Yellow: linc::Colors::Color::Red)),
                    token.value? linc::Logger::format(", value: $", linc::PrimitiveValue(*token.value)): std::string{},
                    token.numberBase? linc::Logger::format(", base: $", linc::PrimitiveValue(linc::Token::baseToInt(*token.numberBase))): std::string{},
                    linc::Colors::pop(), linc::Colors::push(linc::Colors::Color::Purple));
        if(linc::Reporting::hasError()){ linc::Reporting::clearReports(); success = false; continue; }
                
        parser.set(tokens, shell_name);
        auto tree = parser.parseStatement();
        parser.parseEndOfFile();

        if(linc::Reporting::hasError()){ linc::Reporting::clearReports(); success = false; continue; }

        auto program = binder.bindStatement(tree.get());

        if(optimization)
            program = linc::Optimizer::optimizeStatement(program.get());

        if(linc::Reporting::hasError()){ linc::Reporting::clearReports(); success = false; continue; }
        else if(show_tree)
            interpreter.printNodeTree(program.get(), "");

        if(!linc::Reporting::hasError())
        {
            auto result = interpreter.evaluateStatement(program.get());
            if((result.getIfPrimitive() && result.getPrimitive().getKind() != linc::PrimitiveValue::Kind::Invalid)
            || (result.getIfArray() && result.getArray().getKind() != linc::Types::Kind::invalid)
            || result.getIfStructure())
                linc::Logger::println("-> $", result);
            else success = false;
        }
        else success = false;

        linc::Reporting::clearReports();
    }

    return LINC_EXIT_SUCCESS;
}
catch(const linc::Exception& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "[LINC EXCEPTION] $ " LINC_EXCEPTION_WARNING, e.info());

    return LINC_EXIT_FAILURE_LINC_EXCEPTION;
}
catch(const std::exception& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "[STANDARD EXCEPTION] $ " LINC_EXCEPTION_WARNING, e.what());
    return LINC_EXIT_FAILURE_STANDARD_EXCEPTION;
}
catch(...)
{
    linc::Logger::log(linc::Logger::Type::Error, "[UNKNOWN EXCEPTION] Caught unexpected exception type. " LINC_EXCEPTION_WARNING);
    return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
}