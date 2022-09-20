#include "Clipboard.hpp"
#include "../includes.hpp"

void Clipboard::copy(std::string str) {
    execlp("wl-copy", "wl-copy", str.c_str(), NULL);
}