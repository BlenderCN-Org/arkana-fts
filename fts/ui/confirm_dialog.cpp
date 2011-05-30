/**
 * \file confirm_dialog.cpp
 * \author Pompei2
 * \date February 2007
 * \brief This file implements a simple Yes/No confirmation dialog.
 **/

#include <CEGUI.h>

#include "confirm_dialog.h"
#include "utilities/utilities.h"
#include "ui/ui.h"
#include "ui/ui_commands.h"
#include "logging/logger.h"
#include "input/input.h"

using namespace FTS;

/** Constructor for the confirm dialog. */
FTS::ConfirmDlg::ConfirmDlg()
{
    CEGUI::Window *pLastActive = GUI::getSingleton().getActiveWin();
    m_sLastActive = pLastActive ? pLastActive->getName() : "None";
    m_bLastActiveWasModal = pLastActive ? pLastActive->getModalState() : false;

    m_bYesEventSubs = false;
    m_bNoEventSubs = false;
    m_bLoaded = false;
    m_pDlg = NULL;
    m_pfnYes = NULL;
    m_pfnNo = NULL;
}

/** Destructor for the confirm dialog. */
FTS::ConfirmDlg::~ConfirmDlg()
{
    // Now that the dialog gets closed, we can remove both shortcuts from the system.
    InputManager::getSingleton().delShortcut(String(m_pDlg->getName()) + "1");
    InputManager::getSingleton().delShortcut(String(m_pDlg->getName()) + "2");

    try {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_pDlg);
        if(m_sLastActive != "None" && m_bLastActiveWasModal) {
            try {
                CEGUI::WindowManager::getSingleton().getWindow(m_sLastActive)->setModalState(true);
            } catch(CEGUI::Exception &) {
                ;
            }
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// loads an internationalised confirm dialog.
/** This loads a confirm dialog wich displays the message
 *  with ID in_sMessageID that is specified in the file
 *  ui, in the current language.
 *
 * \param in_sMessageID The ID of the message to load/translate
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note If you call only this function, the dialog doesn't show up yet.
 *       Call the show function after that.
 *
 * \author Pompei2
 */
int FTS::ConfirmDlg::loadI18N(const String& in_sMessageID)
{
    if(m_bLoaded) {
        FTS18N("InvParam", MsgType::Horror, "FTS::ConfirmDlg::loadI18N already loaded");
        return -1;
    }

    if(!in_sMessageID) {
        FTS18N("InvParam", MsgType::Horror, "FTS::ConfirmDlg::loadI18N");
        return -1;
    }

    /* Load the message string. */
    m_s18NMessage = getTranslatedString(in_sMessageID, "ui");

    /* Load the dialog and set it's items texts to the just loaded ones. */
    try {
        m_pDlg = GUI::getSingleton().loadLayout("dlg_confirm", true, true, true);
        m_pDlg->setVisible(false);
        m_pDlg->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                               FTS_SUBS(FTS::ConfirmDlg::onNo));
        m_pDlg->getChild("dlg_confirm/Message")->setText(m_s18NMessage);
        m_pDlg->getChild("dlg_confirm/Yes")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::ConfirmDlg::onYes));
        m_pDlg->getChild("dlg_confirm/No")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::ConfirmDlg::onNo));

        m_bLoaded = true;
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

/// loads a confirm dialog.
/** This loads a confirm dialog wich displays a message. (not translated.)
 *
 * \param in_sMessage The message to display
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note If you call only this function, the dialog doesn't show up yet.
 *       Call the show function after that.
 *
 * \author Pompei2
 */
int FTS::ConfirmDlg::load(const String& in_sMessage)
{
    if(m_bLoaded) {
        FTS18N("InvParam", MsgType::Horror, "FTS::ConfirmDlg::load already loaded");
        return -1;
    }

    if(!in_sMessage) {
        FTS18N("InvParam", MsgType::Horror, "FTS::ConfirmDlg::load");
        return -1;
    }

    /* Load the dialog and set it's items texts to the just loaded ones. */
    try {
        m_pDlg = GUI::getSingleton().loadLayout("dlg_confirm", true, true, true);
        m_pDlg->setVisible(false);
        m_pDlg->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                               FTS_SUBS(FTS::ConfirmDlg::onNo));
        m_pDlg->getChild("dlg_confirm/Message")->setText(in_sMessage);
        m_pDlg->getChild("dlg_confirm/Yes")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::ConfirmDlg::onYes));
        m_pDlg->getChild("dlg_confirm/No")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::ConfirmDlg::onNo));

        m_bLoaded = true;
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

/// Show the confirm dialog.
/** This function finally shows up the confirm dialog, but only
 *  if it has been loaded before.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::ConfirmDlg::show()
{
    if(!m_bLoaded)
        return -1;

    try {
        m_pDlg->setVisible(true);
        CEGUI::System::getSingleton().getGUISheet()->addChildWindow(m_pDlg);
        m_pDlg->activate();
    }
    catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return -2;
    }

    // Add a keyboard shortcut to close this messagebox.
    InputManager *pMgr = InputManager::getSingletonPtr();
    ActiveWindowCheckCmd *pCond = NULL;
    CallbackCommand *pCbCmd = NULL;

    // One for the return key.
    pCond = new ActiveWindowCheckCmd(m_pDlg->getName());
    pCbCmd = new CallbackCommand(FTS_SUBS(FTS::ConfirmDlg::onYes));
    pMgr->add(String(m_pDlg->getName()) + "1", SpecialKey::Enter,
              new ConditionalCommand(pCond, pCbCmd));

    // And one for the escape key.
    pCond = new ActiveWindowCheckCmd(m_pDlg->getName());
    pCbCmd = new CallbackCommand(FTS_SUBS(FTS::ConfirmDlg::onNo));
    pMgr->add(String(m_pDlg->getName()) + "2", Key::Escape,
              new ConditionalCommand(pCond, pCbCmd));
    return ERR_OK;
}

/// The yes button callback
/** This function gets called on a click on the yes button, and calls
 *  the user's registered callback.
 *
 * \param in_ea CEGUI event arguments
 *
 * \note This also hides itself, but doesn't destroy itself.
 *
 * \author Pompei2
 */
bool FTS::ConfirmDlg::onYes(const CEGUI::EventArgs& in_ea)
{
    m_pDlg->hide();

    // Copy what we need before deleting myself.
    void (*pfn) () = m_pfnYes;
    CEGUI::Event::Subscriber subs(m_pfnYesS);
    bool bSubs = m_bYesEventSubs;

    // Close the dialog before calling the callback.
    // This is to avoid some window popping up in the callback and thus
    // disturbing the window focus order.
    delete this;

    /* Call the right callback. */
    if(pfn)
        pfn();
    else if(bSubs)
        subs(in_ea);

    return true;
}

/// The no button callback
/** This function gets called on a click on the no button, and calls
 *  the user's registered callback.
 *
 * \param in_ea CEGUI event arguments
 *
 * \note This also hides itself, and if registerSelfCloseNo has been
 *       called, it also destroys itself.
 *
 * \author Pompei2
 */
bool FTS::ConfirmDlg::onNo(const CEGUI::EventArgs& in_ea)
{
    m_pDlg->hide();

    // Copy what we need before deleting myself.
    void (*pfn) () = m_pfnNo;
    CEGUI::Event::Subscriber subs(m_pfnNoS);
    bool bSubs = m_bNoEventSubs;

    // Close the dialog before calling the callback.
    // This is to avoid some window popping up in the callback and thus
    // disturbing the window focus order.
    delete this;

    /* Call the right callback. */
    if(pfn)
        pfn();
    else if(bSubs)
        subs(in_ea);

    return true;
}

/// Register your yes button callback.
/** Use this function to register your callback for the yes button.
 *
 * \param subscriber The CEGUI event subscriber wich describes your
 *                   callback function.
 *
 * \author Pompei2
 */
void FTS::ConfirmDlg::registerYesHandler(CEGUI::Event::Subscriber subscriber)
{
    m_bYesEventSubs = true;
    m_pfnYesS = subscriber;
    m_pfnYes = NULL;
}

/// Register your yes button callback.
/** Use this function to register your callback for the yes button.
 *
 * \param subscriber The callback function.
 *
 * \note The void * argument to the function is 'this'.
 *
 * \author Pompei2
 */
void FTS::ConfirmDlg::registerYesHandler(void (*subscriber) ())
{
    m_bYesEventSubs = false;
    m_pfnYes = subscriber;
}

/// Register your no button callback.
/** Use this function to register your callback for the no button.
 *
 * \param subscriber The CEGUI event subscriber wich describes your
 *                   callback function.
 *
 * \author Pompei2
 */
void FTS::ConfirmDlg::registerNoHandler(CEGUI::Event::Subscriber subscriber)
{
    m_bNoEventSubs = true;
    m_pfnNoS = subscriber;
    m_pfnNo = NULL;
}

/// Register your no button callback.
/** Use this function to register your callback for the no button.
 *
 * \param subscriber The callback function.
 *
 * \note The void * argument to the function is 'this'.
 *
 * \author Pompei2
 */
void FTS::ConfirmDlg::registerNoHandler(void (*subscriber) ())
{
    m_bNoEventSubs = false;
    m_pfnNo = subscriber;
}
