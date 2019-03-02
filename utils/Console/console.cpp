#include <Windows.h>

#include <conio.h>

#include <iostream>

#include "console.h"

using namespace std;

namespace pkn
{

namespace console
{
void pause(const char *prompt)
{
    std::string s(prompt);
    cout << prompt;
    reset_cursor_for_line();
    while (_getch() != '\r')
    {}
}

bool clear_screen()
{
    return 0 == system("cls");
}

bool reset_cursor()
{
    HANDLE output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (output_handle != INVALID_HANDLE_VALUE)
    {
        return SetConsoleCursorPosition(output_handle, COORD{ 0, 0 });
    }
    return false;
}

bool reset_cursor_for_line()
{
    HANDLE output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (output_handle != INVALID_HANDLE_VALUE)
    {
        CONSOLE_SCREEN_BUFFER_INFOEX csbi;
        csbi.cbSize = sizeof(csbi);

        if (::GetConsoleScreenBufferInfoEx(output_handle, &csbi))
        {
            return SetConsoleCursorPosition(output_handle, COORD{ 0, csbi.dwCursorPosition.Y });
        }
    }
    return false;

}

bool set_console_size(short width, short height)
{
    HANDLE output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (output_handle != INVALID_HANDLE_VALUE)
    {
        CONSOLE_SCREEN_BUFFER_INFOEX csbi;
        csbi.cbSize = sizeof(csbi);

        if (::GetConsoleScreenBufferInfoEx(output_handle, &csbi))
        {
            csbi.dwSize.X = width;
            csbi.dwSize.Y = 30000;
            csbi.dwCursorPosition = { 0 };
            csbi.dwMaximumWindowSize = { width + 1000, height + 1000 };
            csbi.bFullscreenSupported = true;

            // the first time, set its buffer size larger
            if (::SetConsoleScreenBufferInfoEx(output_handle, &csbi))
            {
                // the 2nd time, set its window size larger
                csbi.srWindow = { 0, 0, width, height };
                if (::SetConsoleScreenBufferInfoEx(output_handle, &csbi))
                    return true;
            }
        }
    }
    return false;
}

}
}
