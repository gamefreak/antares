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

#ifndef ANTARES_UI_SCREENS_MAIN_HPP_
#define ANTARES_UI_SCREENS_MAIN_HPP_

#include <sfz/sfz.hpp>

#include "ui/interface-screen.hpp"

namespace antares {

class MainScreen : public InterfaceScreen {
  public:
    MainScreen();
    ~MainScreen();

    virtual void become_front();

    virtual bool next_timer(int64_t& time);
    virtual void fire_timer();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);

  private:
    enum Button {
        START_NEW_GAME = 0,
        START_NETWORK_GAME = 1,
        OPTIONS = 2,
        QUIT = 3,
        ABOUT_ARES = 4,
        DEMO = 5,
        REPLAY_INTRO = 6,
    };

    DISALLOW_COPY_AND_ASSIGN(MainScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_MAIN_HPP_
