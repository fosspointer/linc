#include <linc/System.hpp>
#include <linc/Lexer.hpp>
#include <linc/Preprocessor.hpp>
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
#define LINC_NOTICE_FILENAME "notice.txt"

#ifdef LINC_DEBUG
#define LINC_EXCEPTION_WARNING ""
#else
#define LINC_EXCEPTION_WARNING "This is probaby not intended, please contact the developer of this software to fix it."
#endif

static int evaluateFile(std::string filepath, int argc, const char** argv, Arguments& argument_handler, std::string& definitions_code)
{
    filepath = linc::Files::toAbsolute(filepath);

    if(!linc::Files::exists(filepath))
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = linc::Logger::format("Specified filepath `$` does not exist!", filepath)
        });
        return LINC_EXIT_COMPILATION_FAILURE;
    }
    auto raw_code = linc::Files::read(filepath);
    auto code = linc::Code::toSource(definitions_code, "./definitions");
    linc::Code::append(code, linc::Code::toSource(raw_code, filepath));

    linc::Lexer lexer(code, true);
    lexer.appendIncludeDirectories(argument_handler.get('i'));
    auto tokens = lexer();

    linc::Preprocessor preprocessor;
    preprocessor.set(tokens, filepath);
    auto processed_code = preprocessor();

    linc::Parser parser;
    parser.set(processed_code, filepath);
    linc::Binder binder;
    
    auto program = parser();
    auto bound_program = binder.bindProgram(&program);
    
    if(!linc::Reporting::hasError())
    {
        linc::Interpreter interpreter(binder);
        std::vector<linc::NodeListClause<linc::Expression>::DelimitedNode> arguments;
        for(int i{argument_handler.getFirstDefaultIndex()}; i < argc; ++i)
            arguments.push_back(linc::NodeListClause<linc::Expression>::DelimitedNode{
                .delimiter = i == argc - 1? std::nullopt: std::make_optional(linc::Token{.type = linc::Token::Type::Comma}),
                .node = std::make_unique<const linc::LiteralExpression>(linc::Token{
                    .type = linc::Token::Type::StringLiteral,
                    .value = argv[i]
                })
            });

        return interpreter.evaluateProgram(&bound_program, std::make_unique<const linc::ArrayInitializerExpression>(
            linc::Token{.type = linc::Token::Type::SquareLeft}, linc::Token{.type = linc::Token::Type::SquareRight}, 
            std::make_unique<const linc::NodeListClause<linc::Expression>>(std::move(arguments), linc::Token::Info{})
        ));
    }
    else return LINC_EXIT_COMPILATION_FAILURE;
}
void clear()
{
#ifdef LINC_WINDOWS
    system("cls");
#else
    system("clear");
#endif
}

int main(int argument_count, const char** arguments)
try
{
#ifdef LINC_WINDOWS
    linc::Windows::enableAnsi();
#endif
    const static auto option_include = 'i', option_eval = 'e', option_version = 'v', option_optimization = 'O', option_notice = 'C', option_define = 'D', option_log = 'L',
        option_disable_ansi = 'a';
    constexpr const char* notice = 
        #include "notice"
    ;

    Arguments argument_handler(argument_count, arguments, std::unordered_map<char, Arguments::Option>{
        std::pair(option_include, Arguments::Option{.description = "Specify a custom include path."}),
        std::pair(option_eval, Arguments::Option{.description = "Evaluate a given statement."}),
        std::pair(option_version, Arguments::Option{.description = "Display the current Linc version in use.", .flag = true}),
        std::pair(option_optimization, Arguments::Option{.description = "Use optimization.", .flag = true}),
        std::pair(option_notice, Arguments::Option{.description = "Display the legal notice.", .flag = true}),
        std::pair(option_define, Arguments::Option{.description = "Specify a preprocessor definition."}),
        std::pair(option_log, Arguments::Option{.description = "Write logs to a file."}),
        std::pair(option_disable_ansi, Arguments::Option{.description = "Disable ANSI output.", .flag = true}),
    }, std::vector<std::pair<std::string, char>>{
        std::pair("--include", option_include),
        std::pair("--eval", option_eval),
        std::pair("--version", option_version),
        std::pair("--optimization", option_optimization),
        std::pair("--notice", option_notice),
        std::pair("--define", option_define),
        std::pair("--log", option_log),
        std::pair("--disable-ansi", option_disable_ansi),
    });
    std::string log_file{};
    linc::Colors::toggleANSISupport(argument_handler.get(option_disable_ansi).empty());

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
    else if(auto log = argument_handler.get(option_log); !log.empty())
        log_file = log[0ul];

    auto definition_options = argument_handler.get(option_define);
    std::string definitions_code;
    for(const auto& definition: definition_options)
    {
        std::size_t find_index{};
        if((find_index = definition.find_first_of('=')) == std::string::npos || find_index == definition.size() - 1ul)
        {
            linc::Reporting::push(linc::Reporting::Report{
                .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
                .message = linc::Logger::format("Invalid preprocessor definition option `$`. Syntax: -$ <name>=<value>", definition, option_define)
            });
            return LINC_EXIT_COMPILATION_FAILURE;
        }
        auto name = definition.substr(0ul, find_index);
        auto value = definition.substr(find_index + 1ul, definition.length() - find_index);
        
        if(name.empty() || !(std::isalpha(name[0ul]) || name[0ul] == '_') || linc::Keywords::get(name) != linc::Token::Type::InvalidToken
        || !std::all_of(name.begin() + 1ul, name.end(), [](char character){
            return std::isalnum(character) || character == '_';
        }))
        {
            linc::Reporting::push(linc::Reporting::Report{
                .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
                .message = linc::Logger::format("-`$`: The name of a preprocessor definition must be a valid identifier (but given `$`).", option_define, name)
            });
            return LINC_EXIT_COMPILATION_FAILURE;
        }
        linc::Logger::append(definitions_code, "#define $ $ #\n", name, value);
    }

    auto files = argument_handler.getDefaults();
    auto evaluate_expressions = argument_handler.get(option_eval);

    if(!evaluate_expressions.empty())
    {
        for(std::size_t i{0ul}; i < evaluate_expressions.size(); ++i)
        {
            const auto path = "eval-input" + std::to_string(i);
            linc::Binder binder;
            linc::Interpreter interpreter(binder);
            linc::Parser parser;

            auto code = linc::Code::toSource(evaluate_expressions[i], path);
            linc::Lexer lexer(code, true);
            linc::Preprocessor preprocessor;
            preprocessor.set(lexer(), path);
            parser.set(preprocessor(), path);
            
            const auto node = parser.parseVariant();
            const auto bound_node = binder.bindNode(node.get());

            if(linc::Reporting::hasError()) break;
            linc::Logger::println("$::`$` >> $", linc::PrimitiveValue{i}, 
                std::string{linc::Colors::toANSI(linc::Colors::Cyan)} + evaluate_expressions[i] + std::string{linc::Colors::toANSI(linc::Colors::Reset)},
                interpreter.evaluateNode(bound_node.get()));
        }
        return linc::Reporting::getReports().size()? LINC_EXIT_COMPILATION_FAILURE: LINC_EXIT_SUCCESS;
    }

    if(!files.empty())
    {
        auto result = evaluateFile(*files.at(0ul).value, argument_count, arguments, argument_handler, definitions_code);
        if(!log_file.empty())
        {
            std::string report_list;
            for(const auto& report: linc::Reporting::getReports())
                linc::Logger::append(report_list, "$ $\n", linc::Logger::logTypeToString(report.type), report);
            linc::Files::write(log_file, report_list);
        }
        return result;
    }
    bool show_tree{false}, show_lexer{false}, optimization{!argument_handler.get(option_optimization).empty()};

    linc::Preprocessor preprocessor;
    linc::Binder binder;
    linc::Interpreter interpreter(binder);
    linc::Parser parser;
    
    bool success{true};

    auto init = [&]()
    {
        binder.reset();
        interpreter.reset();
        linc::Preprocessor::reset();
        linc::Reporting::setSpansEnabled(false);

        static constexpr auto shell_name = "./include-std";
        static constexpr auto include_code = "#include `std.linc`";
        auto code = linc::Code::toSource(include_code, shell_name);

        linc::Lexer lexer(code, true);
        lexer.appendIncludeDirectories(argument_handler.get('i'));

        preprocessor.set(lexer(), shell_name);
        if(linc::Reporting::hasError()) return;
        auto tokens = preprocessor();
        if(linc::Reporting::hasError()) return;

        parser = linc::Parser();
        parser.set(tokens, shell_name);
        auto tree = parser();
        if(linc::Reporting::hasError()) return;
        auto program = binder.bindProgram(&tree);
        if(linc::Reporting::hasError()) return;
        auto optimized_program = linc::Optimizer::optimizeProgram(program);

        for(const auto& declaration: optimized_program.declarations)
            interpreter.evaluateDeclaration(declaration.get());

        preprocessor = linc::Preprocessor();
        linc::Reporting::setSpansEnabled(true);
        linc::Reporting::clearReports();
    };
    static const auto prompt = linc::Logger::format("$linc $:$$>$ ", linc::Colors::toANSI(linc::Colors::getCurrentColor()),
        linc::Colors::toANSI(success? linc::Colors::Yellow: linc::Colors::Red), linc::Colors::toANSI(linc::Colors::Reset));

    init();
    while(true)
    {
        success = true;

        std::string buffer, buffer_tolower;
        buffer = linc::Logger::read(prompt, true);

        for(char& c: buffer)
            buffer_tolower.push_back(tolower(c));

        if(buffer_tolower.starts_with("/clear"))
        {
            clear();
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
            clear();
            continue;
        }
        else if(buffer_tolower.starts_with("/notice"))
        {
            linc::Logger::log(linc::Logger::Type::Info, "$", notice);
            continue;
        }
        else if(buffer_tolower.starts_with("/symbols"))
        {
            auto list = binder.getSymbols();
            
            for(size_t i{0ul}; i < list.size(); i++)
            {
                const auto& symbol = list[i];
                
                linc::Logger::print(linc::Colors::push(linc::Colors::Yellow));

                const auto index = linc::PrimitiveValue(i).toString();

                if(auto variable = dynamic_cast<const linc::BoundVariableDeclaration*>(symbol->get()))
                    linc::Logger::println("[$]: variable $ of type `$`", index, linc::PrimitiveValue(variable->getName()),
                    linc::PrimitiveValue(variable->getActualType()));
                
                else if(auto function = dynamic_cast<const linc::BoundFunctionDeclaration*>(symbol->get()))
                    linc::Logger::println("[$]: function $ of type `$`", index,
                        linc::PrimitiveValue(function->getName()), linc::PrimitiveValue(function->getFunctionType()));

                else if(auto external_function = dynamic_cast<const linc::BoundExternalDeclaration*>(symbol->get()))
                    linc::Logger::println("[$]: external function $ with return type '$' (# of args: $)", index,
                        linc::PrimitiveValue(external_function->getName()), linc::PrimitiveValue(external_function->getActualType()->getActualType()),
                        linc::PrimitiveValue(external_function->getArguments().size()));

                else if(auto structure = dynamic_cast<const linc::BoundStructureDeclaration*>(symbol->get()))
                    linc::Logger::println("[$]: structure $ of type $", index, linc::PrimitiveValue(structure->getName()),
                        structure->getActualType());

                else if(auto enumeration = dynamic_cast<const linc::BoundEnumerationDeclaration*>(symbol->get()))
                    linc::Logger::println("[$]: enumeration: $ of type `$`", index, linc::PrimitiveValue(enumeration->getName()),
                        linc::PrimitiveValue(enumeration->getActualType()));

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

            evaluateFile(filename, 2ul, _arguments, argument_handler, definitions_code);
            delete[] _arguments;
            continue;
        }
        else if(buffer_tolower == "/?" || buffer_tolower == "/help" || buffer_tolower == "?")
        {
            const static std::initializer_list<std::pair<std::string, std::string>> command_descriptions{
                std::pair{"clear", "runs the shell clear command"},
                std::pair{"tree", "toggles the visual representation of the AST"},
                std::pair{"symbols", "displays the list of all declared symbols of the current scope"},
                std::pair{"reset", "gets rid of all declared variables and runs /clear"},
                std::pair{"lexer", "toggles the lexer token display"},
                std::pair{"file", "evaluate program from file"},
                std::pair{"opt", "toggles the lexer token display"},
                std::pair{"notice", "displays the software notice for lincenv"},
                std::pair{"help", "displays this menu"},
                std::pair{"q", "quits the application"},
            };

            for(const auto& description: command_descriptions)
                linc::Logger::println("$:#3/$:: $:#2$.", description.first, description.second, linc::Colors::pop(), linc::Colors::push(linc::Colors::Yellow));

            continue;
        }
        else if(buffer == "q" || buffer == "/q")
            return LINC_EXIT_SUCCESS;

        constexpr auto shell_name = "./shell-input";
        constexpr auto definitions_name = "./definitions";
        auto code = linc::Code::toSource(definitions_code, definitions_name);
        linc::Code::append(code, linc::Code::toSource(buffer, shell_name));
        linc::Lexer lexer(code, true);
        lexer.appendIncludeDirectories(argument_handler.get('i'));

        preprocessor.set(lexer(), shell_name);
        auto tokens = preprocessor();

        if(show_lexer)
            for(auto& token: tokens)
                linc::Logger::println("$:#4Token$:#3 {type: $:$:$}",
                    linc::Logger::format("$:#2$:$:#1", linc::Token::typeToString(token.type), 
                        linc::Colors::pop(), linc::Colors::push(token.isValid()? linc::Colors::Yellow: linc::Colors::Red)),
                    token.value? linc::Logger::format(", value: $", linc::PrimitiveValue(*token.value)): std::string{},
                    token.numberBase? linc::Logger::format(", base: $", linc::PrimitiveValue(linc::Token::baseToInt(*token.numberBase))): std::string{},
                    linc::Colors::pop(), linc::Colors::push(linc::Colors::Purple));
        if(linc::Reporting::hasError()){ linc::Reporting::clearReports(); success = false; continue; }
                
        parser.set(tokens, shell_name);
        auto tree = parser.parseVariant();
        parser.parseEndOfFile();

        if(linc::Reporting::hasError()){ linc::Reporting::clearReports(); success = false; continue; }

        auto node = binder.bindNode(tree.get());

        if(optimization)
            node = linc::Optimizer::optimizeNode(node.get());

        if(linc::Reporting::hasError()){ linc::Reporting::clearReports(); success = false; continue; }
        else if(show_tree)
            interpreter.printNodeTree(node.get(), std::string{});

        if(!linc::Reporting::hasError())
        {
            auto result = interpreter.evaluateNode(node.get());
            if((result.getIfPrimitive() && result.getPrimitive().getKind() != linc::PrimitiveValue::Kind::Invalid)
            || (result.getIfArray() && !result.getArray().getType().isCompatible(linc::Types::fromKind(linc::Types::Kind::invalid)))
            || result.getIfStructure() || result.getIfEnumerator() || result.getIfFunction())
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
    linc::Logger::log(linc::Logger::Type::Error, "$:#2LINC EXCEPTION$:#1 $ " LINC_EXCEPTION_WARNING, e.info(), 
        linc::Colors::pop(), linc::Colors::push(linc::Colors::Red));
    return LINC_EXIT_FAILURE_LINC_EXCEPTION;
}
catch(const std::exception& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "$:#2STANDARD EXCEPTION$:#1 $ " LINC_EXCEPTION_WARNING, e.what(),
        linc::Colors::pop(), linc::Colors::push(linc::Colors::Red));
    return LINC_EXIT_FAILURE_STANDARD_EXCEPTION;
}
catch(const linc::BreakException& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "$:#1UNEXPECTED EXCEPTION$:#0 Encountered unmatched control-flow exception (break) " LINC_EXCEPTION_WARNING,
        linc::Colors::pop(), linc::Colors::push(linc::Colors::Red));
    return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
}
catch(const linc::ContinueException& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "$:#1UNEXPECTED EXCEPTION$:#0 Encountered unmatched control-flow exception (continue) " LINC_EXCEPTION_WARNING,
        linc::Colors::pop(), linc::Colors::push(linc::Colors::Red));
    return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
}
catch(const linc::ReturnException& e)
{
    linc::Logger::log(linc::Logger::Type::Error, "$:#1UNEXPECTED EXCEPTION$:#0 Encountered unmatched control-flow exception (return) " LINC_EXCEPTION_WARNING,
        linc::Colors::pop(), linc::Colors::push(linc::Colors::Red));
    return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
}
catch(...)
{
    linc::Logger::log(linc::Logger::Type::Error, "$:#1UNEXPECTED EXCEPTION$:#0 Caught unexpected exception type " LINC_EXCEPTION_WARNING,
        linc::Colors::pop(), linc::Colors::push(linc::Colors::Red));
    return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
}