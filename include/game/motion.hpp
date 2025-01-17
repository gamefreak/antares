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

#ifndef ANTARES_GAME_MOTION_HPP_
#define ANTARES_GAME_MOTION_HPP_

#include "data/space-object.hpp"

namespace antares {

const int32_t kUnitsToCheckNumber = 5;

struct adjacentUnitType {
    long                    adjacentUnit;           // the normal adjacent unit
    Point                   superOffset;            // the offset of the super unit (for wrap-around)
};

struct proximityUnitType {
    spaceObjectTypePtr      nearObject;                         // for collision checking
    spaceObjectTypePtr      farObject;                          // for distance checking
    adjacentUnitType        unitsToCheck[kUnitsToCheckNumber];  // adjacent units to check
};

extern coordPointType gGlobalCorner;

void InitMotion( void);
void ResetMotionGlobals( void);

void MotionCleanup( void);
void MoveSpaceObjects( spaceObjectType *, const long, const long);
void CollideSpaceObjects( spaceObjectType *, const long);
void CorrectPhysicalSpace( spaceObjectType *, spaceObjectType *);

}  // namespace antares

#endif // ANTARES_GAME_MOTION_HPP_
