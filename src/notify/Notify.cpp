#include "Notify.hpp"

#include "../includes.hpp"
#include <cstdint>
#include <cstdio>
#include <format>
#include <hyprutils/os/Process.hpp>
#include <iostream>
#include <string>

#include <hyprutils/os/Process.hpp>

void NNotify::send(std::string hexColor, std::string formattedColor) {
    std::string              notifyBody = std::format("<span>You selected the color: <span color='{}'><b>{}</b></span></span>", hexColor, formattedColor);

    std::string              notifyBinary = "notify-send";
    std::vector<std::string> notifyArgs   = {"-t", "5000", "-i", "color-select-symbolic", "Color Picker", notifyBody};

    Hyprutils::OS::CProcess  notify(notifyBinary, notifyArgs);

    notify.runAsync();
}
