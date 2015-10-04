/**
 * $Id: $
 * \file SndTypes.h
 *
 * \author kabey
 * \brief Definition of types, enums etc used in the sound system.
 */
#pragma once

namespace FTS {

//! Defines the kind of sound.
namespace SndType {
    enum Enum {
        Music, ///< music title like an mp3, which represents a stream.
        Sound, ///< short sound like klirr etc.
        Pulse, ///< very short sound, an simple frequency like a beep.
        None   ///< unknown type or not defined.
    };
}

//! Defines the used groups of sounds. \see SndGrp
namespace SndGroup {
    enum Enum {
        Music=0,      ///< eSndTypeMusic
        UnitReaction, ///< eSndTypeSound: reaction or answerers of the units
        Environment,  ///< dog, cat etc.
        UnitAction,   ///< sword "kling"
        Magic,        ///< some magic makes some strange sound.
        Attention,    ///< The player should take his attention to something
        All           ///< no more groups, the master group which means all groups.
    };
}

//! Defines the play mode of a sound.
namespace SndPlayMode {
    enum Enum {
        Repeat,     ///< Sound has to be repeated
        Single      ///< Sound is played once
    };
}

//! Defines the current state of the sound object.
namespace SndObjState {
    enum Enum {
        Nop           ///< Object is in no operation.
      , Playing       ///< Object is active and in playing mode.
      , Stopped       ///< Object is stopped.
      , Paused        ///< Object is interrupted and waiting for continue or stop.
    };
}

} // namespace FTS