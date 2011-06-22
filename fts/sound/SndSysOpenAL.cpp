/*
 * \file SndSysOpenAL.cpp
 *
 * \author kabey
 * \brief Implementation of sound system class.
 * \todo - save/restore state of sound system.
 *       - Playlist for music title which can run in the background
 *
 */

#include"utilities/utilities.h"
#include "logging/logger.h"

#include "fts_Snd.h"
#include "sound/SndObjNone.h"
#include "sound/SndObjOpenAL.h"
#include <assert.h>

namespace FTS {

#if D_SND_SYS == D_FTS_OpenAL

OpenALException::OpenALException() throw()
    : LoggableException(new I18nLoggerCmd("SND", MsgType::Horror, "OpenALException()", "Should never get called"))
{
}

OpenALException::OpenALException(const String& in_sObjectName) throw()
    : LoggableException(new I18nLoggerCmd("SND", MsgType::Error, in_sObjectName, alGetString(alGetError())))
{
}

OpenALException::OpenALException(const String& in_sObjectName, ALenum in_ErrCode) throw()
    : LoggableException(new I18nLoggerCmd("SND", MsgType::Error, in_sObjectName, alGetString(in_ErrCode)))
{
}

/*! ctor. Sets up the sound system and creates all sound groups defined by eSndGroup.
 *
 * \author kabey
 * \todo Use the device enumerator extension to get the soud device.
 * \remark Throws a SndException if the sound sub system can't be loaded.
 */
SndSysOpenAL::SndSysOpenAL()
    : ISndSys()
    , m_bLoaded(false)
    , m_vol(1.0)
{
    ALCcontext *Context;
    ALCdevice *Device;
    int argc = 1;
    char *argv = (char *)"";

    // initialize alut
    alutInitWithoutContext(&argc,&argv);

    //initialize OpenAL

    //Open device
    Device = alcOpenDevice(NULL);
    if(!Device)
        throw OpenALException("Initializing the system (Device)");

    //Create context(s)
    Context=alcCreateContext(Device,NULL);
    if(!Context)
        throw OpenALException("Initializing the system (Context)");

    //Set active context
    if(!alcMakeContextCurrent(Context))
        throw OpenALException("Initializing the system (Making the context current)");

    // Clear Error Code
    alGetError();

    // create the sound groups of the game
    m_groups[SndGroup::Music] = new SndGrp(SndGroup::Music);
    m_groups[SndGroup::UnitReaction] = new SndGrp(SndGroup::UnitReaction);
    m_groups[SndGroup::UnitAction] = new SndGrp(SndGroup::UnitAction);
    m_groups[SndGroup::Environment] = new SndGrp(SndGroup::Environment);
    m_groups[SndGroup::Attention] = new SndGrp(SndGroup::Attention);
    m_groups[SndGroup::Magic] = new SndGrp(SndGroup::Magic);
    m_bLoaded = true;

    UpdateableManager::getSingletonPtr()->add("SoundSystem", this);
}

/*! dtor
 *
 * \author kabey
 *
 */
SndSysOpenAL::~SndSysOpenAL()
{
    if(!m_bLoaded)
        return ;

    for(int i=SndGroup::Music; i<SndGroup::All ; i++) {
        delete m_groups[i];
    }

    // Shutdown
    ALCcontext *Context = alcGetCurrentContext();
    assert(Context);
    ALCdevice *Device = Context ? alcGetContextsDevice(Context) : NULL;
     assert(Device);

    //Disable context
    alcMakeContextCurrent(NULL);
    //Release context(s)
    if(Context)
        alcDestroyContext(Context);
    //Close device
    if(Device)
        alcCloseDevice(Device);

    alutExit();
    UpdateableManager::getSingletonPtr()->rem("SoundSystem");
}

/*! Helper to create a sound object in one step.
 *
 * \author Klaus.Beyer
 *
 * \param[in] in_enumGroup Sound group @see eSndGroup
 * \param[in] in_sName name of the sound file to load, relative to the sound directory.
 * \return ISndObj * The generated sound object, could be a SndObjDummy* in case
 *        of any error while creating the sound object.
 * \note Any error by the SndObj generates a SndErrExpection. The exception
 *       is catched and the error message is written to the log file. Then
 *       a SndObjDummy is returned.
 */
ISndObj* SndSysOpenAL::CreateSndObj(SndGroup::Enum in_enumGroup, const Path& in_sName, SndPlayMode::Enum in_enumMode)
{
    ISndObj* so = NULL;
    try {
        so = new SndObjOpenAL(in_sName);

        so->setGroup(in_enumGroup)
          ->Load()
          ->setPlayMode(in_enumMode)
          ->setVolume(m_groups[in_enumGroup]->getVolume());
    } catch(const LoggableException &e) {
        e.show();
        SAFE_DELETE(so);
        so = new SndObjNone();
    }
    assert( so != NULL );
    return so;
}

/*! Set the volume of a group or of all groups.
 *
 * \author Klaus Beyer
 *
 * \param[in] in_enumGroup Group to set the volume. SndGrp::All sets the volume to all groups.
 * \param[in] volume Volume to set (0..1).
 */
void SndSysOpenAL::setVolume(float volume, SndGroup::Enum in_enumGroup)
{
    assert((volume >= 0.0) && (volume <= 1.));

    if(in_enumGroup==SndGroup::All) {
        m_vol = volume ;
        alListenerf(AL_GAIN, m_vol); // volume for all.
        for(int i=SndGroup::Music; i<SndGroup::All ; i++) {
            m_groups[i]->setVolume(m_vol);
        }
    } else {
        m_groups[in_enumGroup]->setVolume(volume);
    }
}

/*! Update all sound objects created throu the sound system.\n
 * On OpenAL this is a dummy function. However, it is needed to update
 * the sound system play list.
 * \author kabey
 *
 * \return void
 * \note This should be called on the main (game) update loop.
 */
bool SndSysOpenAL::update(const FTS::Clock& c)
{
    if(m_objectsToUpdate.empty() ) return true; // fast path return.
    std::set<FTS::Updateable*>::iterator i;
    for( i = m_objectsToUpdate.begin(); i != m_objectsToUpdate.end() ; ++i ) {
        (*i)->update(c);
    }
    return true;
}

/*! Mute the sound system.
 *
 * \author kabey
 * \param[in] in_mute   Set the mute state. true=on; false=off
 * \return void
 * \note The listener volume is set to zero.
 */
void SndSysOpenAL::Mute(bool in_mute)
{
    assert(alGetError() == AL_NONE);
    if(in_mute) {
        alListenerf(AL_GAIN, 0); // the listener won't hear anything.
    } else {
        alListenerf(AL_GAIN, m_vol ); // restore state.
    }
    assert(alGetError() == AL_NONE);
    return;
}

/*! Set the position of the listener.
 *
 * \author Klaus Beyer
 *
 * \param[in] in_vect The new position of the listener
 * \param[in] in_vel  The new velocity of the listener
 * \return void
 * \remarks The velocity in an optional parameter,
 */
void SndSysOpenAL::setListenerPos(const Vector* in_vect, const Vector* in_vel)
{
    assert(alGetError() == AL_NONE);
    alListener3f(AL_POSITION, in_vect->x(), in_vect->y(), in_vect->z());
    if( in_vel )
        alListener3f(AL_VELOCITY, in_vel->x(),in_vel->y(), in_vel->z());
    assert(alGetError() == AL_NONE);
}
/*! Set the direction of the listener.
 *
 * \author Klaus Beyer
 *
 * \param[in] in_forward The new forward direction of the listener (OpenAL: at)
 * \param[in] in_up The new up direction of the listener
 * \return void
 */
void SndSysOpenAL::setListenerDirection(const Vector* in_forward, const Vector* in_up)
{
    assert(alGetError() == AL_NONE);
    float direction[6] ;
    direction[0] = in_forward->x() ;
    direction[1] = in_forward->y() ;
    direction[2] = in_forward->z() ;
    direction[3] = in_up->x() ;
    direction[4] = in_up->y() ;
    direction[5] = in_up->z() ;
    alListenerfv(AL_ORIENTATION, direction);
    assert(alGetError() == AL_NONE);
}
/*! Attach an object to the update list. On every Update() the object's Update method is called.
 *  \sa detach()
 *
 * \author kabey
 * \param[in] in_pObjToUpdate The sound updateable object to be added.
 */
void SndSysOpenAL::attach(Updateable* in_pObjToUpdate)
{
    if(in_pObjToUpdate == NULL) return;
    if(m_objectsToUpdate.find(in_pObjToUpdate) == m_objectsToUpdate.end()) {
        m_objectsToUpdate.insert(in_pObjToUpdate);
    }
}
/*! Detach an object from the update list.
 *  \sa attach()
 *
 * \author kabey
 * \param[in] in_pObjToUpdate The sound updateable object to be removed from the list.
 *
 */
void SndSysOpenAL::detach(Updateable* in_pObjToUpdate)
{
    m_objectsToUpdate.erase(in_pObjToUpdate);
}

/*! Restore the previous context of the sound system.
 * All loaded sounds are destroyed.
 * \author Klaus Beyer
 */
void SndSysOpenAL::popContext()
{
    assert(!m_saveContext.empty());
    Context ctx = m_saveContext.top();
    m_saveContext.pop();
    m_objectsToUpdate = ctx.m_objectsToUpdate;
    m_vol = ctx.m_vol;
    for( int i = SndGroup::Music ; i < SndGroup::All ; ++i ) {
        delete m_groups[i];
    }
    m_sndPool = ctx.m_sndPool;
    for( int i = SndGroup::Music ; i < SndGroup::All ; ++i ) {
        m_groups[i] = ctx.m_groups[i];
        m_groups[i]->Resume();
    }

}

/*! Save the context of the sound system. In order to have a fresh system.
 * It is used to save the current state when a map is loaded. The map should
 * have it's own sound environment.
 *
 * \author Klaus Beyer
 */
void SndSysOpenAL::pushContext()
{
    Context ctx;
    ctx.m_objectsToUpdate = m_objectsToUpdate;
    ctx.m_vol = m_vol;
    for( int i = SndGroup::Music ; i < SndGroup::All ; ++i ) {
        m_groups[i]->Pause();
        ctx.m_groups[i] = m_groups[i];
        m_groups[i] = new SndGrp(SndGroup::Enum(i));
    }
    ctx.m_sndPool = m_sndPool;
    m_sndPool.clear();
    m_saveContext.push(ctx);
}
#endif
}