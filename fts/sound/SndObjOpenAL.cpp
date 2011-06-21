/*
 * \file SndObjOpenAL.cpp
 *
 * \author kabey
 * \brief Implementing the base sound object.
 *
 */

#include "utilities/utilities.h"
#include "fts_Snd.h"
#include "sound/SndObjOpenAL.h"
#include "sndobjpool.h"
#include "SndSysOpenAL.h"
#include <assert.h>

namespace FTS {

#if D_SND_SYS == D_FTS_OpenAL
/*! ctor
 *
 * \author Klaus.Beyer
 *
 * \param[in] in_sName Sound file to load from file system.
 * \param[in] in_Group This SndObj belongs to SndGrp. Default is eSndGrpMusic.
 * \return void
 */
SndObjOpenAL::SndObjOpenAL(const Path& in_sName, SndGroup::Enum in_Group)
    : ISndObj(in_sName,in_Group)
    , m_source(0)
    , m_vol(1.0)
{
}

/*! Default ctor.
 *
 * \author Klaus.Beyer
 *
 * \return void
 * \note The sFileName has to be set w/ Load() later on.
 */
SndObjOpenAL::SndObjOpenAL()
    : ISndObj()
    , m_source(0)
    , m_vol(1.0)
{
}

/*! dtor
 *
 * \author Klaus.Beyer
 *
 * \return void
 */
SndObjOpenAL::~SndObjOpenAL()
{
    this->removeFromGroup(m_grp);
    Unload();
}

/*! Load a sound file and start it in paused mode. The Play command would actually start
 * playing the sound.
 *
 * \author Klaus.Beyer
 *
 * \param[in] in_sName to be played.
 * \return SndObj * this
 * \note If filename is  not empty it overwrites the one defined by ctor call.\n
 *       A sound of type eSndGrpMusic is loaded as stream.
 * \throw LoggableException any exception that may come from the SndFile constructor.
 * \throw InvalidCallException when called with an empty string argument.
 * \remarks Throws an invalid param exception if m_sFileName and in_sFileName is empty.\n
 *          If it is an already used object. It is released and loaded w/ the new data.
 */
ISndObj* SndObjOpenAL::Load(const Path& in_sName)
{
    assert(alGetError() == AL_NONE);
    m_pos = Vector(0.,0.,0.);
    m_mode = SndPlayMode::Single ;
    this->Unload();
    if(!in_sName.isEmpty()) {
        m_sName = in_sName;
    }
    if(m_sName.isEmpty()) {
        throw InvalidCallException("SndObjOpenAL::Load(NULL)");
    }
    SndSysOpenAL* pSndSys = dynamic_cast<SndSysOpenAL*> (ISndSys::getSingletonPtr());
    SndObjPool& pool = pSndSys->getSndPool();
    pool.add(m_sName);
    alGenSources(1, &m_source);
    alSourcei(this->getSource(), AL_BUFFER, pool.get(m_sName));
    switch(m_grp) { /// \todo FIXME: shouldn't this be based on something else? Or be the other way around? Do we even need this?
        case SndGroup::Music:
            break;
        default:
            alSourcei(this->getSource(), AL_LOOPING, AL_TRUE);
            break;
    }
    assert(alGetError() == AL_NONE);
    return this;
}

/*! Play the sound of this object on a free channel. Usually is the sound started in paused mode
 * by the load command.
 *
 * \author Klaus.Beyer
 *
 * \return SndObj * this
 */
ISndObj* SndObjOpenAL::Play()
{
    assert(alGetError() == AL_NONE);
    alSourcePlay(this->getSource());
    assert(alGetError() == AL_NONE);
    return this;
}

/*! Sets the position in 3D space and the velocity relative to the listener.
 *
 * \author Klaus.Beyer
 *
 * \param[in] in_pos vector defining the 3D position in space. Units are meters.
 * \return SndObj * this
 * \note The distance factor has to be set in SndSystem objects.
 */
ISndObj* SndObjOpenAL::setPos(const Vector* in_pos)
{
//     assert(alGetError() == AL_NONE);
    Vector vel = (*in_pos - m_pos) * static_cast<float>(m_chron.reset());

    m_pos = *in_pos;
    alSource3f(this->getSource(), AL_POSITION, m_pos.x(), m_pos.y(), m_pos.z());
    alSource3f(this->getSource(), AL_VELOCITY, vel.x(), vel.y(), vel.z());
    assert(alGetError() == AL_NONE);
    return this;
}

/*! Sets the sound group of the object.
 *
 * \author KaBey
 *
 * \param[in] in_Group The sound group
 * \return SndObj *
 */
inline ISndObj* SndObjOpenAL::setGroup(SndGroup::Enum in_Group)
{
    this->removeFromGroup(m_grp);
    this->addToGroup(in_Group);
    m_grp = in_Group;
    return this;
}

/*! Set the play mode. The mode is defined by the enum SndMode.
 *
 * \author Klaus.Beyer
 *
 * \param[in] in_mode enum of possible sound modes
 * \return SndObj * this
 *
 * \throw InvalidCallException if called with a bad mode.
 */
ISndObj* SndObjOpenAL::setPlayMode(SndPlayMode::Enum in_mode)
{
    assert(alGetError() == AL_NONE);
    switch(in_mode) {
        case SndPlayMode::Repeat:
            alSourcei(this->getSource(), AL_LOOPING, AL_TRUE);
            break;
        case SndPlayMode::Single:
            alSourcei(this->getSource(), AL_LOOPING, AL_FALSE);
            break;
        default:
            throw InvalidCallException("setPlayMode(bad mode enum)");
            break;
    }
    assert(alGetError() == AL_NONE);
    return this;
}

/*! Close the sound object. All sub objects are closed. This object can be reused
 * w/ new data.
 *
 * \author Klaus.Beyer
 *
 * \return void
 * \note
 */
void SndObjOpenAL::Unload()
{
    assert(alGetError() == AL_NONE);

    if(this->getSource()) {
        this->Stop();
        alSourcei(this->getSource(), AL_BUFFER, AL_NONE); //detach the current buffer from the source.
        alDeleteSources(1, &m_source);
        m_source = 0 ;
        SndSysOpenAL* pSndSys = dynamic_cast<SndSysOpenAL*> (ISndSys::getSingletonPtr());
        SndObjPool& pool = pSndSys->getSndPool();
        pool.remove(m_sName);
    }


    ALenum err = 0;
    if((err = alGetError()) != AL_NONE )
        throw OpenALException(m_sName, err);
}

/*! Set volume.
 *
 * \author Klaus.Beyer
 *
 * \param[in,out] in_vol 0..1
 * \return SndObj *
 */
ISndObj* SndObjOpenAL::setVolume(float in_vol)
{
    assert(alGetError() == AL_NONE);
    alSourcef(this->getSource(), AL_GAIN, (ALfloat)in_vol);
    m_vol = in_vol;
    assert(alGetError() == AL_NONE);
    return this;
}

/*! Set sound in mute state.
 *
 * \author Klaus.Beyer
 *
 * \return SndObj *
 */
ISndObj* SndObjOpenAL::Mute(bool in_muteState)
{
    assert(alGetError() == AL_NONE);
    if(in_muteState)
        alSourcef(this->getSource(), AL_GAIN, 0.0);
    else
        alSourcef(this->getSource(), AL_GAIN, m_vol);
    assert(alGetError() == AL_NONE);
    return this;
}

/*! Get the current state of the sound object.
 *
 * \author Klaus.Beyer
 *
 * \return SndObjState
 */
SndObjState::Enum SndObjOpenAL::getState()
{
    ALint v;
    alGetSourcei(getSource(), AL_SOURCE_STATE , &v);
    switch(v) {
        case AL_INITIAL:
        default:
            return SndObjState::Nop;
        case AL_PLAYING:
            return SndObjState::Playing;
        case AL_PAUSED:
            return SndObjState::Paused;
        case AL_STOPPED:
            return SndObjState::Stopped;
    }
}

/*! Removes this sound object from a group. For internal use only, because
 * it does not update its group member variable.
 *
 * \author Pompei2
 *
 * \param[in] in_Group the Group ID.
 * \return SndObj *
 */
ISndObj *SndObjOpenAL::removeFromGroup(SndGroup::Enum in_Group)
{
    SndGrp *grp = ISndSys::getSingleton().getGroup(in_Group);
    if(grp)
        grp->unregisterSndObj(this);
    return this;
}

/*! Adds this sound object to a group. For internal use only, because
 * it does not update its group member variable.
 *
 * \author Pompei2
 *
 * \param[in] in_Group the Group ID.
 * \return SndObj *
 */
ISndObj *SndObjOpenAL::addToGroup(SndGroup::Enum in_Group)
{
    SndGrp *grp = ISndSys::getSingleton().getGroup(in_Group);
    if(grp)
        grp->registerSndObj(this);
    return this;
}

#endif

}