#include <iostream>
#include "hyprpicker.hpp"


int main(int argc, char** argv, char** envp) {
    g_pHyprpicker = std::make_unique<CHyprpicker>();

    // parse args
    // 1 - format
    int currentlyParsing = 0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (currentlyParsing == 0) {
            if (arg == "--format") {
                currentlyParsing = 1;
                continue;
            } else {
                Debug::log(NONE, "Unrecognized option %s", arg.c_str());
                exit(1);
            }
        } else if (currentlyParsing == 1) {
            if (arg == "hex") g_pHyprpicker->m_bSelectedOutputMode = OUTPUT_HEX;
            else if (arg == "rgb") g_pHyprpicker->m_bSelectedOutputMode = OUTPUT_RGB;
            else {
                Debug::log(NONE, "Unrecognized format %s", arg.c_str());
                exit(1);
            }

            currentlyParsing = 0;
            continue;
        }
    }

    g_pHyprpicker->init();

    return 0;
}