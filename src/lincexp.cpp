#include <linc/ControlFlow.hpp>
#include <linc/Generator.hpp>
#include <linc/Binder.hpp>
#include <linc/BoundTree.hpp>
#include <linc/Tree.hpp>
#include <linc/Parser.hpp>
#include <linc/Preprocessor.hpp>
#include <linc/Lexer.hpp>
#include <linc/System.hpp>

#define LINC_EXIT_SUCCESS 0
#define LINC_EXIT_FAILURE_LINC_EXCEPTION 1
#define LINC_EXIT_FAILURE_STANDARD_EXCEPTION 2
#define LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION 3
#define LINC_EXIT_COMPILATION_FAILURE 4

static void evaluateFile(std::string filepath)
{
    filepath = linc::Files::toAbsolute(filepath);

    if(!linc::Files::exists(filepath))
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = linc::Logger::format("Specified filepath `$` does not exist!", filepath)
        });
        std::exit(LINC_EXIT_COMPILATION_FAILURE);
    }
    auto raw_code = linc::Files::read(filepath);
    auto code = linc::Code::toSource(raw_code, filepath);

    linc::Lexer lexer(code, true);
    auto tokens = lexer();

    linc::Preprocessor preprocessor;
    preprocessor.set(tokens, filepath);
    auto processed_code = preprocessor();

    linc::Parser parser;
    parser.set(processed_code, filepath);
    linc::Binder binder;
    
    auto program = parser();
    auto bound_program = binder.bindProgram(&program);
    
    if(linc::Reporting::hasError())
        std::exit(LINC_EXIT_COMPILATION_FAILURE);

    linc::Lowerer lowerer;
    auto control_flow_program = lowerer.lowerProgram(bound_program, binder);
    std::size_t instruction_pointer{0ul};
    linc::Interpreter interpreter(binder);

    while(instruction_pointer != -1ul)
    {
        auto& block = control_flow_program.back().blocks.at(instruction_pointer);
        struct Visitor
        {
            void operator()(linc::ControlBlock& block) { instruction_pointer = block.edge; }
            void operator()(linc::BasicBlock& block) {
                for(const auto& statement: block.statements)
                    interpreter.evaluateStatement(statement.get());
                instruction_pointer = block.edge;
            }
            void operator()(linc::UnreachableBlock& block) { instruction_pointer = -1ul; } // todo: add warning
            void operator()(linc::ConditionalBlock& block) {
                auto condition = interpreter.evaluateExpression(block.condition.get());
                if(condition.getPrimitive().getBool())
                    instruction_pointer = block.edgeTrue;
                else instruction_pointer = block.edgeFalse;
            }
            void operator()(linc::MapBlock&) { instruction_pointer = -1ul; }
            std::size_t& instruction_pointer;
            linc::Interpreter& interpreter;
        } visitor{instruction_pointer, interpreter};

        std::visit(visitor, block);
    }
}

int main(int argc, char** argv)
try
{
    if(argc != 2)
    {
        std::fputs("No file specified\n", stderr);
        std::exit(LINC_EXIT_COMPILATION_FAILURE);
    }
    evaluateFile(argv[1]);
}
catch(linc::Exception& exception)
{
    linc::Logger::log(linc::Logger::Type::Error, "LINC EXCEPTION $", exception.info());
}
catch(std::exception& exception)
{
    linc::Logger::log(linc::Logger::Type::Error, "STANDARD EXCEPTION", exception.what());
}
catch(...)
{
    linc::Logger::log(linc::Logger::Type::Error, "UNKNOWN EXCEPTION");
}