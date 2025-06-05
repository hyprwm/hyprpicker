#include "Clipboard.hpp"

#include "../includes.hpp"
#include <hyprutils/os/Process.hpp>
#include <string>
#include <vector>

void NClipboard::copy(std::string data) {
    std::string              clipboardBinary = "wl-copy";

    std::vector<std::string> clipboardArgs = {data};

    Hyprutils::OS::CProcess  copy(clipboardBinary, clipboardArgs);

    copy.runAsync();
}
