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

#ifndef ANTARES_GAME_SCENARIO_MAKER_HPP_
#define ANTARES_GAME_SCENARIO_MAKER_HPP_

#include "data/scenario.hpp"
#include "data/space-object.hpp"
#include "drawing/shapes.hpp"
#include "game/globals.hpp"
#include "game/space-object.hpp"
#include "lang/casts.hpp"

namespace antares {

const int16_t kScenarioNoShipTextID = 10000;

extern const Scenario* gThisScenario;

enum {
    kDestroyActionType = 1,
    kExpireActionType = 2,
    kCreateActionType = 3,
    kCollideActionType = 4,
    kActivateActionType = 5,
    kArriveActionType = 6
};

Scenario* mGetScenario(int32_t num);
int32_t mGetRealAdmiralNum(int32_t mplayernum);

void ScenarioMakerInit();
bool ConstructScenario(const Scenario* scenario);
void DeclareWinner(int32_t whichPlayer, int32_t nextLevel, int32_t textID);
void CheckScenarioConditions(int32_t timePass);
int32_t GetRealAdmiralNumber(int32_t whichAdmiral);
void UnhideInitialObject(int32_t whichInitial);
spaceObjectType *GetObjectFromInitialNumber(int32_t initialNumber);
void GetScenarioFullScaleAndCorner(
        const Scenario* scenario, int32_t rotation, coordPointType *corner, int32_t *scale,
        Rect *bounds);
const Scenario* GetScenarioPtrFromChapter(int32_t chapter);
coordPointType Translate_Coord_To_Scenario_Rotation(int32_t h, int32_t v);

}  // namespace antares

#endif // ANTARES_GAME_SCENARIO_MAKER_HPP_
