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

#ifndef ANTARES_GAME_BEAM_HPP_
#define ANTARES_GAME_BEAM_HPP_

#include <stdint.h>

#include "drawing/shapes.hpp"
#include "math/geometry.hpp"

namespace antares {

struct spaceObjectType;

typedef uint8_t beamKindType;
enum beamKindEnum {
    eKineticBeamKind =                  0,  // has velocity, moves
    eStaticObjectToObjectKind =         1,  // static line connects 2 objects
    eStaticObjectToRelativeCoordKind =  2,  // static line goes from object to coord
    eBoltObjectToObjectKind =           3,  // lightning bolt, connects 2 objects
    eBoltObjectToRelativeCoordKind =    4   // lightning bolt, from object to coord
};

static const int kBoltPointNum = 10;

struct beamType {
    beamKindType        beamKind;
    Rect                thisLocation;
    Rect                lastLocation;
    coordPointType      lastGlobalLocation;
    coordPointType      objectLocation;
    coordPointType      lastApparentLocation;
    coordPointType      endLocation;
    uint8_t             color;
    bool                killMe;
    bool                active;
    int32_t             fromObjectNumber;
    int32_t             fromObjectID;
    spaceObjectType*    fromObject;
    int32_t             toObjectNumber;
    int32_t             toObjectID;
    spaceObjectType*    toObject;
    Point               toRelativeCoord;
    uint32_t            boltRandomSeed;
    uint32_t            lastBoldRandomSeed;
    int32_t             boltCycleTime;
    int32_t             boltState;
    int32_t             accuracy;
    int32_t             range;
    Point               thisBoltPoint[kBoltPointNum];
    Point               lastBoltPoint[kBoltPointNum];

    beamType();
};

void InitBeams();
void ResetBeams();
beamType* AddBeam(
        coordPointType* location, uint8_t color, beamKindType kind, int32_t accuracy,
        int32_t beam_range, int32_t* whichBeam);
void SetSpecialBeamAttributes(spaceObjectType* beamObject, spaceObjectType* sourceObject);
void update_beams();
void draw_beams();
void ShowAllBeams();
void CullBeams();

}  // namespace antares

#endif // ANTARES_GAME_BEAM_HPP_
