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

#ifndef ANTARES_UI_SCREENS_SELECT_LEVEL_HPP_
#define ANTARES_UI_SCREENS_SELECT_LEVEL_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "ui/interface-screen.hpp"

namespace antares {

struct Scenario;

class SelectLevelScreen : public InterfaceScreen {
  public:
    SelectLevelScreen(bool* cancelled, const Scenario** scenario);
    ~SelectLevelScreen();

    virtual void become_front();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);
    virtual void draw() const;

  private:
    enum Item {
        // Buttons:
        OK = 0,
        CANCEL = 1,
        PREVIOUS = 2,
        NEXT = 3,

        // Text box:
        NAME = 4,
    };

    enum State {
        SELECTING,
        FADING_OUT,
    };
    State _state;

    void draw_level_name() const;

    bool* _cancelled;
    size_t _index;
    const Scenario** _scenario;
    std::vector<int> _chapters;

    DISALLOW_COPY_AND_ASSIGN(SelectLevelScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_SELECT_LEVEL_HPP_
