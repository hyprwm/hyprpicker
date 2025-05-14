#include "Color.hpp"
#include <algorithm>
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
