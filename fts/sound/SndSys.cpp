/*
 * $Id: $
 * \file SndSys.cpp
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
#include "sound/SndSysNone.h"
#include "dLib/dConf/configuration.h"

namespace FTS {

SndErrException::SndErrException() throw()
    : LoggableException(new I18nLoggerCmd("SND", MsgType::Horror, "SndErrException()", "shound never be called!"))
{
}

SndErrException::SndErrException(const String& in_sObjectName, const String& in_sInfo) throw()
    : LoggableException(new I18nLoggerCmd("SND", MsgType::Error, in_sObjectName, in_sInfo))
{
}

/*! Create a new sound system.
 *  This function creates the right sound system depending on
 *  the settings and also reads the volumes from the settings
 *  and sets everything up correctly.
 *
 * @author Pompei2
 *
 * @return ISndSys * The generated sound system
 *
 * @note This ALWAYS creates a sound system. On failure, it creates
 *       a SndSysNone object.
 */
ISndSys *ISndSys::createSoundSys()
{
    if(FTS::ISndSys::getSingletonPtr())
        delete FTS::ISndSys::getSingletonPtr();

#if defined(TEST_SND)
    return new SndSysOpenAL;
#else
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

#if D_SND_SYS == D_FTS_OpenAL

    // Use The OpenAL driver if the sound is enabled in the options.
    if(conf.getBool("SoundEnabled")) {
        // If we have sound installed the ctor should work. Otherwise an exception is
        // generated. Then the dummy sound system is used.
        // On linux this usually fails if the dsp device is busy.
        try {
            new SndSysOpenAL;

            ISndSys::getSingletonPtr()->setVolume(getNormalizedVolume("SoundVolumeMusic"), SndGroup::Music);
            ISndSys::getSingletonPtr()->setVolume(getNormalizedVolume("SoundVolumeSFXUnitReaction"), SndGroup::UnitReaction);
            ISndSys::getSingletonPtr()->setVolume(getNormalizedVolume("SoundVolumeSFXEnvironment"), SndGroup::Environment);
            ISndSys::getSingletonPtr()->setVolume(getNormalizedVolume("SoundVolumeSFXAction"), SndGroup::UnitAction);
            ISndSys::getSingletonPtr()->setVolume(getNormalizedVolume("SoundVolumeSFXMagic"), SndGroup::Magic);
            ISndSys::getSingletonPtr()->setVolume(getNormalizedVolume("SoundVolumeSFXAttention"), SndGroup::Attention);

            return FTS::ISndSys::getSingletonPtr();
        } catch(const LoggableException &e) {
            e.show();
        }
    }
#endif

    // Nothing was successful until here, create a dummy sound system.
    new SndSysNone;

    // Only display the warning if the user wanted to have sound, if he didn't
    // want any sound, it is normal that we create the dummy sound system here!
    if(conf.getBool("SoundEnabled"))
        FTS18N("SND_NoSys", MsgType::Warning);

    return FTS::ISndSys::getSingletonPtr();
#endif // TEST_SND
}

    float ISndSys::getNormalizedVolume(const String& in_optName)
    {
        Configuration conf ("conf.xml", ArkanaDefaultSettings());

        int iVol = conf.getInt(in_optName);
        float vol = float( iVol < 0 ? 0 : (iVol > 100 ? 100 : iVol) );
        return vol /  100.f ;
    }

/// Default base-class constructor. Just creates the dummy object.
ISndSys::ISndSys()
    : m_pDummyObject(new SndObjNone)
{
}

/// Default base-class destructor. Just destroys the dummy object.
ISndSys::~ISndSys()
{
    SAFE_DELETE(m_pDummyObject);
}

/*! Get a sound object from a certain group.
 *  This searches for a sound object with a certain name (the filename it has
 *  been created with) in a certain group and returns it. If it is not found
 *  in that group, a dummy sound object is returned.
 *
 * @author Pompei2
 */
ISndObj* ISndSys::getSndObj(SndGroup::Enum in_eGroup, const String &in_sName)
{
    SndGrp* grp = this->getGroup(in_eGroup);
    if(grp == NULL)
        return this->getDummyObj();

    return grp->getSndObj(in_sName);
}

}
