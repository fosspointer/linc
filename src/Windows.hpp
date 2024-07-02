#pragma once
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