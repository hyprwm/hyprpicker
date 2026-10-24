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

    static constexpr std::array<SStandardColor, 141> standardColors = {{
        {0xF0, 0xF8, 0xFF, "Alice Blue"},
        {0xFA, 0xEB, 0xD7, "Antique White"},
        {0x00, 0xFF, 0xFF, "Aqua"},
        {0x7F, 0xFF, 0xD4, "Aquamarine"},
        {0xF0, 0xFF, 0xFF, "Azure"},
        {0xF5, 0xF5, 0xDC, "Beige"},
        {0xFF, 0xE4, 0xC4, "Bisque"},
        {0x00, 0x00, 0x00, "Black"},
        {0xFF, 0xEB, 0xCD, "Blanched Almond"},
        {0x00, 0x00, 0xFF, "Blue"},
        {0x8A, 0x2B, 0xE2, "Blue Violet"},
        {0xA5, 0x2A, 0x2A, "Brown"},
        {0xDE, 0xB8, 0x87, "Burlywood"},
        {0x5F, 0x9E, 0xA0, "Cadet Blue"},
        {0x7F, 0xFF, 0x00, "Chartreuse"},
        {0xD2, 0x69, 0x1E, "Chocolate"},
        {0xFF, 0x7F, 0x50, "Coral"},
        {0x64, 0x95, 0xED, "Cornflower Blue"},
        {0xFF, 0xF8, 0xDC, "Cornsilk"},
        {0xDC, 0x14, 0x3C, "Crimson"},
        {0x00, 0xFF, 0xFF, "Cyan"},
        {0x00, 0x00, 0x8B, "Dark Blue"},
        {0x00, 0x8B, 0x8B, "Dark Cyan"},
        {0xB8, 0x86, 0x0B, "Dark Goldenrod"},
        {0xA9, 0xA9, 0xA9, "Dark Gray"},
        {0x00, 0x64, 0x00, "Dark Green"},
        {0xBD, 0xB7, 0x6B, "Dark Khaki"},
        {0x8B, 0x00, 0x8B, "Dark Magenta"},
        {0x55, 0x6B, 0x2F, "Dark Olivegreen"},
        {0xFF, 0x8C, 0x00, "Dark Orange"},
        {0x99, 0x32, 0xCC, "Dark Orchid"},
        {0x8B, 0x00, 0x00, "Dark Red"},
        {0xE9, 0x96, 0x7A, "Dark Salmon"},
        {0x8F, 0xBC, 0x8F, "Dark Seagreen"},
        {0x48, 0x3D, 0x8B, "Dark Slateblue"},
        {0x2F, 0x4F, 0x4F, "Dark Slategray"},
        {0x00, 0xCE, 0xD1, "Dark Turquoise"},
        {0x94, 0x00, 0xD3, "Dark Violet"},
        {0xFF, 0x14, 0x93, "Deep Pink"},
        {0x00, 0xBF, 0xFF, "Deep Sky Blue"},
        {0x69, 0x69, 0x69, "Dim Gray"},
        {0x1E, 0x90, 0xFF, "Dodger Blue"},
        {0xB2, 0x22, 0x22, "Firebrick"},
        {0xFF, 0xFA, 0xF0, "Floral White"},
        {0x22, 0x8B, 0x22, "Forest Green"},
        {0xFF, 0x00, 0xFF, "Fuchsia"},
        {0xDC, 0xDC, 0xDC, "Gainsboro"},
        {0xF8, 0xF8, 0xFF, "Ghost White"},
        {0xFF, 0xD7, 0x00, "Gold"},
        {0xDA, 0xA5, 0x20, "Goldenrod"},
        {0x80, 0x80, 0x80, "Gray"},
        {0x00, 0x80, 0x00, "Green"},
        {0xAD, 0xFF, 0x2F, "Green Yellow"},
        {0xF0, 0xFF, 0xF0, "Honeydew"},
        {0xFF, 0x69, 0xB4, "Hot Pink"},
        {0xCD, 0x5C, 0x5C, "Indian Red"},
        {0x4B, 0x00, 0x82, "Indigo"},
        {0xFF, 0xFF, 0xF0, "Ivory"},
        {0xF0, 0xE6, 0x8C, "Khaki"},
        {0xE6, 0xE6, 0xFA, "Lavender"},
        {0xFF, 0xF0, 0xF5, "Lavender Blush"},
        {0x7C, 0xFC, 0x00, "Lawn Green"},
        {0xFF, 0xFA, 0xCD, "Lemonchiffon"},
        {0xAD, 0xD8, 0xE6, "Light Blue"},
        {0xF0, 0x80, 0x80, "Light Coral"},
        {0xE0, 0xFF, 0xFF, "Light Cyan"},
        {0xFA, 0xFA, 0xD2, "Light Goldenrod Yellow"},
        {0xD3, 0xD3, 0xD3, "Light Gray"},
        {0x90, 0xEE, 0x90, "Light Green"},
        {0xFF, 0xB6, 0xC1, "Light Pink"},
        {0xFF, 0xA0, 0x7A, "Light Salmon"},
        {0x20, 0xB2, 0xAA, "Light Sea Green"},
        {0x87, 0xCE, 0xFA, "Light Sky Blue"},
        {0x77, 0x88, 0x99, "Light Slate Gray"},
        {0xB0, 0xC4, 0xDE, "Light Steel Blue"},
        {0xFF, 0xFF, 0xE0, "Light Yellow"},
        {0x00, 0xFF, 0x00, "Lime"},
        {0x32, 0xCD, 0x32, "Lime Green"},
        {0xFA, 0xF0, 0xE6, "Linen"},
        {0xFF, 0x00, 0xFF, "Magenta"},
        {0x80, 0x00, 0x00, "Maroon"},
        {0x66, 0xCD, 0xAA, "Medium Aquamarine"},
        {0x00, 0x00, 0xCD, "Medium Blue"},
        {0xBA, 0x55, 0xD3, "Medium Orchid"},
        {0x93, 0x70, 0xDB, "Medium Purple"},
        {0x3C, 0xB3, 0x71, "Medium Sea Green"},
        {0x7B, 0x68, 0xEE, "Medium Slate Blue"},
        {0x00, 0xFA, 0x9A, "Medium Spring Green"},
        {0x48, 0xD1, 0xCC, "Medium Turquoise"},
        {0xC7, 0x15, 0x85, "Medium Violet Red"},
        {0x19, 0x19, 0x70, "Midnight Blue"},
        {0xF5, 0xFF, 0xFA, "Mintcream"},
        {0xFF, 0xE4, 0xE1, "Mistyrose"},
        {0xFF, 0xE4, 0xB5, "Moccasin"},
        {0xFF, 0xDE, 0xAD, "Navajo White"},
        {0x00, 0x00, 0x80, "Navy"},
        {0xFD, 0xF5, 0xE6, "Oldlace"},
        {0x80, 0x80, 0x00, "Olive"},
        {0x6B, 0x8E, 0x23, "Olive Drab"},
        {0xFF, 0xA5, 0x00, "Orange"},
        {0xFF, 0x45, 0x00, "Orange Red"},
        {0xDA, 0x70, 0xD6, "Orchid"},
        {0xEE, 0xE8, 0xAA, "Pale Goldenrod"},
        {0x98, 0xFB, 0x98, "Pale green"},
        {0xAF, 0xEE, 0xEE, "Pale Turquoise"},
        {0xDB, 0x70, 0x93, "Pale Violet Red"},
        {0xFF, 0xEF, 0xD5, "Papayawhip"},
        {0xFF, 0xDA, 0xB9, "Peachpuff"},
        {0xCD, 0x85, 0x3F, "Peru"},
        {0xFF, 0xC0, 0xCB, "Pink"},
        {0xDD, 0xA0, 0xDD, "Plum"},
        {0xB0, 0xE0, 0xE6, "Powder Blue"},
        {0x80, 0x00, 0x80, "Purple"},
        {0x66, 0x33, 0x99, "Rebecca Purple"},
        {0xFF, 0x00, 0x00, "Red"},
        {0xBC, 0x8F, 0x8F, "Rosy Brown"},
        {0x41, 0x69, 0xE1, "Royal Blue"},
        {0x8B, 0x45, 0x13, "Saddle Brown"},
        {0xFA, 0x80, 0x72, "Salmon"},
        {0xF4, 0xA4, 0x60, "Sandy Brown"},
        {0x2E, 0x8B, 0x57, "Sea Green"},
        {0xFF, 0xF5, 0xEE, "Seashell"},
        {0xA0, 0x52, 0x2D, "Sienna"},
        {0xC0, 0xC0, 0xC0, "Silver"},
        {0x87, 0xCE, 0xEB, "Sky Blue"},
        {0x6A, 0x5A, 0xCD, "Slate Blue"},
        {0x70, 0x80, 0x90, "Slate Gray"},
        {0xFF, 0xFA, 0xFA, "Snow"},
        {0x00, 0xFF, 0x7F, "Spring Green"},
        {0x46, 0x82, 0xB4, "Steel Blue"},
        {0xD2, 0xB4, 0x8C, "Tan"},
        {0x00, 0x80, 0x80, "Teal"},
        {0xD8, 0xBF, 0xD8, "Thistle"},
        {0xFF, 0x63, 0x47, "Tomato"},
        {0x40, 0xE0, 0xD0, "Turquoise"},
        {0xEE, 0x82, 0xEE, "Violet"},
        {0xF5, 0xDE, 0xB3, "Wheat"},
        {0xFF, 0xFF, 0xFF, "White"},
        {0xF5, 0xF5, 0xF5, "White Smoke"},
        {0xFF, 0xFF, 0x00, "Yellow"},
        {0x9A, 0xCD, 0x32, "Yellow Green"},
    }};

    auto matchColor = [this](const SStandardColor& referenceColor) { return referenceColor.red == r && referenceColor.green == g && referenceColor.blue == b; };

    auto iterator = std::ranges::find_if(standardColors, matchColor);

    if (iterator != standardColors.end()) {
        return iterator->colorName;
    }

    return "";
}
