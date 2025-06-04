#include "Notify.hpp"

#include "../includes.hpp"
#include <cstdint>
#include <cstdio>
#include <format>
#include <iostream>
#include <string>

void Notify::send(std::string hexColor, std::string formattedColor) {
    std::string bodyString = std::format("<span>You selected the color: <span color='{}'><b>{}</b></span></span>", hexColor, formattedColor);

    if (fork() == 0)
        execlp("notify-send", "notify-send", "-t", "5000", "-i", "color-select-symbolic", "Color Picker", bodyString.c_str(), NULL);
}
