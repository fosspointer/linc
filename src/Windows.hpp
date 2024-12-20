#pragma once
#define LINC_ENVIRONMENT_FAILURE 6
typedef void* HANDLE;
typedef unsigned long DWORD;
typedef int BOOL;
typedef DWORD* LPDWORD;

#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x4
#define WINAPI __stdcall

extern "C"
{
    HANDLE WINAPI GetStdHandle(DWORD nStdHandle);
    DWORD WINAPI GetLastError(void);
    DWORD WINAPI GetConsoleMode(HANDLE hConsoleHandle, LPDWORD lpMode);
    BOOL WINAPI SetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode);
}

namespace linc
{
    class Windows
    {
    public:
        Windows() = delete;
        
        static int enableAnsi()
        {
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            if(console == INVALID_HANDLE_VALUE)
            {
                linc::Logger::log(linc::Logger::Type::Warning, "Failed to get console handle.");
                return LINC_ENVIRONMENT_FAILURE;
            }

            DWORD mode;
            if(!GetConsoleMode(console, &mode))
            {
                linc::Logger::log(linc::Logger::Type::Warning, "Failed to get console mode.");
                return LINC_ENVIRONMENT_FAILURE;
            }
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

            if(!SetConsoleMode(console, mode))
            {
                linc::Logger::log(linc::Logger::Type::Warning, "Failed to set console handle.");
                return LINC_ENVIRONMENT_FAILURE;
            }

            return 0;
        }
    };
}