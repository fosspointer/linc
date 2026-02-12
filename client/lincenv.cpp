#include <linc/system/Logger.hpp>
#include <linc/system/Exception.hpp>
#include <linc/system/Arena.hpp>
#include <linc/lexer/Lexer.hpp>

int main()
try
{
    using namespace linc;
    Logger::init();

    auto [file_index, code] = Code::toSource("test.linc");
    Lexer lexer(*code, file_index);
    auto tokens = lexer();

    Logger::println("test: $", code.value());
    for(const auto& token: tokens)
    {
        Logger::println("[token] type = $, file: $::$", Token::kindToString(token.kind),
            Files::filepathAtIndex(token.file), token.line);
        Logger::println(" - lexeme = $", token.lexeme);
    }

    std::fflush(stdout);
}
catch(linc::Exception& e)
{
    linc::Logger::log(linc::Logger::Level::Critical, "LINC EXCEPTION: $", e.info());
}
