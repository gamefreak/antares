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

#ifndef ANTARES_GAME_GLOBALS_HPP_
#define ANTARES_GAME_GLOBALS_HPP_

#include <queue>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/scenario.hpp"
#include "drawing/color.hpp"
#include "drawing/shapes.hpp"
#include "game/starfield.hpp"
#include "sound/fx.hpp"
#include "video/transitions.hpp"

namespace antares {

const size_t kKeyControlNum                = 19;
const size_t kKeyExtendedControlNum     = 44;
const size_t kBarIndicatorNum              = 5;
const size_t kKeyMapBufferNum              = 256;
const size_t kHotKeyNum                    = 10;

const int32_t kRadarSize                = 110;

enum ZoomType {
    kTimesTwoZoom           = 0,
    kActualSizeZoom         = 1,
    kHalfSizeZoom           = 2,
    kQuarterSizeZoom        = 3,
    kEighthSizeZoom         = 4,
    kNearestFoeZoom         = 5,
    kNearestAnythingZoom    = 6,
    kSmallestZoom           = 7,
};

struct barIndicatorType {
    short           top;
    long            thisValue;
    long            lastValue;
    unsigned char   color;
    bool         automatic;      // if it's automatic, it is redrawn automatically
};

struct miniScreenLineType;
struct miniComputerDataType {
    sfz::scoped_array<miniScreenLineType> lineData;
    sfz::scoped_array<spaceObjectType> objectData;
    long                    selectLine;
    long                    pollTime;
    long                    buildTimeBarValue;
    long                    currentScreen;
    long                    clickLine;
};

struct hotKeyType {
    long        objectNum;
    long        objectID;
};

struct admiralType;
struct beamType;
struct destBalanceType;
struct longMessageType;
struct proximityUnitType;
struct screenLabelType;
struct scrollStarType;
class InputSource;
class StringList;

struct ScenarioWinnerType {
    int8_t next;
    int16_t text;
    int8_t player;
};

struct aresGlobalType {
    aresGlobalType();
    ~aresGlobalType();

    unsigned long   gActiveCheats[kMaxPlayerNum];
    unsigned long   gSynchValue;
    sfz::scoped_ptr<InputSource> gInputSource;
    long            gGameOver;
    sfz::scoped_array<admiralType>       gAdmiralData;
    long            gGameTime;
    uint64_t        gLastTime;
    long            gClosestObject;
    long            gFarthestObject;
    long            gCenterScaleH;
    long            gCenterScaleV;
    long            gPlayerShipNumber;
    long            gSelectionLabel;
    ZoomType        gZoomMode;
    long            gPlayerAdmiralNumber;
    ScenarioWinnerType gScenarioWinner;

    Point cursor_coord;
    Point old_cursor_coord;

    int32_t         gRadarCount;            // = 0;
    int32_t         gRadarSpeed;            // = 30;
    int32_t         gRadarRange;            // kRadarSize * 50;
    int32_t         gWhichScaleNum;         // = 0;
    int32_t         gLastScale;             // = SCALE_SCALE;
    int32_t         gInstrumentTop;         // = 0;
    barIndicatorType    gBarIndicator[ kBarIndicatorNum];
    bool            gMouseActive;           // = kMouseOff;
    int             gMouseTimeout;
    std::queue<sfz::linked_ptr<sfz::String> > gMessageData;
    sfz::scoped_array<unsigned char>     gStatusString;
    sfz::scoped_ptr<longMessageType>     gLongMessageData;
    long            gMessageTimeCount;      // = 0;
    long            gMessageLabelNum;       // = -1;
    long            gStatusLabelNum;        // = -1;
    miniComputerDataType    gMiniScreenData;
    sfz::scoped_ptr<StringList>          gMissionStatusStrList;
    sfz::scoped_array<screenLabelType>   gScreenLabelData;
    sfz::scoped_array<beamType>          gBeamData;
    smartSoundHandle    gSound[kSoundNum];
    smartSoundChannel   gChannel[kMaxChannelNum];
    long            gLastSoundTime;         // = 0
    sfz::scoped_ptr<StringList>        gAresCheatStrings;
    KeyMap*         gKeyMapBuffer;          // = NewPtr( sizeof (KeyMap) * (long)kKeyMapBufferNum;
    long            gKeyMapBufferTop;       // = 0;
    long            gKeyMapBufferBottom;    // = 0;
    KeyMap          gLastMessageKeyMap;
    unsigned long   gSerialNumerator;
    unsigned long   gSerialDenominator;
    long            gLastSelectedBuildPrice;
    bool         gAutoPilotOff;          // hack for turning off auto in netgame
    long            levelNum;
    unsigned long   keyMask;
    scenarioInfoType    scenarioFileInfo;   // x-ares; for factory +
                                            // 3rd party files
    long            maxScenarioBrief;
    long            maxScenarioCondition;
    long            maxScenarioInitial;
    long            maxBaseObject;
    long            maxObjectAction;
    long            scenarioNum;

    hotKeyType      hotKey[kHotKeyNum];
    long            hotKeyDownTime;
    long            lastHotKey;

    long            lastSelectedObject;
    long            lastSelectedObjectID;
    bool         destKeyUsedForSelection;
    bool         hotKey_target;

    Starfield starfield;
    Transitions transitions;
};

aresGlobalType* globals();
void init_globals();

extern Rect world;
extern Rect play_screen;
extern Rect viewport;

}  // namespace antares

#endif // ANTARES_GAME_GLOBALS_HPP_
