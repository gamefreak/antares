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

#include "math/rotation.hpp"

#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "math/macros.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::read;

namespace antares {

const int kRotTableSize = 720;
int32_t gRotTable[kRotTableSize];

void RotationInit() {
    Resource rsrc("rotation-table", "rot ", 500);
    BytesSlice in(rsrc.data());
    read(in, gRotTable, kRotTableSize);
    if (!in.empty()) {
        throw Exception("didn't consume all of rotation data");
    }
}

void GetRotPoint(int32_t *x, int32_t *y, int32_t rotpos) {
    int32_t* i;

    i = gRotTable + rotpos * 2L;
    *x = *i;
    i++;
    *y = *i;
}

int32_t GetAngleFromVector(int32_t x, int32_t y) {
    int32_t* h;
    int32_t* v;
    int32_t a, b, test = 0, best = 0, whichBest = -1, whichAngle;

    a = x;
    b = y;

    if ( a < 0) a = -a;
    if ( b < 0) b = -b;
    if ( b < a)
    {
        h = gRotTable + ROT_45 * 2;
        whichAngle = ROT_45;
        v = h + 1;
        do
        {
            test = (*v * a) + (*h * b); // we're adding b/c in my table 45-90 degrees, h < 0
            if ( test < 0) test = -test;
            if (( whichBest < 0)  || ( test < best))
            {
                best = test;
                whichBest = whichAngle;
            }
            h += 2;
            v += 2;
            whichAngle++;
        } while ( ( test == best) && ( whichAngle <= ROT_90));
    } else
    {
        h = gRotTable + ROT_0 * 2;
        whichAngle = ROT_0;
        v = h + 1;
        do
        {
            test = (*v * a) + (*h * b);
            if ( test < 0) test = -test;
            if (( whichBest < 0)  || ( test < best))
            {
                best = test;
                whichBest = whichAngle;
            }
            h += 2;
            v += 2;
            whichAngle++;
        } while (( test == best) &&  ( whichAngle <= ROT_45));
    }
    if ( x > 0)
    {
        if ( y < 0) whichBest = whichBest + ROT_180;
        else whichBest = ROT_POS - whichBest;
    } else if ( y < 0) whichBest = ROT_180 - whichBest;
    if ( whichBest == ROT_POS) whichBest = ROT_0;
    return ( whichBest);
}

}  // namespace antares
