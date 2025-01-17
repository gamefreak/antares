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

#include "game/minicomputer.hpp"

#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/sprite-handling.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/instruments.hpp"
#include "game/messages.hpp"
#include "game/player-ship.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "math/fixed.hpp"
#include "sound/fx.hpp"

using sfz::Bytes;
using sfz::Rune;
using sfz::String;
using sfz::StringSlice;
using sfz::scoped_array;
using sfz::string_to_int;

namespace macroman = sfz::macroman;

namespace antares {

miniComputerDataType    *gMiniScreenData = NULL;

namespace {

const int32_t kMiniScreenCharWidth = 25;

const int32_t kMiniScreenLeft       = 12;
const int32_t kMiniScreenTop        = 320;
const int32_t kMiniScreenRight      = 121;
const int32_t kMiniScreenBottom     = 440;
const int32_t kMiniScreenWidth      = kMiniScreenRight - kMiniScreenLeft - 1;

const int32_t kMiniScreenLeftBuffer = 3;

const int32_t kMiniScreenCharHeight     = 10;  // height of the screen in characters
const int32_t kMiniScreenTrueLineNum    = kMiniScreenCharHeight + 2;

const int32_t kButBoxLeft           = 16;
const int32_t kButBoxTop            = 450;
const int32_t kButBoxRight          = 114;
const int32_t kButBoxBottom         = 475;

const int32_t kMiniScreenNoLineSelected = -1;

const int16_t kMiniScreenStringID   = 3000;
const int16_t kMiniDataStringID     = 3001;

const uint8_t kMiniScreenColor      = GREEN;
const uint8_t kMiniButColor         = AQUA;

const Rune kMiniScreenSpecChar      = '\\';
const Rune kEndLineChar             = 'x';
const Rune kUnderlineEndLineChar    = 'u';
const Rune kIntoButtonChar          = 'I';
const Rune kOutOfButtonChar         = 'O';
const Rune kSelectableLineChar      = 'S';

const int32_t kNoLineButton         = -1;
const int32_t kInLineButton         = kCompAcceptKeyNum;
const int32_t kOutLineButton        = kCompCancelKeyNum;

enum {
    kMainMiniScreen     = 1,
    kBuildMiniScreen    = 2,
    kSpecialMiniScreen  = 3,
    kMessageMiniScreen  = 4,
    kStatusMiniScreen   = 5,
};

enum {
    kMainMiniBuild      = 1,
    kMainMiniSpecial    = 2,
    kMainMiniMessage    = 3,
    kMainMiniStatus     = 4,
};

const int32_t kBuildScreenFirstTypeLine = 2;
const int32_t kBuildScreenWhereNameLine = 1;

enum {
    kSpecialMiniTransfer        = 1,
    kSpecialMiniHold            = 2,
    kSpecialMiniGoToMe          = 3,
    kSpecialMiniFire1           = 4,
    kSpecialMiniFire2           = 5,
    kSpecialMiniFireSpecial     = 6,
};

enum {
    kMessageMiniNext        = 1,
    kMessageMiniPrevious    = 2,
    kMessageMiniLast        = 3,
};

const int32_t kStatusMiniScreenFirstLine = 1;

enum {
    kNoStatusData           = -1,  // no status for this line
    kPlainTextStatus        = 0,
    kTrueFalseCondition     = 1,   // 0 = F, 1 = T, use condition not score
    kIntegerValue           = 2,   // interpret score as int
    kSmallFixedValue        = 3,   // interpret score as fixed
    kIntegerMinusValue      = 4,   // value - designated score
    kSmallFixedMinusValue   = 5,   // small fixed - designated score
    kMaxStatusTypeValue     = kSmallFixedMinusValue,
};

const int32_t kMiniComputerPollTime = 60;

const int32_t kMiniObjectDataNum    = 2;
const int32_t kMiniSelectObjectNum  = 0;
const int32_t kMiniSelectTop        = 180;

const int32_t kMiniIconHeight       = 22;
const int32_t kMiniIconWidth        = 24;
const int32_t kMiniIconLeft         = (kMiniScreenLeft + 2);

const int32_t kMiniHealthLeft       = (kMiniIconLeft + kMiniIconWidth + 2);
const int32_t kMiniBarWidth         = 11;
const int32_t kMiniBarHeight        = 18;

const int32_t kMiniEnergyLeft       = (kMiniHealthLeft + kMiniBarWidth + 2);

const int32_t kMiniRightColumnLeft  = 57;
const int32_t kMiniWeapon1LineNum   = 2;
const int32_t kMiniWeapon2LineNum   = 3;
const int32_t kMiniWeapon3LineNum   = 1;
const int32_t kMiniNameLineNum      = 1;

const int32_t kMiniDestLineNum      = 4;

const int32_t kMiniTargetObjectNum  = 1;
const int32_t kMiniTargetTop        = 252;

const int32_t kMiniAmmoTop          = 161;
const int32_t kMiniAmmoBottom       = 170;
const int32_t kMiniAmmoSingleWidth  = 21;
const int32_t kMiniAmmoLeftOne      = 27;
const int32_t kMiniAmmoLeftTwo      = 64;
const int32_t kMiniAmmoLeftSpecial  = 100;
const int32_t kMiniAmmoTextHBuffer  = 2;

inline void mPlayBeep3() {
    PlayVolumeSound(kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

inline void mPlayBeepBad() {
    PlayVolumeSound(kWarningTone, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

const int32_t kMaxShipBuffer = 40;

void pad_to(String& s, size_t width) {
    if (s.size() < width) {
        String result;
        result.append((width - s.size()) / 2, ' ');
        result.append(s);
        result.append((1 + width - s.size()) / 2, ' ');
        swap(result, s);
    }
}

const int32_t MiniIconMacLineTop() {
    return (gDirectText->height * 2);
}

inline void mBlackMiniScreenLine(
        long mtop, long mlinenum, long mleft, long mright, Rect& mbounds, PixMap* mpixbase) {
    mbounds.left = kMiniScreenLeft + mleft;
    mbounds.top = mtop + mlinenum * mDirectFontHeight();
    mbounds.right = kMiniScreenLeft + mright;
    mbounds.bottom = mbounds.top + mDirectFontHeight();
    DrawNateRect(mpixbase, &mbounds, RgbColor::kBlack);
}

inline long mGetLineNumFromV(long mV) {
    return (((mV) - (kMiniScreenTop + globals()->gInstrumentTop)) / mDirectFontHeight());
}

// for copying the fields of a space object relevant to the miniscreens:
inline void mCopyMiniSpaceObject(
        spaceObjectType& mdestobject, const spaceObjectType& msourceobject) {
    (mdestobject).id = (msourceobject).id;
    (mdestobject).beamType = (msourceobject).beamType;
    (mdestobject).pulseType = (msourceobject).pulseType;
    (mdestobject).specialType = (msourceobject).specialType;
    (mdestobject).destinationLocation.h = (msourceobject).destinationLocation.h;
    (mdestobject).destinationLocation.v = (msourceobject).destinationLocation.v;
    (mdestobject).destinationObject = (msourceobject).destinationObject;
    (mdestobject).destObjectPtr = (msourceobject).destObjectPtr;
    (mdestobject).health = (msourceobject).health;
    (mdestobject).energy = (msourceobject).energy;
    (mdestobject).whichBaseObject = (msourceobject).whichBaseObject;
    (mdestobject).pixResID = (msourceobject).pixResID;
    (mdestobject).attributes = (msourceobject).attributes;
    (mdestobject).location = (msourceobject).location;
    (mdestobject).owner = (msourceobject).owner;
    (mdestobject).nextFarObject = (msourceobject).nextFarObject;
    (mdestobject).distanceGrid = (msourceobject).distanceGrid;
    (mdestobject).nextNearObject = (msourceobject).nextNearObject;
    (mdestobject).collisionGrid = (msourceobject).collisionGrid;
    (mdestobject).remoteFriendStrength = (msourceobject).remoteFriendStrength;
    (mdestobject).remoteFoeStrength = (msourceobject).remoteFoeStrength;
    (mdestobject).escortStrength = (msourceobject).escortStrength;
    (mdestobject).baseType = (msourceobject).baseType;
}

inline void mCopyBlankLineString(miniScreenLineType* mline, StringSlice mstring) {
    mline->string.assign(mstring);
    if (mline->string.size() > kMiniScreenCharWidth) {
        mline->string.resize(kMiniScreenCharWidth);
    }
}

inline spaceObjectType* mGetMiniObjectPtr(long mwhich) {
    return globals()->gMiniScreenData.objectData.get() + mwhich;
}

}  // namespace

void MiniComputerSetStatusStrings( void);
long MiniComputerGetStatusValue( long);
void MiniComputerMakeStatusString(int32_t which_line, String& string);

void MiniScreenInit() {
    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;
    globals()->gMiniScreenData.currentScreen = kMainMiniScreen;
    globals()->gMiniScreenData.pollTime = 0;
    globals()->gMiniScreenData.buildTimeBarValue = 0;
    globals()->gMiniScreenData.clickLine = kMiniScreenNoLineSelected;

    globals()->gMiniScreenData.lineData.reset(new miniScreenLineType[kMiniScreenTrueLineNum]);
    globals()->gMiniScreenData.objectData.reset(new spaceObjectType[kMiniObjectDataNum]);

    ClearMiniScreenLines();
    ClearMiniObjectData();
}

void MiniScreenCleanup() {
    globals()->gMiniScreenData.lineData.reset();
    globals()->gMiniScreenData.objectData.reset();
}

#pragma mark -

void SetMiniScreenStatusStrList(short strID) {
    DisposeMiniScreenStatusStrList();
    if (strID > 0) {
        globals()->gMissionStatusStrList.reset(new StringList(strID));
    }
}

void DisposeMiniScreenStatusStrList( void) {
    globals()->gMissionStatusStrList.reset();
}

void ClearMiniScreenLines() {
    miniScreenLineType* c = globals()->gMiniScreenData.lineData.get();
    for (int32_t b = 0; b < kMiniScreenTrueLineNum; b++) {
        c->string.clear();
        c->hiliteLeft = c->hiliteRight = 0;
        c->whichButton = kNoLineButton;
        c->selectable = cannotSelect;
        c->underline = false;
        c->lineKind = plainLineKind;
        c->sourceData = NULL;
        c++;
    }
}

void ClearMiniObjectData( void)

{
    spaceObjectType *o;

    o = mGetMiniObjectPtr( kMiniSelectObjectNum);
    o->id = -1;
    o->beamType = -1;
    o->pulseType = -1;
    o->specialType = -1;
    o->destinationLocation.h = o->destinationLocation.v = -1;
    o->destinationObject = -1;
    o->destObjectPtr = NULL;
    o->health = 0;
    o->energy = 0;
    o->whichBaseObject = -1;
    o->pixResID = -1;
    o->attributes = 0;
    o->baseType = NULL;

    o = mGetMiniObjectPtr( kMiniTargetObjectNum);
    o->id = -1;
    o->beamType = -1;
    o->pulseType = -1;
    o->specialType = -1;
    o->destinationLocation.h = o->destinationLocation.v = -1;
    o->destinationObject = -1;
    o->destObjectPtr = NULL;
    o->health = 0;
    o->energy = 0;
    o->whichBaseObject = -1;
    o->pixResID = -1;
    o->attributes = 0;
    o->baseType = NULL;

    globals()->gMiniScreenData.buildTimeBarValue = 0;
    globals()->gMiniScreenData.pollTime = 0;
}

void DrawMiniScreen( void)

{
    Rect                mRect;
    Rect            lRect, cRect;
    miniScreenLineType  *c;
    RgbColor            color, lightcolor, darkcolor, textcolor;
    unsigned char lineColor = kMiniScreenColor;
    long                count, lineCorrect = 0;

    mSetDirectFont( kComputerFontNum);

    lRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);
    color = GetRGBTranslateColorShade(kMiniScreenColor, DARKEST);
    cRect = lRect;
    DrawNateRect(gOffWorld, &cRect, color);

    mRect.left = kMiniScreenLeft;
    mRect.top = kMiniScreenTop + globals()->gInstrumentTop;
    mRect.right = kMiniScreenRight;
    mRect.bottom = kMiniScreenBottom + globals()->gInstrumentTop;

    c = globals()->gMiniScreenData.lineData.get();

    for ( count = 0; count < kMiniScreenTrueLineNum; count++)
    {
        if ( count == kMiniScreenCharHeight)
        {
            lRect.left = mRect.left = kButBoxLeft;
            lRect.top = mRect.top = kButBoxTop + globals()->gInstrumentTop;
            lRect.right = mRect.right = kButBoxRight;
            lRect.bottom = mRect.bottom = kButBoxBottom + globals()->gInstrumentTop;
            color = GetRGBTranslateColorShade(kMiniScreenColor, DARKEST);
            cRect = lRect;
            DrawNateRect(gOffWorld, &cRect, color);
            lineCorrect = -kMiniScreenCharHeight;
            lineColor = kMiniButColor;
        }

        if ( c->underline)
        {
            MoveTo( mRect.left, mRect.top + (count + lineCorrect) * ((
                gDirectText->height)/* * 2*/) + gDirectText->ascent/* * 2*/);
            const RgbColor color = GetRGBTranslateColorShade(lineColor, MEDIUM);
            MacLineTo(gOffWorld, mRect.right - 1, mRect.top + (count + lineCorrect) * ((
                gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */, color);
        }

        if ( c->hiliteLeft < c->hiliteRight)
        {
            if ( c->selectable == selectDim)
                textcolor = GetRGBTranslateColorShade(lineColor, VERY_DARK);
            else
                textcolor = GetRGBTranslateColorShade(lineColor, VERY_LIGHT);
            switch( c->lineKind)
            {
                case plainLineKind:
                    if ( c->hiliteRight > c->hiliteLeft)
                    {
                        cRect.left = c->hiliteLeft;
                        cRect.top = mRect.top + (( count + lineCorrect) * ( gDirectText->height /* * 2 */));
                        cRect.right = c->hiliteRight;
                        cRect.bottom = cRect.top + gDirectText->height /* * 2 */;
//                      color = GetTranslateColorShade( lineColor, DARK);
                        color = GetRGBTranslateColorShade(lineColor, DARK);
                        lightcolor = GetRGBTranslateColorShade(lineColor, MEDIUM);
                        darkcolor = GetRGBTranslateColorShade(lineColor, DARKER);
                        DrawNateShadedRect(gOffWorld, &cRect, lRect, color, lightcolor, darkcolor);
                    }
                    break;

                case buttonOffLineKind:
                    cRect.left = c->hiliteLeft - 2;
                    cRect.top = lRect.top + (( count + lineCorrect) * ( gDirectText->height /* * 2 */));
                    cRect.right = c->hiliteRight + 2;
                    cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                    color = GetRGBTranslateColorShade(lineColor, MEDIUM);
                    lightcolor = GetRGBTranslateColorShade(lineColor, LIGHT);
                    darkcolor = GetRGBTranslateColorShade(lineColor, DARK);
                    DrawNateShadedRect(gOffWorld, &cRect, lRect, color, lightcolor, darkcolor);
                    break;

                case buttonOnLineKind:
                    cRect.left = c->hiliteLeft - 2;
                    cRect.top = lRect.top + (( count + lineCorrect) * ( gDirectText->height /* * 2 */));
                    cRect.right = lRect.right; //c->hiliteRight + 2;
                    cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                    color = GetRGBTranslateColorShade(lineColor, LIGHT);
                    lightcolor = GetRGBTranslateColorShade(lineColor, VERY_LIGHT);
                    darkcolor = GetRGBTranslateColorShade(lineColor, MEDIUM);
                    DrawNateShadedRect(gOffWorld, &cRect, lRect, color, lightcolor, darkcolor);
                    textcolor = RgbColor::kBlack;
                    break;

            }
        } else
        {
            if ( c->selectable == selectDim)
                textcolor = GetRGBTranslateColorShade(lineColor, MEDIUM);
            else
                textcolor = GetRGBTranslateColorShade(lineColor, VERY_LIGHT);
        }
        DrawDirectTextStringClipped(
                Point(
                    mRect.left + kMiniScreenLeftBuffer,
                    mRect.top + (count + lineCorrect) * gDirectText->height + gDirectText->ascent),
                c->string, textcolor, gOffWorld, lRect);
        c++;
    }
}

void DrawAndShowMiniScreenLine( long whichLine)

{
    Rect                tRect;
    Rect            lRect, cRect;
    miniScreenLineType  *c;
    RgbColor            color, lightcolor, darkcolor, textcolor;
    unsigned char lineColor = kMiniScreenColor;
    long                lineCorrect = 0;

    if ( whichLine < 0) return;

    mSetDirectFont( kComputerFontNum);

    if ( whichLine < kMiniScreenCharHeight)
    {
        lRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                    kMiniScreenBottom + globals()->gInstrumentTop);
        cRect = lRect;
        cRect.top = lRect.top + whichLine * gDirectText->height;
        cRect.bottom = cRect.top + gDirectText->height;
    } else
    {
        lRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                    kButBoxBottom + globals()->gInstrumentTop);
        lineCorrect = -kMiniScreenCharHeight;
        lineColor = kMiniButColor;
        cRect = lRect;
        cRect.top = lRect.top + (whichLine - kMiniScreenCharHeight) * gDirectText->height;
        cRect.bottom = cRect.top + gDirectText->height;
    }

    color = GetRGBTranslateColorShade(lineColor, DARKEST);
    DrawNateRect(gOffWorld, &cRect, color);

    c = globals()->gMiniScreenData.lineData.get() + whichLine;

    if ( c->underline)
    {
        MoveTo( lRect.left, lRect.top + (whichLine + lineCorrect) * ((
            gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */);
        const RgbColor color = GetRGBTranslateColorShade( lineColor, MEDIUM);
        MacLineTo(gOffWorld, lRect.right - 1, lRect.top + (whichLine + lineCorrect) * ((
            gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */, color);
    }

    if ( c->hiliteLeft < c->hiliteRight)
    {
        if ( c->selectable == selectDim)
            textcolor = GetRGBTranslateColorShade(lineColor, VERY_DARK);
        else
            textcolor = GetRGBTranslateColorShade(lineColor, VERY_LIGHT);
        switch( c->lineKind)
        {
            case plainLineKind:
                    if ( c->hiliteRight > c->hiliteLeft)
                    {
                        cRect.left = c->hiliteLeft;
                        cRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
                        cRect.right = c->hiliteRight;
                        cRect.bottom = cRect.top + gDirectText->height /* * 2 */;
//                      color = GetTranslateColorShade( lineColor, DARK);
                        color = GetRGBTranslateColorShade(lineColor, DARK);
                        lightcolor = GetRGBTranslateColorShade(lineColor, MEDIUM);
                        darkcolor = GetRGBTranslateColorShade(lineColor, DARKER);
                        DrawNateShadedRect(gOffWorld, &cRect, lRect, color, lightcolor, darkcolor);
                    }
                break;

            case buttonOffLineKind:
                cRect.left = c->hiliteLeft - 2;
                cRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
                cRect.right = c->hiliteRight + 2;
                cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                color = GetRGBTranslateColorShade(lineColor, MEDIUM);
                lightcolor = GetRGBTranslateColorShade(lineColor, LIGHT);
                darkcolor = GetRGBTranslateColorShade(lineColor, DARK);
                DrawNateShadedRect(gOffWorld, &cRect, lRect, color, lightcolor, darkcolor);
                break;

            case buttonOnLineKind:
                cRect.left = c->hiliteLeft - 2;
                cRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
                cRect.right = lRect.right; //c->hiliteRight + 2;
                cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                color = GetRGBTranslateColorShade(lineColor, LIGHT);
                lightcolor = GetRGBTranslateColorShade(lineColor, VERY_LIGHT);
                darkcolor = GetRGBTranslateColorShade(lineColor, MEDIUM);
                DrawNateShadedRect(gOffWorld, &cRect, lRect, color, lightcolor, darkcolor);
                textcolor = RgbColor::kBlack;
                break;
        }
    } else
    {
        if ( c->selectable == selectDim)
            textcolor = GetRGBTranslateColorShade(lineColor, MEDIUM);
        else
            textcolor = GetRGBTranslateColorShade(lineColor, VERY_LIGHT);
    }


    DrawDirectTextStringClipped(
            Point(
                lRect.left + kMiniScreenLeftBuffer,
                lRect.top + (whichLine + lineCorrect) * gDirectText->height + gDirectText->ascent),
            c->string, textcolor, gOffWorld, lRect);

    tRect.left = lRect.left;
    tRect.right = kMiniScreenRight;
    tRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
    tRect.bottom = tRect.top + gDirectText->height/* * 2 */;
    copy_world(*gRealWorld, *gOffWorld, tRect);
}

void ShowWholeMiniScreen( void)

{
    Rect                tRect;

    tRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);
    copy_world(*gRealWorld, *gOffWorld, tRect);
    tRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);
    copy_world(*gRealWorld, *gOffWorld, tRect);
}

void MakeMiniScreenFromIndString(short whichString) {
    mSetDirectFont(kComputerFontNum);

    Rect mRect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    mRect.offset(0, globals()->gInstrumentTop);

    ClearMiniScreenLines();
    globals()->gMiniScreenData.currentScreen = whichString;
    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;

    StringList string_list(kMiniScreenStringID);
    StringSlice string = string_list.at(whichString - 1);

    miniScreenLineType* const line_begin = globals()->gMiniScreenData.lineData.get();
    miniScreenLineType* const line_switch = line_begin + kMiniScreenCharHeight;
    miniScreenLineType* const line_end = line_begin + kMiniScreenTrueLineNum;
    miniScreenLineType* line = line_begin;

    bool escape = false;
    SFZ_FOREACH(Rune r, string, {
        if (escape) {
            escape = false;
            switch (r) {
              case kUnderlineEndLineChar:
                line->underline = true;
                // fall through.
              case kEndLineChar:
                ++line;
                if (line == line_end) {
                    return;
                } else if (line == line_switch) {
                    mRect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
                    mRect.offset(0, globals()->gInstrumentTop);
                }
                break;

              case kSelectableLineChar:
                line->selectable = selectable;
                if (globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected) {
                    globals()->gMiniScreenData.selectLine = line - line_begin;
                    line->hiliteLeft = mRect.left;
                    line->hiliteRight = mRect.right;
                }
                break;

              case kIntoButtonChar:
                {
                    line->lineKind = buttonOffLineKind;
                    line->whichButton = kInLineButton;
                    line->hiliteLeft
                        = mRect.left + kMiniScreenLeftBuffer
                        + gDirectText->logicalWidth * line->string.size();

                    sfz::String key_name;
                    GetKeyNumName(Preferences::preferences()->key(kCompAcceptKeyNum), &key_name);
                    pad_to(key_name, kKeyNameLength);
                    line->string.append(key_name);

                    line->hiliteRight
                        = mRect.left + kMiniScreenLeftBuffer
                        + gDirectText->logicalWidth * line->string.size() - 1;
                }
                break;

              case kOutOfButtonChar:
                {
                    line->lineKind = buttonOffLineKind;
                    line->whichButton = kOutLineButton;
                    line->hiliteLeft
                        = mRect.left + kMiniScreenLeftBuffer
                        + gDirectText->logicalWidth * line->string.size();

                    sfz::String key_name;
                    GetKeyNumName(
                            Preferences::preferences()->key(kCompCancelKeyNum), &key_name);
                    pad_to(key_name, kKeyNameLength);
                    line->string.append(key_name);

                    line->hiliteRight
                        = mRect.left + kMiniScreenLeftBuffer
                        + gDirectText->logicalWidth * line->string.size() - 1;
                }
                break;

              case kMiniScreenSpecChar:
                line->string.append(1, kMiniScreenSpecChar);
                break;
            }
        } else if (r == kMiniScreenSpecChar) {
            escape = true;
        } else {
            line->string.append(1, r);
        }

        while (line->string.size() > kMiniScreenCharWidth) {
            String excess(line->string.slice(kMiniScreenCharWidth));
            line->string.resize(kMiniScreenCharWidth);

            ++line;
            if (line == line_end) {
                return;
            } else if (line == line_switch) {
                mRect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
                mRect.offset(0, globals()->gInstrumentTop);
            }
            line->string.assign(excess);
        }
    });
}

void MiniComputerHandleKeys( unsigned long theseKeys, unsigned long lastKeys)

{
    miniScreenLineType  *line;
    long                count, scrap;
    Rect                mRect;

    if (( theseKeys | lastKeys) & kCompAcceptKey)
    {
        // find out which line, if any, contains this button
        line = globals()->gMiniScreenData.lineData.get();
        count = 0;
        while (( line->whichButton !=kInLineButton) && ( count < kMiniScreenTrueLineNum))
        {
            count++;
            line++;
        }

        // hilite/unhilite this button
        if ( count < kMiniScreenTrueLineNum)
        {
            if (( theseKeys & kCompAcceptKey) && ( line->lineKind != buttonOnLineKind))
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( count);
                mPlayBeep3();
            } else if ((!( theseKeys & kCompAcceptKey)) && ( line->lineKind != buttonOffLineKind))
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( count);

                MiniComputerDoAccept();
            }
        }
    }

    if (( theseKeys | lastKeys) & kCompCancelKey)
    {
        // find out which line, if any, contains this button
        line = globals()->gMiniScreenData.lineData.get();
        count = 0;
        while (( line->whichButton !=kOutLineButton) && ( count < kMiniScreenTrueLineNum))
        {
            count++;
            line++;
        }

        if ( count < kMiniScreenCharHeight)
        {
            mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
        } else
        {
            mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                        kButBoxBottom + globals()->gInstrumentTop);
        }

        // hilite/unhilite this button
        if ( count < kMiniScreenTrueLineNum)
        {
            if (( theseKeys & kCompCancelKey) && ( line->lineKind != buttonOnLineKind))
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( count);
                mPlayBeep3();
            } else if ((!( theseKeys & kCompCancelKey)) && ( line->lineKind != buttonOffLineKind))
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( count);

                MiniComputerDoCancel();
            }
        }
    }
    if (( theseKeys & kCompUpKey) && ( !(lastKeys & kCompUpKey)) && ( globals()->gMiniScreenData.selectLine !=
            kMiniScreenNoLineSelected))
    {
        scrap = globals()->gMiniScreenData.selectLine;
        line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
        line->hiliteLeft = line->hiliteRight = 0;
        do
        {
            line--;
            globals()->gMiniScreenData.selectLine--;
            if ( globals()->gMiniScreenData.selectLine < 0)
            {
                globals()->gMiniScreenData.selectLine = kMiniScreenTrueLineNum - 1;
                line = globals()->gMiniScreenData.lineData.get() + kMiniScreenTrueLineNum - 1L;
            }
        } while ( line->selectable == cannotSelect);

        if ( globals()->gMiniScreenData.selectLine < kMiniScreenCharHeight)
        {
            mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
        } else
        {
            mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                        kButBoxBottom + globals()->gInstrumentTop);
        }

        line->hiliteLeft = mRect.left;
        line->hiliteRight = mRect.right;
        if ( scrap != globals()->gMiniScreenData.selectLine)
        {
            DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            DrawAndShowMiniScreenLine( scrap);
        }
    }

    if (( theseKeys & kCompDownKey) && ( !(lastKeys & kCompDownKey)) && ( globals()->gMiniScreenData.selectLine !=
            kMiniScreenNoLineSelected))
    {
        scrap = globals()->gMiniScreenData.selectLine;
        line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
        line->hiliteLeft = line->hiliteRight = 0;
        do
        {
            line++;
            globals()->gMiniScreenData.selectLine++;
            if ( globals()->gMiniScreenData.selectLine >= kMiniScreenTrueLineNum)
            {
                globals()->gMiniScreenData.selectLine = 0;
                line = globals()->gMiniScreenData.lineData.get();
            }
        } while ( line->selectable == cannotSelect);

        if ( globals()->gMiniScreenData.selectLine < kMiniScreenCharHeight)
        {
            mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
        } else
        {
            mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                        kButBoxBottom + globals()->gInstrumentTop);
        }

        line->hiliteLeft = mRect.left;
        line->hiliteRight = mRect.right;
        if ( scrap != globals()->gMiniScreenData.selectLine)
        {
            DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            DrawAndShowMiniScreenLine( scrap);
        }
    }


}

void MiniComputerHandleNull( long unitsToDo)

{
    destBalanceType     *buildAtObject = NULL;
    long                count;
    spaceObjectType     *realObject = NULL, *myObject = NULL, newObject;

    globals()->gMiniScreenData.pollTime += unitsToDo;
    if ( globals()->gMiniScreenData.pollTime > kMiniComputerPollTime)
    {
        globals()->gMiniScreenData.pollTime = 0;
        UpdateMiniScreenLines();

        // handle control/command/selected object

        myObject = mGetMiniObjectPtr( kMiniSelectObjectNum);
        count = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
        if ( count >= 0)
        {
            realObject = gSpaceObjectData.get() + count;
            mCopyMiniSpaceObject( newObject, *realObject);
        } else
        {
            newObject.id = -1;
            newObject.beamType = -1;
            newObject.pulseType = -1;
            newObject.specialType = -1;
            newObject.destinationLocation.h = newObject.destinationLocation.v = -1;
            newObject.destinationObject = -1;
            newObject.destObjectPtr = NULL;
            newObject.health = 0;
            newObject.energy = 0;
            newObject.whichBaseObject = -1;
            newObject.pixResID = -1;
            newObject.attributes = 0;
            newObject.baseType = NULL;
        }
        UpdateMiniShipData( myObject, &newObject, YELLOW, kMiniSelectTop, kMiniSelectObjectNum + 1);

        myObject = mGetMiniObjectPtr( kMiniTargetObjectNum);
        count = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
        if ( count >= 0)
        {
            realObject = gSpaceObjectData.get() + count;
            mCopyMiniSpaceObject( newObject, *realObject);
        } else
        {
            newObject.id = -1;
            newObject.beamType = -1;
            newObject.pulseType = -1;
            newObject.specialType = -1;
            newObject.destinationLocation.h = newObject.destinationLocation.v = -1;
            newObject.destinationObject = -1;
            newObject.destObjectPtr = NULL;
            newObject.health = 0;
            newObject.energy = 0;
            newObject.whichBaseObject = -1;
            newObject.pixResID = -1;
            newObject.attributes = 0;
            newObject.baseType = NULL;
        }
        UpdateMiniShipData( myObject, &newObject, SKY_BLUE, kMiniTargetTop, kMiniTargetObjectNum + 1);

        count = GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber);
        if ( count >= 0)
        {
            buildAtObject = mGetDestObjectBalancePtr( GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber));
            count = buildAtObject->buildTime * kMiniBuildTimeHeight;
            if ( buildAtObject->totalBuildTime > 0)
            {
                count /= buildAtObject->totalBuildTime;
            } else count = 0;
        } else count = 0;
        if ( count != globals()->gMiniScreenData.buildTimeBarValue)
        {
            globals()->gMiniScreenData.buildTimeBarValue = count;
            DrawBuildTimeBar( globals()->gMiniScreenData.buildTimeBarValue);
        }
    }
    if ( globals()->gPlayerShipNumber >= 0)
    {
        myObject = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
        if ( myObject->active)
        {
            UpdatePlayerAmmo(
                (myObject->pulseType >= 0) ?
                    (( myObject->pulseBase->frame.weapon.ammo > 0) ?
                        ( myObject->pulseAmmo):(-1)):
                    (-1),
                (myObject->beamType >= 0) ?
                    (( myObject->beamBase->frame.weapon.ammo > 0) ?
                        ( myObject->beamAmmo):(-1)):
                    (-1),
                (myObject->specialType >= 0) ?
                    (( myObject->specialBase->frame.weapon.ammo > 0) ?
                        ( myObject->specialAmmo):(-1)):
                    (-1)
                );
        }
    }
}


// only for updating volitile lines--doesn't draw whole screen!
void UpdateMiniScreenLines( void)

{
    admiralType         *admiral = NULL;
    miniScreenLineType  *line = NULL;
    baseObjectType      *buildObject = NULL;
    long                lineNum, count;
    Rect                mRect;

    mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
    switch( globals()->gMiniScreenData.currentScreen)
    {
        case kBuildMiniScreen:
            admiral = globals()->gAdmiralData.get() + globals()->gPlayerAdmiralNumber;
            line = globals()->gMiniScreenData.lineData.get() +
                kBuildScreenWhereNameLine;
            if ( line->value !=
                GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber))
            {
                if ( globals()->gMiniScreenData.selectLine !=
                        kMiniScreenNoLineSelected)
                {
                    line = globals()->gMiniScreenData.lineData.get()
                        + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = line->hiliteRight = 0;
                    globals()->gMiniScreenData.selectLine =
                        kMiniScreenNoLineSelected;
                }
                MiniComputerSetBuildStrings();
                DrawMiniScreen();
                ShowWholeMiniScreen();
            } else if ( GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber)
                >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + kBuildScreenFirstTypeLine;
                lineNum = kBuildScreenFirstTypeLine;

                for ( count = 0; count < kMaxShipCanBuild; count++)
                {
                    buildObject = line->sourceData;
                    if ( buildObject != NULL)
                    {
                        if ( buildObject->price > mFixedToLong(admiral->cash))
                        {
                            if ( line->selectable != selectDim)
                            {
                                line->selectable = selectDim;
                                DrawAndShowMiniScreenLine( lineNum);
                            }
                        } else
                        {
                            if (line->selectable != selectable)
                            {
                                if ( globals()->gMiniScreenData.selectLine ==
                                    kMiniScreenNoLineSelected)
                                {
                                    globals()->gMiniScreenData.selectLine =
                                        lineNum;
                                    line->hiliteLeft = mRect.left;
                                    line->hiliteRight = mRect.right;
                                }
                                line->selectable = selectable;
                                DrawAndShowMiniScreenLine( lineNum);
                            }
                        }
                    }
                    line++;
                    lineNum++;
                }
            }

            break;

        case kStatusMiniScreen:
            for ( count = kStatusMiniScreenFirstLine; count <
                kMiniScreenCharHeight; count++)
            {
                line =
                    globals()->gMiniScreenData.lineData.get() +
                        count;
                lineNum = MiniComputerGetStatusValue( count);
                if ( line->value != lineNum)
                {
                    line->value = lineNum;
                    MiniComputerMakeStatusString(count, line->string);
                    DrawAndShowMiniScreenLine( count);
                }

            }
            break;
    }
}

void UpdatePlayerAmmo( long thisOne, long thisTwo, long thisSpecial)

{
    static long         lastOne = -1, lastTwo = -1, lastSpecial = -1;
    RgbColor            lightcolor;
    Rect            lRect, clipRect;
    Rect                mRect;
    bool             update = false;

    mSetDirectFont( kComputerFontNum);

    clipRect.left = kMiniScreenLeft;
    lRect.top = clipRect.top = kMiniAmmoTop + globals()->gInstrumentTop;
    clipRect.right = kMiniScreenRight;
    lRect.bottom = clipRect.bottom = kMiniAmmoBottom + globals()->gInstrumentTop;

    if ( thisOne != lastOne)
    {
        lightcolor = GetRGBTranslateColorShade(RED, VERY_LIGHT);

        lRect.left = kMiniAmmoLeftOne;
        lRect.right = lRect.left + kMiniAmmoSingleWidth;

        DrawNateRect(gOffWorld, &lRect, RgbColor::kBlack);

        if (thisOne >= 0) {
            const char digits[] = {
                '0' + ((thisOne % 1000) / 100),
                '0' + ((thisOne % 100) / 10),
                '0' + ((thisOne % 10) / 1),
                '\0'
            };

            DrawDirectTextStringClipped(
                    Point(lRect.left + kMiniAmmoTextHBuffer, lRect.bottom - 1), digits, lightcolor,
                    gOffWorld, clipRect);
        }

        update = true;
    }

    if ( thisTwo != lastTwo)
    {
        lightcolor = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);

        lRect.left = kMiniAmmoLeftTwo;
        lRect.right = lRect.left + kMiniAmmoSingleWidth;

        DrawNateRect(gOffWorld, &lRect, RgbColor::kBlack);

        if (thisTwo >= 0) {
            const char digits[] = {
                '0' + ((thisTwo % 1000) / 100),
                '0' + ((thisTwo % 100) / 10),
                '0' + ((thisTwo % 10) / 1),
                '\0'
            };

            DrawDirectTextStringClipped(
                    Point(lRect.left + kMiniAmmoTextHBuffer, lRect.bottom - 1), digits, lightcolor,
                    gOffWorld, clipRect);
        }
        update = true;
    }

    if ( thisSpecial != lastSpecial)
    {
        lightcolor = GetRGBTranslateColorShade(ORANGE, VERY_LIGHT);

        lRect.left = kMiniAmmoLeftSpecial;
        lRect.right = lRect.left + kMiniAmmoSingleWidth;

        DrawNateRect(gOffWorld, &lRect, RgbColor::kBlack);

        if (thisSpecial >= 0) {
            const char digits[] = {
                '0' + ((thisSpecial % 1000) / 100),
                '0' + ((thisSpecial % 100) / 10),
                '0' + ((thisSpecial % 10) / 1),
                '\0'
            };

            DrawDirectTextStringClipped(
                    Point(lRect.left + kMiniAmmoTextHBuffer, lRect.bottom - 1), digits, lightcolor,
                    gOffWorld, clipRect);
        }
        update = true;
    }

    if ( update)
    {
        mRect.left = clipRect.left;
        mRect.right = clipRect.right;
        mRect.top = clipRect.top;
        mRect.bottom = clipRect.bottom;

        // copy the dirty rect
        copy_world(*gRealWorld, *gOffWorld, mRect);
    }

    lastOne = thisOne;
    lastTwo = thisTwo;
    lastSpecial = thisSpecial;
}


void UpdateMiniShipData( spaceObjectType *oldObject, spaceObjectType *newObject, unsigned char headerColor,
                    short screenTop, short whichString)

{
    RgbColor            color, lightcolor, darkcolor;
    coordPointType      coord;
    Point               where;
    short               whichShape;
    spaceObjectType     *dObject = NULL;
    long                tlong, thisScale;
    Rect                mRect;
    Rect            lRect, dRect, spriteRect, uRect, clipRect;

    clipRect.left = kMiniScreenLeft;
    clipRect.top = screenTop + globals()->gInstrumentTop;
    clipRect.right = kMiniScreenRight;
    clipRect.bottom = clipRect.top + 64;

    mSetDirectFont( kComputerFontNum);

    uRect.left = uRect.top = uRect.bottom = -1;

    if ( oldObject->id != newObject->id)
    {

        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, 0, 0, kMiniScreenWidth, lRect, gOffWorld);
        color = GetRGBTranslateColorShade(headerColor, LIGHT);
        lightcolor = GetRGBTranslateColorShade(headerColor, VERY_LIGHT);
        darkcolor = GetRGBTranslateColorShade(headerColor, MEDIUM);

        DrawNateShadedRect(gOffWorld, &lRect, clipRect, color, lightcolor, darkcolor);

        String text(StringList(kMiniDataStringID).at(whichString - 1));
        DrawDirectTextStringClipped(
                Point(lRect.left + kMiniScreenLeftBuffer, lRect.top + gDirectText->ascent),
                text, RgbColor::kBlack, gOffWorld, clipRect);
        uRect = lRect;
        uRect = clipRect;

        if ( newObject->attributes & kIsDestination)
        {
            // blacken the line for the object type name
            mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniNameLineNum, 0, kMiniScreenWidth, lRect, gOffWorld);

            // get the color for writing the name
            color = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);

            // move to the 1st line in the selection miniscreen
            String text(GetDestBalanceName(newObject->destinationObject));
            DrawDirectTextStringClipped(
                    Point(lRect.left + kMiniScreenLeftBuffer, lRect.top + gDirectText->ascent),
                    text, color, gOffWorld, clipRect);
            if ( uRect.left == -1)
            {
                uRect = lRect;
            } else
            {
                uRect.enlarge_to(lRect);
            }
        } else if ( oldObject->whichBaseObject != newObject->whichBaseObject)
        {

            // blacken the line for the object type name
            mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniNameLineNum, 0, kMiniScreenWidth, lRect, gOffWorld);

            if ( newObject->whichBaseObject >= 0)
            {
                // get the color for writing the name
                color = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);

                // move to the 1st line in the selection miniscreen, write the name
                String text(StringList(kSpaceObjectShortNameResID).at(newObject->whichBaseObject));
                DrawDirectTextStringClipped(
                        Point(lRect.left + kMiniScreenLeftBuffer, lRect.top + gDirectText->ascent),
                        text, color, gOffWorld, clipRect);
            }

            if ( uRect.left == -1)
            {
                uRect = lRect;
            } else
            {
                uRect.enlarge_to(lRect);
            }
        }
    }
        // set the rect for drawing the "icon" of the object type

    if ( oldObject->pixResID != newObject->pixResID)
    {
        dRect.left = kMiniIconLeft;
        dRect.top = screenTop + globals()->gInstrumentTop + MiniIconMacLineTop();
        dRect.right = kMiniScreenLeft + kMiniIconWidth;
        dRect.bottom = dRect.top + kMiniIconHeight;

        // erase the area

        DrawNateRect(gOffWorld, &dRect, RgbColor::kBlack);

        if (( newObject->whichBaseObject >= 0) && ( newObject->pixResID >= 0))
        {
            NatePixTable* pixTable = GetPixTable( newObject->pixResID);

            if (pixTable != NULL) {
                if (newObject->attributes & kIsSelfAnimated) {
                    whichShape = more_evil_fixed_to_long(newObject->baseType->frame.animation.firstShape);
                } else {
                    whichShape = 0;
                }

                // get the picture data
                const NatePixTable::Frame& frame = pixTable->at(whichShape);

                // calculate the correct size

                tlong = (kMiniIconHeight - 2) * SCALE_SCALE;
                tlong /= frame.height();
                thisScale = (kMiniIconWidth - 2) * SCALE_SCALE;
                thisScale /= frame.width();

                if ( tlong < thisScale) thisScale = tlong;
                if ( thisScale > SCALE_SCALE) thisScale = SCALE_SCALE;

                // calculate the correct position

                coord.h = frame.center().h;
                coord.h *= thisScale;
                coord.h >>= SHIFT_SCALE;
                tlong = frame.width();
                tlong *= thisScale;
                tlong >>= SHIFT_SCALE;
                where.h = ( kMiniIconWidth / 2) - ( tlong / 2);
                where.h += dRect.left + coord.h;

                coord.v = frame.center().v;
                coord.v *= thisScale;
                coord.v >>= SHIFT_SCALE;
                tlong = frame.height();
                tlong *= thisScale;
                tlong >>= SHIFT_SCALE;
                where.v = ( kMiniIconHeight / 2) - ( tlong / 2);
                where.v += dRect.top + coord.v;


                // draw the sprite

                OptScaleSpritePixInPixMap(frame, where, thisScale, &spriteRect, dRect, gOffWorld);
            }
        }

        color = GetRGBTranslateColorShade(PALE_GREEN, MEDIUM);
        DrawNateVBracket(gOffWorld, dRect, clipRect, color);

        if ( uRect.left == -1)
        {
            uRect = dRect;
        }
        else
        {
            uRect.enlarge_to(dRect);
        }
    }

    if ( oldObject->health != newObject->health)
    {
        dRect.left = kMiniHealthLeft;
        dRect.top = screenTop + globals()->gInstrumentTop + MiniIconMacLineTop();
        dRect.right = dRect.left + kMiniBarWidth;
        dRect.bottom = dRect.top + kMiniIconHeight;

        // erase the area

        DrawNateRect(gOffWorld, &dRect, RgbColor::kBlack);

        if ( newObject->baseType != NULL)
        {
            if (( newObject->baseType->health > 0) && ( newObject->health > 0))
            {
                tlong = newObject->health * kMiniBarHeight;
                tlong /= newObject->baseType->health;

                color = GetRGBTranslateColorShade(SKY_BLUE, DARK);

                lRect.left = dRect.left + 2;
                lRect.top = dRect.top + 2;
                lRect.right = dRect.right - 2;
                lRect.bottom = dRect.bottom - 2 - tlong;
                DrawNateRect(gOffWorld, &lRect, color);

                color = GetRGBTranslateColorShade(SKY_BLUE, LIGHT);
                lRect.top = dRect.bottom - 2 - tlong;
                lRect.bottom = dRect.bottom - 2;
                DrawNateRect(gOffWorld, &lRect, color);

                color = GetRGBTranslateColorShade(SKY_BLUE, MEDIUM);
                DrawNateVBracket(gOffWorld, dRect, clipRect, color);
            }
        }


        if ( uRect.left == -1)
        {
            uRect = dRect;
        }
        else
        {
            uRect.enlarge_to(dRect);
        }

    }

    if (oldObject->energy != newObject->energy)
    {
        dRect.left = kMiniEnergyLeft;
        dRect.top = screenTop + globals()->gInstrumentTop + MiniIconMacLineTop();
        dRect.right = dRect.left + kMiniBarWidth;
        dRect.bottom = dRect.top + kMiniIconHeight;

        // erase the area

        DrawNateRect(gOffWorld, &dRect, RgbColor::kBlack);

        if ( newObject->baseType != NULL)
        {
            if (( newObject->baseType->energy > 0) && ( newObject->energy > 0))
            {
                tlong = newObject->energy * kMiniBarHeight;
                tlong /= newObject->baseType->energy;

                color = GetRGBTranslateColorShade(YELLOW, DARK);

                lRect.left = dRect.left + 2;
                lRect.top = dRect.top + 2;
                lRect.right = dRect.right - 2;
                lRect.bottom = dRect.bottom - 2 - tlong;
                DrawNateRect(gOffWorld, &lRect, color);

                color = GetRGBTranslateColorShade(YELLOW, LIGHT);
                lRect.top = dRect.bottom - 2 - tlong;
                lRect.bottom = dRect.bottom - 2;
                DrawNateRect(gOffWorld, &lRect, color);

                color = GetRGBTranslateColorShade(YELLOW, MEDIUM);
                DrawNateVBracket(gOffWorld, dRect, clipRect, color);
            }
        }

        if ( uRect.left == -1)
        {
            uRect = dRect;
        }
        else
        {
            uRect.enlarge_to(dRect);
        }
    }

    if ( oldObject->beamType != newObject->beamType)
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniWeapon1LineNum, kMiniRightColumnLeft, kMiniScreenWidth, lRect, gOffWorld);

        // get the color for writing the name
        color = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);

        // move to the 1st line in the selection miniscreen, write the name
        if ( newObject->beamType >= 0)
        {
            String text(StringList(kSpaceObjectShortNameResID).at(newObject->beamType));
            DrawDirectTextStringClipped(
                    Point(lRect.left, lRect.top + gDirectText->ascent), text, color, gOffWorld,
                    clipRect);
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    if ( oldObject->pulseType != newObject->pulseType)
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniWeapon2LineNum, kMiniRightColumnLeft, kMiniScreenWidth, lRect, gOffWorld);

        // get the color for writing the name
        color = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);

        // move to the 1st line in the selection miniscreen, write the name
        if ( newObject->pulseType >= 0)
        {
            String text(StringList(kSpaceObjectShortNameResID).at(newObject->pulseType));
            DrawDirectTextStringClipped(
                    Point(lRect.left, lRect.top + gDirectText->ascent), text, color, gOffWorld,
                    clipRect);
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    if (( oldObject->specialType != newObject->specialType) && ( ! (newObject->attributes & kIsDestination)))
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniWeapon3LineNum, kMiniRightColumnLeft, kMiniScreenWidth, lRect, gOffWorld);

        // get the color for writing the name
        color = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);

        // move to the 1st line in the selection miniscreen, write the name
        if ( newObject->specialType >= 0)
        {
            String text(StringList(kSpaceObjectShortNameResID).at(newObject->specialType));
            DrawDirectTextStringClipped(
                    Point(lRect.left, lRect.top + gDirectText->ascent), text, color, gOffWorld,
                    clipRect);
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    if ( oldObject->destinationObject != newObject->destinationObject)
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniDestLineNum, 0, kMiniScreenWidth, lRect, gOffWorld);

        // write the name
        if ( newObject->destinationObject >= 0)
        {
            if ( newObject->destObjectPtr != NULL)
            {
                dObject = newObject->destObjectPtr;

                // get the color for writing the name
                if ( dObject->owner == globals()->gPlayerAdmiralNumber)
                {
                    color = GetRGBTranslateColorShade(GREEN, VERY_LIGHT);
                } else
                {
                    color = GetRGBTranslateColorShade(RED, VERY_LIGHT);
                }

                if ( dObject->attributes & kIsDestination)
                {
                    String text(GetDestBalanceName(dObject->destinationObject));
                    DrawDirectTextStringClipped(
                            Point(lRect.left, lRect.top + gDirectText->ascent), text, color,
                            gOffWorld, clipRect);
                } else
                {
                    String text(StringList(kSpaceObjectNameResID).at(dObject->whichBaseObject));
                    DrawDirectTextStringClipped(
                            Point(lRect.left, lRect.top + gDirectText->ascent), text, color,
                            gOffWorld, clipRect);
                }
            }
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    mRect.left = uRect.left;
    mRect.right = uRect.right;
    mRect.top = uRect.top;
    mRect.bottom = uRect.bottom;

    // copy the dirty rect
    copy_world(*gRealWorld, *gOffWorld, mRect);

    mCopyMiniSpaceObject( *oldObject, *newObject);
}

void MiniComputerDoAccept( void)

{
    if (true) {  // TODO(sfiera): if non-networked.
        MiniComputerExecute( globals()->gMiniScreenData.currentScreen,
            globals()->gMiniScreenData.selectLine, globals()->gPlayerAdmiralNumber);
    } else {
#ifdef NETSPROCKET_AVAILABLE
        if ( !SendMenuMessage( globals()->gGameTime + gNetLatency, globals()->gMiniScreenData.currentScreen,
            globals()->gMiniScreenData.selectLine))
            StopNetworking();
#endif NETSPROCKET_AVAILABLE
    }
}

void MiniComputerExecute( long whichPage, long whichLine, long whichAdmiral)

{
    spaceObjectType *anObject, *anotherObject;
    long            l;

    switch ( whichPage)
    {
        case kMainMiniScreen:
            if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
            {
                switch ( whichLine)
                {
                    case kMainMiniBuild:
                        MakeMiniScreenFromIndString( kBuildMiniScreen);
                        MiniComputerSetBuildStrings();
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    case kMainMiniSpecial:
                        MakeMiniScreenFromIndString( kSpecialMiniScreen);
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    case kMainMiniMessage:
                        MakeMiniScreenFromIndString( kMessageMiniScreen);
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    case kMainMiniStatus:
                        MakeMiniScreenFromIndString( kStatusMiniScreen);
                        MiniComputerSetStatusStrings();
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    default:
                        break;
                }
            }

            break;

        case kBuildMiniScreen:
            if ( globals()->keyMask & kComputerBuildMenu) return;
            if ( whichLine != kMiniScreenNoLineSelected)
            {
                if ( CountObjectsOfBaseType( -1, -1) <
                    (kMaxSpaceObject - kMaxShipBuffer))
                {
                    if (AdmiralScheduleBuild( whichAdmiral,
                        whichLine - kBuildScreenFirstTypeLine) == false)
                    {
                        if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
                            mPlayBeepBad();
                    }
                } else
                {
                    if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
                    {
                        SetStatusString("Maximum number of ships built", ORANGE);
                    }
                }
            }
            break;

        case kSpecialMiniScreen:
            if ( globals()->keyMask & kComputerSpecialMenu) return;
            switch ( whichLine)
            {
                case kSpecialMiniTransfer:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    anObject = GetAdmiralFlagship( whichAdmiral);
                    if ( anObject != NULL)
                    {
                        if ( l != kNoShip)
                        {
                            anotherObject = gSpaceObjectData.get() + l;
                            if (( anotherObject->active != kObjectInUse) ||
                                ( !(anotherObject->attributes & kCanThink)) ||
                                ( anotherObject->attributes & kStaticDestination)
                                || ( anotherObject->owner != anObject->owner) ||
                                (!(anotherObject->attributes & kCanAcceptDestination))
                                || ( !(anotherObject->attributes & kCanBeDestination))
                                || ( anObject->active != kObjectInUse))
                            {
                                if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
                                    mPlayBeepBad();
                            } else
                            {
                                ChangePlayerShipNumber( whichAdmiral, l);
                            }
                        } else
                        {
                            PlayerShipBodyExpire( anObject, false);
                        }
                    }
                    break;

                case kSpecialMiniFire1:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kOneKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniFire2:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kTwoKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniFireSpecial:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kEnterKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniHold:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        SetObjectLocationDestination( anObject, &(anObject->location));
                    }
                    break;

                case kSpecialMiniGoToMe:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        anotherObject = GetAdmiralFlagship( whichAdmiral);
                        SetObjectLocationDestination( anObject, &(anotherObject->location));
                    }
                    break;

                default:
                    break;
            }
            break;

        case kMessageMiniScreen:
            if ( globals()->keyMask & kComputerMessageMenu) return;
            if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
            {
                switch ( whichLine)
                {
                    case kMessageMiniNext:
                        AdvanceCurrentLongMessage();
                        break;

                    case kMessageMiniLast:
                        ReplayLastLongMessage();
                        break;

                    case kMessageMiniPrevious:
                        PreviousCurrentLongMessage();
                        break;

                    default:
                        break;
                }
            }
            break;

        default:
            break;
    }
}

void MiniComputerDoCancel( void)

{
    switch ( globals()->gMiniScreenData.currentScreen)
    {
        case kBuildMiniScreen:
        case kSpecialMiniScreen:
        case kMessageMiniScreen:
        case kStatusMiniScreen:
            MakeMiniScreenFromIndString( kMainMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();

            break;

        default:
            break;
    }
}

void MiniComputerSetBuildStrings( void) // sets the ship type strings for the build screen
// also sets up the values = base object num

{
    baseObjectType      *buildObject = NULL;
    admiralType         *admiral = NULL;
    destBalanceType     *buildAtObject = NULL;
    miniScreenLineType  *line = NULL;
    long                count, baseNum, lineNum, buildAtObjectNum;
    Rect                mRect;

    mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);

    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;
    if ( globals()->gMiniScreenData.currentScreen == kBuildMiniScreen)
    {
        admiral = globals()->gAdmiralData.get() + globals()->gPlayerAdmiralNumber;
        line = globals()->gMiniScreenData.lineData.get() +
            kBuildScreenWhereNameLine;
        buildAtObjectNum =
            GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber);
        line->value = buildAtObjectNum;

        if ( buildAtObjectNum >= 0)
        {
            buildAtObject = mGetDestObjectBalancePtr( buildAtObjectNum);
            mCopyBlankLineString( line, buildAtObject->name);

            line = globals()->gMiniScreenData.lineData.get() + kBuildScreenFirstTypeLine;
            lineNum = kBuildScreenFirstTypeLine;

            for ( count = 0; count < kMaxShipCanBuild; count++)
            {
                mGetBaseObjectFromClassRace( buildObject, baseNum, buildAtObject->canBuildType[count], admiral->race);
                line->value = baseNum;
                line->sourceData = buildObject;
                if ( buildObject != NULL)
                {
                    mCopyBlankLineString(line, StringList(kSpaceObjectNameResID).at(baseNum));
                    if ( buildObject->price > mFixedToLong(admiral->cash))
                        line->selectable = selectDim;
                    else line->selectable = selectable;
                    if ( globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected)
                    {
                        globals()->gMiniScreenData.selectLine = lineNum;
                        line->hiliteLeft = mRect.left;
                        line->hiliteRight = mRect.right;
                    }

                } else
                {
                    line->string.clear();
                    line->selectable = cannotSelect;
                    if ( globals()->gMiniScreenData.selectLine == (count + kBuildScreenFirstTypeLine))
                    {
                        line->hiliteLeft = line->hiliteRight = 0;
                        globals()->gMiniScreenData.selectLine++;
                    }
                    line->value = -1;
                }
                lineNum++;
                line++;
            }
            line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
            if ( line->selectable == cannotSelect)
                globals()->gMiniScreenData.selectLine =
                kMiniScreenNoLineSelected;
        } else
        {
            globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;

            line =
                globals()->gMiniScreenData.lineData.get() +
                kBuildScreenFirstTypeLine;
            for ( count = 0; count < kMaxShipCanBuild; count++)
            {
                line->string.clear();
                line->selectable = cannotSelect;
                line->hiliteLeft = line->hiliteRight = 0;
                line++;
            }
        }
    }
}

// MiniComputerGetPriceOfCurrentSelection:
//  If the Build Menu is up, returns the price of the currently selected
//  ship, regardless of whether or not it is affordable.
//
//  If the selection is not legal, or the current Menu is not the Build Menu,
//  returns 0

long MiniComputerGetPriceOfCurrentSelection( void)
{
    miniScreenLineType  *line = NULL;
    baseObjectType      *buildObject = NULL;

    if (( globals()->gMiniScreenData.currentScreen != kBuildMiniScreen) ||
            ( globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected))
        return (0);

        line = globals()->gMiniScreenData.lineData.get() +
            globals()->gMiniScreenData.selectLine;

        if ( line->value < 0) return( 0);

        buildObject = mGetBaseObjectPtr( line->value);

        if ( buildObject->price < 0) return( 0);

        return( mLongToFixed(buildObject->price));
}

void MiniComputerSetStatusStrings() {
    // the strings must be in this format:
    //  type\number\player\negativevalue\falsestring\truestring\basestring\poststring
    //
    //  where type = 0...5
    //
    //  number = which score/condition #
    //
    //  player = which player score (if any); -1 = you, -2 = first not you
    //  ( 0 if you're player 1, 1 if you're player 0)
    //
    //  negative value = value to use for kIntegerMinusValue or kSmallFixedMinusValue
    //
    //  falsestring = string to use if false
    //
    //  truestring = string to use if true
    //
    //  basestring = first part of string
    //
    //  for example, the string 1\0\\0\0\N\Y\SHIP DESTROYED:
    //  would result in the status line SHIP DESTROYED, based on condition 0;
    //  if false, line reads SHIP DESTROYED: N, and if true SHIP DESTROYED: Y
    //
    //  example #2, string 2\1\0\10\\\Samples Left:
    //  would result in the status line "Samples Left: " + score 1 of player 0
    //  so if player 0's score 1 was 3, the line would read:
    //  Samples Left: 7
    //

    miniScreenLineType  *line;

    if (globals()->gMissionStatusStrList.get() == NULL) {
        for (int count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
            line = globals()->gMiniScreenData.lineData.get() + count;
            line->statusType = kNoStatusData;
            line->value = -1;
            line->string.clear();
        }
        return;
    }

    for (int count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
        line = globals()->gMiniScreenData.lineData.get() + count;

        if (implicit_cast<size_t>(count - kStatusMiniScreenFirstLine) <
                globals()->gMissionStatusStrList->size()) {
            // we have some data for this line to interpret

            StringSlice sourceString =
                globals()->gMissionStatusStrList->at(count - kStatusMiniScreenFirstLine);

            if (sourceString.at(0) == '_') {
                line->underline = true;
                sourceString = sourceString.slice(1);
            }

            if (sourceString.at(0) == '-') {
                // - = abbreviated string, just plain text
                line->statusType = kPlainTextStatus;
                line->value = 0;
                line->string.assign(sourceString.slice(1));
            } else {
                //////////////////////////////////////////////
                // get status type
                StringSlice status_type_string;
                if (partition(status_type_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(status_type_string, value)) {
                        if ((0 <= value) && (value <= kMaxStatusTypeValue)) {
                            line->statusType = value;
                        }
                    }
                }

                //////////////////////////////////////////////
                // get score/condition number
                StringSlice score_condition_string;
                if (partition(score_condition_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(score_condition_string, value)) {
                        line->whichStatus = value;
                    }
                }

                //////////////////////////////////////////////
                // get player number
                StringSlice player_number_string;
                if (partition(player_number_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(player_number_string, value)) {
                        line->statusPlayer = value;
                    }
                }

                //////////////////////////////////////////////
                // get negative value
                StringSlice negative_value_string;
                if (partition(negative_value_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(negative_value_string, value)) {
                        line->negativeValue = value;
                    }
                }

                //////////////////////////////////////////////
                // get falseString
                StringSlice status_false_string;
                if (partition(status_false_string, "\\", sourceString)) {
                    line->statusFalse.assign(status_false_string);
                }

                //////////////////////////////////////////////
                // get trueString
                StringSlice status_true_string;
                if (partition(status_true_string, "\\", sourceString)) {
                    line->statusTrue.assign(status_true_string);
                }

                //////////////////////////////////////////////
                // get statusString
                StringSlice status_string;
                if (partition(status_string, "\\", sourceString)) {
                    line->statusString.assign(status_string);
                }

                //////////////////////////////////////////////
                // get postString
                line->postString.assign(sourceString);

                line->value = MiniComputerGetStatusValue( count);
                MiniComputerMakeStatusString(count, line->string);
            }
        } else {
            line->statusType = kNoStatusData;
            line->value = -1;
            line->string.clear();
        }
    }
}

void MiniComputerMakeStatusString(int32_t which_line, String& string) {
    string.clear();

    const miniScreenLineType& line = globals()->gMiniScreenData.lineData[which_line];
    if (line.statusType == kNoStatusData) {
        return;
    }

    print(string, line.statusString);
    switch (line.statusType) {
        case kTrueFalseCondition:
            if (line.value == 1) {
                print(string, line.statusTrue);
            } else {
                print(string, line.statusFalse);
            }
            break;

        case kIntegerValue:
        case kIntegerMinusValue:
            print(string, line.value);
            break;

        case kSmallFixedValue:
        case kSmallFixedMinusValue:
            print(string, fixed(line.value));
            break;
    }
    if (line.statusType != kPlainTextStatus) {
        print(string, line.postString);
    }
}

long MiniComputerGetStatusValue( long whichLine)
{
    miniScreenLineType  *line;

    line = globals()->gMiniScreenData.lineData.get() +
        whichLine;

    if ( line->statusType == kNoStatusData)
        return( -1);

    switch ( line->statusType)
    {
        case kPlainTextStatus:
            return( 0);
            break;

        case kTrueFalseCondition:
            if (gThisScenario->condition(line->whichStatus)->true_yet()) {
                return 1;
            } else {
                return 0;
            }
            break;

        case kIntegerValue:
        case kSmallFixedValue:
            return( GetAdmiralScore( GetRealAdmiralNumber( line->statusPlayer),
                line->whichStatus));
            break;

        case kIntegerMinusValue:
        case kSmallFixedMinusValue:
            return( line->negativeValue - GetAdmiralScore(
                GetRealAdmiralNumber( line->statusPlayer), line->whichStatus));
            break;

        default:
            return( 0);
            break;
    }
}

void MiniComputerHandleClick( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        globals()->gMiniScreenData.clickLine = lineNum;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3();
            }
            if ( outLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() +
                    outLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( outLineButtonLine);
                }
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3();
            }
            if ( inLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( inLineButtonLine);
                }
            }
        }
    } else
    {

        // make sure both buttons are off
        if ( inLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( inLineButtonLine);
            }
        }
        if ( outLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( outLineButtonLine);
            }
        }

        mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);

        // if click is in main menu screen
        if (mRect.contains(where)) {
            if ( globals()->gMiniScreenData.selectLine !=
                kMiniScreenNoLineSelected)
            {
                line = globals()->gMiniScreenData.lineData.get() +
                    globals()->gMiniScreenData.selectLine;
                line->hiliteLeft = line->hiliteRight = 0;
                DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            }

            lineNum = mGetLineNumFromV( where.v);
            globals()->gMiniScreenData.clickLine = lineNum;
            line = globals()->gMiniScreenData.lineData.get() + lineNum;
            if (( line->selectable == selectable) || (line->selectable == selectDim))
            {
                globals()->gMiniScreenData.selectLine = lineNum;

                line = globals()->gMiniScreenData.lineData.get() +
                    globals()->gMiniScreenData.selectLine;
                line->hiliteLeft = mRect.left;
                line->hiliteRight = mRect.right;
                DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            }
        } else globals()->gMiniScreenData.clickLine = kMiniScreenNoLineSelected;
    }
}

void MiniComputerHandleDoubleClick( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3();
            }
            if ( outLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( outLineButtonLine);
                }
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3();
            }
            if ( inLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( inLineButtonLine);
                }
            }
        }
    } else
    {

        // make sure both buttons are off
        if ( inLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( inLineButtonLine);
            }
        }
        if ( outLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( outLineButtonLine);
            }
        }

        mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);

        // if click is in main menu screen
        if (mRect.contains(where)) {
            lineNum = mGetLineNumFromV( where.v);
            if ( lineNum == globals()->gMiniScreenData.selectLine)
            {
                mPlayBeep3();
                MiniComputerDoAccept();
            } else
            {
                if ( globals()->gMiniScreenData.selectLine !=
                    kMiniScreenNoLineSelected)
                {
                    line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = line->hiliteRight = 0;
                    DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
                }

                lineNum = mGetLineNumFromV( where.v);
                line = globals()->gMiniScreenData.lineData.get() + lineNum;
                if (( line->selectable == selectable) || (line->selectable == selectDim))
                {
                    globals()->gMiniScreenData.selectLine = lineNum;

                    line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = mRect.left;
                    line->hiliteRight = mRect.right;
                    DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
                }
            }
        }
    }
}

void MiniComputerHandleMouseUp( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind == buttonOnLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                MiniComputerDoAccept();
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind == buttonOnLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                MiniComputerDoCancel();
            }
        }
    }
}

void MiniComputerHandleMouseStillDown( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if (( line->whichButton == kInLineButton) &&
            ( lineNum == globals()->gMiniScreenData.clickLine))
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
            }
        } else if (( line->whichButton == kOutLineButton) &&
            ( lineNum == globals()->gMiniScreenData.clickLine))
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
            }
        } else ( lineNum = -1);
    } else lineNum = -1;

    if ( lineNum == -1)
    {
        line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
        if ( line->lineKind == buttonOnLineKind)
        {
            line->lineKind = buttonOffLineKind;
            DrawAndShowMiniScreenLine( inLineButtonLine);
        }
        line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
        if ( line->lineKind == buttonOnLineKind)
        {
            line->lineKind = buttonOffLineKind;
            DrawAndShowMiniScreenLine( outLineButtonLine);
        }
    }
}

// for ambrosia tutorial, a horrific hack
void MiniComputer_SetScreenAndLineHack( long whichScreen, long whichLine)
{
    Point   w;

    switch ( whichScreen)
    {
        case kBuildMiniScreen:
            MakeMiniScreenFromIndString( kBuildMiniScreen);
            MiniComputerSetBuildStrings();
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        case kSpecialMiniScreen:
            MakeMiniScreenFromIndString( kSpecialMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        case kMessageMiniScreen:
            MakeMiniScreenFromIndString( kMessageMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        case kStatusMiniScreen:
            MakeMiniScreenFromIndString( kStatusMiniScreen);
            MiniComputerSetStatusStrings();
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        default:
            MakeMiniScreenFromIndString( kMainMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;
    }

    mSetDirectFont( kComputerFontNum);
    w.v = (whichLine * gDirectText->height) + ( kMiniScreenTop +
                    globals()->gInstrumentTop);
    w.h = kMiniScreenLeft + 5;
    MiniComputerHandleClick( w);    // what an atrocious hack! oh well
}

}  // namespace antares
