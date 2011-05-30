#pragma once
#include <string>
#include "dLib/dString/dString.h"
#include "sound/SndGrp.h"

namespace FTS {
    class ISndObj;
}

class Music
{
public:
    Music(const char * fileName);
    virtual ~Music(void);
    void load(const char * fileName);
    void play();
    void stop();
    void pause();
    void resume();
    void unload();
    void volume(float v);
    void setType(int sndType);
private:
    Music() {};
    FTS::String name;
    FTS::ISndObj* getObj();
    FTS::ISndObj* getObj(FTS::SndGroup::Enum grp);
    FTS::SndGroup::Enum sndGrp;
};

