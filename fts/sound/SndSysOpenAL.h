/*
 * \file SndSysOpenAL.h
 *
 * \author kabey
 * \brief Interface file to the concrete OpenAL sound system class.
 */
#pragma once

#include <set>

#include "main/Exception.h"
#include "logging/logger.h"
#include "SndSys.h"
#include "SndGrp.h"
#include <stack>

#if D_SND_SYS == D_FTS_OpenAL
#include "sndobjpool.h"

namespace FTS {
    class SndPlayList;
    class ISndObj;

//! Exception class to get out the error messages of the sound system.
/// This exception is thrown when there is some hardware limit reached, making
/// the call fail, for example no more memory.
class OpenALException : public virtual SndErrException {
    OpenALException() throw();
public:
    OpenALException(const String& in_sObjectName) throw();
    OpenALException(const String& in_sObjectName, ALenum in_ErrCode) throw();
};

//! Sound system class which is the anchor of all sound objects.
// It initialize the under laying sound system. In the current implementation
// OpenAL is used. This class needs only one instance.
class SndSysOpenAL : public ISndSys
{
public:
    SndSysOpenAL();
    virtual ~SndSysOpenAL();
    String getType() {return "OpenAL";};
    ISndObj* CreateSndObj(SndGroup::Enum in_enumGroup, const Path& in_filename, SndPlayMode::Enum in_enumMode = SndPlayMode::Single);
    void setVolume(float volume, SndGroup::Enum in_enumGroup = SndGroup::All);
    SndGrp* getGroup(SndGroup::Enum in_enumGroup);
    bool update(const Clock& c);
    void Mute(bool in_mute = true);
    void setListenerPos(const Vector* in_vect, const Vector* in_vel = NULL);
    void setListenerDirection(const Vector* in_forward, const Vector* in_up);
    void attach(Updateable* in_pObjToUpdate) ;
    void detach(Updateable* in_pObjToUpdate) ;
    SndObjPool& getSndPool() {return m_sndPool;}
    void popContext();
    void pushContext();
private:
    SndGrp* m_groups[SndGroup::All]; ///< All the groups in the game. \see eSndGroup.
    bool m_bLoaded;
    ALfloat m_vol; ///< Listener volume.
    std::set<Updateable*> m_objectsToUpdate;
    SndObjPool m_sndPool;
    class Context {
    public:
        SndGrp* m_groups[SndGroup::All]; ///< All the groups in the game. \see eSndGroup.
        std::set<Updateable*> m_objectsToUpdate;
        ALfloat m_vol; ///< Listener volume.
        SndObjPool m_sndPool;
    };
    std::stack<Context> m_saveContext;
};

/*! Returns the sound group object of the specified sound type.
 *
 * \author Klaus Beyer
 *
 * \param[in] in_enumGroup Sound group type to be returned.
 * \return SndGrp * or NULL if in_enumGroup is eSndGrpMax.
 */
inline SndGrp* SndSysOpenAL::getGroup(SndGroup::Enum in_enumGroup)
{
    if(in_enumGroup < SndGroup::All)
        return m_groups[in_enumGroup];

    return NULL;
}

} // namespace FTS

#endif // D_SND_SYS == D_FTS_OpenAL
