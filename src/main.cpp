#include <iostream>
#include "hyprpicker.hpp"


int main(int argc, char** argv, char** envp) {
    g_pHyprpicker = std::make_unique<CHyprpicker>();
    g_pHyprpicker->init();

    return 0;
}