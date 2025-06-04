#pragma once

#include <cstdint>
#include <string>
#define NOTIFYBODYSIZE 128

namespace Notify {
    void send(std::string hexColor, std::string formattedColor);
}
