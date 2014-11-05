#ifndef D_ANIM_H
#define D_ANIM_H

#include <chrono>

#include "main.h"

#include "dLib/dString/dPath.h"
#include "dLib/dConf/DefaultOptions.h"

namespace FTS {
    class Graphic;

class Anim {
private:
    /** This is an array of all the pictures. */
    Graphic **m_pGraphs;
    /** The time we first drew the animation. */
    std::chrono::time_point<std::chrono::system_clock> m_timeStart;
    /** The total time the animation should take. */
    uint64_t m_iLenght;
    /** The number of frames the animation got. */
    uint64_t m_nFrames;
    /** If we already started the anim or not (to set start). */
    bool m_bStarted;
    /** This is whether the animation should loop or stop. */
    bool m_bLoop;
    /** The name of the file where it's take from */
    Path m_sFile;
    /** The name of the temp directory where all the pics got extracted. */
    Path m_sTmpDir;
    /** If it is loaded or not. */
    bool m_bLoaded;
    /** If we want it to be drawable or not. */
    bool m_bShow;
    class Settings : public DefaultOptions {
    public:
        Settings() {
            add("Frames", 24);
            add("Time", 1000);
            add("Loop", false);
        }
    };
public:
    Anim(const Path &in_sFile, bool in_bShow = 1);
    virtual ~Anim();

    int   load();
    int unload();

    int draw  (int in_iX, int in_iY);
    int drawEx(int in_iX, int in_iY,
               int in_iSubX = 0, int in_iXubY = 0, int in_iSubW = 0, int in_iSubH = 0,
               float in_fRotate = 0.0f, float in_fZoomX = 1.0f, float in_fZoomY = 1.0f,
               float in_fR = 0.0f, float in_fg = 0.0f, float in_fb = 0.0f, float in_fa = 0.0f);
    void rewind() { m_bStarted = false; m_timeStart = std::chrono::time_point<std::chrono::system_clock>(); };
    uint64_t getCurrPic();
};

}

#endif /* D_ANIM_H */

 /* EOF */
