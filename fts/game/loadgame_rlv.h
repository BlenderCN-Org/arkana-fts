/**
 * \file loadgame_rlv.h
 * \author Pompei2
 * \date 18 December 2008
 * \brief This file defines the runlevel that loads the game.
 **/

#ifndef D_LOADGAME_RLV_H
#define D_LOADGAME_RLV_H

#include "main.h"
#include "main/runlevels.h"
#include "dLib/dString/dPath.h"
#include "map/terrain.h" // Hmmm, don't know how to fwd-decl that stuff.
#include "dLib/dString/dString.h"

namespace CEGUI {
    class Window;
    class Tooltip;
}

namespace FTS {
    class MapInfo;
    class GameRlv;

/** This class is the runlevel of the game, that is while playing.
 *  Its load method does not load the game itself, this is done during the
 *  LoadGame runlevel, it is only the real "game" itself.
 */
class LoadGameRlv : public Runlevel {
public:
    class ILoadGameState;
private:
    ILoadGameState* m_loadState = nullptr;
    float m_fPercentDone = 0.f;
    /// The root of the CEGUI menu.
    CEGUI::Window *m_pRootWindow = nullptr;

    /// The game runlevel that is being loaded.
    GameRlv *m_pGame = nullptr;

    Path m_sFile;       ///< The name of the map file to load.
    uint8_t m_nPlayers = 0; ///< The number of players that play in this map.

    uint32_t m_uiBeginTime = 0; ///< The time that stage began. Used for time measuring.

    /// The informations needed to load the terrain.
    Terrain::SLoadingInfo *m_pTerrainLoadingInfo = nullptr;

    /// Whether this runlevel is waiting for user interaction to start the game.
    bool m_bWaiting = false;

    bool cbAnyKeyPressed(const CEGUI::EventArgs &);

    void setupLoadscreenPlayers();
    void setupLoadscreenDetails(MapInfo *in_pDetails);
    void addToProgress(float in_fPercentForStage, const String &in_sDetail);

    void finishStage(const String &in_sProgress, const String &in_sStat = String::EMPTY);
public:
    class ILoadGameState
    {
    public:
        virtual ~ILoadGameState() = default;
        virtual void doLoad(LoadGameRlv * context) = 0;
        virtual float getStatePercentage() = 0 ;
    };

    class StateLoadBeginning : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.0f;}
    };
    class StateLoadMapInfo : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.1f;}
    };
    class StateLoadTerrainInfo : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.1f;}
    };
    class StateLoadTerrainQuads : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.2f;}
    };
    class StateLoadTerrainLoadLowerTiles : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.1f;}
    };
    class StateLoadTerrainCompileLowerTileset : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.2f;}
    };
    class StateLoadTerrainUpperTiles : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.1f;}
    };
    class StateLoadTerrainPrecalc : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.1f;}
    };
    class StateLoadForests : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.1f;}
    };
    class StateLoadScripts : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.0f;}
    };
    class StateLoadFinalize : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.0f;}
    };
    class StateLoadDone : public ILoadGameState
    {
    public:
        void doLoad(LoadGameRlv * context);
        float getStatePercentage() {return 0.0f;}
    };
    LoadGameRlv(const Path &in_sMapFile, uint8_t in_nPlayers);
    virtual ~LoadGameRlv();
    bool load() override;
    bool unload() override;
    void render2D(const Clock&) override;
    String getName() override;
    bool update(const Clock&) override;
    void setState(ILoadGameState* state);
};
};

#endif /* D_LOADGAME_RLV_H */

 /* EOF */
