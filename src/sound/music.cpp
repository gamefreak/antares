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

#include "sound/music.hpp"

#include "config/preferences.hpp"
#include "sound/driver.hpp"

using sfz::format;
using sfz::scoped_ptr;

namespace antares {

namespace {

bool playing = false;
scoped_ptr<Sound> song;
scoped_ptr<SoundChannel> channel;

}  // namespace

void MusicInit() {
    playing = false;
    song.reset();
    SoundDriver::driver()->open_channel(channel);
}

void MusicCleanup() {
    channel->quiet();
    channel.reset();
    song.reset();
    playing = false;
}

void PlaySong() {
    channel->activate();
    song->loop();
    playing = true;
}

void StopSong() {
    channel->quiet();
    playing = false;
}

void ToggleSong() {
    if (playing) {
        StopSong();
    } else {
        PlaySong();
    }
}

bool SongIsPlaying() {
    return playing;
}

void StopAndUnloadSong() {
    StopSong();
    song.reset();
}

void LoadSong(int id) {
    StopSong();
    SoundDriver::driver()->open_sound(format("/music/{0}.mp3", id), song);
}

void SetSongVolume(double volume) {
    channel->amp(255 * volume);
}

}  // namespace antares
