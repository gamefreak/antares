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

#ifndef ANTARES_UI_SCREENS_PLAY_AGAIN_HPP_
#define ANTARES_UI_SCREENS_PLAY_AGAIN_HPP_

#include <sfz/sfz.hpp>

#include "ui/interface-screen.hpp"

namespace antares {

class PlayAgainScreen : public InterfaceScreen {
  public:
    enum Item {
        RESTART = 0,
        QUIT = 1,
        RESUME = 4,
        SKIP = 5,

        BOX = 3,
    };

    PlayAgainScreen(bool allow_resume, bool allow_skip, Item* button_pressed);
    ~PlayAgainScreen();

    virtual void become_front();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);

  private:
    enum State {
        ASKING,
        FADING_OUT,
    };
    State _state;

    Item* _button_pressed;

    DISALLOW_COPY_AND_ASSIGN(PlayAgainScreen);
};

void print_to(sfz::PrintTarget out, PlayAgainScreen::Item item);

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_PLAY_AGAIN_HPP_
