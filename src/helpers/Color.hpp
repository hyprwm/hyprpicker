#pragma once

#include "../defines.hpp"

class CColor {
  public:
    uint8_t r = 0, g = 0, b = 0, a = 0;
    void    getCMYK(float& c, float& m, float& y, float& k) const;
    void    getHSV(float& h, float& s, float& v) const;
    void    getHSL(float& h, float& s, float& l) const;
};
