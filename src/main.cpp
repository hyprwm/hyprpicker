#include <strings.h>

#include <iostream>

#include "hyprpicker.hpp"

static void help(void) {
    std::cout << "Hyprpicker usage: hyprpicker [arg [...]].\n\nArguments:\n"
              << " -a | --autocopy          | Automatically copies the output to the clipboard (requires wl-clipboard)\n"
              << " -f | --format=fmt        | Specifies the output format (cmyk, hex, rgb, hsl, hsv)\n"
              << " -n | --no-fancy          | Disables the \"fancy\" (aka. colored) outputting\n"
              << " -h | --help              | Show this help message\n"
              << " -r | --render-inactive   | Render (freeze) inactive displays\n"
              << " -z | --no-zoom           | Disable the zoom lens\n";
}

int main(int argc, char** argv, char** envp) {
    g_pHyprpicker = std::make_unique<CHyprpicker>();

    while (true) {
        int                  option_index   = 0;
        static struct option long_options[] = {{"autocopy", no_argument, NULL, 'a'},
                                               {"format", required_argument, NULL, 'f'},
                                               {"help", no_argument, NULL, 'h'},
                                               {"no-fancy", no_argument, NULL, 'n'},
                                               {"render-inactive", no_argument, NULL, 'r'},
                                               {"no-zoom", no_argument, NULL, 'z'},
                                               {NULL, 0, NULL, 0}};

        int                  c = getopt_long(argc, argv, ":f:hnarz", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'f':
                if (strcasecmp(optarg, "cmyk") == 0)
                    g_pHyprpicker->m_bSelectedOutputMode = OUTPUT_CMYK;
                else if (strcasecmp(optarg, "hex") == 0)
                    g_pHyprpicker->m_bSelectedOutputMode = OUTPUT_HEX;
                else if (strcasecmp(optarg, "rgb") == 0)
                    g_pHyprpicker->m_bSelectedOutputMode = OUTPUT_RGB;
                else if (strcasecmp(optarg, "hsl") == 0)
                    g_pHyprpicker->m_bSelectedOutputMode = OUTPUT_HSL;
                else if (strcasecmp(optarg, "hsv") == 0)
                    g_pHyprpicker->m_bSelectedOutputMode = OUTPUT_HSV;
                else {
                    Debug::log(NONE, "Unrecognized format %s", optarg);
                    exit(1);
                }
                break;
            case 'h': help(); exit(0);
            case 'n': g_pHyprpicker->m_bFancyOutput = false; break;
            case 'a': g_pHyprpicker->m_bAutoCopy = true; break;
            case 'r': g_pHyprpicker->m_bRenderInactive = true; break;
            case 'z': g_pHyprpicker->m_bNoZoom = true; break;

            default: help(); exit(1);
        }
    }

    if (!isatty(fileno(stdout)) || getenv("NO_COLOR"))
        g_pHyprpicker->m_bFancyOutput = false;

    g_pHyprpicker->init();

    return 0;
}
