/**
 * \file game_rlv.h
 * \author Pompei2
 * \date 9 December 2008
 * \brief This file defines the game's main runlevel.
 **/

#ifndef D_GAME_RLV_H
#define D_GAME_RLV_H

#include "main.h"
#include "main/runlevels.h"
#include "dLib/dString/dString.h"

namespace FTS {
    class Map;
    class Archive;

/** This class is the runlevel of the game, that is while playing.
 *  Its load method does not load the game itself, this is done during the
 *  LoadGame runlevel, it is only the real "game" itself.
 */
class GameRlv : public Runlevel {
private:
    /// The root of the CEGUI menu.
    CEGUI::Window *m_pRootWindow = nullptr;
    /// The map we play the game on :)
    Map *m_pMap = nullptr;

    /// The archive that this map was loaded from. We need to keep track of it
    /// not to extract things of it (done by the file class automatically)
    /// but to unload it after the game is over.
    Archive *m_pMapArchive = nullptr;
    uint16_t m_screenHeight = 0;
    uint16_t m_screenWidth = 0;
    uint16_t getW() {return m_screenWidth;}
    uint16_t getH() {return m_screenHeight;}
#ifdef DEBUG
    /// In debug mode, we want to be able to display a coordinate system.
    bool m_bRenderCoordinateSystem = false;
    inline bool cbDbgSwapShowCoordSys(const CEGUI::EventArgs &) {
        m_bRenderCoordinateSystem = !m_bRenderCoordinateSystem;
        return true;
    };
#endif

    bool cbLeave(const CEGUI::EventArgs &);
    bool cbLeaveYes(const CEGUI::EventArgs &);

public:
    GameRlv();
    virtual ~GameRlv();
    bool load() override;
    bool unload() override;
    void render2D(const Clock&) override;
    void render3D(const Clock&) override;
    String getName() override;

    inline Map *getMap() {return m_pMap;};
    inline void giveMapArchive(Archive *in_pArch) {if(m_pMapArchive == nullptr) m_pMapArchive = in_pArch;};
};
};

#endif /* D_GAME_RLV_H */

 /* EOF */
