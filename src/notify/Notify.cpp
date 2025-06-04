#include "Notify.hpp"

#include "../includes.hpp"
#include <cstdint>
#include <cstdio>
#include <iostream>

void Notify::send(const char* color) {
    char bodyBuf[NOTIFYBODYSIZE];
    char colorBuf[64];

    snprintf(bodyBuf, sizeof(bodyBuf), "<span>You selected the color: <span color='%s'><b>%s</b></span></span>", color, color);

    if (fork() == 0)
        execlp("notify-send", "notify-send", "-t", "5000", "-i", "color-select-symbolic", "Color Picker", bodyBuf, NULL);

    std::cout << colorBuf;
}
