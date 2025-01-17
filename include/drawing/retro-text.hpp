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

#ifndef ANTARES_DRAWING_RETRO_TEXT_HPP_
#define ANTARES_DRAWING_RETRO_TEXT_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "math/geometry.hpp"

namespace antares {

class Picture;
class PixMap;

class RetroText {
  public:
    RetroText(const sfz::StringSlice& text, int font, RgbColor fore_color, RgbColor back_color);
    ~RetroText();

    void set_tab_width(int tab_width);
    void wrap_to(int width, int line_spacing);

    int size() const;
    int tab_width() const;
    int width() const;
    int height() const;
    int auto_width() const;

    void draw(PixMap* pix, const Rect& bounds) const;
    void draw_char(PixMap* pix, const Rect& bounds, int index) const;

  private:
    enum SpecialChar {
        NONE,
        TAB,
        WORD_BREAK,
        LINE_BREAK,
    };

    struct RetroChar {
        RetroChar(
                uint32_t character, SpecialChar special, const RgbColor& fore_color,
                const RgbColor& back_color);

        uint32_t character;
        SpecialChar special;
        RgbColor fore_color;
        RgbColor back_color;
        int h;
        int v;
    };

    int move_word_down(int index, int v);

    std::vector<RetroChar> _chars;
    int _tab_width;
    int _width;
    int _height;
    int _auto_width;
    int _line_spacing;
    const int _font;

    DISALLOW_COPY_AND_ASSIGN(RetroText);
};

}  // namespace antares

#endif  // ANTARES_DRAWING_RETRO_TEXT_HPP_
