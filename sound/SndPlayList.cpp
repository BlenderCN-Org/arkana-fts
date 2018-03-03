/*
 * \file SndPlayList.cpp
 *
 * \author kabey
 * \brief Implementation file of the play  list class.
 * \todo Random mode.
 */


#include "fts_Snd.h"
#include "SndPlayList.h"
#include "dLib/dFile/dFile.h"
#include <utility>

namespace FTS {
    SndPlayList* SndPlayList::m_pSingleton = NULL ; // Currently only one play list is allowed.
    /// ctor
    SndPlayList::SndPlayList(void)
        : m_mode(PlayListMode::Default)
        , m_state(PlayListState::Init)
    {
        m_currentPos = m_playListNames.begin();
        if( SndPlayList::m_pSingleton ) {
            ISndSys::getSingleton().detach(this->m_pSingleton);
            delete SndPlayList::m_pSingleton;
        }
        SndPlayList::m_pSingleton = this;
        ISndSys::getSingleton().attach(this);
    }
    /// dtor
    SndPlayList::~SndPlayList(void)
    {
        ListNames::iterator iter ;
        while(m_playListNames.size()) {
            iter = m_playListNames.begin();
            Remove(*iter);
        }
        ISndSys::getSingleton().detach(this);
        SndPlayList::m_pSingleton = NULL;
    }

/*! Creates a new SndPlayList object.
 *
 * \author kabey
 * \param[in] in_playListName   The play list name.
 * \return A new play list object.
 * \note The SndPlayList object is the client and its method create() or load() calls this method to set the play list.
 */
    SndPlayList* SndPlayList::Create(const String& in_playListName)
    {
        SndPlayList* thePlaylist = new SndPlayList;
        thePlaylist->m_name = in_playListName;
        return thePlaylist;
    }
/*! Creates a new SndPlayList object and loads the contains from a play list file.
 *
 * \author kabey
 * \param[in] in_playListFileName   The play list file containing all the sound file names to load.
 * \return A new play list object.
 * \throws Any exception that may come from File::open.
 */
    SndPlayList* SndPlayList::Load(const Path& in_playListFileName)
    {
        SndPlayList* thePlayList = SndPlayList::Create(in_playListFileName.withoutExt().basename());

        File::Ptr theFile = File::open(in_playListFileName, File::Read);
        if( !theFile->eof() ) {
            PlayListMode::Enum mode = (PlayListMode::Enum)theFile->readui32();
            thePlayList->SetMode(mode);
        }
        while( !theFile->eof() ) {
            String soundName = theFile->readstr();
            thePlayList->Add(soundName);
        }
        return thePlayList;
    }


/*! Sorts the play list according to the default sort method of a list w/ strings.
 *
 * \author kabey
 */
    SndPlayList* SndPlayList::Sort()
    {
        m_playListNames.sort();
        Rewind();
        return this;
    }
/*! Add a new sound to the existing list.
 *
 * \author kabey
 * \param[in] in_soundFileName   The file name of the sound to add.
 * \param[in] in_position        The relative position in the list where to insert the new item. -1 means at the end and is the default.
 */
    SndPlayList* SndPlayList::Add(const Path& in_soundFileName, uint32_t in_position)
    {
        ISndObj* pObj = ISndSys::getSingleton().CreateSndObj(SndGroup::Music, in_soundFileName, SndPlayMode::Single);
        PlayListEntry* pElem = new PlayListEntry(in_soundFileName, pObj);
        String baseName = in_soundFileName.withoutExt().basename();
        m_playListElements.insert(std::make_pair(baseName,pElem));
        if( in_position == (uint32_t)-1 ) {
            m_playListNames.push_back(in_soundFileName.withoutExt().basename());
        } else {
            ListNames::iterator iter = m_playListNames.begin();
            std::advance(iter, in_position);
            m_playListNames.insert(iter , in_soundFileName.withoutExt().basename());
        }
        Rewind();

        return this;
    }

/*! Remove sound from the existing list.
 *
 * \author kabey
 * \param[in] in_soundFileName   The file name of the sound to remove.
 */
    SndPlayList* SndPlayList::Remove(const Path& in_soundFileName)
    {
        PlayListMap::iterator elem = m_playListElements.find(in_soundFileName);
        if( elem != m_playListElements.end() ) {
            String name = in_soundFileName ; // parameter could be taken from the element and will be deleted by erase, so make a copy.
            delete elem->second->m_snd;
            delete elem->second;
            m_playListElements.erase(elem);
            m_playListNames.remove(name);
        }
        Rewind();
        return this;
    }
/*! Remove sound from the existing list.
 *
 * \author kabey
 * \param[in] in_index   The position in the list of the sound to remove.
 */
    SndPlayList* SndPlayList::Remove(uint32_t in_index)
    {
        PlayListMap::iterator elem = m_playListElements.begin();
        std::advance(elem, in_index);
        String name = elem->first ; // since first will  be deleted by remove, so make a copy
        return Remove(elem->first);
    }

/*! Move sound from one position in the list to an other one. Whereby the new postion means behind the disered one.
 *
 * \author kabey
 * \param[in] in_positon   The position in the list of the sound to move.
 * \param[in] in_positonTo   The new position of the sound. Means, the sound is inserted at that position and the
 *                        prev element is moved one position down( including the rest of the list).
 */
    SndPlayList* SndPlayList::Move(uint32_t in_position, uint32_t in_positionTo)
    {
        ListNames::iterator elem = m_playListNames.begin();
        ListNames::iterator pos = m_playListNames.begin();
        std::advance(elem, in_position);
        // default -1 is converted to unsigned always bigger then size :-)
        if( in_positionTo >= m_playListNames.size() ) {
            pos = m_playListNames.end();
        } else {
            std::advance(pos, in_positionTo);
        }

        if( elem != m_playListNames.end()) {
            m_playListNames.insert(pos, *elem);
            m_playListNames.erase(elem);
        }
        Rewind();
        return this;
    }
/*! Start or resume playing the list.
 *
 * \author kabey
 * \param[in] in_index   The start position in the list.
 */
    SndPlayList* SndPlayList::Play(uint32_t in_index)
    {
        if( m_state == PlayListState::Init ) {
            m_currentPos = m_playListNames.begin();
        }
        if( m_state == PlayListState::Run ) {
            Stop();
        }
        ListNames::iterator elem = m_playListNames.begin();
        std::advance(elem, in_index);
        m_currentPos = elem;
        m_playListElements[*m_currentPos]->m_snd->Play();
        m_state = PlayListState::Run;

        return this;
    }
/*! Stop playing the list.
 *
 * \author kabey
 */
    SndPlayList* SndPlayList::Stop()
    {
        m_playListElements[*m_currentPos ]->m_snd->Stop();
        m_state = PlayListState::Stop;

        return this;
    }
/*! Rewind the list.
 *
 * \author kabey
 */
    SndPlayList* SndPlayList::Rewind()
    {
        m_currentPos = m_playListNames.begin();
        return this;
    }
/*! Mute the list.
 *
 * \author kabey
 * \param[in] in_muteState  Set the mute state to this value.
 */
    SndPlayList* SndPlayList::Mute(bool in_muteState)
    {
        ISndSys::getSingleton().getGroup(SndGroup::Music)->Mute(in_muteState);
        return this;
    }
/*! Set the playing mode, how to traverse the list.
 *
 * \author kabey
 * \param[in] in_mode   The mode to use.
 * \note If the list is currently running, it will be stopped and restarted w/ the new mode.
 */
    SndPlayList* SndPlayList::SetMode(const PlayListMode::Enum& in_mode)
    {
        m_mode = in_mode ;
        if( m_state == PlayListState::Run ) {
            Stop();
            Play();
        }
        return this;
    }
/*! Stop playing the list.
 *
 * \author kabey
 * \note Not implemented yet.
 */
    SndPlayList* SndPlayList::Save()
    {
        // TODO At the time where the File class supports writing back to the archive, this will be implemented too.
        return this;
    }
/*! Update the playing list. Means to check the current playing sound object if it still playing. If not, to switch to
 * the next in the list.
 *
 * \author kabey
 * \note Is called by the Update() method of the sound system.
 * \note Here we have to check if the performance is enough to do it in the game loop, or if it should use a thread to handle the list.
 */
    bool SndPlayList::update(const Clock&)
    {
        if( m_state != PlayListState::Run )
            return true;
        // 1. get state of current snd obj
        // 2. if it is still playing , just return
        // 3. select next snd object in list and play it.
        // 3a if a random mode, the selection has to be done different.
        if(m_playListElements[*m_currentPos]->m_snd->getState() != SndObjState::Playing ) {
            ++m_currentPos;
            if( m_currentPos == m_playListNames.end() ) {
                m_state = PlayListState::Stop;
                Rewind();
                return true;
            }
            m_playListElements[*m_currentPos]->m_snd->Play();
        }
        return true;
    }
};
