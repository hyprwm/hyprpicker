#include "Clipboard.hpp"

#include "../includes.hpp"

void Clipboard::copy(const char* fmt, ...) {
    char    buf[CLIPBOARDMESSAGESIZE] = "";
    char*   outputStr;

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);

    outputStr = strdup(buf);

    if (fork() == 0)
        // clang-tidy didn't warn about that NULL... what?
        execlp("wl-copy", "wl-copy", outputStr, nullptr);

    free(outputStr);
}
