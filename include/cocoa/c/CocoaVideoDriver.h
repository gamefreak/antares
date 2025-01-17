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

#ifndef ANTARES_COCOA_C_COCOA_VIDEO_DRIVER_H_
#define ANTARES_COCOA_C_COCOA_VIDEO_DRIVER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void antares_menu_bar_hide();
void antares_menu_bar_show();
int64_t antares_double_click_interval_usecs();

typedef struct AntaresEventTranslator AntaresEventTranslator;
AntaresEventTranslator* antares_event_translator_create(
        int32_t screen_width, int32_t screen_height);
void antares_event_translator_destroy(AntaresEventTranslator* translator);

void antares_get_mouse_location(AntaresEventTranslator* translator, int32_t* x, int32_t* y);
void antares_get_mouse_button(AntaresEventTranslator* translator, int32_t* button);

void antares_event_translator_set_mouse_down_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int button, int32_t x, int32_t y, void* userdata), void* userdata);
void antares_event_translator_set_mouse_up_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int button, int32_t x, int32_t y, void* userdata), void* userdata);
void antares_event_translator_set_mouse_move_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int32_t x, int32_t y, void* userdata), void* userdata);
void antares_event_translator_set_key_down_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int32_t key, void* userdata), void* userdata);
void antares_event_translator_set_key_up_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int32_t key, void* userdata), void* userdata);

void antares_event_translator_enqueue(AntaresEventTranslator* translator, int64_t until);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANTARES_COCOA_C_COCOA_VIDEO_DRIVER_H_
