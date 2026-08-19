#include "Color.hpp"
#include <algorithm>
#include <cstdint>
#include <string_view>
#include "../hyprpicker.hpp"

static float fmax3(float a, float b, float c) {
    return (a > b && a > c) ? a : (b > c) ? b : c;
}

static float fmin3(float a, float b, float c) {
    return (a < b && a < c) ? a : (b < c) ? b : c;
}

static bool floatEq(float a, float b) {
    return std::nextafter(a, std::numeric_limits<double>::lowest()) <= b && std::nextafter(a, std::numeric_limits<double>::max()) >= b;
}

void CColor::getCMYK(float& c, float& m, float& y, float& k) const {
    // http://www.codeproject.com/KB/applications/xcmyk.aspx

    float rf = 1 - (r / 255.0f), gf = 1 - (g / 255.0f), bf = 1 - (b / 255.0f);
    k       = fmin3(rf, gf, bf);
    float K = (k == 1) ? 1 : 1 - k;
    c       = (rf - k) / K;
    m       = (gf - k) / K;
    y       = (bf - k) / K;

    c = std::round(c * 100);
    m = std::round(m * 100);
    y = std::round(y * 100);
    k = std::round(k * 100);
}
void CColor::getHSV(float& h, float& s, float& v) const {
    // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB

    float rf = r / 255.0f, gf = g / 255.0f, bf = b / 255.0f;
    float max = fmax3(rf, gf, bf), min = fmin3(rf, gf, bf);
    float c = max - min;

    v = max;
    if (c == 0)
        h = 0;
    else if (v == rf)
        h = 60 * (0 + (gf - bf) / c);
    else if (v == gf)
        h = 60 * (2 + (bf - rf) / c);
    else /* v == bf */
        h = 60 * (4 + (rf - gf) / c);

    v = max;
    s = floatEq(v, 0.0f) ? 0 : c / v;

    h = std::round(h < 0 ? h + 360 : h);
    v = std::round(v * 100);
    s = std::round(s * 100);
}
void CColor::getHSL(float& h, float& s, float& l) const {
    // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB

    float rf = r / 255.0f, gf = g / 255.0f, bf = b / 255.0f, v;
    float max = fmax3(rf, gf, bf), min = fmin3(rf, gf, bf);
    float c = max - min;

    v = max;
    if (c == 0)
        h = 0;
    else if (v == rf)
        h = 60 * (0 + (gf - bf) / c);
    else if (v == gf)
        h = 60 * (2 + (bf - rf) / c);
    else /* v == bf */
        h = 60 * (4 + (rf - gf) / c);

    v = max;
    s = floatEq(v, 0.0f) ? 0 : c / v;
    l = (max + min) / 2;
    s = (floatEq(l, 0.0f) || floatEq(l, 1.0f)) ? 0 : (v - l) / std::min(l, 1 - l);
    h = std::round(h < 0 ? h + 360 : h);
    s = std::round(s * 100);
    l = std::round(l * 100);
}

std::string_view CColor::getStandardColorName() const {
    struct SStandardColor {
        uint8_t          red;
        uint8_t          green;
        uint8_t          blue;
        std::string_view colorName;
    };

    static constexpr std::array<SStandardColor, 141> standardColors = {{{0xF0, 0xF8, 0xFF, "aliceblue"},
                                                                        {0xFA, 0xEB, 0xD7, "antiquewhite"},
                                                                        {0x00, 0xFF, 0xFF, "aqua"},
                                                                        {0x7F, 0xFF, 0xD4, "aquamarine"},
                                                                        {0xF0, 0xFF, 0xFF, "azure"},
                                                                        {0xF5, 0xF5, 0xDC, "beige"},
                                                                        {0xFF, 0xE4, 0xC4, "bisque"},
                                                                        {0x00, 0x00, 0x00, "black"},
                                                                        {0xFF, 0xEB, 0xCD, "blanchedalmond"},
                                                                        {0x00, 0x00, 0xFF, "blue"},
                                                                        {0x8A, 0x2B, 0xE2, "blueviolet"},
                                                                        {0xA5, 0x2A, 0x2A, "brown"},
                                                                        {0xDE, 0xB8, 0x87, "burlywood"},
                                                                        {0x5F, 0x9E, 0xA0, "cadetblue"},
                                                                        {0x7F, 0xFF, 0x00, "chartreuse"},
                                                                        {0xD2, 0x69, 0x1E, "chocolate"},
                                                                        {0xFF, 0x7F, 0x50, "coral"},
                                                                        {0x64, 0x95, 0xED, "cornflowerblue"},
                                                                        {0xFF, 0xF8, 0xDC, "cornsilk"},
                                                                        {0xDC, 0x14, 0x3C, "crimson"},
                                                                        {0x00, 0xFF, 0xFF, "cyan"},
                                                                        {0x00, 0x00, 0x8B, "darkblue"},
                                                                        {0x00, 0x8B, 0x8B, "darkcyan"},
                                                                        {0xB8, 0x86, 0x0B, "darkgoldenrod"},
                                                                        {0xA9, 0xA9, 0xA9, "darkgray"},
                                                                        {0x00, 0x64, 0x00, "darkgreen"},
                                                                        {0xBD, 0xB7, 0x6B, "darkkhaki"},
                                                                        {0x8B, 0x00, 0x8B, "darkmagenta"},
                                                                        {0x55, 0x6B, 0x2F, "darkolivegreen"},
                                                                        {0xFF, 0x8C, 0x00, "darkorange"},
                                                                        {0x99, 0x32, 0xCC, "darkorchid"},
                                                                        {0x8B, 0x00, 0x00, "darkred"},
                                                                        {0xE9, 0x96, 0x7A, "darksalmon"},
                                                                        {0x8F, 0xBC, 0x8F, "darkseagreen"},
                                                                        {0x48, 0x3D, 0x8B, "darkslateblue"},
                                                                        {0x2F, 0x4F, 0x4F, "darkslategray"},
                                                                        {0x00, 0xCE, 0xD1, "darkturquoise"},
                                                                        {0x94, 0x00, 0xD3, "darkviolet"},
                                                                        {0xFF, 0x14, 0x93, "deeppink"},
                                                                        {0x00, 0xBF, 0xFF, "deepskyblue"},
                                                                        {0x69, 0x69, 0x69, "dimgray"},
                                                                        {0x1E, 0x90, 0xFF, "dodgerblue"},
                                                                        {0xB2, 0x22, 0x22, "firebrick"},
                                                                        {0xFF, 0xFA, 0xF0, "floralwhite"},
                                                                        {0x22, 0x8B, 0x22, "forestgreen"},
                                                                        {0xFF, 0x00, 0xFF, "fuchsia"},
                                                                        {0xDC, 0xDC, 0xDC, "gainsboro"},
                                                                        {0xF8, 0xF8, 0xFF, "ghostwhite"},
                                                                        {0xFF, 0xD7, 0x00, "gold"},
                                                                        {0xDA, 0xA5, 0x20, "goldenrod"},
                                                                        {0x80, 0x80, 0x80, "gray"},
                                                                        {0x00, 0x80, 0x00, "green"},
                                                                        {0xAD, 0xFF, 0x2F, "greenyellow"},
                                                                        {0xF0, 0xFF, 0xF0, "honeydew"},
                                                                        {0xFF, 0x69, 0xB4, "hotpink"},
                                                                        {0xCD, 0x5C, 0x5C, "indianred"},
                                                                        {0x4B, 0x00, 0x82, "indigo"},
                                                                        {0xFF, 0xFF, 0xF0, "ivory"},
                                                                        {0xF0, 0xE6, 0x8C, "khaki"},
                                                                        {0xE6, 0xE6, 0xFA, "lavender"},
                                                                        {0xFF, 0xF0, 0xF5, "lavenderblush"},
                                                                        {0x7C, 0xFC, 0x00, "lawngreen"},
                                                                        {0xFF, 0xFA, 0xCD, "lemonchiffon"},
                                                                        {0xAD, 0xD8, 0xE6, "lightblue"},
                                                                        {0xF0, 0x80, 0x80, "lightcoral"},
                                                                        {0xE0, 0xFF, 0xFF, "lightcyan"},
                                                                        {0xFA, 0xFA, 0xD2, "lightgoldenrodyellow"},
                                                                        {0xD3, 0xD3, 0xD3, "lightgray"},
                                                                        {0x90, 0xEE, 0x90, "lightgreen"},
                                                                        {0xFF, 0xB6, 0xC1, "lightpink"},
                                                                        {0xFF, 0xA0, 0x7A, "lightsalmon"},
                                                                        {0x20, 0xB2, 0xAA, "lightseagreen"},
                                                                        {0x87, 0xCE, 0xFA, "lightskyblue"},
                                                                        {0x77, 0x88, 0x99, "lightslategray"},
                                                                        {0xB0, 0xC4, 0xDE, "lightsteelblue"},
                                                                        {0xFF, 0xFF, 0xE0, "lightyellow"},
                                                                        {0x00, 0xFF, 0x00, "lime"},
                                                                        {0x32, 0xCD, 0x32, "limegreen"},
                                                                        {0xFA, 0xF0, 0xE6, "linen"},
                                                                        {0xFF, 0x00, 0xFF, "magenta"},
                                                                        {0x80, 0x00, 0x00, "maroon"},
                                                                        {0x66, 0xCD, 0xAA, "mediumaquamarine"},
                                                                        {0x00, 0x00, 0xCD, "mediumblue"},
                                                                        {0xBA, 0x55, 0xD3, "mediumorchid"},
                                                                        {0x93, 0x70, 0xDB, "mediumpurple"},
                                                                        {0x3C, 0xB3, 0x71, "mediumseagreen"},
                                                                        {0x7B, 0x68, 0xEE, "mediumslateblue"},
                                                                        {0x00, 0xFA, 0x9A, "mediumspringgreen"},
                                                                        {0x48, 0xD1, 0xCC, "mediumturquoise"},
                                                                        {0xC7, 0x15, 0x85, "mediumvioletred"},
                                                                        {0x19, 0x19, 0x70, "midnightblue"},
                                                                        {0xF5, 0xFF, 0xFA, "mintcream"},
                                                                        {0xFF, 0xE4, 0xE1, "mistyrose"},
                                                                        {0xFF, 0xE4, 0xB5, "moccasin"},
                                                                        {0xFF, 0xDE, 0xAD, "navajowhite"},
                                                                        {0x00, 0x00, 0x80, "navy"},
                                                                        {0xFD, 0xF5, 0xE6, "oldlace"},
                                                                        {0x80, 0x80, 0x00, "olive"},
                                                                        {0x6B, 0x8E, 0x23, "olivedrab"},
                                                                        {0xFF, 0xA5, 0x00, "orange"},
                                                                        {0xFF, 0x45, 0x00, "orangered"},
                                                                        {0xDA, 0x70, 0xD6, "orchid"},
                                                                        {0xEE, 0xE8, 0xAA, "palegoldenrod"},
                                                                        {0x98, 0xFB, 0x98, "palegreen"},
                                                                        {0xAF, 0xEE, 0xEE, "paleturquoise"},
                                                                        {0xDB, 0x70, 0x93, "palevioletred"},
                                                                        {0xFF, 0xEF, 0xD5, "papayawhip"},
                                                                        {0xFF, 0xDA, 0xB9, "peachpuff"},
                                                                        {0xCD, 0x85, 0x3F, "peru"},
                                                                        {0xFF, 0xC0, 0xCB, "pink"},
                                                                        {0xDD, 0xA0, 0xDD, "plum"},
                                                                        {0xB0, 0xE0, 0xE6, "powderblue"},
                                                                        {0x80, 0x00, 0x80, "purple"},
                                                                        {0x66, 0x33, 0x99, "rebeccapurple"},
                                                                        {0xFF, 0x00, 0x00, "red"},
                                                                        {0xBC, 0x8F, 0x8F, "rosybrown"},
                                                                        {0x41, 0x69, 0xE1, "royalblue"},
                                                                        {0x8B, 0x45, 0x13, "saddlebrown"},
                                                                        {0xFA, 0x80, 0x72, "salmon"},
                                                                        {0xF4, 0xA4, 0x60, "sandybrown"},
                                                                        {0x2E, 0x8B, 0x57, "seagreen"},
                                                                        {0xFF, 0xF5, 0xEE, "seashell"},
                                                                        {0xA0, 0x52, 0x2D, "sienna"},
                                                                        {0xC0, 0xC0, 0xC0, "silver"},
                                                                        {0x87, 0xCE, 0xEB, "skyblue"},
                                                                        {0x6A, 0x5A, 0xCD, "slateblue"},
                                                                        {0x70, 0x80, 0x90, "slategray"},
                                                                        {0xFF, 0xFA, 0xFA, "snow"},
                                                                        {0x00, 0xFF, 0x7F, "springgreen"},
                                                                        {0x46, 0x82, 0xB4, "steelblue"},
                                                                        {0xD2, 0xB4, 0x8C, "tan"},
                                                                        {0x00, 0x80, 0x80, "teal"},
                                                                        {0xD8, 0xBF, 0xD8, "thistle"},
                                                                        {0xFF, 0x63, 0x47, "tomato"},
                                                                        {0x40, 0xE0, 0xD0, "turquoise"},
                                                                        {0xEE, 0x82, 0xEE, "violet"},
                                                                        {0xF5, 0xDE, 0xB3, "wheat"},
                                                                        {0xFF, 0xFF, 0xFF, "white"},
                                                                        {0xF5, 0xF5, 0xF5, "whitesmoke"},
                                                                        {0xFF, 0xFF, 0x00, "yellow"},
                                                                        {0x9A, 0xCD, 0x32, "yellowgreen"}}};

    auto matchColor = [this](const SStandardColor& referenceColor) { return referenceColor.red == r && referenceColor.green == g && referenceColor.blue == b; };

    auto iterator = std::ranges::find_if(standardColors, matchColor);

    if (iterator != standardColors.end()) {
        return iterator->colorName;
    }

    return "";
}
