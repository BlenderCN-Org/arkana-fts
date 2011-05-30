/*
 * \file SndSysNone.h
 *
 * \author kabey
 * \brief Interface file to the sound system class which has the none or empty implementation.
 */
#pragma once

#include <set>

#include "main/Exception.h"
#include "logging/logger.h"
#include "SndGrp.h"
#include "sound/SndSys.h"
#include "sound/SndObjNone.h"

namespace FTS {
    class ISndObj;
    class Vector;

class SndSysNone : public ISndSys
{
public:
    SndSysNone() : m_group(new SndGrp(SndGroup::Music))
    {
        /// FIXME: do we really need this in the none snd sys?
        UpdateableManager::getSingletonPtr()->add("SoundSystem", this);
    }
    virtual ~SndSysNone()
    {
        delete m_group;
        UpdateableManager::getSingletonPtr()->rem("SoundSystem");
    }
    virtual String getType() {return "None";}
    virtual ISndObj* CreateSndObj(SndGroup::Enum in_enumGroup, const Path& in_filename, SndPlayMode::Enum in_enumMode = SndPlayMode::Single);
    virtual void setVolume(float volume, SndGroup::Enum in_enumGroup = SndGroup::All) {};
    virtual SndGrp* getGroup(SndGroup::Enum in_enumGroup) {return m_group;}
    virtual bool update(const Clock&) {return true;}
    virtual void Mute(bool in_mute = true) {return; }
    virtual void setListenerPos(const Vector* in_vect, const Vector* in_vel = NULL) {}
    virtual void setListenerDirection(const Vector* in_forward, const Vector* in_up) {}
    virtual void attach(Updateable* in_pObjToUpdate) {}
    virtual void detach(Updateable* in_pObjToUpdate) {}
    virtual void popContext() {};
    virtual void pushContext() {};
private:
    SndGrp* m_group;
};

inline ISndObj* SndSysNone::CreateSndObj(SndGroup::Enum in_enumGroup, const Path& in_filename, SndPlayMode::Enum in_enumMode)
{
    return new SndObjNone;
}

} // namespace FTS