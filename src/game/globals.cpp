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

#include "game/globals.hpp"

#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/admiral.hpp"
#include "game/beam.hpp"
#include "game/input-source.hpp"
#include "game/labels.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/starfield.hpp"
#include "sound/driver.hpp"

namespace antares {

static aresGlobalType* gAresGlobal;

aresGlobalType* globals() {
    return gAresGlobal;
}

void init_globals() {
    gAresGlobal = new aresGlobalType;
}

aresGlobalType::aresGlobalType() {
    for (int player = 0; player < kMaxPlayerNum; player++) {
        gActiveCheats[player] = 0;
    }
    gKeyMapBuffer = new KeyMap[kKeyMapBufferNum];
    gKeyMapBufferTop = 0;
    gKeyMapBufferBottom = 0;
    gGameOver = 1;
    gGameTime = 0;
    gClosestObject = 0;
    gFarthestObject = 0;
    gCenterScaleH = 0;
    gCenterScaleV = 0;
    gPlayerShipNumber = 0;
    gSelectionLabel = -1;
    gZoomMode = kTimesTwoZoom;
    gRadarCount = 0;
    gRadarSpeed = 30;
    gRadarRange = kRadarSize * 50;
    gWhichScaleNum = 0;
    gLastScale = SCALE_SCALE;
    gInstrumentTop = 0;
    gMouseActive = false;
    gMouseTimeout = 0;
    gMessageTimeCount = 0;
    gMessageLabelNum = -1;
    gStatusLabelNum = -1;
    gLastSoundTime = 0;
    gLastSelectedBuildPrice = 0;
    gAutoPilotOff = true;
    levelNum = 31;
    keyMask = 0;
    gSerialNumerator = 0;
    gSerialDenominator = 0;

    hotKeyDownTime = -1;
}

aresGlobalType::~aresGlobalType() {
}

}  // namespace antares
