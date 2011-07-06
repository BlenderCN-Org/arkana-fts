/*
 * $Id: $
 * \file SndSys.h
 *
 * \author kabey
 * \brief Interface file to the sound system class.
 * \todo Rename createsndobj to get or load & implement sound pool for all loaded sounds which can be
 * get on getorload obj.
 */
#pragma once

#include <set>

#include "main/Exception.h"
#include "main/Updateable.h"
#include "main/Stackable.h"
#include "logging/logger.h"
#include "SndGrp.h"
#include "3d/Mathfwd.h"

namespace FTS {
    class SndPlayList;
    class ISndObj;

//! Exception class to get out the error messages of the sound system.
/// This exception is thrown when there is some hardware limit reached, making
/// the call fail, for example no more memory.
class SndErrException : public virtual LoggableException {
protected:
    SndErrException() throw();
public:
    SndErrException(const String& in_sObjectName, const String& in_sInfo) throw();
};

/*! Interface class to the sound system. This need a implementation class for the
 * real access to the used sound architecture. The primary implementation is the
 * class SndSys which is on top of the OpenAL.
 * \author Klaus.Beyer
 */
class ISndSys : public Singleton<ISndSys> , public Updateable, public Stackable
{
protected:
    ISndSys();

public:
    static ISndSys* createSoundSys();
    virtual ~ISndSys();

    virtual String getType() = 0;
    virtual ISndObj* CreateSndObj(SndGroup::Enum in_enumGroup, const Path& in_filename, SndPlayMode::Enum in_enumMode = SndPlayMode::Single) = 0;
    virtual void setVolume(float volume, SndGroup::Enum in_enumGroup = SndGroup::All)  = 0;
    virtual SndGrp* getGroup(SndGroup::Enum in_enumGroup) = 0;
    virtual void Mute(bool in_mute = true) = 0;
    virtual void setListenerPos(const Vector* in_vect, const Vector* in_vel = NULL) = 0;
    virtual void setListenerDirection(const Vector* in_forward , const Vector* in_up) = 0;
    virtual void attach(Updateable* in_pObjToUpdate) = 0;
    virtual void detach(Updateable* in_pObjToUpdate) = 0;
    ISndObj* getSndObj(SndGroup::Enum in_eGroup, const String &in_sName);
    inline ISndObj* getDummyObj() {return m_pDummyObject;};
private:
    ISndObj *m_pDummyObject; ///< Used to not return NULL on failure in some methods.
    static float getNormalizedVolume(const String& in_optName);
};

} // namespace FTS