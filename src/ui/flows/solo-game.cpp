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

#include "ui/flows/solo-game.hpp"

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/input-source.hpp"
#include "game/main.hpp"
#include "game/scenario-maker.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "ui/screens/debriefing.hpp"
#include "ui/screens/scroll-text.hpp"
#include "ui/screens/select-level.hpp"
#include "video/transitions.hpp"

using sfz::Exception;
using sfz::format;

namespace antares {

namespace {

const int32_t kHackLevelMax = 26;

}  // namespace

SoloGame::SoloGame()
        : _state(NEW),
          _game_result(NO_GAME) { }

SoloGame::~SoloGame() { }

void SoloGame::become_front() {
    switch (_state) {
      case NEW:
        _state = SELECT_LEVEL;
        stack()->push(new SelectLevelScreen(&_cancelled, &_scenario));
        break;

      case SELECT_LEVEL:
        _state = START_LEVEL;
        if (_cancelled) {
            _state = QUIT;
            stack()->pop(this);
            break;
        }
        // else fall through.

      case START_LEVEL:
        _state = PROLOGUE;
        if (_scenario->prologue_id() > 0) {
            stack()->push(new ScrollTextScreen(_scenario->prologue_id(), 450, 15.0, 4002));
            break;
        }
        // else fall through

      case PROLOGUE:
      case RESTART_LEVEL:
        _state = PLAYING;
        _game_result = NO_GAME;
        globals()->gInputSource.reset(new UserInputSource());
        stack()->push(new MainPlay(_scenario, false, &_game_result));
        break;

      case PLAYING:
        handle_game_result();
        break;

      case EPILOGUE:
        epilogue_done();
        break;

      case QUIT:
        stack()->pop(this);
        break;
    }
}

void SoloGame::handle_game_result() {
    switch (_game_result) {
      case WIN_GAME:
        {
            _state = EPILOGUE;
            const int epilogue = _scenario->epilogue_id();
            if (epilogue > 0) {
                // normal scrolltext song
                int scroll_song = 4002;
                if (globals()->gScenarioWinner.next == -1) {
                    // we win but no next level? Play triumph song
                    scroll_song = 4003;
                }
                stack()->push(new ScrollTextScreen(epilogue, 450, 15.0, scroll_song));
            } else {
                become_front();
            }
        }
        break;

      case RESTART_GAME:
        _state = RESTART_LEVEL;
        become_front();
        break;

      case QUIT_GAME:
        _state = QUIT;
        become_front();
        break;

      default:
        throw Exception(format("_play_again was invalid after PLAY_AGAIN ({0})", _play_again));
    }
}

void SoloGame::epilogue_done() {
    if (Preferences::preferences()->play_idle_music()) {
        StopAndUnloadSong();
    }

    if (globals()->gScenarioWinner.next == -1) {
        _scenario = NULL;
    } else {
        _scenario = GetScenarioPtrFromChapter(globals()->gScenarioWinner.next);
    }

    if (_scenario != NULL) {
        const int32_t chapter = _scenario->chapter_number();
        if (chapter >= 0) {
            Ledger::ledger()->unlock_chapter(chapter);
        } else {
            _scenario = NULL;
        }
    }

    if (_scenario != NULL) {
        _state = START_LEVEL;
    } else {
        _state = QUIT;
    }
    become_front();
}

}  // namespace antares
