#include <linc/Preprocessor.hpp>
#include <linc/System.hpp>
#include <linc/Lexer.hpp>
#include <linc/Tree.hpp>
#include <linc/Parser.hpp>
#include <linc/BoundTree.hpp> 
#include <linc/Binder.hpp>
#include <linc/Generator.hpp>

[[nodiscard]] static std::unique_ptr<const linc::BoundStatement> const evaluate_statement(const std::string& statement_raw)
{
    linc::Preprocessor preprocessor(statement_raw, "testing");
        
    auto code = preprocessor();
    
    linc::Lexer lexer(code);
    linc::Parser parser(lexer());
    linc::Binder binder;
    
    auto statement = parser.parseStatement();

    if(!statement)
        return nullptr;

    auto bound_statement = binder.bindStatement(statement.get());
    
    bool errors = {false};
    
    for(const auto& report: linc::Reporting::getReports())
        if(report.type == linc::Reporting::Type::Error)
            errors = true;

    return std::move(bound_statement);
}


[[nodiscard]] static linc::Types::type evaluate_and_compare(const std::string& first_statement_raw, const std::string& second_statment_raw)
{
    auto first_statement = evaluate_statement(first_statement_raw);
    auto second_statement = evaluate_statement(second_statment_raw);

    if(!first_statement)
    {
        linc::Logger::println("[TEST] Evaluation failed! The first statement evaluated to null.");
        return linc::Types::invalidType;
    }

    if(!second_statement)
    {
        linc::Logger::println("[TEST] Evaluation failed! The second statement evaluated to null.");
        return linc::Types::invalidType;
    }

    if(first_statement->getType() != second_statement->getType())
    {
        linc::Logger::println("[TEST] Type comparison failed! The two statements are not type-equivalent.");
        return linc::Types::invalidType;
    }

    linc::Interpreter interpreter;
    auto first_value = interpreter.evaluateStatement(first_statement.get());
    auto second_value = interpreter.evaluateStatement(second_statement.get());

    if(first_value != second_value)
    {
        linc::Logger::println("[TEST] Value comparison failed! The two statements do not evaluate to the same value ('$' vs '$').",
            first_value, second_value);
        return linc::Types::invalidType;
    }

    return first_statement->getType();

}

int main(int argument_count, char** arguments)
try {
    if(argument_count != 4ull)
    {
        linc::Logger::println("[TEST] Incorrect number of arguments given to test.");
        return EXIT_FAILURE;
    }

    auto raw_statement_initial = arguments[1ull];
    auto raw_statement_comparison = arguments[2ull];
    auto raw_type = arguments[3ull];

    auto result = evaluate_and_compare(raw_statement_initial, raw_statement_comparison);
    auto type = linc::Types::kindFromUserString(raw_type);

    if(result.kind == linc::Types::Kind::invalid)
        return EXIT_FAILURE;

    else if(result.kind != type)
    {
        linc::Logger::println("[TEST] Type check failed! The statement does not type match the given type (expression evaluated to type '$', but '$' was given).",
            linc::Types::kindToString(result.kind), linc::Types::kindToString(type));
        return EXIT_FAILURE;
    }

    linc::Logger::println("[TEST] Test succeeded.");
    return EXIT_SUCCESS;
}
catch(const linc::Exception& e)
{
    linc::Logger::println("[LINC EXCEPTION] $", e.info());
    return EXIT_FAILURE;
}
catch(const std::exception& e)
{
    linc::Logger::println("[STANDARD EXCEPTION] $", e.what());
    return EXIT_FAILURE;
}
catch(...)
{
    linc::Logger::println("[UNKNOWN EXCPETION]");
    return EXIT_FAILURE;
}