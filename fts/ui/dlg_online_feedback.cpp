/**
 * \file dlg_online_feedback.cpp
 * \author Pompei2
 * \date 01 August 2009
 * \brief This file implements a dialog that lets the user give us feedback to the master-server.
 **/

#include <CEGUI.h>

#include "dlg_online_feedback.h"

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
FTS::OnlineFeedbackWindow::OnlineFeedbackWindow()
    : Dlg("dlg_onlineFeedback")
{
    // Failed to load the layout.
    if(!m_pRoot) {
        delete this;
        return ;
    }

    try {
        // Connect the events to the member functions.
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(OnlineFeedbackWindow::cbCancel));
        m_pRoot->getChild("dlg_onlineFeedback/Cancel")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(OnlineFeedbackWindow::cbCancel));
        m_pRoot->getChild("dlg_onlineFeedback/Send")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(OnlineFeedbackWindow::cbSend));

    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    this->addShortcut(SpecialKey::Enter, FTS_SUBS(OnlineFeedbackWindow::cbSend));
    this->addShortcut(Key::Escape, FTS_SUBS(OnlineFeedbackWindow::cbCancel));
}

/// Default destructor
FTS::OnlineFeedbackWindow::~OnlineFeedbackWindow()
{
}

/// Called on a click on the send button in the feedback dialog.
/** This callback is called when the user clicks the send button that
 *  is placed in the feedback dialog accessible via the online
 *  main menu.
 *
 *  Will send the feedback to the masterserver.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineFeedbackWindow::cbSend(const CEGUI::EventArgs &)
{
    String sMessage = CEGUI::WindowManager::getSingleton()
                             .getWindow("dlg_onlineFeedback/edMessage")
                            ->getText();

    // We don't want mini-messages!
    if(sMessage.len() < 8) {
        FTS18N("Ogm_feedback_tooshort", MsgType::Warning, String::nr(8));
        return true;
    }

    // Send the message over to the master-server.
    Connection *pCon = g_pMeHacky->og_getConnection();
    Packet p(DSRV_MSG_FEEDBACK);
    p.append(g_pMeHacky->og_getMD5());
    p.append(sMessage);

    // Only close myself if everything was successful.
    if( pCon->mreq( &p ) == FTSC_ERR::OK && p.get() == ERR_OK ) {
        CEGUI::EventArgs ea;
        this->cbCancel(ea);
    }

    return true;
}

/// Called on a click on the cancel button in the feedback dialog.
/** This callback is called when the user clicks the cancel button that
 *  is placed in the feedback dialog accessible via the online
 *  main menu.
 *
 *  Just closes the dialog.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineFeedbackWindow::cbCancel(const CEGUI::EventArgs &)
{
    delete this;

    return true;
}

 /* EOF */
