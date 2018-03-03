#include "Music.h"
#include "../sound/fts_Snd.h"

using namespace FTS;
Music::Music(const char * fileName)
{
    load(fileName);
}

Music::~Music(void)
{
}

void Music::load(const char * fileName)
{
    name = FTS::Path(fileName);
    sndGrp = FTS::SndGroup::Music;
    FTS::ISndSys::getSingletonPtr()->CreateSndObj(FTS::SndGroup::Music, name);
}
void Music::play()
{
    getObj()->Play();
}
void Music::stop()
{
    getObj()->Stop();
}
void Music::pause()
{
    getObj()->Pause();
}
void Music::resume()
{
    getObj()->Continue();
}
void Music::unload()
{
    getObj()->Unload();
}

void Music::volume(float v)
{
    getObj()->setVolume(v);
}

ISndObj* Music::getObj()
{
    SndGrp * grp = FTS::ISndSys::getSingletonPtr()->getGroup(sndGrp);
    ISndObj* sndObj = grp->getSndObj(name);
    return sndObj;
}

void Music::setType( int sndType )
{
    ISndObj* sndObj = getObj();
    sndGrp = (FTS::SndGroup::Enum) sndType;
    sndObj->setGroup(sndGrp);
}


