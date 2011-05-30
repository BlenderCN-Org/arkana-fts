/*
 * \file SndObjOpenAL.h
 * \author KaBey
 * \brief Interface file for the open AL based sound object
 */
#pragma once

#include "SndTypes.h"

#include "dLib/dString/dPath.h"
#include "3d/math/Vector.h"
#include "sound/SndObj.h"

namespace FTS {

#if D_SND_SYS == D_FTS_OpenAL
/*! The base sound object class represents basic sound functionality.
 * This implementation uses the OpenAL to access the local sound system.
 * \author Klaus.Beyer (KaBey)
 * \todo Set min max distance.
 * \note All member functions throw SndErrException.
 */
class SndObjOpenAL : public ISndObj
{
public:
    SndObjOpenAL();
    SndObjOpenAL(const Path& in_sName, SndGroup::Enum in_Group = SndGroup::Music);
    virtual ISndObj* Load(const Path& in_sName);
    virtual ISndObj* Play();
    virtual ISndObj* Pause();
    virtual ISndObj* Continue();
    virtual ISndObj* Stop();
    virtual ISndObj* setGroup(SndGroup::Enum in_Group);
    virtual ISndObj* setPos(const Vector* in_pos);
    virtual ISndObj* setPlayMode(SndPlayMode::Enum in_mode);
    virtual ISndObj* setVolume(float in_vol);
    virtual ISndObj* Mute(bool in_muteState = true);
    virtual void Unload();
    virtual SndObjState::Enum getState();
    virtual ~SndObjOpenAL();
protected:
    ALuint getSource()const {return m_source;};
    ISndObj* removeFromGroup(SndGroup::Enum in_Group);
    ISndObj* addToGroup(SndGroup::Enum in_Group);

private:
    ALuint m_source;
    ALfloat m_vol;
};

/*! Pause/suspend
 *
 * @author Klaus.Beyer
 *
 * @return SndObj * this
 */
inline ISndObj* SndObjOpenAL::Pause()
{
    alSourcePause(m_source);
    return this;
}

/*! Continue to play the sound from the position where it was paused.
 *
 * @author Klaus.Beyer
 *
 * @return SndObj * this
 */
inline ISndObj* SndObjOpenAL::Continue()
{
    if( getState() != SndObjState::Playing) {
        alSourcePlay(m_source);
    }
    return this;
}
/*! Stop playing the sound. The internal channel could be reused by the sound system.
 *
 * @author Klaus.Beyer
 *
 * @return SndObj * this
 */
inline ISndObj* SndObjOpenAL::Stop()
{
    alSourceStop(m_source);
    return this;
}


#endif
} // namespace FTS
