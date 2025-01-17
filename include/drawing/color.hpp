// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_DRAWING_COLOR_HPP_
#define ANTARES_DRAWING_COLOR_HPP_

#include <stdint.h>
#include <vector>
#include <sfz/sfz.hpp>

namespace antares {

enum {
    RED             = 15,
    ORANGE          = 1,
    YELLOW          = 2,
    BLUE            = 3,
    GREEN           = 4,
    PURPLE          = 5,
    INDIGO          = 6,
    SALMON          = 7,
    GOLD            = 8,
    AQUA            = 9,
    PINK            = 10,
    PALE_GREEN      = 11,
    PALE_PURPLE     = 12,
    SKY_BLUE        = 13,
    TAN             = 14,
    GRAY            = 0,
};

enum {
    BLACK           = 0xff,
    WHITE           = 0x00,
};

enum {
    VERY_LIGHT      = 16,
    LIGHTER         = 14,
    LIGHT           = 12,
    MEDIUM          = 9,
    DARK            = 7,
    DARKER          = 5,
    VERY_DARK       = 3,
    DARKEST         = 1,
};

enum {
    kLighterColor           = 2,
    kDarkerColor            = -2,
    kSlightlyLighterColor   = 1,
    kSlightlyDarkerColor    = -1,
};

enum {
    kVisibleShadeNum    = 15,
};

class RgbColor {
  public:
    static const RgbColor kBlack;
    static const RgbColor kWhite;
    static const RgbColor kClear;

    RgbColor();
    RgbColor(uint8_t red, uint8_t green, uint8_t blue);
    RgbColor(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue);

    static const RgbColor& at(uint8_t index);

    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

void read_from(sfz::ReadSource in, RgbColor& color);
void write_to(sfz::WriteTarget out, const RgbColor& color);
void print_to(sfz::PrintTarget out, const RgbColor& color);

inline bool operator==(const RgbColor& lhs, const RgbColor& rhs) {
    return memcmp(&lhs, &rhs, sizeof(RgbColor)) == 0;
}

inline bool operator!=(const RgbColor& lhs, const RgbColor& rhs) {
    return memcmp(&lhs, &rhs, sizeof(RgbColor)) != 0;
}

uint8_t GetRetroIndex(uint8_t which);
uint8_t GetTranslateIndex(uint8_t which);
uint8_t GetTranslateColorShade(uint8_t color, uint8_t shade);

RgbColor GetRGBTranslateColorShade(uint8_t color, uint8_t shade);
RgbColor GetRGBTranslateColor(uint8_t color);

}  // namespace antares

#endif  // ANTARES_DRAWING_COLOR_HPP_
