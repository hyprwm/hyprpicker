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
            if (arg == "--autocopy") {
                g_pHyprpicker->m_bAutoCopy = true;

                std::filesystem::path wlcopy_bin_path{ "/usr/bin/wl-copy" };
                if (std::filesystem::exists(wlcopy_bin_path)) {
                    continue;
                } else {
                    Debug::log(INFO, "If you want to use the --autocopy argument you need to install wl-copy.");
                    exit(1);
                }

            } else if (arg == "--format") {
                currentlyParsing = 1;
                continue;
            } else if (arg == "--no-fancy") {
                g_pHyprpicker->m_bFancyOutput = false;
            } else {
                std::cout << "Hyprpicker usage: hyprpicker [arg [...]].\n\nArguments:\n" <<
                    " --autocopy      | Automatically copies the output to the clipboard\n" <<
                    " --format [fmt]  | Specifies the output format (hex, rgb)\n" <<
                    " --no-fancy      | Disables the \"fancy\" (aka. colored) outputting\n" <<
                    " --help          | Show this help message\n";
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

    if (!isatty(fileno(stdout)) || getenv("NO_COLOR"))
        g_pHyprpicker->m_bFancyOutput = false;

    g_pHyprpicker->init();

    return 0;
}
