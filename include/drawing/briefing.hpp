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

#ifndef ANTARES_DRAWING_BRIEFING_HPP_
#define ANTARES_DRAWING_BRIEFING_HPP_

#include "drawing/shapes.hpp"

namespace antares {

struct Scenario;

void Briefing_Objects_Render(
        PixMap* destmap, long maxSize, Rect *bounds, coordPointType *corner, long scale);

void BriefPoint_Data_Get(
        long whichPoint, const Scenario* scenario, long *headerID, long *headerNumber,
        long *contentID, Rect *hiliteBounds, coordPointType *corner, long scale,
        long minSectorSize, long maxSize, Rect *bounds);

}  // namespace antares

#endif // ANTARES_DRAWING_BRIEFING_HPP_
