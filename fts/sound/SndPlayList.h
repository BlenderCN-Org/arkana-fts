/*
 * $Id: $
 * \file SndPlayList.h
 *
 * \author kabey
 * \brief Interface file to the play list class of the sound system.
 * \detail
 */
#pragma once
#include <list>
#include <map>

#include "dLib/dString/dString.h"
#include <main/Updateable.h>

namespace FTS {

namespace PlayListMode {
    enum Enum {
          Default // play the list as they is, from start to end.
        , Random  // play the list in an random order w/o taking care if a song was already played.
        , Repeat  // play the list forever
    };
}

namespace PlayListState {
    enum Enum {
          Init  // list is created.
        , Stop  // list is stopped.
        , Pause // list is stopped and waiting for a play command.
        , Run   // list is playing the music.
    };
}

class PlayListEntry
{
    friend class SndPlayList;
public:
    PlayListEntry(const String& name, ISndObj* pSndObj)
        : m_name(name)
        , m_snd(pSndObj)
    {};
    virtual ~PlayListEntry() {};
private:
    PlayListEntry() {};
    String m_name;
    ISndObj* m_snd;
};

typedef std::list<String> ListNames;
typedef std::map<String, PlayListEntry*> PlayListMap;
typedef std::pair<String, PlayListEntry*> MapPair;

class SndPlayList : public Updateable
{
public:
    static SndPlayList* Create(const String& in_playListName);
    static SndPlayList* Load(const Path& in_playListFileName);
    virtual ~SndPlayList();
    SndPlayList* Add(const Path& in_soundFileName, uint32_t in_position = -1);
    SndPlayList* Remove(const Path& in_soundFileName);
    SndPlayList* Remove(uint32_t in_index);
    SndPlayList* Move(uint32_t in_position, uint32_t in_positionTo = -1);
    SndPlayList* Sort();
/*! Sorts the play list according to the given compare functor.
 *
 * \author kabey
 * \param[in] _Comp   The compare functor.
 */
    template <typename Traits>
    SndPlayList* Sort(Traits in_Comp){
        m_playListNames.sort(in_Comp);
        Rewind();
        return this;
    };
    SndPlayList* Play(uint32_t in_index = 0);
    SndPlayList* Stop();
    SndPlayList* Rewind();
    SndPlayList* Mute(bool in_muteState = true);
    SndPlayList* SetMode(const PlayListMode::Enum& in_mode);
    SndPlayList* Save();
    std::list<String> getList() const {return m_playListNames;};
    bool update(const Clock&);
private:
    SndPlayList();
    String m_name;
    ListNames m_playListNames;
    PlayListMap m_playListElements;
    PlayListMode::Enum m_mode;
    PlayListState::Enum m_state;
    ListNames::iterator m_currentPos;
    static SndPlayList* m_pSingleton;
};

};
