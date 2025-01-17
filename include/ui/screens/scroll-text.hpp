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

#ifndef ANTARES_UI_SCREENS_SCROLL_TEXT_HPP_
#define ANTARES_UI_SCREENS_SCROLL_TEXT_HPP_

#include <sfz/sfz.hpp>

#include "math/geometry.hpp"
#include "ui/card.hpp"

namespace antares {

class Sprite;

class ScrollTextScreen : public Card {
  public:
    ScrollTextScreen(int text_id, int width, double speed);
    ScrollTextScreen(int text_id, int width, double speed, int song_id);

    virtual void become_front();
    virtual void resign_front();

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void key_down(const KeyDownEvent& event);

    virtual bool next_timer(int64_t& time);
    virtual void fire_timer();

    virtual void draw() const;

  private:
    sfz::scoped_ptr<Sprite> _sprite;
    const double _speed;
    const bool _play_song;
    const int _song_id;

    int64_t _start;
    int64_t _next_shift;
    Rect _clip;
    Rect _position;

    DISALLOW_COPY_AND_ASSIGN(ScrollTextScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_SCROLL_TEXT_HPP_
