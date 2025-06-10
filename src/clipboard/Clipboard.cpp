#include "Clipboard.hpp"

#include "../includes.hpp"
#include <hyprutils/os/Process.hpp>
#include <string>
#include <vector>

void NClipboard::copy(std::string data) {
    Hyprutils::OS::CProcess copy("wl-copy", {data});

    copy.runAsync();
}
