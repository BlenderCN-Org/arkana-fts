/**
 * \file load_rlv.h
 * \author Pompei2
 * \date 9 December 2008
 * \brief This file defines everything about the fts loading run level.
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

    /// The root window that contains progress bar etc.
    CEGUI::Window *m_pRootWin = nullptr;
    uint16_t m_screenHeight = 0;
    uint16_t m_screenWidth = 0;
    uint16_t getW() {return m_screenWidth;}
    uint16_t getH() {return m_screenHeight;}

    int initCEGUI();

    enum {
        LoadBeginning = 0,
        LoadCEGUI,
        LoadGraphics,
        LoadSound,
        LoadNetwork,
        LoadFinal,
        LoadDone,
        LoadStageCount
    } m_eNextTodo = LoadBeginning;  ///< The next thing that has to be loaded.

    void updateProgressbar();

public:
    LoadFTSRlv();
    virtual ~LoadFTSRlv();
    bool load() override;
    bool unload() override;
    void render2D(const Clock&) override;
    bool update(const Clock&) override;
    String getName() override;
};

} // namespace FTS

#endif /* D_LOAD_RLV_H */

 /* EOF */
