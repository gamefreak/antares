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

#include "game/admiral.hpp"

#include "data/space-object.hpp"
#include "data/string-list.hpp"
#include "game/cheat.hpp"
#include "game/globals.hpp"
#include "game/space-object.hpp"
#include "lang/casts.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::scoped_array;
using std::min;
namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int32_t kNoFreeAdmiral            = -1;
const int32_t kDestNoObject             = -1;

const int32_t kDestinationNameLen        = 17;
const int32_t kAdmiralNameLen            = 31;

const Fixed kUnimportantTarget          = 0x00000000;
const Fixed kMostImportantTarget        = 0x00000200;
const Fixed kLeastImportantTarget       = 0x00000100;
const Fixed kVeryImportantTarget        = 0x00000160;
const Fixed kImportantTarget            = 0x00000140;
const Fixed kSomewhatImportantTarget    = 0x00000120;
const Fixed kAbsolutelyEssential        = 0x00008000;

scoped_array<destBalanceType> gDestBalanceData;

}  // namespace

void AdmiralInit() {
    globals()->gAdmiralData.reset(new admiralType[kMaxPlayerNum]);
    ResetAllAdmirals();
    gDestBalanceData.reset(new destBalanceType[kMaxDestObject]);
    ResetAllDestObjectData();
}

void AdmiralCleanup() {
    globals()->gAdmiralData.reset();
    gDestBalanceData.reset();
}

void ResetAllAdmirals() {
    admiralType* a = globals()->gAdmiralData.get();

    for (int i = 0; i < kMaxPlayerNum; ++i) {
        a->active = false;
        a->attributes = 0;
        a->destinationObject = kNoDestinationObject;
        a->destinationObjectID = -1;
        a->flagship = kNoShip;
        a->flagshipID = -1;
        a->destType = kNoDestinationType;
        a->considerShip = a->considerDestination = kNoShip;
        a->considerShipID = -1;
        a->buildAtObject = kNoShip;
        a->cash = a->kills = a->losses =a->saveGoal = 0;
        a->thisFreeEscortStrength = a->lastFreeEscortStrength = 0;
        a->blitzkrieg = 0;
        a->shipsLeft = 0;
        for (int j = 0; j < kAdmiralScoreNum; ++j) {
            a->score[j] = 0;
        }

        for (int j = 0; j < kMaxNumAdmiralCanBuild; ++j) {
            a->canBuildType[j].baseNum = -1;
            a->canBuildType[j].base = NULL;
            a->canBuildType[j].chanceRange = -1;
        }
        a++;
        globals()->gActiveCheats[i] = 0;
    }
}

void ResetAllDestObjectData() {
    destBalanceType* d = mGetDestObjectBalancePtr(0);
    for (int i = 0; i < kMaxDestObject; ++i) {
        d->whichObject = kDestNoObject;
        d->name.clear();
        d->earn = d->totalBuildTime = d->buildTime = 0;
        d->buildObjectBaseNum = kNoShip;
        for (int j = 0; j < kMaxTypeBaseCanBuild; ++j) {
            d->canBuildType[j] = kNoShip;
        }
        for (int j = 0; j < kMaxPlayerNum; ++j) {
            d->occupied[j] = 0;
        }
        d++;
    }
}

destBalanceType* mGetDestObjectBalancePtr(long whichObject) {
    return gDestBalanceData.get() + whichObject;
}

admiralType* mGetAdmiralPtr(long mwhichAdmiral) {
    return globals()->gAdmiralData.get() + mwhichAdmiral;
}

long MakeNewAdmiral(
        long flagship, long destinationObject, destinationType dType, unsigned long attributes,
        long race, short nameResID, short nameStrNum, Fixed earningPower) {
    long n = 0;
    spaceObjectType* destObject;

    admiralType* a = globals()->gAdmiralData.get();
    while ((a->active) && (n < kMaxPlayerNum)) {
        a++;
        n++;
    }

    if (n == kMaxPlayerNum) {
        return kNoFreeAdmiral;
    }

    a->active = true;
    a->attributes = attributes;
    a->earningPower = earningPower;

    a->destinationObject = destinationObject;
    if (destinationObject >= 0) {
        destObject = gSpaceObjectData.get() + destinationObject;
        a->destinationObjectID = destObject->id;
    } else {
        a->destinationObjectID = -1;
    }

    a->flagship = flagship;
    if (flagship >= 0) {
        destObject = gSpaceObjectData.get() + flagship;
        a->flagshipID = destObject->id;
    } else {
        a->flagshipID = -1;
    }

    a->destType = dType;
    a->race = race;
    a->color = 0;
    a->blitzkrieg = 1200;  // about a 2 minute blitzkrieg
    a->cash = a->kills = a->losses = a->saveGoal = 0;
    a->thisFreeEscortStrength = a->lastFreeEscortStrength = 0;
    for (int i = 0; i < kAdmiralScoreNum; i++) {
        a->score[i] = 0;
    }
    for (int i = 0; i < kMaxNumAdmiralCanBuild; i++) {
        a->canBuildType[i].baseNum = -1;
        a->canBuildType[i].base = NULL;
        a->canBuildType[i].chanceRange = -1;
    }
    a->totalBuildChance = 0;
    a->hopeToBuild = -1;
    a->shipsLeft = 0;

    if ((nameResID >= 0)) {
        a->name.assign(StringList(nameResID).at(nameStrNum - 1));
        if (a->name.size() > kAdmiralNameLen) {
            a->name.resize(kAdmiralNameLen);
        }
    }
    // for now set strategy balance to 0 -- we may want to calc this if player added on the fly?
    return n;
}

long MakeNewDestination(
        long whichObject, int32_t* canBuildType, Fixed earn, short nameResID,
        short nameStrNum) {
    long i = 0;
    spaceObjectType* object = gSpaceObjectData.get() + whichObject;

    destBalanceType* d = mGetDestObjectBalancePtr(0);
    while ((i < kMaxDestObject) && (d->whichObject != kDestNoObject)) {
        i++;
        d++;
    }

    if (i == kMaxDestObject) {
        return -1;
    } else {
        d->whichObject = whichObject;
        d->earn = earn;
        d->totalBuildTime = d->buildTime = 0;

        if (canBuildType != NULL) {
            for (int j = 0; j < kMaxTypeBaseCanBuild; j++) {
                d->canBuildType[j] = *canBuildType;
                canBuildType++;
            }
        } else {
            for (int j = 0; j < kMaxTypeBaseCanBuild; j++) {
                d->canBuildType[j] = kNoShip;
            }
        }

        if ((nameResID >= 0)) {
            d->name.assign(StringList(nameResID).at(nameStrNum - 1));
            if (d->name.size() > kDestinationNameLen) {
                d->name.resize(kDestinationNameLen);
            }
        }

        if (object->attributes & kNeutralDeath) {
            for (int j = 0; j < kMaxPlayerNum; j++) {
                d->occupied[j] = 0;
            }

            if (object->owner >= 0) {
                d->occupied[object->owner] = object->baseType->initialAgeRange;
            }
        }

        return i;
    }
}

void RemoveDestination(long whichDestination) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);
    admiralType* a;

    if ((whichDestination >= 0) && (whichDestination < kMaxDestObject)) {
        a = globals()->gAdmiralData.get();

        for (int i = 0; i < kMaxPlayerNum; i++) {
            if (a->active) {
                if (a->destinationObject == d->whichObject) {
                    a->destinationObject = kNoDestinationObject;
                    a->destinationObjectID = -1;
                    a->destType = kNoDestinationType;
                }
                if (a->considerDestination == whichDestination) {
                    a->considerDestination = kNoDestinationObject;
                }

                if (a->buildAtObject == whichDestination) {
                    a->buildAtObject = kNoShip;
                }
            }
            a++;
        }

        d->whichObject = kDestNoObject;
        d->name.clear();
        d->earn = d->totalBuildTime = d->buildTime = 0;
        d->buildObjectBaseNum = kNoShip;
        for (int i = 0; i < kMaxTypeBaseCanBuild; i++) {
            d->canBuildType[i] = kNoShip;
        }

        for (int i = 0; i < kMaxPlayerNum; i++) {
            d->occupied[i] = 0;
        }
    }
}

void RecalcAllAdmiralBuildData() {
    admiralType* a = globals()->gAdmiralData.get();
    spaceObjectType* anObject= NULL;
    baseObjectType* baseObject = NULL;
    destBalanceType* d = mGetDestObjectBalancePtr(0);
    long l;

    // first clear all the data
    for (int i = 0; i < kMaxPlayerNum; i++) {
        for (int j = 0; j < kMaxNumAdmiralCanBuild; j++) {
            a->canBuildType[j].baseNum = -1;
            a->canBuildType[j].base = NULL;
            a->canBuildType[j].chanceRange = -1;
        }
        a->totalBuildChance = 0;
        a->hopeToBuild = -1;
        a++;
    }

    for (int i = 0; i < kMaxDestObject; i++) {
        if (d->whichObject != kDestNoObject) {
            anObject = gSpaceObjectData.get() + d->whichObject;
            if (anObject->owner >= 0) {
                a = globals()->gAdmiralData.get() + anObject->owner;
                for (int k = 0; k < kMaxTypeBaseCanBuild; k++) {
                    if (d->canBuildType[k] >= 0) {
                        int j = 0;
                        while ((a->canBuildType[j].baseNum != d->canBuildType[k])
                                && (j < kMaxNumAdmiralCanBuild)) {
                            j++;
                        }
                        if (j == kMaxNumAdmiralCanBuild) {
                            mGetBaseObjectFromClassRace(
                                    baseObject, l, d->canBuildType[k], a->race);
                            j = 0;
                            while ((a->canBuildType[j].baseNum != -1)
                                    && (j < kMaxNumAdmiralCanBuild)) {
                                j++;
                            }
                            if (j == kMaxNumAdmiralCanBuild) {
                                throw Exception("Too Many Types to Build!");
                            }
                            a->canBuildType[j].baseNum = d->canBuildType[k];
                            a->canBuildType[j].base = baseObject;
                            a->canBuildType[j].chanceRange = a->totalBuildChance;
                            if (baseObject != NULL) {
                                a->totalBuildChance += baseObject->buildRatio;
                            }
                        }
                    }
                }
            }
        }
        d++;
    }
}

void SetAdmiralAttributes(long whichAdmiral, unsigned long attributes) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    a->attributes = attributes;
}

void SetAdmiralColor(long whichAdmiral, unsigned char color) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    a->color = color;
}

unsigned char GetAdmiralColor(long whichAdmiral) {
    if (whichAdmiral < 0) {
        return 0;
    }
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    return a->color;
}

long GetAdmiralRace(long whichAdmiral) {
    if (whichAdmiral < 0) {
        return -1;
    }
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    return a->race;
}

void SetAdmiralFlagship(long whichAdmiral, long whichShip) {
    if (whichAdmiral < 0) {
        throw Exception ("Can't set flagship of -1 admiral.");
    }
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    if (whichShip >= 0) {
        a->flagship = whichShip;
        spaceObjectType* anObject = gSpaceObjectData.get() + whichShip;
        a->flagshipID = anObject->id;
    } else {
        a->flagshipID = -1;
    }
}

spaceObjectType* GetAdmiralFlagship(long whichAdmiral) {
    if (whichAdmiral < 0) {
        return NULL;
    }
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    if (a->flagship == kNoShip) {
        return NULL;
    }
    spaceObjectType* anObject = gSpaceObjectData.get() + a->flagship;
    if (anObject->id == a->flagshipID) {
        return anObject;
    } else {
        return NULL;
    }
}

void SetAdmiralEarningPower(long whichAdmiral, Fixed power) {
    if (whichAdmiral >= 0) {
        admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
        a->earningPower = power;
    }
}

Fixed GetAdmiralEarningPower(long whichAdmiral) {
    if (whichAdmiral >= 0) {
        admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
        return a->earningPower;
    } else {
        return 0;
    }
}

void SetAdmiralDestinationObject(long whichAdmiral, long whichObject, destinationType dType) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    a->destinationObject = whichObject;
    if (whichObject >= 0) {
        spaceObjectType* destObject = gSpaceObjectData.get() + whichObject;
        a->destinationObjectID = destObject->id;
    } else {
        a->destinationObjectID = -1;
    }
    a->destType = dType;
}

long GetAdmiralDestinationObject(long whichAdmiral) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;

    if (a->destinationObject < 0) {
        return (a->destinationObject);
    }

    spaceObjectType* destObject = gSpaceObjectData.get() + a->destinationObject;
    if ((destObject->id == a->destinationObjectID)
            && (destObject->active == kObjectInUse)) {
        return a->destinationObject;
    } else {
        a->destinationObject = -1;
        a->destinationObjectID = -1;
        return -1;
    }
}

void SetAdmiralConsiderObject(long whichAdmiral, long whichObject) {
    spaceObjectType* anObject= gSpaceObjectData.get() + whichObject;
    destBalanceType* d = mGetDestObjectBalancePtr(0);

    if (whichAdmiral < 0) {
        throw Exception("Can't set consider ship for -1 admiral.");
    }
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    a->considerShip = whichObject;
    if (whichObject >= 0) {
        a->considerShipID = anObject->id;
        if (anObject->attributes & kCanAcceptBuild) {
            int buildAtNum = 0;
            while ((d->whichObject != whichObject) && (buildAtNum < kMaxDestObject)) {
                buildAtNum++;
                d++;
            }
            if (buildAtNum < kMaxDestObject) {
                int l = 0;
                while ((l < kMaxShipCanBuild) && (d->canBuildType[l] == kNoShip)) {
                    l++;
                }
                if (l < kMaxShipCanBuild) {
                    a->buildAtObject = buildAtNum;
                }
            }
        }
    } else {
        a->considerShipID = -1;
    }
}

bool BaseHasSomethingToBuild(long whichObject) {
    destBalanceType* d = mGetDestObjectBalancePtr(0);
    spaceObjectType* anObject= gSpaceObjectData.get() + whichObject;

    if (anObject->attributes & kCanAcceptBuild) {
        int buildAtNum = 0;
        while ((d->whichObject != whichObject) && (buildAtNum < kMaxDestObject)) {
            buildAtNum++;
            d++;
        }
        if (buildAtNum < kMaxDestObject) {
            int l = 0;
            while ((l < kMaxShipCanBuild) && (d->canBuildType[l] == kNoShip)) {
                l++;
            }
            if (l < kMaxShipCanBuild) {
                return true;
            }
        }
    }
    return false;
}

long GetAdmiralConsiderObject(long whichAdmiral) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    spaceObjectType* anObject;

    if (whichAdmiral < 0) {
        return -1;
    }
    if (a->considerShip >= 0) {
        anObject = gSpaceObjectData.get() + a->considerShip;
        if ((anObject->id == a->considerShipID)
                && (anObject->active == kObjectInUse)
                && (anObject->owner == whichAdmiral)) {
            return a->considerShip;
        } else {
            a->considerShip = -1;
            a->considerShipID = -1;
            return -1;
        }
    } else {
        if (a->considerShip != -1) {
            throw Exception("Strange Admiral Consider Ship");
        }
        return a->considerShip;
    }
}

long GetAdmiralBuildAtObject(long whichAdmiral) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    if (a->buildAtObject >= 0) {
        destBalanceType* destBalance = mGetDestObjectBalancePtr(a->buildAtObject);
        if (destBalance->whichObject >= 0) {
            spaceObjectType* anObject = gSpaceObjectData.get() + destBalance->whichObject;
            if (anObject->owner != whichAdmiral) {
                a->buildAtObject = kNoShip;
            }
        } else {
            a->buildAtObject = kNoShip;
        }
    }
    return a->buildAtObject;
}

void SetAdmiralBuildAtObject(long whichAdmiral, long whichObject) {
    spaceObjectType* anObject = gSpaceObjectData.get() + whichObject;
    destBalanceType* d = mGetDestObjectBalancePtr(0);

    if (whichAdmiral < 0) {
        throw Exception("Can't set consider ship for -1 admiral.");
    }
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    if (whichObject >= 0) {
        if (anObject->attributes & kCanAcceptBuild) {
            int buildAtNum = 0;
            while ((d->whichObject != whichObject) && (buildAtNum < kMaxDestObject)) {
                buildAtNum++;
                d++;
            }
            if (buildAtNum < kMaxDestObject) {
                int l = 0;
                while ((l < kMaxShipCanBuild) && (d->canBuildType[l] == kNoShip)) {
                    l++;
                }
                if (l < kMaxShipCanBuild) {
                    a->buildAtObject = buildAtNum;
                }
            }
        }
    }
}

StringSlice GetAdmiralBuildAtName(long whichAdmiral) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    destBalanceType* destObject = mGetDestObjectBalancePtr(a->buildAtObject);
    return destObject->name;
}

void SetAdmiralBuildAtName(long whichAdmiral, StringSlice name) {
    admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
    destBalanceType* destObject = mGetDestObjectBalancePtr(a->buildAtObject);
    destObject->name.assign(name.slice(0, min<size_t>(name.size(), kDestinationNameLen)));
}

StringSlice GetDestBalanceName(long whichDestObject) {
    destBalanceType* destObject = mGetDestObjectBalancePtr(whichDestObject);
    return (destObject->name);
}

StringSlice GetAdmiralName(long whichAdmiral) {
    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
        return a->name;
    } else {
        return NULL;
    }
}

void SetAdmiralName(long whichAdmiral, StringSlice name) {
    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        admiralType* a = globals()->gAdmiralData.get() + whichAdmiral;
        a->name.assign(name.slice(0, min<size_t>(name.size(), kAdmiralNameLen)));
    }
}

void SetObjectLocationDestination(spaceObjectType *o, coordPointType *where) {
    // if the object does not have an alliance, then something is wrong here--forget it
    if (o->owner <= kNoOwner) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectID = -1;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
        return;
    }

    // if this object can't accept a destination, then forget it
    if (!(o->attributes & kCanAcceptDestination)) {
        return;
    }

    // if this object has a locked destination, then forget it
    if (o->attributes & kStaticDestination) {
        return;
    }

    // if the owner is not legal, something is very very wrong
    if ((o->owner < 0) || (o->owner >= kMaxPlayerNum)) {
        return;
    }

    admiralType* a = globals()->gAdmiralData.get() + o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if (!a->active) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
    } else {
        // the object is OK, the admiral is OK, then go about setting its destination
        if (o->attributes & kCanAcceptDestination) {
            o->timeFromOrigin = kTimeToCheckHome;
        } else {
            o->timeFromOrigin = 0;
        }

        // remove this object from its destination
        if (o->destinationObject != kNoDestinationObject) {
            RemoveObjectFromDestination(o);
        }

        o->destinationLocation = o->originLocation = *where;
        o->destinationObject = kNoDestinationObject;
        o->destObjectPtr = NULL;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
    }
}

void SetObjectDestination(spaceObjectType* o, spaceObjectType* overrideObject) {
    spaceObjectType* dObject = overrideObject;

    // if the object does not have an alliance, then something is wrong here--forget it
    if (o->owner <= kNoOwner) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectID = -1;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
        return;
    }

    // if this object can't accept a destination, then forget it
    if (!(o->attributes & kCanAcceptDestination)) {
        return;
    }

    // if this object has a locked destination, then forget it
    if ((o->attributes & kStaticDestination) && (overrideObject == NULL)) {
        return;
    }

    // if the owner is not legal, something is very very wrong
    if ((o->owner < 0) || (o->owner >= kMaxPlayerNum)) {
        return;
    }

    // get the admiral
    admiralType* a = globals()->gAdmiralData.get() + o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if ((dObject == NULL) &&
            ((!a->active)
             || (a->destType == kNoDestinationType)
             || (a->destinationObject == kNoDestinationObject)
             || (a->destinationObjectID == o->id))) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
    } else {
        // the object is OK, the admiral is OK, then go about setting its destination

        // first make sure we're still looking at the same object
        if (dObject == NULL) {
            dObject = gSpaceObjectData.get() + a->destinationObject;
        }

        if ((dObject->active == kObjectInUse) &&
                ((dObject->id == a->destinationObjectID)
                 || (overrideObject != NULL))) {
            if (o->attributes & kCanAcceptDestination) {
                o->timeFromOrigin = kTimeToCheckHome;
            } else {
                o->timeFromOrigin = 0;
            }
            // remove this object from its destination
            if (o->destinationObject != kNoDestinationObject) {
                RemoveObjectFromDestination(o);
            }

            // add this object to its destination
            if (o != dObject) {
                o->runTimeFlags &= ~kHasArrived;
                o->destinationObject = dObject->entryNumber;
                o->destObjectPtr = dObject;
                o->destObjectDest = dObject->destinationObject;
                o->destObjectDestID = dObject->destObjectID;
                o->destObjectID = dObject->id;

                if (dObject->owner == o->owner) {
                    dObject->remoteFriendStrength += o->baseType->offenseValue;
                    dObject->escortStrength += o->baseType->offenseValue;
                    if (dObject->attributes & kIsDestination) {
                        if (dObject->escortStrength < dObject->baseType->friendDefecit) {
                            o->duty = eGuardDuty;
                        } else {
                            o->duty = eNoDuty;
                        }
                    } else {
                        if (dObject->escortStrength < dObject->baseType->friendDefecit) {
                            o->duty = eEscortDuty;
                        } else {
                            o->duty = eNoDuty;
                        }
                    }
                } else {
                    dObject->remoteFoeStrength += o->baseType->offenseValue;
                    if (dObject->attributes & kIsDestination) {
                        o->duty = eAssaultDuty;
                    } else {
                        o->duty = eAssaultDuty;
                    }
                }
            } else {
                o->destinationObject = kNoDestinationObject;
                o->destObjectDest = kNoDestinationObject;
                o->destObjectPtr = NULL;
                o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
                o->timeFromOrigin = 0;
                o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
                o->originLocation = o->location;
            }
        } else {
            o->destinationObject = kNoDestinationObject;
            o->destObjectDest = kNoDestinationObject;
            o->destObjectPtr = NULL;
            o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
            o->timeFromOrigin = 0;
            o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
            o->originLocation = o->location;
        }
    }
}

void RemoveObjectFromDestination(spaceObjectType* o) {
    spaceObjectType* dObject = o;
    if ((o->destinationObject != kNoDestinationObject) && (o->destObjectPtr != NULL)) {
        dObject = o->destObjectPtr;
        if (dObject->id == o->destObjectID) {
            if (dObject->owner == o->owner) {
                dObject->remoteFriendStrength -= o->baseType->offenseValue;
                dObject->escortStrength -= o->baseType->offenseValue;
            } else {
                dObject->remoteFoeStrength -= o->baseType->offenseValue;
            }
        }
    }

    o->destinationObject = kNoDestinationObject;
    o->destObjectDest = kNoDestinationObject;
    o->destObjectID = -1;
    o->destObjectPtr = NULL;
}

void AdmiralThink() {
    admiralType* a =globals()->gAdmiralData.get();
    spaceObjectType* anObject;
    spaceObjectType* destObject;
    spaceObjectType* otherDestObject;
    spaceObjectType* stepObject;
    destBalanceType* destBalance;
    long origObject, origDest, baseNum, difference;
    Fixed  friendValue, foeValue, thisValue;
    baseObjectType* baseObject;
    Point gridLoc;

    destBalance = mGetDestObjectBalancePtr(0);
    for (int i = 0; i < kMaxDestObject; i++) {
        destBalance->buildTime -= 10;
        if (destBalance->buildTime <= 0) {
            destBalance->buildTime = 0;
            if (destBalance->buildObjectBaseNum != kNoShip) {
                anObject = gSpaceObjectData.get() + destBalance->whichObject;
                AdmiralBuildAtObject(anObject->owner, destBalance->buildObjectBaseNum, i);
                destBalance->buildObjectBaseNum = kNoShip;
            }
        }

        anObject = gSpaceObjectData.get() + destBalance->whichObject;
        if (anObject->owner >= 0) {
            PayAdmiral(anObject->owner, destBalance->earn);
        }
        destBalance++;
    }

    for (int i = 0; i < kMaxPlayerNum; i++) {
        if ((a->attributes & kAIsComputer) && (!(a->attributes & kAIsRemote))) {
            if (a->blitzkrieg > 0) {
                a->blitzkrieg--;
                if (a->blitzkrieg <= 0) {
                    // Really 48:
                    a->blitzkrieg = 0 - (RandomSeeded(1200, &gRandomSeed, 'adm1', -1) + 1200);
                    anObject = gSpaceObjectData.get();
                    for (int j = 0; j < kMaxSpaceObject; j++) {
                        if (anObject->owner == i) {
                            anObject->currentTargetValue = 0x00000000;
                        }
                        anObject++;
                    }
                }
            } else {
                a->blitzkrieg++;
                if (a->blitzkrieg >= 0) {
                    // Really 48:
                    a->blitzkrieg = RandomSeeded(1200, &gRandomSeed, 'adm2', -1) + 1200;
                    anObject = gSpaceObjectData.get();
                    for (int j = 0; j < kMaxSpaceObject; j++) {
                        if (anObject->owner == i) {
                            anObject->currentTargetValue = 0x00000000;
                        }
                        anObject++;
                    }
                }
            }

            // get the current object
            if (a->considerShip < 0) {
                a->considerShip = gRootObjectNumber;
                anObject = gSpaceObjectData.get() + a->considerShip;
                a->considerShipID = anObject->id;
            } else {
                anObject = gSpaceObjectData.get() + a->considerShip;
            }

            if (a->destinationObject < 0) {
                a->destinationObject = gRootObjectNumber;
            }

            if (anObject->active != kObjectInUse) {
                a->considerShip = gRootObjectNumber;
                anObject = gSpaceObjectData.get() + a->considerShip;
                a->considerShipID = anObject->id;
            }

            if (a->destinationObject >= 0) {
                destObject = gSpaceObjectData.get() + a->destinationObject;
                if (destObject->active != kObjectInUse) {
                    destObject = gRootObject;
                    a->destinationObject = gRootObjectNumber;
                }
                origDest = a->destinationObject;
                do {
                    a->destinationObject = destObject->nextObjectNumber;

                    // if we've gone through all of the objects
                    if (a->destinationObject < 0) {
                        // ********************************
                        // SHIP MUST DECIDE, THEN INCREASE CONSIDER SHIP
                        // ********************************
                        if ((anObject->duty != eEscortDuty)
                                && (anObject->duty != eHostileBaseDuty)
                                && (anObject->bestConsideredTargetValue >
                                    anObject->currentTargetValue)) {
                            a->destinationObject = anObject->bestConsideredTargetNumber;
                            a->destType = kObjectDestinationType;
                            if (a->destinationObject >= 0) {
                                destObject = gSpaceObjectData.get() + a->destinationObject;
                                if (destObject->active == kObjectInUse) {
                                    a->destinationObjectID = destObject->id;
                                    anObject->currentTargetValue
                                        = anObject->bestConsideredTargetValue;
                                    thisValue = RandomSeeded(
                                            mFloatToFixed(0.5), &(anObject->randomSeed), 'adm3',
                                            anObject->whichBaseObject) - mFloatToFixed(0.25);
                                    thisValue = mMultiplyFixed(
                                            thisValue, anObject->currentTargetValue);
                                    anObject->currentTargetValue += thisValue;
                                    SetObjectDestination(anObject, NULL);
                                }
                            }
                            a->destType = kNoDestinationType;
                        }

                        if ((anObject->duty != eEscortDuty)
                                && (anObject->duty != eHostileBaseDuty)) {
                            a->thisFreeEscortStrength += anObject->baseType->offenseValue;
                        }

                        anObject->bestConsideredTargetValue = 0xffffffff;
                        // start back with 1st ship
                        a->destinationObject = gRootObjectNumber;
                        destObject = gRootObject;

                        // >>> INCREASE CONSIDER SHIP
                        origObject = a->considerShip;
                        anObject = gSpaceObjectData.get() + a->considerShip;
                        if (anObject->active != kObjectInUse) {
                            anObject = gRootObject;
                            a->considerShip = gRootObjectNumber;
                            a->considerShipID = anObject->id;
                        }
                        do {
                            a->considerShip = anObject->nextObjectNumber;
                            if (a->considerShip < 0) {
                                a->considerShip = gRootObjectNumber;
                                anObject = gRootObject;
                                a->considerShipID = anObject->id;
                                a->lastFreeEscortStrength = a->thisFreeEscortStrength;
                                a->thisFreeEscortStrength = 0;
                            } else {
                                anObject = anObject->nextObject;
                                a->considerShipID = anObject->id;
                            }
                        } while (((anObject->owner != i)
                                    || (!(anObject->attributes & kCanAcceptDestination))
                                    || (anObject->active != kObjectInUse))
                                && (a->considerShip != origObject));
                    } else {
                        destObject = destObject->nextObject;
                    }
                    a->destinationObjectID = destObject->id;
                } while (((!(destObject->attributes & (kCanBeDestination)))
                            || (a->destinationObject == a->considerShip)
                            || (destObject->active != kObjectInUse)
                            || (!(destObject->attributes & kCanBeDestination)))
                        && (a->destinationObject != origDest));

            // if our object is legal and our destination is legal
            if ((anObject->owner == i)
                    && (anObject->attributes & kCanAcceptDestination)
                    && (anObject->active == kObjectInUse)
                    && (destObject->attributes & (kCanBeDestination))
                    && (destObject->active == kObjectInUse)
                    && ((anObject->owner != destObject->owner)
                        || (anObject->baseType->destinationClass <
                            destObject->baseType->destinationClass))) {
                gridLoc = destObject->distanceGrid;
                stepObject = otherDestObject = destObject;
                while (stepObject->nextFarObject != NULL) {
                    if ((stepObject->distanceGrid.h == gridLoc.h)
                            && (stepObject->distanceGrid.v == gridLoc.v)) {
                        otherDestObject = stepObject;
                    }
                    stepObject = stepObject->nextFarObject;
                }
                if (otherDestObject->owner == anObject->owner) {
                    friendValue = otherDestObject->localFriendStrength;
                    foeValue = otherDestObject->localFoeStrength;
                } else {
                    foeValue = otherDestObject->localFriendStrength;
                    friendValue = otherDestObject->localFoeStrength;
                }


                thisValue = kUnimportantTarget;
                if (destObject->owner == anObject->owner) {
                    if (destObject->attributes & kIsDestination) {
                        if (destObject->escortStrength < destObject->baseType->friendDefecit) {
                            thisValue = kAbsolutelyEssential;
                        } else if (foeValue) {
                            if (foeValue >= friendValue) {
                                thisValue = kMostImportantTarget;
                            } else if (foeValue > (friendValue >> 1)) {
                                thisValue = kVeryImportantTarget;
                            } else {
                                thisValue = kUnimportantTarget;
                            }
                        } else {
                            if ((a->blitzkrieg > 0) && (anObject->duty == eGuardDuty)) {
                                thisValue = kUnimportantTarget;
                            } else {
                                if (foeValue > 0) {
                                    thisValue = kSomewhatImportantTarget;
                                } else {
                                    thisValue = kUnimportantTarget;
                                }
                            }
                        }
                        if (anObject->baseType->orderFlags & kTargetIsBase) {
                            thisValue <<= 3;
                        }
                        if (anObject->baseType->orderFlags & kHardTargetIsNotBase) {
                            thisValue = 0;
                        }
                    } else {
                        if (destObject->baseType->destinationClass
                                > anObject->baseType->destinationClass) {
                            if (foeValue > friendValue) {
                                thisValue = kMostImportantTarget;
                            } else {
                                if (destObject->escortStrength
                                        < destObject->baseType->friendDefecit) {
                                    thisValue = kMostImportantTarget;
                                } else {
                                    thisValue = kUnimportantTarget;
                                }
                            }
                        } else {
                            thisValue = kUnimportantTarget;
                        }
                        if (anObject->baseType->orderFlags & kTargetIsNotBase) {
                            thisValue <<= 3;
                        }
                        if (anObject->baseType->orderFlags & kHardTargetIsBase) {
                            thisValue = 0;
                        }
                    }
                    if (anObject->baseType->orderFlags & kTargetIsFriend) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsFoe) {
                        thisValue = 0;
                    }
                } else if (destObject->owner >= 0) {
                    if ((anObject->duty == eGuardDuty) || (anObject->duty == eNoDuty)) {
                        if (destObject->attributes & kIsDestination) {
                            if (foeValue < friendValue) {
                                thisValue = kMostImportantTarget;
                            } else {
                                thisValue = kSomewhatImportantTarget;
                            }
                            if (a->blitzkrieg > 0) {
                                thisValue <<= 2;
                            }
                            if (anObject->baseType->orderFlags & kTargetIsBase) {
                                thisValue <<= 3;
                            }

                            if (anObject->baseType->orderFlags & kHardTargetIsNotBase) {
                                thisValue = 0;
                            }
                        } else {
                            if (friendValue) {
                                if (friendValue < foeValue) {
                                    thisValue = kSomewhatImportantTarget;
                                } else {
                                    thisValue = kUnimportantTarget;
                                }
                            } else {
                                thisValue = kLeastImportantTarget;
                            }
                            if (anObject->baseType->orderFlags & kTargetIsNotBase) {
                                thisValue <<= 1;
                            }

                            if (anObject->baseType->orderFlags & kHardTargetIsBase) {
                                thisValue = 0;
                            }
                        }
                    }
                    if (anObject->baseType->orderFlags & kTargetIsFoe) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsFriend) {
                        thisValue = 0;
                    }
                } else {
                    if (destObject->attributes & kIsDestination) {
                        thisValue = kVeryImportantTarget;
                        if (a->blitzkrieg > 0) {
                            thisValue <<= 2;
                        }
                        if (anObject->baseType->orderFlags & kTargetIsBase) {
                            thisValue <<= 3;
                        }
                        if (anObject->baseType->orderFlags & kHardTargetIsNotBase) {
                            thisValue = 0;
                        }
                    } else {
                        if (anObject->baseType->orderFlags & kTargetIsNotBase) {
                            thisValue <<= 3;
                        }
                        if (anObject->baseType->orderFlags & kHardTargetIsBase) {
                            thisValue = 0;
                        }
                    }
                    if (anObject->baseType->orderFlags & kTargetIsFoe) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsFriend) {
                        thisValue = 0;
                    }
                }

                difference = ABS(implicit_cast<long>(destObject->location.h)
                        - implicit_cast<long>(anObject->location.h));
                gridLoc.h = difference;
                difference =  ABS(implicit_cast<long>(destObject->location.v)
                        - implicit_cast<long>(anObject->location.v));
                gridLoc.v = difference;

                if ((gridLoc.h < kMaximumRelevantDistance)
                        && (gridLoc.v < kMaximumRelevantDistance)) {
                    if (anObject->baseType->orderFlags & kTargetIsLocal) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsRemote) {
                        thisValue = 0;
                    }
                } else {
                    if (anObject->baseType->orderFlags & kTargetIsRemote) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsLocal) {
                        thisValue = 0;
                    }
                }


                if (((anObject->baseType->orderFlags & kLevelKeyTagMask) != 0)
                        && ((anObject->baseType->orderFlags & kLevelKeyTagMask)
                            == (destObject->baseType->buildFlags & kLevelKeyTagMask))) {
                    thisValue <<= 3;
                } else if (anObject->baseType->orderFlags & kHardMatchingFoe) {
                    thisValue = 0;
                }

                if (thisValue > 0) {
                    thisValue += RandomSeeded(
                            thisValue >> 1, &(anObject->randomSeed), 'adm4',
                            anObject->whichBaseObject) - (thisValue >> 2);
                }
                if (thisValue > anObject->bestConsideredTargetValue) {
                    anObject->bestConsideredTargetValue = thisValue;
                    anObject->bestConsideredTargetNumber = a->destinationObject;
                }
            }
        }

        // if we've saved enough for our dreams
        if (a->cash > a->saveGoal) {
                a->saveGoal = 0;

                // consider what ship to build
                if (a->buildAtObject < 0) {
                    a->buildAtObject = 0;
                }
                origDest = a->buildAtObject;
                destBalance = mGetDestObjectBalancePtr(a->buildAtObject);

                // try to find the next destination object that we own & that can build
                do {
                    a->buildAtObject++;
                    destBalance++;
                    if (a->buildAtObject >= kMaxDestObject) {
                        a->buildAtObject = 0;
                        destBalance = mGetDestObjectBalancePtr(0);
                    }
                    if (destBalance->whichObject >= 0) {
                        anObject = gSpaceObjectData.get() + destBalance->whichObject;
                        if ((anObject->owner != i)
                                || (!(anObject->attributes & kCanAcceptBuild))) {
                            anObject = NULL;
                        }
                    } else anObject = NULL;
                } while ((anObject == NULL) && (a->buildAtObject != origDest));

                // if we have a legal object
                if (anObject != NULL) {
                    if (destBalance->buildTime <= 0) {
                        if (a->hopeToBuild < 0) {
                            int k = 0;
                            while ((a->hopeToBuild < 0) && (k < 7)) {
                                k++;
                                // choose something to build
                                thisValue = RandomSeeded(
                                        a->totalBuildChance, &gRandomSeed, 'adm5', -1);
                                friendValue = 0xffffffff; // equals the highest qualifying object
                                for (int j = 0; j < kMaxNumAdmiralCanBuild; ++j) {
                                    if ((a->canBuildType[j].chanceRange <= thisValue)
                                            && (a->canBuildType[j].chanceRange > friendValue)) {
                                        friendValue = a->canBuildType[j].chanceRange;
                                        a->hopeToBuild = a->canBuildType[j].baseNum;
                                    }
                                }
                                if (a->hopeToBuild >= 0) {
                                    mGetBaseObjectFromClassRace(
                                            baseObject, baseNum, a->hopeToBuild, a->race);
                                    if (baseObject->buildFlags & kSufficientEscortsExist) {
                                        anObject = gSpaceObjectData.get();
                                        int j = 0;
                                        while (j < kMaxSpaceObject) {
                                            if ((anObject->active)
                                                    && (anObject->owner == i)
                                                    && (anObject->whichBaseObject == baseNum)
                                                    && (anObject->escortStrength <
                                                        baseObject->friendDefecit)) {
                                                a->hopeToBuild = -1;
                                                j = kMaxSpaceObject;
                                            }
                                            j++;
                                            anObject++;
                                        }
                                    }

                                    if (baseObject->buildFlags & kMatchingFoeExists) {
                                        thisValue = 0;
                                        anObject = gSpaceObjectData.get();
                                        for (int j = 0; j < kMaxSpaceObject; j++) {
                                            if ((anObject->active)
                                                    && (anObject->owner != i)
                                                    && ((anObject->baseType->buildFlags
                                                            & kLevelKeyTagMask)
                                                        == (baseObject->orderFlags
                                                            & kLevelKeyTagMask))) {
                                                thisValue = 1;
                                            }
                                            anObject++;
                                        }
                                        if (!thisValue) {
                                            a->hopeToBuild = -1;
                                        }
                                    }
                                }
                            }
                        }
                        int j = 0;
                        while ((destBalance->canBuildType[j] != a->hopeToBuild)
                                && (j < kMaxTypeBaseCanBuild)) {
                            j++;
                        }
                        if ((j < kMaxTypeBaseCanBuild) && (a->hopeToBuild != kNoShip)) {
                            mGetBaseObjectFromClassRace(baseObject, baseNum, a->hopeToBuild,
                                    a->race);
                            if (a->cash >= mLongToFixed(baseObject->price)) {
                                AdmiralScheduleBuild(i, j);
                                a->hopeToBuild = -1;
                                a->saveGoal = 0;
                            } else {
                                a->saveGoal = mLongToFixed(baseObject->price);
                            }
                        } // otherwise just wait until we get to it
                    }
                }
            }
        }
        a++;
    }
}

Fixed HackGetObjectStrength(spaceObjectType* anObject) {
    spaceObjectType* tObject = anObject;
    Point           gridLoc;
    long            owner = anObject->owner;

    gridLoc = anObject->distanceGrid;
    while (anObject->nextFarObject != NULL) {
        if ((anObject->distanceGrid.h == gridLoc.h) && (anObject->distanceGrid.v == gridLoc.v)) {
            tObject = anObject;
        }
        anObject = anObject->nextFarObject;
    }

    if (tObject->owner == owner) {
        return (tObject->localFriendStrength - tObject->localFoeStrength
                + tObject->escortStrength);
    } else {
        return (tObject->localFoeStrength - tObject->localFriendStrength
                - tObject->escortStrength);
    }
}

// assumes you can afford it & base has time
void AdmiralBuildAtObject(long whichAdmiral, long baseTypeNum, long whichDestObject) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;
    destBalanceType* buildAtDest = mGetDestObjectBalancePtr(whichDestObject);
    spaceObjectType* buildAtObject = NULL;
    long            newObject;
    coordPointType  coord;
    fixedPointType  v = {0, 0};

    if ((baseTypeNum >= 0) && (admiral->buildAtObject >= 0)) {
        buildAtObject = gSpaceObjectData.get() + buildAtDest->whichObject;
        coord = buildAtObject->location;

        newObject = CreateAnySpaceObject(baseTypeNum, &v, &coord, 0, whichAdmiral, 0, -1);

        if (newObject >= 0) {
            buildAtObject = gSpaceObjectData.get() + newObject;
            SetObjectDestination(buildAtObject, NULL);
            if (whichAdmiral == globals()->gPlayerAdmiralNumber) {
                PlayVolumeSound(kComputerBeep2, kMediumVolume, kMediumPersistence,
                        kLowPrioritySound);
            }
        }
    }
}

bool AdmiralScheduleBuild(long whichAdmiral, long buildWhichType) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;
    destBalanceType* buildAtDest = mGetDestObjectBalancePtr(admiral->buildAtObject);
    baseObjectType* buildBaseObject = NULL;
    long            baseNum;

    GetAdmiralBuildAtObject(whichAdmiral);
    if ((buildWhichType >= 0)
            && (buildWhichType < kMaxTypeBaseCanBuild)
            && (admiral->buildAtObject >= 0) && (buildAtDest->buildTime <= 0)) {
        mGetBaseObjectFromClassRace(buildBaseObject, baseNum,
                buildAtDest->canBuildType[buildWhichType], admiral->race);
        if ((buildBaseObject != NULL) && (buildBaseObject->price <= mFixedToLong(admiral->cash))) {
            admiral->cash -= (mLongToFixed(buildBaseObject->price));
            if (globals()->gActiveCheats[whichAdmiral] & kBuildFastBit) {
                buildAtDest->buildTime = 9;
                buildAtDest->totalBuildTime = 9;
            } else {
                buildAtDest->buildTime = buildBaseObject->buildTime;
                buildAtDest->totalBuildTime = buildAtDest->buildTime;
            }
            buildAtDest->buildObjectBaseNum = baseNum;
            return true;
        }
    }
    return false;
}

void StopBuilding(long whichDestObject) {
    destBalanceType* destObject;

    destObject = mGetDestObjectBalancePtr(whichDestObject);
    destObject->totalBuildTime = destObject->buildTime = 0;
    destObject->buildObjectBaseNum = kNoShip;
}

void PayAdmiral(long whichAdmiral, Fixed howMuch) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        admiral->cash += mMultiplyFixed(howMuch, admiral->earningPower);
    }
}

void PayAdmiralAbsolute(long whichAdmiral, Fixed howMuch) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        admiral->cash += howMuch;
        if (admiral->cash < 0) {
            admiral->cash = 0;
        }
    }
}

void AlterAdmiralScore(long whichAdmiral, long whichScore, long amount) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)
            && (whichScore >= 0) && (whichScore < kAdmiralScoreNum)) {
        admiral->score[whichScore] += amount;
    }
}

long GetAdmiralScore(long whichAdmiral, long whichScore) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)
            && (whichScore >= 0) && (whichScore < kAdmiralScoreNum)) {
        return admiral->score[whichScore];
    } else {
        return 0;
    }
}

long GetAdmiralShipsLeft(long whichAdmiral) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        return admiral->shipsLeft;
    } else {
        return 0;
    }
}

long AlterDestinationObjectOccupation(long whichDestination, long whichAdmiral, long amount) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);

    if (whichAdmiral >= 0) {
        d->occupied[whichAdmiral] += amount;
        return(d->occupied[whichAdmiral]);
    } else {
        return -1;
    }
}

void ClearAllOccupants(long whichDestination, long whichAdmiral, long fullAmount) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);

    for (int i = 0; i < kMaxPlayerNum; i++) {
        d->occupied[i] = 0;
    }
    if (whichAdmiral >= 0) {
        d->occupied[whichAdmiral] = fullAmount;
    }
}

void AddKillToAdmiral(spaceObjectType* anObject) {
    // only for player
    admiralType* admiral = globals()->gAdmiralData.get() + globals()->gPlayerAdmiralNumber;

    if (anObject->attributes & kCanAcceptDestination) {
        if (anObject->owner == globals()->gPlayerAdmiralNumber) {
            admiral->losses++;
        } else {
            admiral->kills++;
        }
    }
}

long GetAdmiralLoss(long whichAdmiral) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        return admiral->losses;
    } else {
        return 0;
    }
}

long GetAdmiralKill(long whichAdmiral) {
    admiralType* admiral = globals()->gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        return admiral->kills;
    } else {
        return 0;
    }
}

}  // namespace antares
