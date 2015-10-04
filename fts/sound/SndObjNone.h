/*
 * $Id: $
 * \file SndObj.h
 * \author KaBey
 * \brief Interface file for the dummy sound object
 */

#pragma once

#include "sound/SndObj.h"

namespace FTS {
/*! The empty or dummy sound object class. It does nothing only an sound object w/o any
 * attached sound system.
 * This implementation can be used if no sound system is available.
 * \author Klaus.Beyer (KaBey)
 */
class SndObjNone : public ISndObj
{
public:
    SndObjNone():ISndObj(){};
    SndObjNone(const Path& in_sFilename, SndGroup::Enum in_Group = SndGroup::Music)
        : ISndObj(in_sFilename, in_Group) {};
    virtual ISndObj* Load(const Path& in_sFilename) {return this;};
    virtual ISndObj* Play() {return this;}
    virtual ISndObj* Pause() {return this;}
    virtual ISndObj* Continue() {return this;}
    virtual ISndObj* Stop() {return this;}
    virtual ISndObj* setGroup(SndGroup::Enum in_Group) {return this;}
    virtual ISndObj* setPos(const Vector* in_pos) {return this;}
    virtual ISndObj* setPlayMode(SndPlayMode::Enum in_mode) {return this;}
    virtual ISndObj* setVolume(float in_vol) {return this;}
    virtual ISndObj* Mute(bool in_muteState = true) {return this;}
    virtual void Unload() {};
    virtual SndObjState::Enum getState() {return SndObjState::Nop;}
    virtual ~SndObjNone() {};
};

}