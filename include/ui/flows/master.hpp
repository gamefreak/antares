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

#ifndef ANTARES_UI_FLOWS_MASTER_HPP_
#define ANTARES_UI_FLOWS_MASTER_HPP_

#include <sfz/sfz.hpp>

#include "game/main.hpp"
#include "ui/card.hpp"
#include "ui/screens/play-again.hpp"

namespace antares {

class Master : public Card {
  public:
    Master();

    virtual void become_front();
    virtual void draw();

  private:
    static void init();

    enum State {
        START,
        PUBLISHER_PICT,
        EGO_PICT,
        TITLE_SCREEN_PICT,
        INTRO_SCROLL,
        MAIN_SCREEN,
    };

    State _state;
    bool _skipped;
};

}  // namespace antares

#endif  // ANTARES_UI_FLOWS_MASTER_HPP_
