#include <linc/Preprocessor.hpp>
#include <linc/System.hpp>
#include <linc/Lexer.hpp>
#include <linc/Tree.hpp>
#include <linc/Parser.hpp>
#include <linc/BoundTree.hpp> 
#include <linc/Binder.hpp>
#include <linc/Generator.hpp>

[[nodiscard]] static std::unique_ptr<const linc::BoundExpression> const evaluateExpression(const std::string& expression_raw, linc::Binder& binder)
{
    const auto test_path = "testing";
    auto code = linc::Code::toSource(expression_raw);
    linc::Lexer lexer(code, true);
    linc::Preprocessor preprocessor;
    preprocessor.set(lexer(), test_path);
    linc::Parser parser;
    parser.set(preprocessor(), test_path);
    
    auto expression = parser.parseExpression();

    if(!expression)
        return nullptr;

    auto bound_expression = binder.bindExpression(expression.get());
    return bound_expression;
}


[[nodiscard]] static linc::Types::type evaluateAndCompare(const std::string& first_expression_raw, const std::string& second_expression_raw)
{
    linc::Binder binder;
    auto first_expression = evaluateExpression(first_expression_raw, binder);
    auto second_expression = evaluateExpression(second_expression_raw, binder);

    if(!first_expression)
    {
        linc::Logger::println("[TEST] Evaluation failed! The first expression evaluated to null.");
        return linc::Types::invalidType;
    }

    if(!second_expression)
    {
        linc::Logger::println("[TEST] Evaluation failed! The second expression evaluated to null.");
        return linc::Types::invalidType;
    }

    if(first_expression->getType() != second_expression->getType())
    {
        linc::Logger::println("[TEST] Type comparison failed! The two statements are not type-equivalent.");
        return linc::Types::invalidType;
    }


    linc::Interpreter interpreter(binder);
    auto first_value = interpreter.evaluateExpression(first_expression.get());
    auto second_value = interpreter.evaluateExpression(second_expression.get());

    if(first_value != second_value)
    {
        linc::Logger::println("[TEST] Value comparison failed! The two statements do not evaluate to the same value ('$' vs '$').",
            first_value, second_value);
        return linc::Types::invalidType;
    }

    return first_expression->getType();

}

int main(int argument_count, char** arguments)
try {
    if(argument_count != 4ul)
    {
        linc::Logger::println("[TEST] Incorrect number of arguments given to test.");
        return EXIT_FAILURE;
    }

    auto raw_statement_initial = arguments[1ul];
    auto raw_statement_comparison = arguments[2ul];
    auto raw_type = arguments[3ul];

    auto result = evaluateAndCompare(raw_statement_initial, raw_statement_comparison);
    auto type = linc::Types::kindFromUserString(raw_type);

    if(result.primitive == linc::Types::Kind::invalid)
        return EXIT_FAILURE;

    else if(result.primitive != type)
    {
        linc::Logger::println("[TEST] Type check failed! The expression does not type match the given type (expression evaluated to type '$', but '$' was given).",
            linc::Types::kindToString(result.primitive), linc::Types::kindToString(type));
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