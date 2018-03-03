/**
 * \file dlg_online_newGame.cpp
 * \author Pompei2
 * \date 27 October 2007
 * \brief This file defines a dialog used to enter data to host a new game.
 **/

#include <CEGUI.h>
#include "packet.h"
#include "connection.h"

#include "dlg_online_newGame.h"

#include "logging/logger.h"
#include "graphic/graphic.h"
#include "map/mapinfo.h"
#include "ui/ui.h"
#include "ui/file_dialog.h"
#include "ui/confirm_dialog.h"
#include "ui/cegui_items/simple_list_item.h"

#include "game/player.h"

using namespace FTS;

void cbTestYes()
{
    Packet p(DSRV_MSG_GAME_START);
    p.append(g_pMeHacky->og_getMD5());
    if(FTSC_ERR::OK != g_pMeHacky->og_getConnection()->mreq(&p)) {
        return ;
    }
}

void cbTestNo()
{
    Packet p(DSRV_MSG_GAME_REM);
    p.append(g_pMeHacky->og_getMD5());
    if( FTSC_ERR::OK != g_pMeHacky->og_getConnection()->mreq( &p ) ) {
        return ;
    }
}

/// Default constructor
/** This is the default constructor that creates the dialog, sets
 *  up all callbacks etc.
 *
 * \author Pompei2
 */
FTS::DlgOnlineNewGame::DlgOnlineNewGame()
    : Dlg("dlg_onlineNewGame")
{
    if(m_pRoot == NULL) {
        delete this;
        return;
    }

    m_pMapInfo = new MapInfo;

    try {
        // Connect the events to the member functions.
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(FTS::DlgOnlineNewGame::cbCancel));
        m_pRoot->getChild("dlg_onlineNewGame/btnCancel")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::DlgOnlineNewGame::cbCancel));
        m_pRoot->getChild("dlg_onlineNewGame/btnOk")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::DlgOnlineNewGame::cbOk));
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/Load")
            ->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
                             FTS_SUBS(FTS::DlgOnlineNewGame::cbMapTypeChanged));
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/Resume")
            ->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
                             FTS_SUBS(FTS::DlgOnlineNewGame::cbMapTypeChanged));
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/Random")
            ->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
                             FTS_SUBS(FTS::DlgOnlineNewGame::cbMapTypeChanged));
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnLoadFile")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::DlgOnlineNewGame::cbLoadFile));
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnResumeFile")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::DlgOnlineNewGame::cbResumeFile));

        // Disable some widgets by default.
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/edLoadFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnLoadFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/edResumeFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnResumeFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnRandomSetup")->disable();

        // Add the different random map types to the listbox.
        CEGUI::Combobox *cb = (CEGUI::Combobox *)m_pRoot->getChild("dlg_onlineNewGame/frmMap/cbRandomType");
        (new SimpleListItem("Bla"))->addAsDefault(cb);
        (new SimpleListItem("Bli"))->addAsDefault(cb);
        (new SimpleListItem("Blo"))->addAsDefault(cb);

        ((CEGUI::RadioButton *)m_pRoot->getChild("dlg_onlineNewGame/frmMap/Load"))->setSelected(true);
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/Resume")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/Random")->disable();
    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    // Add keyboard shortcuts to close this messagebox.
    this->addShortcut(SpecialKey::Enter, FTS_SUBS(FTS::DlgOnlineNewGame::cbOk));
    this->addShortcut(Key::Escape, FTS_SUBS(FTS::DlgOnlineNewGame::cbCancel));
}

/// Default destructor
/** This is the default destructor that deletes the dialog
 *  and maybe gives the modal state back to the parent.
 *
 * \author Pompei2
 */
FTS::DlgOnlineNewGame::~DlgOnlineNewGame()
{
    SAFE_DELETE(m_pMapInfo);
}

bool FTS::DlgOnlineNewGame::cbCancel(const CEGUI::EventArgs & in_ea)
{
    delete this;
    return true;
}

bool FTS::DlgOnlineNewGame::cbOk(const CEGUI::EventArgs & in_ea)
{
    String sGameName;

    try {
        sGameName = m_pRoot->getChild("dlg_onlineNewGame/edGameName")->getText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    if(sGameName.empty() || !m_pMapInfo->isLoaded()) {
        FTS18N("Ogm_game_missdata", MsgType::Error);
        return true;
    }

    /// \TODO: start the real server exe when done and let it do this.

    Packet p(DSRV_MSG_GAME_INS);

    p.append(g_pMeHacky->og_getMD5());
    p.append(sGameName.str() );
    p.append((uint16_t)12345);
    m_pMapInfo->writeToPacket(&p);

    if( FTSC_ERR::OK != g_pMeHacky->og_getConnection()->mreq( &p ) ) {
        return true;
    }

    // Check if the game has been created.
    char c = ERR_OK;
    if(ERR_OK != (c = p.get())) {
        FTS18N("Ogm_game_make", MsgType::Error, String::nr(c));
        return true;
    }

    this->cbCancel(in_ea);

    // Open a TEST dialog to start the game or quit the game. Auto-deletes itself.
    ConfirmDlg *pConfDlg = new ConfirmDlg();

    pConfDlg->load("What to do now ?\nClick Yes button to simulate starting the game.\nClick No button to simulate closing the game.");
    pConfDlg->registerYesHandler(cbTestYes);
    pConfDlg->registerNoHandler(cbTestNo);
    pConfDlg->show();
    // END TEST

    return true;
}

bool FTS::DlgOnlineNewGame::cbMapTypeChanged(const CEGUI::EventArgs & in_ea)
{
    m_pMapInfo->unload();

    try {
        // First, disable all windows
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/edLoadFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnLoadFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/edResumeFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnResumeFile")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/cbRandomType")->disable();
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnRandomSetup")->disable();

        // Then re-enable the needed ones.
        if(((CEGUI::RadioButton *)m_pRoot->getChild("dlg_onlineNewGame/frmMap/Load"))->isSelected()) {
            m_pRoot->getChild("dlg_onlineNewGame/frmMap/edLoadFile")->enable();
            m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnLoadFile")->enable();
        } else if(((CEGUI::RadioButton *)m_pRoot->getChild("dlg_onlineNewGame/frmMap/Resume"))->isSelected()) {
            m_pRoot->getChild("dlg_onlineNewGame/frmMap/edResumeFile")->enable();
            m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnResumeFile")->enable();
        } else if(((CEGUI::RadioButton *)m_pRoot->getChild("dlg_onlineNewGame/frmMap/Random"))->isSelected()) {
            m_pRoot->getChild("dlg_onlineNewGame/frmMap/cbRandomType")->enable();
            m_pRoot->getChild("dlg_onlineNewGame/frmMap/btnRandomSetup")->enable();
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

bool FTS::DlgOnlineNewGame::cbLoadFile(const CEGUI::EventArgs & in_ea)
{
    // Initialise the file dialog.
    (new FileDlg())->registerHandler(FTS_SUBS(FTS::DlgOnlineNewGame::cbLoadFileChosen))
                   ->loadOpenDlg("*.ftsm", Path::datadir("Maps"));
    return true;
}

bool FTS::DlgOnlineNewGame::cbResumeFile(const CEGUI::EventArgs & in_ea)
{
    // Initialise the file dialog.
    (new FileDlg())->registerHandler(FTS_SUBS(FTS::DlgOnlineNewGame::cbResumeFileChosen))
                   ->loadOpenDlg("*.ftsm", Path::datadir("Maps"));
    return true;
}

bool FTS::DlgOnlineNewGame::cbLoadFileChosen(const CEGUI::EventArgs & in_ea)
{
    String sFile = static_cast<const FileDlgEventArgs &>(in_ea).getFile();

    // If he clicked on cancel, do nothing.
    if(sFile.empty()) {
        return true;
    }

    // Get some info about the map.
    if(ERR_OK != m_pMapInfo->loadFromMap(sFile)) {
        return true;
    }

    try {
        m_pRoot->getChild("dlg_onlineNewGame/frmMap/edLoadFile")->setText(sFile);
        m_pRoot->getChild("dlg_onlineNewGame/frmMapInfo/sName")
               ->setText(m_pMapInfo->getName());
        m_pRoot->getChild("dlg_onlineNewGame/frmMapInfo/sAuthor")
               ->setText(m_pMapInfo->getAuthor());
        m_pRoot->getChild("dlg_onlineNewGame/frmMapInfo/sLastModif")
               ->setText(m_pMapInfo->getLastModif().toStr());
        m_pRoot->getChild("dlg_onlineNewGame/frmMapInfo/sMinMax")
            ->setText(String::nr(m_pMapInfo->getMinPlayers()) + "/" + String::nr(m_pMapInfo->getMaxPlayers()));
        m_pRoot->getChild("dlg_onlineNewGame/frmMapInfo/sSugg")
               ->setText(m_pMapInfo->getSuggPlayers());

        if(NULL != m_pMapInfo->getPreview()->createCEGUI("map_preview", true)) {
            m_pRoot->getChild("dlg_onlineNewGame/MapImage")
                   ->setProperty("ImageNormal", "set:map_preview image:image");
        }

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

bool FTS::DlgOnlineNewGame::cbResumeFileChosen(const CEGUI::EventArgs & in_ea)
{
    String sFile = static_cast<const FileDlgEventArgs &>(in_ea).getFile();

    // If he clicked on cancel, do nothing.
    if(sFile.empty()) {
        return true;
    }

    return true;
}

 /* EOF */
