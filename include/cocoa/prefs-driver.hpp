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

#ifndef ANTARES_COCOA_PREFS_DRIVER_HPP_
#define ANTARES_COCOA_PREFS_DRIVER_HPP_

#include <sfz/sfz.hpp>

#include "config/preferences.hpp"

namespace antares {

class CoreFoundationPrefsDriver : public PrefsDriver {
  public:
    CoreFoundationPrefsDriver();

    virtual void load(Preferences* preferences);
    virtual void save(const Preferences& preferences);

  private:
    DISALLOW_COPY_AND_ASSIGN(CoreFoundationPrefsDriver);
};

}  // namespace antares

#endif  // ANTARES_COCOA_PREFS_DRIVER_HPP_
