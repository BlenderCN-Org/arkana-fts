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
    virtual bool load();
    virtual bool unload();
    virtual void render2D(const Clock&);
    virtual void render3D(const Clock&);
    virtual String getName();

    /* This method may be overloaded. It has to return a reference to the
     *  runlevel's currently used camera. You only need to overwrite this
     *  method if the runlevel does use more then one default camera.\n
     *
     *  \return A reference to the currently active camera.
     *
     *  \author Pompei2
     */
//     virtual CFTSCamera &getActiveCamera() {return m_defaultCamera;};

    /* This method may be overloaded. It has to return a reference to the
     *  runlevel's main camera. That must not be the currently active camera.
     *  For example the game's main camera is the one in bird-view, but
     *  during a cut-scene there may be a lot of cameras and one of them may be
     *  currently active.\n
     *  You only need to overwrite this method if you have more then one camera.
     *
     *  \return A reference to the runlevel's main camera.
     *
     *  \author Pompei2
     */
//     virtual CFTSCamera &getMainCamera() {return m_defaultCamera;};

    /* This method may be overloaded. It has to return a pointer to the
     *  runlevel's currently used cursor. You only need to overwrite this
     *  method if the runlevel does use more then one default cursor.\n
     *
     *  \return A pointer to the currently active cursor.
     *
     *  \author Pompei2
     */
//     virtual SCursor *getActiveCursor() {return m_pDefCursor;};

    /* This method may be overloaded. It has to return a pointer to the
     *  runlevel's main cursor. That must not be the currently active cursor.
     *  For example the game's main cursor is the one pointer, but while the
     *  game is scrolling, there is another cursor being displayed with arrows. \n
     *  You only need to overwrite this method if you have more then one cursor.
     *
     *  \return A pointer to the runlevel's main cursor.
     *
     *  \author Pompei2
     */
//     virtual SCursor *getMainCursor() {return m_pDefCursor;};

    inline Map *getMap() {return m_pMap;};
    inline void giveMapArchive(Archive *in_pArch) {if(m_pMapArchive == nullptr) m_pMapArchive = in_pArch;};
};
};

#endif /* D_GAME_RLV_H */

 /* EOF */
