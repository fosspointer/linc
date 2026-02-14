#include <iostream>
#include <linc/system/Logger.hpp>
#include <linc/system/Exception.hpp>
#include <linc/system/Arena.hpp>
#include <linc/lexer/Lexer.hpp>

int main()
try
{
    using namespace linc;
    Logger::init();

    for(;;)
    {
        std::cout << " > ";
        std::string input;
        std::getline(std::cin, input);
        Lexer lexer(input, 0ul);
        auto tokens = lexer();

        for(const auto& token: tokens)
        {
            Logger::println("[token] type = $, file: $::$", Token::kindToString(token.kind),
                token.file, token.line);
            Logger::println(" - lexeme = $", token.lexeme);
            std::fflush(stdout);
        }
    }
}
catch(linc::Exception& e)
{
    linc::Logger::log(linc::Logger::Level::Critical, "LINC EXCEPTION: $", e.info());
}
