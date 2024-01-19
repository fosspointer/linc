#include <linc/Preprocessor.hpp>
#include <linc/System.hpp>
#include <linc/Lexer.hpp>
#include <linc/Tree.hpp>
#include <linc/Parser.hpp>
#include <linc/BoundTree.hpp> 
#include <linc/Binder.hpp>
#include <linc/Generator.hpp>

#define LINC_EXIT_SUCCESS 0
#define LINC_EXIT_FAILURE_LINC_EXCEPTION 1
#define LINC_EXIT_FAILURE_STANDARD_EXCEPTION 2
#define LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION 3
#define LINC_EXIT_COMPILATION_FAILURE 4

static std::string filename_to_directory(const std::string& str)
{
    return str.substr(0ul, str.find_last_of('/'));
}

static int evaluate_file(const std::string& filename)
{
    auto raw_code = linc::Files::read(filename);

    linc::Preprocessor preprocessor(raw_code, filename_to_directory(filename));
        
    auto code = preprocessor();
    
    linc::Lexer lexer(code);
    linc::Parser parser(lexer());
    linc::Binder binder;
    
    auto program = parser();
    auto bound_program = binder.bind(&program);
    
    bool errors = {false};
    
    for(const auto& report: linc::Reporting::getReports())
        if(report.type == linc::Reporting::Type::Error)
            errors = true;

    if(!errors)
    {
        linc::Interpreter interpreter;
        return interpreter.evaluateProgram(&bound_program, binder);
    }
    else return LINC_EXIT_COMPILATION_FAILURE;
}

int main(int argument_count, char** arguments)
try
{
    if(argument_count >= 2)
        return evaluate_file(arguments[1]);

    bool show_tree{false}, show_lexer{false};

    linc::Binder binder;
    linc::Interpreter interpreter;

    while(true)
    {
        linc::Logger::print("linc > ");

        std::string buffer, buffer_tolower, working_directory;
        std::getline(std::cin, buffer);

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
        else if(buffer_tolower.starts_with("/reset"))
        {
            binder.reset();
            interpreter.reset();
            system("clear");
            continue;
        }
        else if(buffer_tolower.starts_with("/symbols"))
        {
            auto& list = binder.getSymbols();
            
            for(size_t i = 0; i < list.size(); i++)
            {
                const auto& symbol = list[i];
                
                if(auto variable = dynamic_cast<const linc::BoundVariableDeclaration*>(symbol.get()))
                    linc::Logger::println("#$:: var '$' of type '$' ($)", i, variable->getName(), linc::Types::toString(variable->getType()),
                        variable->getMutable()? "mutable": "immutable");
                
                else if(auto argument = dynamic_cast<const linc::BoundArgumentDeclaration*>(symbol.get()))
                    linc::Logger::println("#$:: var '$' of type '$' ($)", i, argument->getName(), linc::Types::toString(argument->getType()),
                        argument->getMutable()? "mutable": "immutable");
                
                else if(auto function = dynamic_cast<const linc::BoundFunctionDeclaration*>(symbol.get()))
                    linc::Logger::println("#$:: function '$' with return type '$' (# of args: $)", i,
                        function->getName(), linc::Types::toString(function->getType()), function->getArguments().size());
            }
            
            if(list.empty())
                linc::Logger::println("Variable index is empty");

            continue;
        }
        else if(buffer_tolower.starts_with("/file"))
        {
            std::string filename;

            linc::Logger::print("Enter the file to evaluate: ");
            std::getline(std::cin, filename);

            evaluate_file(filename);
            continue;
        }
        else if(buffer_tolower == "/?" || buffer_tolower == "/help" || buffer_tolower == "?")
        {
            linc::Logger::println("/clear: runs the GNU command of the same name.");
            linc::Logger::println("/tree: toggles the visual representation of the AST.");
            linc::Logger::println("/symbols: displays the list of all declared symbols of the current scope (Variables/Functions).");
            linc::Logger::println("/reset: gets rid of any declared variables.");
            linc::Logger::println("/lexer: toggles the lexer token display.");
            linc::Logger::println("/file: evaluate program from file.");
            linc::Logger::println("/help: displays this menu.");
            linc::Logger::println("/q: quits the application.");
            continue;
        }
        else if(buffer == "q" || buffer == "/q")
            return LINC_EXIT_SUCCESS;
        
        linc::Preprocessor preprocessor(buffer, working_directory);
        auto code = preprocessor();

        linc::Lexer lexer(code);
        auto tokens = lexer();

        if(show_lexer)
            for(auto& token: tokens)
                linc::Logger::println("Token {type: '$', value: '$'}", linc::Token::typeToString(token.type), token.value.value_or(""));

        linc::Parser parser(tokens);
        auto tree = parser.parseStatement();

        if(show_tree)
            interpreter.printNodeTree(binder, tree.get(), "");

        auto program = binder.bindStatement(tree.get());

        if(linc::Reporting::getReports().size() == 0)
        {
            auto res = interpreter.evaluateStatement(program.get());
            if(res.getType() != linc::Types::Type::invalid)
                linc::Logger::println("-> $", res);
        }

        linc::Reporting::clearReports();
    }

    return LINC_EXIT_SUCCESS;
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