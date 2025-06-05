#include "Clipboard.hpp"

#include "../includes.hpp"
#include <string>

void NClipboard::copy(std::string color) {
    if (fork() == 0)
        execlp("wl-copy", "wl-copy", color.c_str(), NULL);
}
