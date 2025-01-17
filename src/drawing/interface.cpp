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

#include "drawing/interface.hpp"

#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/interface.hpp"
#include "data/picture.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/interface-text.hpp"
#include "drawing/shapes.hpp"
#include "drawing/text.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::scoped_ptr;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int32_t kInterfaceLargeHBorder    = 13;
const int32_t kInterfaceSmallHBorder    = 3;
const int32_t kInterfaceVEdgeHeight     = 1;
const int32_t kInterfaceVCornerHeight   = 2;
const int32_t kInterfaceVLipHeight      = 1;
const int32_t kInterfaceHTop            = 2;
const int32_t kLabelBottomHeight        = 6;
const int32_t kInterfaceContentBuffer   = 2;

const int32_t kIndicatorVOffset         = 4;
const int32_t kRadioIndicatorHOffset    = 4;
const int32_t kCheckIndicatorHOffset    = 4;

const int32_t kMaxKeyNameLength         = 4;  // how many chars can be in name of key for plainButton

// DrawInterfaceString:
//  Relies on roman alphabet for upper/lower casing.  NOT WORLD-READY!

void SetInterfaceLargeUpperFont(interfaceStyleType style) {
    if ( style == kSmall) {
        mSetDirectFont( kButtonSmallFontNum);
    } else {
        mSetDirectFont( kButtonFontNum);
    }
}

void SetInterfaceLargeLowerFont(interfaceStyleType style) {
    if ( style == kSmall) {
        mSetDirectFont( kButtonSmallFontNum);
    } else {
        mSetDirectFont( kButtonFontNum);
    }
}

void DrawInterfaceString(
        Point p, StringSlice s, interfaceStyleType style, PixMap* pix, const RgbColor& color) {
    SetInterfaceLargeUpperFont(style);
    DrawDirectTextStringClipped(p, s, color, pix, pix->size().as_rect());
}

void FrameOval(PixMap* pix, Rect rect, RgbColor color) {
    throw Exception("FrameOval() not implemented");
}

void PaintOval(PixMap* pix, Rect rect, RgbColor color) {
    throw Exception("PaintOval() not implemented");
}

}  // namespace

enum inlineKindType {
    kNoKind = 0,
    kVPictKind = 1,
    kVClearPictKind = 2
};

inline void mDrawPuffUpRect(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    const RgbColor color = GetRGBTranslateColorShade(mcolor, mshade);
    pix->view(mrect).fill(color);
    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left, mrect.bottom - 1);
    MacLineTo(pix, mrect.left, mrect.top, lighter);
    MacLineTo(pix, mrect.right - 1, mrect.top, lighter);
    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    MacLineTo(pix, mrect.right - 1, mrect.bottom - 1, darker);
    MacLineTo(pix, mrect.left, mrect.bottom - 1, darker);
}

inline void mDrawPuffUpOval(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    mrect.left++;
    mrect.right++;
    FrameOval(pix, mrect, darker);
    mrect.left--;
    mrect.right--;
    mrect.top++;
    mrect.bottom++;
    FrameOval(pix, mrect, darker);
    mrect.top--;
    mrect.bottom--;

    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    mrect.left--;
    mrect.right--;
    FrameOval(pix, mrect, lighter);
    mrect.left++;
    mrect.right++;
    mrect.top--;
    mrect.bottom--;
    FrameOval(pix, mrect, lighter);
    mrect.top++;
    mrect.bottom++;

    const RgbColor color = GetRGBTranslateColorShade(mcolor, mshade);
    PaintOval(pix, mrect, color);
}

inline void mDrawPuffDownRect(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    pix->view(mrect).fill(RgbColor::kBlack);
    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    MoveTo(mrect.left - 1, mrect.bottom);
    MacLineTo(pix, mrect.left - 1, mrect.top - 1, darker);
    MacLineTo(pix, mrect.right, mrect.top - 1, darker);
    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    MacLineTo(pix, mrect.right, mrect.bottom, lighter);
    MacLineTo(pix, mrect.left - 1, mrect.bottom, lighter);
}

inline void mDrawPuffDownOval(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    mrect.left++;
    mrect.right++;
    FrameOval(pix, mrect, lighter);
    mrect.left--;
    mrect.right--;
    mrect.top++;
    mrect.bottom++;
    FrameOval(pix, mrect, lighter);
    mrect.top--;
    mrect.bottom--;

    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    mrect.left--;
    mrect.right--;
    FrameOval(pix, mrect, darker);
    mrect.left++;
    mrect.right++;
    mrect.top--;
    mrect.bottom--;
    FrameOval(pix, mrect, darker);
    mrect.top++;
    mrect.bottom++;

    PaintOval(pix, mrect, RgbColor::kBlack);
}

inline void mDrawPuffUpTopBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade,
        int mthisHBorder, PixMap* pix) {
    const RgbColor color = GetRGBTranslateColorShade(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.left, mrect.top);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.right,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.right + mthisHBorder, mrect.top);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.left,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.right, mrect.top  - kInterfaceVLipHeight);
    pix->view(mtrect).fill(color);
    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    MoveTo(mrect.left - mthisHBorder, mrect.top);
    MacLineTo(pix, mrect.left, mrect.top, darker);
    MacLineTo(pix, mrect.left, mrect.top - kInterfaceVLipHeight, darker);
    MacLineTo(pix, mrect.right, mrect.top - kInterfaceVLipHeight, darker);
    MacLineTo(pix, mrect.right, mrect.top, darker);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.top, darker);
    MacLineTo(pix, mrect.right + mthisHBorder,
            mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight, darker);
    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    MacLineTo(pix, mrect.left - mthisHBorder,
            mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight, lighter);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.top, lighter);
}

inline void mDrawPuffUpBottomBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade,
        int mthisHBorder, PixMap* pix) {
    const RgbColor color = GetRGBTranslateColorShade(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.bottom,
        mrect.left, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.right,
        mrect.bottom,
        mrect.right + mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.left,
        mrect.bottom + kInterfaceVLipHeight,
        mrect.right, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    pix->view(mtrect).fill(color);
    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left - mthisHBorder,
            mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.bottom, lighter);
    MacLineTo(pix, mrect.left, mrect.bottom, lighter);
    MacLineTo(pix, mrect.left, mrect.bottom + kInterfaceVLipHeight, lighter);
    MacLineTo(pix, mrect.right, mrect.bottom + kInterfaceVLipHeight, lighter);
    MacLineTo(pix, mrect.right, mrect.bottom, lighter);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.bottom, lighter);
    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    MacLineTo(pix, mrect.right + mthisHBorder,
            mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight, darker);
    MacLineTo(pix, mrect.left - mthisHBorder,
            mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight, darker);
}

inline void mDrawPuffUpTBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade, int msheight, int mthisHBorder, PixMap* pix) {
    const RgbColor color = GetRGBTranslateColorShade(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.top + msheight,
        mrect.left,
        mrect.top + msheight + kLabelBottomHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.right,
        mrect.top + msheight,
        mrect.right + mthisHBorder,
        mrect.top + msheight + kLabelBottomHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.left,
        mrect.top + msheight + kInterfaceVLipHeight,
        mrect.right,
        mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);
    pix->view(mtrect).fill(color);

    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left - mthisHBorder, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.top + msheight, lighter);
    MacLineTo(pix, mrect.left, mrect.top + msheight, lighter);
    MacLineTo(pix, mrect.left, mrect.top + msheight + kInterfaceVLipHeight, lighter);
    MacLineTo(pix, mrect.right, mrect.top + msheight + kInterfaceVLipHeight, lighter);
    MacLineTo(pix, mrect.right, mrect.top + msheight, lighter);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.top + msheight, lighter);

    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.top + msheight + kLabelBottomHeight, darker);
    MacLineTo(pix, mrect.right, mrect.top + msheight + kLabelBottomHeight, darker);
    MacLineTo(pix, mrect.right, mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight,
            darker);
    MacLineTo(pix, mrect.left, mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight,
            darker);
    MacLineTo(pix, mrect.left, mrect.top + msheight + kLabelBottomHeight, darker);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.top + msheight + kLabelBottomHeight, darker);
}

void DrawPlayerInterfacePlainRect(
        const Rect& rect, uint8_t color, interfaceStyleType style, PixMap* pix) {
    Rect            tRect, uRect;
    short           vcenter, thisHBorder = kInterfaceSmallHBorder;

    if (style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = rect;
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border
    mDrawPuffUpTopBorder( tRect, uRect, color, DARK, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder, pix);

    // main part left border

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);
}

void DrawPlayerInterfaceTabBox(
        const Rect& rect, uint8_t color, interfaceStyleType style, PixMap* pix,
        int top_right_border_size) {
    Rect            tRect, uRect;
    short           vcenter, thisHBorder = kInterfaceSmallHBorder;
    unsigned char   shade;

    if ( style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = rect;
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border
    shade = MEDIUM;
    const RgbColor rgb = GetRGBTranslateColorShade(color, shade);
    uRect = Rect((tRect).left - thisHBorder,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).left, (tRect).top);
    pix->view(uRect).fill(rgb);
    uRect = Rect((tRect).right,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).right + thisHBorder, (tRect).top);
    pix->view(uRect).fill(rgb);
    uRect = Rect((tRect).left,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).left + 6, (tRect).top  - kInterfaceVLipHeight);
    pix->view(uRect).fill(rgb);
    uRect = Rect((tRect).right - top_right_border_size,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).right, (tRect).top  - kInterfaceVLipHeight);
    pix->view(uRect).fill(rgb);
    const RgbColor darker = GetRGBTranslateColorShade(color, shade + kDarkerColor);
    MoveTo( (tRect).left - thisHBorder, (tRect).top);
    MacLineTo(pix, (tRect).left, (tRect).top, darker);
    MacLineTo(pix, (tRect).left, (tRect).top - kInterfaceVLipHeight, darker);
    MacLineTo(pix, (tRect).left + 5, (tRect).top - kInterfaceVLipHeight, darker);
    MoveTo( (tRect).right - top_right_border_size, (tRect).top - kInterfaceVLipHeight);
    MacLineTo(pix, (tRect).right, (tRect).top - kInterfaceVLipHeight, darker);
    MacLineTo(pix, (tRect).right, (tRect).top, darker);
    MacLineTo(pix, (tRect).right + thisHBorder, (tRect).top, darker);
    MacLineTo(pix, (tRect).right + thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight, darker);
    const RgbColor lighter = GetRGBTranslateColorShade(color, shade + kLighterColor);
    MacLineTo(pix, (tRect).right - top_right_border_size, (tRect).top - 3 - kInterfaceVCornerHeight,
            lighter);
    MoveTo( (tRect).left + 5, (tRect).top - 3 - kInterfaceVCornerHeight);
    MacLineTo(pix, (tRect).left - thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight, lighter);
    MacLineTo(pix, (tRect).left - thisHBorder, (tRect).top, lighter);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder, pix);

    // main part left border

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);
}

void DrawPlayerInterfaceButton(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    unsigned char   shade;
    RgbColor        color;

    if (item.style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds;

    uRect = tRect;
    uRect.right++;
    uRect.bottom++;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (item.item.plainButton.status == kDimmed) {
        shade = VERY_DARK;
    } else {
        shade = MEDIUM;
    }

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if (item.item.plainButton.status == kIH_Hilite) {
        shade = LIGHT;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
    } else {
        if (item.item.plainButton.status == kDimmed) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM + kSlightlyLighterColor;
        }
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
    }


    if (item.item.plainButton.key == 0)
    {
        uRect = Rect(tRect.left +  kInterfaceContentBuffer,
            tRect.top + kInterfaceContentBuffer,
            tRect.left +  kInterfaceContentBuffer,
            tRect.bottom - kInterfaceContentBuffer);

        if (item.item.plainButton.status == kIH_Hilite)
            shade = LIGHT;
        else shade = DARK;//DARKEST + kSlightlyLighterColor;
        uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);

        color = GetRGBTranslateColorShade(item.color, shade);
        pix->view(uRect).fill(color);

        if (item.item.plainButton.status == kIH_Hilite) {
            color = GetRGBTranslateColorShade(item.color, DARKEST);
        } else if (item.item.plainButton.status == kDimmed) {
            color = GetRGBTranslateColorShade(item.color, VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(item.color, LIGHTER);
        }
        StringList strings(item.item.plainButton.label.stringID);
        StringSlice s = strings.at(item.item.plainButton.label.stringNumber - 1);
        swidth = GetInterfaceStringWidth(s, item.style);
        swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), s, item.style, pix, color);
    } else
    {
        // draw the key code
        {
            if (item.item.plainButton.status == kDimmed)
                shade = VERY_DARK;
            else shade = LIGHT;
            String s;
            GetKeyNumName(item.item.plainButton.key, &s);
            swidth = GetInterfaceFontWidth(item.style) * kMaxKeyNameLength;

            uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            mDrawPuffUpRect(uRect, item.color, shade, pix);

            if (item.item.plainButton.status == kIH_Hilite)
                shade = LIGHT;
            else shade = DARK;//DARKEST;
            vRect = Rect(
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
                    tRect.top + kInterfaceContentBuffer,
                    tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            color = GetRGBTranslateColorShade(item.color, shade);
            pix->view(vRect).fill(color);

            swidth = GetInterfaceStringWidth(s, item.style);
            swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
            if (item.item.plainButton.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.color, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(item.color, DARKEST);
            }

            DrawInterfaceString(
                    Point(swidth, uRect.top + GetInterfaceFontAscent(item.style)), s, item.style,
                    pix, color);
        }

        // draw the button title
        {
            if (item.item.plainButton.status == kIH_Hilite) {
                color = GetRGBTranslateColorShade(item.color, DARKEST);
            } else if (item.item.plainButton.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.color, DARKEST + kSlightlyLighterColor);
            } else {
                color = GetRGBTranslateColorShade(item.color, LIGHTER);
            }

            StringList strings(item.item.plainButton.label.stringID);
            StringSlice s = strings.at(item.item.plainButton.label.stringNumber - 1);
            swidth = GetInterfaceStringWidth(s, item.style);
            swidth = uRect.right + ( tRect.right - uRect.right) / 2 - swidth / 2;
            sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, item.style, pix, color);
        }
    }
}

void DrawPlayerInterfaceTabBoxButton(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    unsigned char   shade;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = item.bounds;

    uRect = tRect;
    uRect.right++;
    uRect.bottom++;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if ( item.item.radioButton.status == kDimmed)
        shade = VERY_DARK;
    else shade = MEDIUM;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

//  mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder)

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if ( !item.item.radioButton.on)
    {
        if ( item.item.radioButton.status == kIH_Hilite)
        {
            shade = LIGHT;
            mDrawPuffUpRect( uRect, item.color, shade, pix);
            mDrawPuffUpRect( vRect, item.color, shade, pix);
        } else
        {
            if ( item.item.radioButton.status == kDimmed)
                shade = VERY_DARK;
            else shade = DARK;
            mDrawPuffUpRect( uRect, item.color, shade, pix);
            mDrawPuffUpRect( vRect, item.color, shade, pix);
        }
        uRect = Rect(uRect.left, uRect.bottom, uRect.right, uRect.bottom + 3);
        vRect = Rect(vRect.left, vRect.bottom, vRect.right, vRect.bottom + 3);
        pix->view(uRect).fill(RgbColor::kBlack);
        pix->view(vRect).fill(RgbColor::kBlack);
        uRect = Rect(uRect.left - 3, uRect.bottom, vRect.right + 3, uRect.bottom + 3);
        shade = MEDIUM;
        color = GetRGBTranslateColorShade(item.color, shade);
        pix->view(uRect).fill(color);
        const RgbColor lighter = GetRGBTranslateColorShade(item.color, shade + kLighterColor);
        MoveTo( uRect.left, uRect.top - 1);
        MacLineTo(pix, uRect.right - 1, uRect.top - 1, lighter);
        const RgbColor darker = GetRGBTranslateColorShade(item.color, shade + kDarkerColor);
        MoveTo( uRect.left, uRect.bottom);
        MacLineTo(pix, uRect.right - 1, uRect.bottom, darker);
    } else
    {
        if ( item.item.radioButton.status == kIH_Hilite)
        {
            shade = LIGHT;
        } else
        {
            if ( item.item.radioButton.status == kDimmed)
                shade = VERY_DARK;
            else shade = MEDIUM;
        }
        uRect.bottom += 7;
        vRect.bottom += 7;
        color = GetRGBTranslateColorShade(item.color, shade);
        pix->view(uRect).fill(color);
        pix->view(vRect).fill(color);
        const RgbColor lighter = GetRGBTranslateColorShade(item.color, shade + kLighterColor);
        MoveTo( uRect.right - 2, uRect.top);
        MacLineTo(pix, uRect.left, uRect.top, lighter);
        MacLineTo(pix, uRect.left, uRect.bottom - 5, lighter);
        MacLineTo(pix, uRect.left - 3, uRect.bottom - 5, lighter);
        MoveTo( vRect.right - 2, vRect.top);
        MacLineTo(pix, vRect.left, vRect.top, lighter);
        MacLineTo(pix, vRect.left, vRect.bottom - 2, lighter);
        MoveTo( vRect.right, vRect.bottom - 5);
        MacLineTo(pix, vRect.right + 2, vRect.bottom - 5, lighter);
        const RgbColor darker = GetRGBTranslateColorShade(item.color, shade + kDarkerColor);
        MoveTo( uRect.right - 1, uRect.top);
        MacLineTo(pix, uRect.right - 1, uRect.bottom - 1, darker);
        MacLineTo(pix, uRect.left - 3, uRect.bottom - 1, darker);
        MoveTo( vRect.right - 1, vRect.top);
        MacLineTo(pix, vRect.right - 1, vRect.bottom - 4, darker);
        MoveTo( vRect.left, vRect.bottom - 1);
        MacLineTo(pix, vRect.right + 2, vRect.bottom - 1, darker);
        uRect = Rect(uRect.left - 3, uRect.bottom - 4, uRect.right - 1, uRect.bottom - 1);
        const RgbColor color = GetRGBTranslateColorShade(item.color, shade);
        pix->view(uRect).fill(color);
        vRect = Rect(vRect.left + 1, vRect.bottom - 4, vRect.right + 3, vRect.bottom - 1);
        pix->view(vRect).fill(color);
        uRect.top--;
        uRect.bottom++;
        uRect.left = uRect.right + 1;
        uRect.right = vRect.left - 1;
        pix->view(uRect).fill(RgbColor::kBlack);
    }


    if ( item.item.radioButton.key == 0)
    {
        uRect = Rect(tRect.left +  kInterfaceContentBuffer,
            tRect.top + kInterfaceContentBuffer,
            tRect.left +  kInterfaceContentBuffer,
            tRect.bottom - kInterfaceContentBuffer);

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
                shade = LIGHT;
            else shade = DARKER;//DARKEST + kSlightlyLighterColor;
        } else
        {
            shade = MEDIUM;
        }
        uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(item.color, shade);
        pix->view(uRect).fill(color);

        if (!item.item.radioButton.on) {
            if (item.item.radioButton.status == kIH_Hilite) {
                color = GetRGBTranslateColorShade(item.color, DARKEST);
            } else if (item.item.radioButton.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.color, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(item.color, LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(item.color, VERY_LIGHT);
        }

        StringList strings(item.item.radioButton.label.stringID);
        StringSlice s = strings.at(item.item.radioButton.label.stringNumber - 1);
        swidth = GetInterfaceStringWidth( s, item.style);
        swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), s, item.style, pix, color);
    } else
    {
        // draw the key code


        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
                shade = VERY_LIGHT;
            else shade = DARK;//DARKEST + kSlightlyLighterColor;
        } else
        {
            shade = MEDIUM + kLighterColor;
        }
        String s;
        GetKeyNumName(item.item.radioButton.key, &s);
        swidth = GetInterfaceFontWidth( item.style) * kMaxKeyNameLength;

        uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        mDrawPuffUpRect( uRect, item.color, shade, pix);

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
                shade = VERY_LIGHT;
            else shade = DARKER;//DARKEST + kSlightlyLighterColor;
        } else
        {
            shade = MEDIUM;
        }
        vRect = Rect(tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
        tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(item.color, shade);
        pix->view(vRect).fill(color);

        swidth = GetInterfaceStringWidth( s, item.style);
        swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
        if (item.item.radioButton.status == kDimmed) {
            color = GetRGBTranslateColorShade(item.color, VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(item.color, DARKEST);
        }

        DrawInterfaceString(
                Point(swidth, uRect.top + GetInterfaceFontAscent(item.style)), s, item.style, pix,
                color);

        // draw the button title
        if (!item.item.radioButton.on) {
            if (item.item.radioButton.status == kIH_Hilite) {
                color = GetRGBTranslateColorShade(item.color, DARKEST);
            } else if (item.item.radioButton.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.color, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(item.color, LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(item.color, VERY_LIGHT);
        }

        {
            StringList strings(item.item.radioButton.label.stringID);
            StringSlice s = strings.at(item.item.radioButton.label.stringNumber - 1);
            swidth = GetInterfaceStringWidth( s, item.style);
            swidth = uRect.right + ( tRect.right - uRect.right) / 2 - swidth / 2;
            sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, item.style, pix, color);
        }
    }
}


void DrawPlayerInterfaceRadioButton(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect, wRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    unsigned char   shade;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = item.bounds;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if ( item.item.radioButton.status == kDimmed)
        shade = VERY_DARK;
    else shade = MEDIUM;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;
    swidth = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
    sheight = (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
            swidth;

    wRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
            tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

    uRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - 2,
            tRect.top + kInterfaceHTop,
            tRect.left + 1,
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);

    if ( item.item.radioButton.status == kIH_Hilite)
    {
        shade = LIGHT;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);

        wRect.left += 2;
        wRect.right += 2;
        FrameOval(pix, wRect, RgbColor::kBlack);
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval(wRect, item.color, shade, pix);

        wRect.inset(3, 3);
        mDrawPuffDownOval(wRect, item.color, shade, pix);
        wRect.inset(1, 1);

        if (!item.item.radioButton.on) {
            PaintOval(pix, wRect, RgbColor::kBlack);
        } else {
            const RgbColor color = GetRGBTranslateColorShade(item.color, VERY_LIGHT);
            PaintOval(pix, wRect, color);
        }
    } else
    {
        if ( item.item.radioButton.status == kDimmed)
            shade = VERY_DARK;
        else shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
        wRect.left += 2;
        wRect.right += 2;
        FrameOval(pix, wRect, RgbColor::kBlack);
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval(wRect, item.color, shade, pix);

        wRect.inset(3, 3);
        mDrawPuffDownOval(wRect, item.color, shade, pix);
        wRect.inset(1, 1);
        if (!item.item.radioButton.on) {
            PaintOval(pix, wRect, RgbColor::kBlack);
        } else if (item.item.radioButton.status == kActive) {
            const RgbColor color = GetRGBTranslateColorShade(item.color, LIGHT);
            PaintOval(pix, wRect, color);
        } else {
            const RgbColor color = GetRGBTranslateColorShade(item.color, MEDIUM);
            PaintOval(pix, wRect, color);
        }
    }

    uRect = Rect(tRect.left +  kInterfaceContentBuffer,
        tRect.top + kInterfaceContentBuffer,
        tRect.left +  kInterfaceContentBuffer,
        tRect.bottom - kInterfaceContentBuffer);

    if ( item.item.radioButton.status == kIH_Hilite)
        shade = LIGHT;
    else shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
    color = GetRGBTranslateColorShade(item.color, shade);
    pix->view(uRect).fill(color);

    if (item.item.radioButton.status == kIH_Hilite) {
        color = GetRGBTranslateColorShade(item.color, DARKEST);
    } else if (item.item.radioButton.status == kDimmed) {
        color = GetRGBTranslateColorShade(item.color, DARK);
    } else {
        color = GetRGBTranslateColorShade(item.color, LIGHT);
    }
    StringList strings(item.item.radioButton.label.stringID);
    StringSlice s = strings.at(item.item.radioButton.label.stringNumber - 1);
    swidth = GetInterfaceStringWidth( s, item.style);
    swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
    sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
    DrawInterfaceString(Point(swidth, sheight), s, item.style, pix, color);
}

void DrawPlayerInterfaceCheckBox(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect, wRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    unsigned char   shade;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = item.bounds;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if ( item.item.checkboxButton.status == kDimmed)
        shade = VERY_DARK;
    else shade = MEDIUM;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;
    swidth = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
    sheight = (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
            swidth;

    wRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
            tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

    uRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset + 2,
            tRect.top + kInterfaceHTop,
            tRect.left + 1,
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);

    if (item.item.checkboxButton.status == kIH_Hilite) {
        shade = LIGHT;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
        mDrawPuffUpRect( wRect, item.color, shade, pix);
        wRect.inset(3, 3);
        mDrawPuffDownRect( wRect, item.color, shade, pix);
        wRect.inset(1, 1);
        if ( !item.item.checkboxButton.on) {
            color = RgbColor::kBlack;
        } else {
            color = GetRGBTranslateColorShade(item.color, VERY_LIGHT);
        }
        pix->view(wRect).fill(color);
    } else {
        if ( item.item.checkboxButton.status == kDimmed)
            shade = VERY_DARK;
        else shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
        mDrawPuffUpRect( wRect, item.color, shade, pix);
        wRect.inset(3, 3);
        mDrawPuffDownRect( wRect, item.color, shade, pix);
        wRect.inset(1, 1);
        if (!item.item.checkboxButton.on) {
            color = RgbColor::kBlack;
        } else if (item.item.checkboxButton.status == kActive) {
            color = GetRGBTranslateColorShade(item.color, LIGHT);
        } else {
            color = GetRGBTranslateColorShade(item.color, MEDIUM);
        }
        pix->view(wRect).fill(color);
    }

    uRect = Rect(tRect.left +  kInterfaceContentBuffer,
        tRect.top + kInterfaceContentBuffer,
        tRect.left +  kInterfaceContentBuffer,
        tRect.bottom - kInterfaceContentBuffer);

    if ( item.item.checkboxButton.status == kIH_Hilite)
        shade = LIGHT;
    else shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
    color = GetRGBTranslateColorShade(item.color, shade);
    pix->view(uRect).fill(color);

    if (item.item.checkboxButton.status == kIH_Hilite) {
        color = GetRGBTranslateColorShade(item.color, DARKEST);
    } else if ( item.item.checkboxButton.status == kDimmed) {
        color = GetRGBTranslateColorShade(item.color, DARK);
    } else {
        color = GetRGBTranslateColorShade(item.color, LIGHT);
    }

    StringList strings(item.item.checkboxButton.label.stringID);
    StringSlice s = strings.at(item.item.checkboxButton.label.stringNumber - 1);
    swidth = GetInterfaceStringWidth( s, item.style);
    swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
    sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
    DrawInterfaceString(Point(swidth, sheight), s, item.style, pix, color);
}

void DrawPlayerInterfaceLabeledBox(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    unsigned char   shade;
    RgbColor        color;

    if (item.style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds;
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer + GetInterfaceFontHeight(item.style) +
            kInterfaceTextVBuffer * 2 + kLabelBottomHeight;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    shade = DARK;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);


    // draw the string

    StringList strings(item.item.labeledRect.label.stringID);
    StringSlice s = strings.at(item.item.labeledRect.label.stringNumber - 1);
    swidth = GetInterfaceStringWidth( s, item.style) + kInterfaceTextHBuffer * 2;
    swidth = ( tRect.right - tRect.left) - swidth;
    sheight = GetInterfaceFontHeight( item.style) + kInterfaceTextVBuffer * 2;

    uRect = Rect(tRect.left + kInterfaceTextHBuffer - 1,
        tRect.top + kInterfaceHTop,
        tRect.right - swidth - kInterfaceTextHBuffer + 1,
        tRect.top + sheight - kInterfaceHTop);
    color = GetRGBTranslateColorShade(item.color, VERY_DARK);
    pix->view(uRect).fill(color);

    color = GetRGBTranslateColorShade(item.color, LIGHT);

    DrawInterfaceString(
            Point(
                tRect.left + kInterfaceTextHBuffer,
                tRect.top + GetInterfaceFontAscent( item.style) + kInterfaceTextVBuffer),
            s, item.style, pix, color);

    // string left border

    shade = MEDIUM;
    vcenter = sheight / 2;

    uRect = Rect(tRect.left - thisHBorder,
            tRect.top + kInterfaceHTop,
            tRect.left + 1, tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade, pix);

    // string right border

    shade = MEDIUM;
    uRect = Rect(tRect.right - swidth,
        tRect.top + kInterfaceHTop,
        tRect.right - 2,
        tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade, pix);
    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade, pix);

    // string bottom border

    mDrawPuffUpTBorder( tRect, uRect, item.color, DARK, sheight, thisHBorder, pix);

    // main part left border

    tRect.top += sheight + kLabelBottomHeight;

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, item.color, DARKER, pix);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, VERY_DARK, pix);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, item.color, DARKER, pix);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, VERY_DARK, pix);
}

void DrawInterfaceTextRect(const interfaceItemType& item, PixMap* pix) {
    Resource rsrc("text", "txt", item.item.textRect.textID);
    String data(macroman::decode(rsrc.data()));
    DrawInterfaceTextInRect(item.bounds, data, item.style, item.color, pix, NULL);
}

void DrawInterfaceTextInRect(
        const Rect& tRect, const StringSlice& text, interfaceStyleType style,
        unsigned char textcolor, PixMap* pix, inlinePictType* inlinePict) {
    RgbColor color;
    color = GetRGBTranslateColorShade(textcolor, VERY_LIGHT);
    InterfaceText interface_text(text, style, color);
    interface_text.wrap_to(tRect.width(), kInterfaceTextHBuffer, kInterfaceTextVBuffer);

    if (inlinePict != NULL) {
        for (size_t i = 0; i < kMaxInlinePictNum; ++i) {
            if (i < interface_text.inline_picts().size()) {
                inlinePict[i] = interface_text.inline_picts()[i];
            } else {
                inlinePict[i].id = -1;
                inlinePict[i].bounds = Rect(0, 0, 0, 0);
            }
        }
    }

    interface_text.draw(pix, tRect);
}

short GetInterfaceTextHeightFromWidth(
        const StringSlice& text, interfaceStyleType style, short boundsWidth) {
    InterfaceText interface_text(text, style, RgbColor::kBlack);
    interface_text.wrap_to(boundsWidth, kInterfaceTextHBuffer, kInterfaceTextVBuffer);
    return interface_text.height();
}

void DrawInterfacePictureRect(const interfaceItemType& item, PixMap* pix) {
    if (item.item.pictureRect.visibleBounds) {
        DrawPlayerInterfacePlainRect(item.bounds, item.color, item.style, pix);
    }

    Picture pict(item.item.pictureRect.pictureID);
    Rect from = pict.size().as_rect();
    Rect to = pict.size().as_rect();
    to.offset(item.bounds.left, item.bounds.top);

    if (to.left < 0) {
        from.left -= to.left;
        to.left -= to.left;
    }
    if (to.top < 0) {
        from.top -= to.top;
        to.top -= to.top;
    }
    if (to.right >= pix->size().width) {
        from.right += (pix->size().width - to.right);
        to.right += (pix->size().width - to.right);
    }
    if (to.bottom >= pix->size().height) {
        from.bottom += (pix->size().height - to.bottom);
        to.bottom += (pix->size().height - to.bottom);
    }

    pix->view(to).copy(pict.view(from));
}

void DrawAnyInterfaceItem(const interfaceItemType& item, PixMap* pix) {
    switch (item.kind) {
        case kPlainRect:
            if (item.item.pictureRect.visibleBounds) {
                DrawPlayerInterfacePlainRect(item.bounds, item.color, item.style, pix);
            }
            break;

        case kTabBox:
            DrawPlayerInterfaceTabBox(
                    item.bounds, item.color, item.style, pix, item.item.tabBox.topRightBorderSize);
            break;

        case kLabeledRect:
            DrawPlayerInterfaceLabeledBox(item, pix);
            break;

        case kListRect:
            throw Exception("Interface type list is no longer supported");
            break;

        case kTextRect:
             DrawInterfaceTextRect(item, pix);
            break;

        case kPlainButton:
            DrawPlayerInterfaceButton(item, pix);
            break;

        case kRadioButton:
            DrawPlayerInterfaceRadioButton(item, pix);
            break;

        case kTabBoxButton:
            DrawPlayerInterfaceTabBoxButton(item, pix);
            break;

        case kCheckboxButton:
            DrawPlayerInterfaceCheckBox(item, pix);
            break;

        case kPictureRect:
            DrawInterfacePictureRect(item, pix);
            break;

        default:
            break;
    }
}

void GetAnyInterfaceItemGraphicBounds(const interfaceItemType& item, Rect *bounds) {
    short   thisHBorder = kInterfaceSmallHBorder;

    *bounds = item.bounds;

    if (item.style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }

    bounds->left -= kInterfaceContentBuffer;
    bounds->top -= kInterfaceContentBuffer;
    bounds->right += kInterfaceContentBuffer + 1;
    bounds->bottom += kInterfaceContentBuffer + 1;

    switch (item.kind) {
        case kPlainRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kLabeledRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= GetInterfaceFontHeight(item.style) + kInterfaceTextVBuffer * 2 +
                            kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kTabBox:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kListRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer * 2 +
                            kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kTextRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kPlainButton:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kRadioButton:
            bounds->left -= bounds->bottom - bounds->top + 2 * kInterfaceVEdgeHeight +
                            2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + thisHBorder +
                            kRadioIndicatorHOffset;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kTabBoxButton:
            bounds->left -= thisHBorder + 5;
            bounds->right += thisHBorder + 5;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
            break;

        case kCheckboxButton:
            bounds->left -= bounds->bottom - bounds->top + 2 * kInterfaceVEdgeHeight +
                            2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + thisHBorder +
                            kCheckIndicatorHOffset;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kPictureRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        default:
            break;
    }
}

void GetAnyInterfaceItemContentBounds(const interfaceItemType& item, Rect *bounds) {
    *bounds = item.bounds;
}

short GetInterfaceStringWidth(const StringSlice& s, interfaceStyleType style) {
    long            width, height;

    SetInterfaceLargeUpperFont( style);
    mGetDirectStringDimensions(s, width, height);

    return ( width);
}

// GetInterfaceFontWidth:       -- NOT WORLD-READY! --
//
//  We're not using fontInfo.widMax because we know we're never going to use the ultra-wide
//  characters like &oelig; and the like, and we're not using a mono-spaced font.  Therefore, we're
//  using the width of 'R' which is about as wide as our normal letters get.
//

short GetInterfaceFontWidth(interfaceStyleType style) {
    SetInterfaceLargeUpperFont(style);
    return gDirectText->logicalWidth;
}

short GetInterfaceFontHeight(interfaceStyleType style) {
    SetInterfaceLargeUpperFont(style);
    return gDirectText->height;
}

short GetInterfaceFontAscent( interfaceStyleType style) {
    SetInterfaceLargeUpperFont(style);
    return gDirectText->ascent;
}

}  // namespace antares
