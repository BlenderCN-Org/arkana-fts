/**
 * \file dlg_online_findGame.cpp
 * \author Pompei2
 * \date !!NOW!!
 * \brief This file implements a dialog to see a list of games and join them.
 **/

#include <CEGUI.h>

#include "dlg_online_findGame.h"
#include "ui/cegui_items/imaged_list_item.h"
#include "game/player.h"
#include "graphic/graphic.h"
#include "logging/logger.h"
#include "net/connection.h"
#include "dLib/dString/dTranslation.h"

#define MAX_GAME_ICON_W 32
#define MAX_GAME_ICON_H 32

using namespace FTS;

/// Default constructor
/** This is the default constructor that creates the dialog, sets
 *  up all callbacks etc.
 *
 * \author Pompei2
 */
FTS::DlgOnlineFindGame::DlgOnlineFindGame()
    : Dlg("dlg_onlineFindGame")
    , m_usPort(0)
    , m_nPlayers(0)
{
    // Failed to load the layout.
    if(!m_pRoot) {
        delete this;
        return ;
    }

    // Try to hide the online main menu.
    try {
        CEGUI::WindowManager::getSingleton().getWindow("menu_online_main")->hide();
    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    try {
        // Connect the events to the member functions.
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(FTS::DlgOnlineFindGame::cbCancel));
        m_pRoot->getChild("dlg_onlineFindGame/btnCancel")
               ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::DlgOnlineFindGame::cbCancel));
        m_pRoot->getChild("dlg_onlineFindGame/btnJoin")
               ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::DlgOnlineFindGame::cbJoin));
        m_pRoot->getChild("dlg_onlineFindGame/btnRefresh")
               ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::DlgOnlineFindGame::cbRefresh));
        m_pRoot->getChild("dlg_onlineFindGame/lbGames")
            ->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
                             FTS_SUBS(FTS::DlgOnlineFindGame::cbGameClicked));
    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    // Add keyboard shortcuts to close this dialog.
    this->addShortcut(SpecialKey::Enter, FTS_SUBS(FTS::DlgOnlineFindGame::cbJoin));
    this->addShortcut(Key::Escape, FTS_SUBS(FTS::DlgOnlineFindGame::cbCancel));

    // Get a list of all games.
    CEGUI::EventArgs ea;
    this->cbRefresh(ea);
    this->clearGameInfo();
}

/// Default destructor
/** This is the default destructor that deletes the dialog
 *  and maybe gives the modal state back to the parent.
 *
 * \author Pompei2
 */
FTS::DlgOnlineFindGame::~DlgOnlineFindGame()
{
    this->clearGameInfo();

    // Try to show the online main menu again.
    try {
        CEGUI::WindowManager::getSingleton().getWindow("menu_online_main")->show();
    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Refreshes the list of games.
/** This function clears the game list, then gets a new list from the master-
 *  server.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::DlgOnlineFindGame::cbRefresh(const CEGUI::EventArgs &)
{
    Packet p(DSRV_MSG_GAME_LST);
    p.append(g_pMeHacky->og_getMD5());

    if(ERR_OK != g_pMeHacky->og_getConnection()->mreq(&p)) {
        // No need for an error message, it will be retried later.
        return true;
    }

    int8_t iRet = 0;
    int16_t nGames = 0;
    p.get(iRet);
    p.get(nGames);

    // Get the listbox and empty it first.
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, lbGames, "dlg_onlineFindGame/lbGames");
        lbGames->resetList();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    // The list has been cleared, stop here in case there was an error.
    if(iRet != ERR_OK)
        return true;

    // Add all games to the listbox.
    for(int16_t i = 0 ; i < nGames ; i++) {
        String sGameName = p.get_string();
        String sMapName = p.get_string();
        bool bStarted = p.get() == 1;
        Graphic *gIcon = GraphicManager::getSingleton().readGraphicFromPacket(&p);

        // Need to shrink the game's icon?
        if(gIcon->getH() > MAX_GAME_ICON_H) {
            GraphicManager::getSingleton().resizeGraphic(gIcon, (float)MAX_GAME_ICON_H / (float)gIcon->getH());
        }
        // More shrinking?
        if(gIcon->getW() > MAX_GAME_ICON_W) {
            GraphicManager::getSingleton().resizeGraphic(gIcon, (float)MAX_GAME_ICON_W / (float)gIcon->getW());
        }

        try {
            FTSGetConvertWinMacro(CEGUI::Listbox, lbGames, "dlg_onlineFindGame/lbGames");
            ImagedListItem *ili = new ImagedListItem(sGameName, gIcon);
            if(bStarted)
                ili->setTextColours(CEGUI::colour(1.0f, 0.0f, 0.0f, 1.0f));
            lbGames->addItem(ili);
        } catch(CEGUI::Exception & e) {
            FTS18N("CEGUI", MsgType::Error, e.getMessage());
        }
        // The list item made a copy of it. We no more need it.
        GraphicManager::getSingleton().destroyGraphic(gIcon);
    }

    return true;
}

/// Closes this dialog.
/** This deletes itself, closing the dialog.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::DlgOnlineFindGame::cbCancel(const CEGUI::EventArgs &)
{
    delete this;

    return true;
}

/// Called when one game gets selected in the list.
/** Gets various more detailed informations about the game and displays them in
 *  the right.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::DlgOnlineFindGame::cbGameClicked(const CEGUI::EventArgs &)
{
    Packet p(DSRV_MSG_GAME_INFO);
    p.append(g_pMeHacky->og_getMD5());

    this->clearGameInfo();

    // Get the name of the selected game.
    String sGameName;
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, lbGames, "dlg_onlineFindGame/lbGames");
        CEGUI::ListboxItem *li = lbGames->getFirstSelectedItem();
        if(li == NULL)
            return true;

        sGameName = li->getText();
        if(sGameName.empty())
            return true;

        p.append(sGameName);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    if((ERR_OK != g_pMeHacky->og_getConnection()->mreq(&p)) || p.get() != ERR_OK) {
        return true;
    }

    // Get the infos out of the packet.
    p.get(m_sIP);
    p.get(m_usPort);
    p.get(m_sHost);
    p.get(m_nPlayers);
    for(uint8_t i = 0; i<m_nPlayers ;i++) {
        m_sPlayers.push_back(p.get_string());
    }

    if(ERR_OK != m_mapInfo.readFromPacket(&p))
        return true;

    // Write the info into the GUI.
    CEGUI::WindowManager &wm = CEGUI::WindowManager::getSingleton();
    Translation trans("ui");
    // Top part: the map infos.
    try {
        wm.getWindow("dlg_onlineFindGame/lblMapName")->setText(m_mapInfo.getName());
        wm.getWindow("dlg_onlineFindGame/lblMapName")->show();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblMapDesc")->setText(m_mapInfo.getDesc());
        wm.getWindow("dlg_onlineFindGame/lblMapDesc")->show();
    } catch(CEGUI::Exception &) { }
    try {
        String sFmt = trans.get("gameinfo_MapAuthorAndModif");
        String sTxt = sFmt.fmt(m_mapInfo.getAuthor(), m_mapInfo.getLastModif().toStr());
        wm.getWindow("dlg_onlineFindGame/lblMapAuthorAndModif")->setText(sTxt);
        wm.getWindow("dlg_onlineFindGame/lblMapAuthorAndModif")->show();
    } catch(CEGUI::Exception &) { }
    try {
        String sFmt = trans.get("gameinfo_MapnPlayers");
        String sTxt = sFmt.fmt(String::nr(m_mapInfo.getMinPlayers()), String::nr(m_mapInfo.getMaxPlayers()));
        wm.getWindow("dlg_onlineFindGame/lblMapPlayers")->setText(sTxt);
        wm.getWindow("dlg_onlineFindGame/lblMapPlayers")->show();
    } catch(CEGUI::Exception &) { }
    try {
        if(NULL != m_mapInfo.getPreview()->createCEGUI("map_preview", true)) {
            wm.getWindow("dlg_onlineFindGame/MapImage")
             ->setProperty("ImageNormal", "set:map_preview image:image");
            wm.getWindow("dlg_onlineFindGame/MapImage")->show();
        }
    } catch(CEGUI::Exception &) { }

    // Bottom part: the game infos:
    try {
        wm.getWindow("dlg_onlineFindGame/lblGameName")->setText(sGameName);
        wm.getWindow("dlg_onlineFindGame/lblGameName")->show();
    } catch(CEGUI::Exception &) { }
    try {
        String sFmt = trans.get("gameinfo_GameHost");
        wm.getWindow("dlg_onlineFindGame/lblGameHost")->setText(sFmt.fmt(m_sHost));
        wm.getWindow("dlg_onlineFindGame/lblGameHost")->show();
    } catch(CEGUI::Exception &) { }
    try {
        String sFmt = trans.get("gameinfo_GamePlayers");
        wm.getWindow("dlg_onlineFindGame/lblGamePlayers")->setText(sFmt.fmt(String::nr(m_nPlayers)));
        wm.getWindow("dlg_onlineFindGame/lblGamePlayers")->show();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblGameSugg")->setText(m_mapInfo.getSuggPlayers());
        wm.getWindow("dlg_onlineFindGame/lblGameSugg")->show();
    } catch(CEGUI::Exception &) { }
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, lb, "dlg_onlineFindGame/lbPlayers");
        for(std::vector<String>::iterator i = m_sPlayers.begin() ; i != m_sPlayers.end() ; ++i) {
            lb->addItem(new ImagedListItem(*i, "FTSUI", "UserIcon"));
        }
        lb->show();
    } catch(CEGUI::Exception &) { }
    return true;
}

/// Clears all infos about the game.
/** When selecting a game in the list, the user gets more details about it from
 *  the master server, these details are displayed on the right.\n
 *  This method clears all of the details as well in the member variables as in
 *  the GUI.
 *
 * \author Pompei2
 */
void FTS::DlgOnlineFindGame::clearGameInfo()
{
    // Clear the data from the previously selected game.
    m_sIP = String::EMPTY;
    m_usPort = 0;
    m_sHost = String::EMPTY;
    m_nPlayers = 0;
    m_sPlayers.clear();
    m_mapInfo.unload();

    // Clear the GUI.
    CEGUI::WindowManager &wm = CEGUI::WindowManager::getSingleton();
    CEGUI::ImagesetManager &im = CEGUI::ImagesetManager::getSingleton();

    try {
        wm.getWindow("dlg_onlineFindGame/lblMapName")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblMapName")->hide();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblMapDesc")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblMapDesc")->hide();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblMapAuthorAndModif")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblMapAuthorAndModif")->hide();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblMapPlayers")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblMapPlayers")->show();
    } catch(CEGUI::Exception &) { }
    try {
        im.destroyImageset("map_preview");
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/MapImage")
         ->setProperty("ImageNormal", "set:ArkanaLook image:OneTransparentPixel");
        wm.getWindow("dlg_onlineFindGame/MapImage")->hide();
    } catch(CEGUI::Exception &) { }

    // Bottom part: the game infos:
    try {
        wm.getWindow("dlg_onlineFindGame/lblGameName")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblGameName")->hide();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblGameHostPlayers")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblGameHostPlayers")->hide();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblGamePlayers")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblGamePlayers")->hide();
    } catch(CEGUI::Exception &) { }
    try {
        wm.getWindow("dlg_onlineFindGame/lblGameSugg")->setText("");
        wm.getWindow("dlg_onlineFindGame/lblGameSugg")->hide();
    } catch(CEGUI::Exception &) { }
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, lb, "dlg_onlineFindGame/lbPlayers");
        lb->resetList();
    } catch(CEGUI::Exception &) { }
}

/// Joins a game.
/** \TODO Implement the joining of a game.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::DlgOnlineFindGame::cbJoin(const CEGUI::EventArgs &)
{
    // Do something.
    FTS18N("NotImpYet", MsgType::Warning);
    return true;
}

 /* EOF */
