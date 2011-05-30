/**
 * \file load_rlv.h
 * \author Pompei2
 * \date 9 December 2008
 * \brief This file defines everything about the fts loading runlevel.
 **/

#ifndef D_LOAD_FTS_RLV_H
#define D_LOAD_FTS_RLV_H

#include "main.h"
#include "main/runlevels.h"

#include "dLib/dString/dString.h"

namespace FTS {
    class Graphic;

class LoadFTSRlv : public Runlevel {
private:
    /// The name of the logo to display.
    String m_sLogoFile;

    /// The root window that contains progressbar etc.
    CEGUI::Window *m_pRootWin;
    uint16_t m_screenHeight;
    uint16_t m_screenWidth;
    uint16_t getW() {return m_screenWidth;}
    uint16_t getH() {return m_screenHeight;}

    int initCEGUI();
    int initSDL();

    enum {
        LoadBeginning = 0,
        LoadCEGUI,
        LoadGraphics,
        LoadSound,
        LoadNetwork,
        LoadFinal,
        LoadDone,
        LoadStageCount
    } m_eNextTodo;  ///< The next thing that has to be loaded.

    void updateProgressbar();

public:
    LoadFTSRlv();
    virtual ~LoadFTSRlv();
    virtual bool load();
    virtual bool unload();
    virtual void render2D(const Clock&);
    virtual bool update(const Clock&);
    virtual String getName();
};

} // namespace FTS

#endif /* D_LOAD_RLV_H */

 /* EOF */
