/**
 * \file dlg_online_findGame.h
 * \author Pompei2
 * \date 11 November 2007
 * \brief This file defines a dialog to see a list of games and join them.
 **/

#ifndef D_DLGONLINEFINDGAME_H
#define D_DLGONLINEFINDGAME_H

#include "main.h"
#include "ui/ui.h"

#include "map/mapinfo.h"

namespace FTS {

class DlgOnlineFindGame : public Dlg {
private:
    bool cbRefresh(const CEGUI::EventArgs & in_ea);
    bool cbCancel(const CEGUI::EventArgs & in_ea);
    bool cbGameClicked(const CEGUI::EventArgs &in_ea);
    bool cbJoin(const CEGUI::EventArgs & in_ea);

    void clearGameInfo();

    // Infos of the currently selected map.

    /// The IPv4 address of the host (xxx.xxx.xxx.xxx) of the currently selected game.
    String m_sIP;
    /// The portnumber on what you can connect to the host of the curr. sel. game.
    uint16_t m_usPort;
    /// The name of the player that crated the currently selected game.
    String m_sHost;
    /// The number of players currently in the currently selected game.
    uint8_t m_nPlayers;
    /// The name of the players that are in the currently selected game.
    std::vector<String>m_sPlayers;
    /// More detailed informations about the map.
    FTS::MapInfo m_mapInfo;
public:
    DlgOnlineFindGame();
    virtual ~DlgOnlineFindGame();
};

} // namespace FTS

#endif                          /* D_DLGONLINEFINDGAME_H */

 /* EOF */
