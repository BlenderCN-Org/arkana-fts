/**
 * \file dlg_online_joinchannel.cpp
 * \author Pompei2
 * \date 11 October 2007
 * \brief This file implements a dialog that lets the user choose a channel to join.
 **/

#include <CEGUI.h>

#include "dlg_online_joinchannel.h"

#include "ui/ui.h"
#include "ui/cegui_items/simple_list_item.h"
#include "ui/ui_menu_online_main.h"
#include "game/player.h"
#include "logging/logger.h"
#include "net/connection.h"

using namespace FTS;

/// Default constructor
/** This is the default constructor that creates the dialog, sets
 *  up all callbacks etc.
 *
 * \author Pompei2
 */
FTS::OnlineJoinChannelWindow::OnlineJoinChannelWindow(OnlineMenuRlv &in_rlv)
    : Dlg("dlg_onlineChannelJoin"),
      m_rlv(in_rlv)
{
    // Failed to load the layout.
    if(!m_pRoot) {
        delete this;
        return ;
    }

    try {
        // Connect the envents to the member functions.
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(OnlineJoinChannelWindow::cbCancel));
        m_pRoot->getChild("dlg_onlineChannelJoin/Cancel")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(OnlineJoinChannelWindow::cbCancel));
        m_pRoot->getChild("dlg_onlineChannelJoin/Join")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(OnlineJoinChannelWindow::cbJoin));
        m_pRoot->getChild("dlg_onlineChannelJoin/List")
            ->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
                             FTS_SUBS(OnlineJoinChannelWindow::cbChanSel));

        // Fill all fields with the data.

        // Get the list of public channels from the server.
        Connection *pCon = g_pMeHacky->og_getConnection();
        Packet p(DSRV_MSG_CHAT_PUBLICS);
        p.append(g_pMeHacky->og_getMD5());
        if( pCon->mreq( &p ) == FTSC_ERR::OK && p.get() == ERR_OK ) {
            uint32_t nChans = 0;
            p.get(nChans);

            // Get the listbox.
            FTSGetConvertWinMacro(CEGUI::Listbox, lbChans, "dlg_onlineChannelJoin/List");

            // Add all public channels to the listbox.
            String sCurChan = g_pMeHacky->og_chatGetCurChannel();
            for(uint32_t i = 0 ; i < nChans ; i++) {
                String sChan = p.get_string();
                // Select the current channel.
                if(sChan == sCurChan) {
                    (new SimpleListItem(sChan))->addAsDefault(lbChans);
                } else {
                    lbChans->addItem(new SimpleListItem(sChan));
                }
            }
        }

    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    this->addShortcut(SpecialKey::Enter, FTS_SUBS(OnlineJoinChannelWindow::cbJoin));
    this->addShortcut(Key::Escape, FTS_SUBS(OnlineJoinChannelWindow::cbCancel));
}

/// Default destructor
FTS::OnlineJoinChannelWindow::~OnlineJoinChannelWindow()
{
}

/// Called on a click on the join button in the join channel dialog.
/** This callback is called when the user clicks the join button that
 *  is placed in the join channel dialog accessible via the online
 *  main menu.
 *
 *  Will join or create a channel.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineJoinChannelWindow::cbJoin(const CEGUI::EventArgs &)
{
    String sChan;

    // Get the channel name.
    try {
        sChan = m_pRoot->getChild("dlg_onlineChannelJoin/edChanName")->getText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    // Only close myself if successfully joined.
    if( ERR_OK == m_rlv.join(sChan) ) {
        CEGUI::EventArgs ea;
        this->cbCancel(ea);
    }
    return true;
}

/// Called on a click on the cancel button in the join channel dialog.
/** This callback is called when the user clicks the cancel button that
 *  is placed in the join channel dialog accessible via the online
 *  main menu.
 *
 *  Just closes the dialog.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineJoinChannelWindow::cbCancel(const CEGUI::EventArgs &)
{
    delete this;

    return true;
}

/// Apply the selection change in the join channel dialog.
/** This function gets called when the user selects something in
 *  the listbox in the join channel dialog. It gets the name of
 *  the channel the user selected and puts it into the editbox.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineJoinChannelWindow::cbChanSel(const CEGUI::EventArgs &)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, lb, "dlg_onlineChannelJoin/List");
        SimpleListItem *sli = dynamic_cast<SimpleListItem *>(lb->getFirstSelectedItem());
        if(!sli)
            return true;

        m_pRoot->getChild("dlg_onlineChannelJoin/edChanName")->setText(sli->getText());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

 /* EOF */
