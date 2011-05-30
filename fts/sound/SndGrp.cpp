/*
 * \file SndGrp.cpp
 *
 * \author kabey
 * \brief Implementation of sound group class.
 * \todo Set alSourcei(AL_SOURCE_RELATIVE, AL_FALSE) on music objects
 */

#if defined(TEST_SOUND)
#  include "TestSound/TestSound.h"
#  include <string.h>
#endif

#include "fts_Snd.h"

#include <algorithm>

namespace FTS {

/*! ctor.
 *
 * \author Klaus.Beyer
 *
 * \param[in] ID   The SndGroup ident.
 * \return void
 */
SndGrp::SndGrp(SndGroup::Enum ID)
    : m_sndgrpID(ID)
    , m_vol(1.0)
{
}
/*! dtor.
 *
 *  Unloads all sounds that are still loaded in the group.
 *
 * \author Klaus.Beyer
 *
 * \return void
 */
SndGrp::~SndGrp()
{
    ISndObj *pObj = NULL;
    while(!m_snds.empty()) {
        std::map<String, ISndObj*>::iterator i = m_snds.begin();
        pObj = i->second;
        SAFE_DELETE(pObj); // This automatically takes pObj out of m_snds.
        // So DON'T (!!!) iterate over the m_snds here!
    }
    m_snds.clear();
}


/*! Sets the volume of a group of sound objects. \sa SndGroup
 *
 * \author Klaus.Beyer
 *
 * \param[in] volume   volume 0 ... 1.0
 * \return void
 */
void SndGrp::setVolume(float volume)
{
    m_vol = volume;
    std::for_each(m_snds.begin(), m_snds.end(), [volume](std::pair<String, ISndObj*> i) 
    {
        i.second->setVolume(volume);
    }
    );
}

/*! Mutes all sound objects of a group. 
 *
 * \author Klaus.Beyer
 *
 * \return void
 */
void SndGrp::Mute(bool in_muteState)
{
    std::for_each(m_snds.begin(), m_snds.end(), [in_muteState](std::map<String, ISndObj*>::value_type i){
        i.second->Mute(in_muteState);
    });
}

/*! Pause all playing sound objects of a group. 
 *
 * \author Klaus.Beyer
 *
 * \return void
 */
void SndGrp::Pause()
{
    std::for_each(m_snds.begin(), m_snds.end(), [](std::map<String, ISndObj*>::value_type i){
        if( i.second->getState() == SndObjState::Playing ) 
            i.second->Pause();
    });
}

/*! Resume all paused sound objects of a group. 
 *
 * \author Klaus.Beyer
 *
 * \return void
 */
void SndGrp::Resume()
{
    std::for_each(m_snds.begin(), m_snds.end(), [](std::map<String, ISndObj*>::value_type i){
        if( i.second->getState() == SndObjState::Paused ) 
            i.second->Continue();
    });
}

/*! Check if a sound object is present in this group.
 *
 * \author Pompei2
 *
 * \param[in] in_sName   The name of the sound object to find in this group.
 * \return true if the sound object is present, false if not.
 */
bool SndGrp::isSndObjPresent(const String &in_sName)
{
    std::map<String, ISndObj*>::iterator i = m_snds.find(in_sName);

    return i != m_snds.end();
}

/*! Add a sound object to this group
 * If the sound object is not yet in this group, it will be added. Note
 * that this function does not set the sound-object's group member,
 * thus it is HIGHLY RECOMMENDED to use the soundobject's setgroup function.
 *
 * If the sound object is already present in this group, nothing is done.
 *
 * \author Pompei2
 *
 * \param[in] so   The sound object to register in this group.
 * \return this
 */
SndGrp *SndGrp::registerSndObj(ISndObj* so)
{
    if(so == NULL)
        return this;

    if(this->isSndObjPresent(so->getName()))
        return this;

    m_snds[so->getName()] = so;
    return this;
}

/*! Remove a sound object from this group
 * The sound object is removed from this group, but the sound-object itself
 * stays intact. It will not be destroyed.
 *
 * \author Pompei2
 *
 * \param[in] so   The sound object to remove from this group.
 * \return this
 */
SndGrp *SndGrp::unregisterSndObj(ISndObj* so)
{
    if(so == NULL)
        return this;

    // Not in the list - everything is ok ... or not ?
    if(!this->isSndObjPresent(so->getName()))
        return this;

    m_snds.erase(so->getName());
    return this;
}

/*! Get a sound object from the group.
 * This gives you a pointer to a sound object with a certain name that is within
 * the group. If there is no such object in the group, a dummy object is returned.
 *
 * \author Pompei2
 *
 * \param[in] so   The sound object to remove from this group.
 * \return this
 */
ISndObj *SndGrp::getSndObj(const String &in_sName)
{
    if(!this->isSndObjPresent(in_sName))
        return ISndSys::getSingleton().getDummyObj();

    return m_snds[in_sName];
}
}