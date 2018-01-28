/*
 * \file SndObj.h
 * \author KaBey
 * \brief Interface file for the base sound object
 */
#pragma once

#include "SndTypes.h"

#include "dLib/dString/dPath.h"
#include "3d/Math.h"
#include "logging/Chronometer.h"

namespace FTS {

/*! The sound object interface class. This needs an implementation of a real sound
 * object class which then realize the sounds according to a specific sound architecture.
 */
class ISndObj
{
public:
    ISndObj() {};
    ISndObj(const Path& in_sName, SndGroup::Enum in_Group = SndGroup::Music) : m_sName(in_sName),m_grp(in_Group) {};
    virtual ISndObj* Load(const Path& in_sName = String::EMPTY) = 0;
    virtual ISndObj* Play() = 0;
    virtual ISndObj* Pause() = 0;
    virtual ISndObj* Continue() = 0;
    virtual ISndObj* Stop() = 0;
    virtual ISndObj* setGroup(SndGroup::Enum in_Group) = 0;
    virtual ISndObj* setPos(const Vector* in_pos) = 0;
    virtual ISndObj* setPlayMode(SndPlayMode::Enum in_mode) = 0 ;
    virtual ISndObj* setVolume(float in_vol) = 0 ;
    virtual ISndObj* Mute(bool in_muteState = true) = 0 ;
    virtual void Unload() = 0;
    virtual SndObjState::Enum getState() = 0;
    virtual ~ISndObj(){};
    inline String getName() const{return m_sName;};
protected:
    Vector m_pos;                                   ///< Current position in the 3D space.
    Chronometer m_chron;                            ///< Chronometer to calculate the movement speed.
    Path m_sName;                                   ///< Name of the sound file. (Relative to the sound directory)
    SndGroup::Enum m_grp = SndGroup::Music;         ///< Sound group \see SndGroup
    SndPlayMode::Enum m_mode = SndPlayMode::Single; ///< Sound play mode \see SndPlayMode
};

}