#pragma once


namespace pkn
{
namespace console
{
void pause(const char *prompt = "press enter to continue\n");
bool clear_screen();
bool reset_cursor();
bool reset_cursor_for_line();
bool set_console_size(short width, short height);
}
}