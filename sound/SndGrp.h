/*
 * $Id: $
 * \file SndGrp.h
 *
 * \author kabey
 * \brief Interface file to the sound group class.
 */
#pragma once
#if !defined(TEST_SOUND)
#  include "logging/logger.h"
#endif

#include "SndTypes.h"
#include "dLib/dString/dString.h"

#include <map>

namespace FTS {
    class ISndObj;

//! Every sound object belongs to a group defined by SndGroup::Enum
class SndGrp {
public:
    SndGrp(SndGroup::Enum ID) ;
    ~SndGrp();
    void setVolume(float volume);
    float getVolume() const {return m_vol;};
    void Mute(bool in_muteState = true);
    void Pause();
    void Resume();
    SndGrp *registerSndObj(ISndObj* so);
    SndGrp *unregisterSndObj(ISndObj* so);
    ISndObj *getSndObj(const String &in_sName);

protected:
    bool isSndObjPresent(const String &in_sName);

private:
    SndGroup::Enum m_sndgrpID;///< enum SndGroup
    std::map<String, ISndObj*> m_snds;
    float m_vol;
};

} // namespace FTS
