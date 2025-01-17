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

#include "data/scenario-list.hpp"

#include <glob.h>
#include <sfz/sfz.hpp>
#include "data/scenario.hpp"

using sfz::BytesSlice;
using sfz::CString;
using sfz::MappedFile;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::linked_ptr;
using sfz::read;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, sizeof(data), 0); }
    ~ScopedGlob() { globfree(&data); }
};

void u32_to_version(uint32_t in, Version& out) {
    using std::swap;
    vector<int> components;
    components.push_back((in & 0xff000000) >> 24);
    components.push_back((in & 0x00ff0000) >> 16);
    if (in & 0x0000ffff) {
        components.push_back((in & 0x0000ff00) >> 8);
    }
    if (in & 0x000000ff) {
        components.push_back(in & 0x000000ff);
    }
    swap(out.components, components);
}

}  // namespace

ScenarioList::ScenarioList() {
    linked_ptr<Entry> factory_scenario(new Entry);
    factory_scenario->identifier.assign("com.biggerplanet.ares");
    factory_scenario->title.assign("Ares");
    factory_scenario->download_url.assign("http://www.arescentral.com");
    factory_scenario->author.assign("Bigger Planet");
    factory_scenario->author_url.assign("http://www.biggerplanet.com");
    u32_to_version(0x01010100, factory_scenario->version);
    _scenarios.push_back(factory_scenario);

    ScopedGlob g;
    const String home(utf8::decode(getenv("HOME")));
    const StringSlice scenarios("Library/Application Support/Antares/Scenarios");
    const StringSlice info("scenario-info/128.nlAG");
    String str(format("{0}/{1}/*/{2}", home, scenarios, info));
    CString c_str(str);
    glob(c_str.data(), 0, NULL, &g.data);

    size_t prefix_len = home.size() + scenarios.size() + 2;
    size_t suffix_len = info.size() + 1;
    for (int i = 0; i < g.data.gl_matchc; ++i) {
        const String path(utf8::decode(g.data.gl_pathv[i]));
        StringSlice identifier = path.slice(prefix_len, path.size() - prefix_len - suffix_len);
        if (identifier == factory_scenario->identifier) {
            continue;
        }

        MappedFile file(path);
        BytesSlice data(file.data());
        scenarioInfoType info;
        read(data, info);
        linked_ptr<Entry> entry(new Entry);
        entry->identifier.assign(identifier);
        entry->title.assign(info.titleString);
        entry->download_url.assign(info.downloadURLString);
        entry->author.assign(info.authorNameString);
        entry->author_url.assign(info.authorURLString);
        u32_to_version(info.version, entry->version);
        _scenarios.push_back(entry);
    }
}

size_t ScenarioList::size() const {
    return _scenarios.size();
}

const ScenarioList::Entry& ScenarioList::at(size_t index) const {
    return *_scenarios.at(index);
}

void print_to(sfz::PrintTarget out, const Version& v) {
    for (vector<int>::const_iterator begin = v.components.begin(), end = v.components.end();
            begin != end; ++begin) {
        if (begin != v.components.begin()) {
            print(out, ".");
        }
        print(out, *begin);
    }
}

}  // namespace antares
