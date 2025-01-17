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

#ifndef ANTARES_DATA_SCENARIO_LIST_HPP_
#define ANTARES_DATA_SCENARIO_LIST_HPP_

#include <vector>
#include <sfz/sfz.hpp>

namespace antares {

struct Version {
    std::vector<int> components;
};
void print_to(sfz::PrintTarget out, const Version& v);

class ScenarioList {
  public:
    struct Entry {
        sfz::String identifier;
        sfz::String title;
        sfz::String download_url;
        sfz::String author;
        sfz::String author_url;
        Version version;
    };

    ScenarioList();
    size_t size() const;
    const Entry& at(size_t index) const;

  private:
    std::vector<sfz::linked_ptr<Entry> > _scenarios;

    DISALLOW_COPY_AND_ASSIGN(ScenarioList);
};

}  // namespace antares

#endif // ANTARES_DATA_SCENARIO_LIST_HPP_
