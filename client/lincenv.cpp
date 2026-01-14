#include <chrono>
#include <linc/system/Logger.hpp>
#include <linc/system/Exception.hpp>
#include <linc/system/Arena.hpp>
#include <linc/lexer/Lexer.hpp>

int main()
try
{
    using namespace linc;
    Logger::init();

    auto code = Code::toSource("test.linc");
    Lexer lexer(code, 0ul);
    auto tokens = lexer();

    Logger::println("tokens:");
    for(const auto& token: tokens)
    {
        Logger::println("[token] type = $, file: $::$", (int)token.kind, token.info.file, token.info.line);
        Logger::println(" - value = $", token.value.value_or("none"));
    }

    std::fflush(stdout);
}
catch(linc::Exception& e)
{
    linc::Logger::log(linc::Logger::Level::Critical, "LINC EXCEPTION: $", e.info());
}
