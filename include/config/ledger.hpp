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

#ifndef ANTARES_CONFIG_LEDGER_HPP_
#define ANTARES_CONFIG_LEDGER_HPP_

#include <set>
#include <vector>
#include <sfz/sfz.hpp>

namespace antares {

class Ledger {
  public:
    virtual ~Ledger();

    static Ledger* ledger();
    static void set_ledger(Ledger* ledger);

    virtual void unlock_chapter(int chapter) = 0;
    virtual void unlocked_chapters(std::vector<int>* chapters) = 0;
};

class NullLedger : public Ledger {
  public:
    NullLedger();
    virtual void unlock_chapter(int chapter);
    virtual void unlocked_chapters(std::vector<int>* chapters);

  private:
    std::set<int> _chapters;

    DISALLOW_COPY_AND_ASSIGN(NullLedger);
};

class DirectoryLedger : public Ledger {
  public:
    DirectoryLedger(const sfz::String& directory);
    virtual void unlock_chapter(int chapter);
    virtual void unlocked_chapters(std::vector<int>* chapters);

  private:
    class Visitor;

    void load();
    void save();

    const sfz::String _directory;
    std::set<int> _chapters;

    DISALLOW_COPY_AND_ASSIGN(DirectoryLedger);
};

}  // namespace antares

#endif  // ANTARES_CONFIG_LEDGER_HPP_
