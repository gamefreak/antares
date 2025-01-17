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

#include "data/replay-list.hpp"

#include <glob.h>
#include <sfz/sfz.hpp>
#include "config/preferences.hpp"

using sfz::CString;
using sfz::MappedFile;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::read;
using std::vector;

namespace utf8 = sfz::utf8;
namespace path = sfz::path;

namespace antares {

namespace {

struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, sizeof(data), 0); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

ReplayList::ReplayList() {
    ScopedGlob g;
    const String home(utf8::decode(getenv("HOME")));
    const StringSlice scenarios("Library/Application Support/Antares/Scenarios");
    const StringSlice scenario = Preferences::preferences()->scenario_identifier();
    String str(format("{0}/{1}/{2}/replays/*.NLRP", home, scenarios, scenario));
    CString c_str(str);
    glob(c_str.data(), 0, NULL, &g.data);

    for (int i = 0; i < g.data.gl_matchc; ++i) {
        const String path(utf8::decode(g.data.gl_pathv[i]));
        StringSlice basename = path::basename(path);
        StringSlice id_string = basename.slice(0, basename.size() - 5);
        int16_t id;
        if (string_to_int(id_string, id)) {
            _replays.push_back(id);
        }
    }
}

size_t ReplayList::size() const {
    return _replays.size();
}

int16_t ReplayList::at(size_t index) const {
    return _replays.at(index);
}

}  // namespace antares
