/**
 * \file ui_menu_online_main.cpp
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file contains all the callbacks of the online main menu in FTS.
 **/

#include <CEGUI.h>

#include "ui/ui.h"
#include "ui/cegui_items/chat.h"
#include "ui/dlg_online_acctInfo.h"
#include "ui/dlg_online_findGame.h"
#include "ui/dlg_online_joinchannel.h"
#include "ui/dlg_online_newGame.h"
#include "ui/dlg_online_feedback.h"
#include "ui/ui_menu.h"
#include "ui/ui_menu_online.h"
#include "ui/ui_menu_online_main.h"
#include "ui/dlg_options.h"
#include "game/player.h"
#include "logging/logger.h"
#include "utilities/utilities.h"
#include "main/runlevels.h"
#include "net/connection.h"
#include "sound/fts_Snd.h"
#include "tools/server2/constants.h"
#include "dLib/dConf/configuration.h"

#define D_MAX_SENT_MESSAGES_STORED 100

using namespace FTS;

/// Default constructor.
FTS::OnlineMenuRlv::OnlineMenuRlv()
    : m_pRoot(NULL)
{
}

/// Default destructor.
FTS::OnlineMenuRlv::~OnlineMenuRlv()
{
}

/** This method will be called during the loading of the runlevel.\n
 *  The GUI is loaded here, other stuff is initialised too.
 *
 *  \return This method should return true only if it successfully loaded
 *          the whole runlevel. If it returns false, the previous runlevel
 *          will be backed up again.
 *  \author Pompei2
 */
bool FTS::OnlineMenuRlv::load()
{
    this->loadDefaultCursor();

    // Load the Online login menu
    if((m_pRoot = GUI::getSingleton().loadLayout("menu_online_main", true)) == NULL)
        throw ErrorAlreadyShownException();

    CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();

    // Setup the callbacks for the mainmenu.
    try {
        if(pWM->isWindowPresent("menu_online_main/btnChat"))
            pWM->getWindow("menu_online_main/btnChat")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbChan));
        if(pWM->isWindowPresent("menu_online_main/btnAcct"))
            pWM->getWindow("menu_online_main/btnAcct")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbAcct));
        if(pWM->isWindowPresent("menu_online_main/btnFind"))
            pWM->getWindow("menu_online_main/btnFind")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbFind));
        if(pWM->isWindowPresent("menu_online_main/btnHost"))
            pWM->getWindow("menu_online_main/btnHost")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbHost));
        if(pWM->isWindowPresent("menu_online_main/btnFeedback"))
            pWM->getWindow("menu_online_main/btnFeedback")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbFeedback));
        if(pWM->isWindowPresent("menu_online_main/btnSettings"))
            pWM->getWindow("menu_online_main/btnSettings")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbSettings));
        if(pWM->isWindowPresent("menu_online_main/btnQuit"))
            pWM->getWindow("menu_online_main/btnQuit")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbLogout));
        if(pWM->isWindowPresent("menu_online_main/btnSend"))
            pWM->getWindow("menu_online_main/btnSend")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbSend));
        if(pWM->isWindowPresent("menu_online_main/lbUsers"))
            pWM->getWindow("menu_online_main/lbUsers")
               ->subscribeEvent(CEGUI::Window::EventMouseButtonUp, FTS_SUBS(OnlineMenuRlv::cbListClick));
        if(pWM->isWindowPresent("menu_online_main/pmUser/info"))
            pWM->getWindow("menu_online_main/pmUser/info")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbUserInfo));
        if(pWM->isWindowPresent("menu_online_main/pmUser/whisp"))
            pWM->getWindow("menu_online_main/pmUser/whisp")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbUserWhisp));
        if(pWM->isWindowPresent("menu_online_main/pmUser/mute"))
            pWM->getWindow("menu_online_main/pmUser/mute")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbUserMute));
        if(pWM->isWindowPresent("menu_online_main/pmUser/unmute"))
            pWM->getWindow("menu_online_main/pmUser/unmute")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbUserUnmute));
        if(pWM->isWindowPresent("menu_online_main/pmUser/kick"))
            pWM->getWindow("menu_online_main/pmUser/kick")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbUserKick));
        if(pWM->isWindowPresent("menu_online_main/pmUser/op"))
            pWM->getWindow("menu_online_main/pmUser/op")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbUserOp));
        if(pWM->isWindowPresent("menu_online_main/pmUser/deop"))
            pWM->getWindow("menu_online_main/pmUser/deop")
               ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(OnlineMenuRlv::cbUserDeop));
        // Give the chat message editbox the focus.
        if(pWM->isWindowPresent("menu_online_main/edMessage"))
            GUI::getSingleton().setActiveWidget(pWM->getWindow("menu_online_main/edMessage"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    // Setup keyboard shortcuts.
    InputManager *pMgr = InputManager::getSingletonPtr();
    if(pMgr) {
        // One for the return key to send the message.
        pMgr->add("Online_Enter", SpecialKey::Enter,
                  new InterpretCurrMsgCmd("menu_online_main/edMessage",true));
        // Two for the up key to go back in history.
        pMgr->add("Online_History_Back", Key::ArrowUp,
                  new HistoryNavigationCmd("menu_online_main/edMessage",
                                           HistoryNavigationCmd::Back));
        // Three for the down key to go forth in history.
        pMgr->add("Online_History_Forth", Key::ArrowDown,
                  new HistoryNavigationCmd("menu_online_main/edMessage",
                                           HistoryNavigationCmd::Forth));
        pMgr->registerDefaultMenuShortcuts(false);
    }

    // Join the default channel. If this doesn't work, we can continue,
    // But will periodically retry this.
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    String defaultChannel = conf.get("DefaultChannel");

    this->join(defaultChannel);

    return true;
}

/** This method will be called during the cleaning of the runlevel (when quitting).\n
 *  It unloads the GUI, unregister all your keyboard shortcuts etc.
 *
 *  \return This method should return true if it successfully unloaded
 *          the whole runlevel. If it returns false, nothing special is done
 *          (the runlevel is still unloaded).
 *  \author Pompei2
 */
bool FTS::OnlineMenuRlv::unload()
{
    this->unloadDefaultCursor();

    // Now that the dialog gets closed, we can remove both shortcuts from the system.
    InputManager::getSingleton().delShortcut("Online_Enter");
    InputManager::getSingleton().delShortcut("Online_History_Back");
    InputManager::getSingleton().delShortcut("Online_History_Forth");
    InputManager::getSingleton().unregisterDefaultMenuShortcuts(false);

    // Unload the CEGUI layout.
    try {
        if(m_pRoot != NULL) {
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRoot);
        }
    } catch(CEGUI::Exception &) {
    }

    return true;
}

/** This method will be called once every frame,
 *  after the 3D rendering is done and all matrices are setup to render
 *  in two dimensions now.\n
 *  Thus you should use the commands glVertex2i(x,y) to draw something, x
 *  and y being screen-space pixels (from 0 to w, 0 to h).\n
 *
 *  Here, it only draws CEGUI and the cursor.
 *
 * \author Pompei2
 */
void FTS::OnlineMenuRlv::render2D(const Clock& in_c)
{
    // Draw GUI first.
    this->renderCEGUI();

    // And draw the cursor at last.
    drawCursor(in_c, this->getActiveCursor());

    GUI::getSingleton().updateGUIInfo();
}

/** This method will be called once every game tick, that is usually once every
 *  frame, right before the rendering is set up and done.\n
 *
 *  We use this method to see if there is a new network message for us.
 *
 *  \author Pompei2
 */
bool FTS::OnlineMenuRlv::update(const Clock&)
{
    g_pMeHacky->og_chatCheckEvents();
    return true;
}

/** This method returns a unique name for the runlevel.
 *
 * \return The name of this runlevel.
 *
 *  \author Pompei2
 */
String FTS::OnlineMenuRlv::getName()
{
    return "Online Menu";
}

/** Construct a InterpretCurrMsgCmd object.
 *
 * \param in_sTextHolderName The name of the CEGUI::Window that will hold the
 *                           text of the message to interpret when this command
 *                           gets executed.
 * \param in_bClearWindow If this is set to true, the text in the CEGUI::Window
 *                        specified by \a in_sTextHolderName will be cleared
 *                        after execution of the command. If set to false, it
 *                        will be left as-is.
 *
 * \author Pompei2
 */
FTS::OnlineMenuRlv::InterpretCurrMsgCmd::InterpretCurrMsgCmd(const String &in_sTextHolderName, bool in_bClearWindow)
    : m_sTextHolderName(in_sTextHolderName),
      m_bClearWindow(in_bClearWindow)
{
}

/// Default destructor.
FTS::OnlineMenuRlv::InterpretCurrMsgCmd::~InterpretCurrMsgCmd()
{
}

/// Interprets the command written in a CEGUI::Window.
/** This function will then interpret the command that is written in the
 *  specified CEGUI::Window, if it exists. It will call the corresponding
 *  methods of the player class if some special command is found. Else, it
 *  will just send the text as a chat message to the current channel.
 *
 * \return true if the window with the name given in the constructor currently
 *         exists. It even returns true if nothing will be done, as long as it
 *         exists.
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::InterpretCurrMsgCmd::exec()
{
    String sMessage;

    try {
        // Get the content of the specified window.
        CEGUI::Window *pW = CEGUI::WindowManager::getSingleton()
                                .getWindow(m_sTextHolderName);
        sMessage = pW->getText();

        // Clear it if needed and give it the focus.
        if(m_bClearWindow)
            pW->setText("");
        pW->activate();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return false;
    }

    if(sMessage.isEmpty())
        return true;

    // Get the current runlevel to call appropriate methods.
    Runlevel *pBRlv = RunlevelManager::getSingleton().getCurrRunlevel();
    OnlineMenuRlv *pRlv = dynamic_cast<OnlineMenuRlv *>(pBRlv);

    if(!pRlv)
        return false;

    // If we have a valid message, store it in the list of sent messages.
    pRlv->m_lSentMessages.push_back(sMessage);
    while(pRlv->m_lSentMessages.size() > D_MAX_SENT_MESSAGES_STORED) {
        pRlv->m_lSentMessages.pop_front();
    }
    pRlv->m_ilSentMessagesPos = pRlv->m_lSentMessages.end();

    if(sMessage.nicmp("/help",5) && sMessage.len() >= 5) {
        // Maybe the user asked help about a certain topic.
        CParser p;
        char *pszCmd = NULL;
        p.loadStr(sMessage);
        p.parse("/help% %r", &pszCmd );

        if(pszCmd == NULL) {
            sMessage = getTranslatedString("Chat_Help","ui");
        } else {
            sMessage = getTranslatedString("Chat_Help_" + String(pszCmd).lower(), "ui");
            SAFE_FREE(pszCmd);
        }

        if(sMessage.isEmpty())
            sMessage = getTranslatedString("Chat_Unknown_cmd", "ui");

        if(pRlv)
            pRlv->gotSystemMessage(sMessage);

    } else if(sMessage.nicmp("/join ",6) && sMessage.len() >= 5) {
        // The user wants to join a channel, get the channel name.
        String sChan = sMessage.mid(6, 0).trim();
        if(pRlv)
            pRlv->join(sChan);

    } else if(sMessage.nicmp("/motto ",7) && sMessage.len() >= 6) {
        // The user wants to set the channel motto, get the new motto.
        String sMotto = sMessage.mid(7, 0).trim();
        g_pMeHacky->og_chatSetMotto(sMotto);

    } else if(sMessage.nicmp("/kick ",6) && sMessage.len() >= 5) {
        // The user wants to kick someone, get the name of the player to kick.
        String sPlayer = sMessage.mid(6, 0).trim();
        g_pMeHacky->og_chatKick(sPlayer);

    } else if(sMessage.nicmp("/op ",4) && sMessage.len() >= 3) {
        // The user wants to op someone, get the name of the player to op.
        String sPlayer = sMessage.mid(4, 0).trim();
        g_pMeHacky->og_chatOp(sPlayer);

    } else if(sMessage.nicmp("/deop ",6) && sMessage.len() >= 5) {
        // The user wants to deop someone, get the name of the player to deop.
        String sPlayer = sMessage.mid(6, 0).trim();
        g_pMeHacky->og_chatDeop(sPlayer);

    } else if(sMessage.nicmp("/w ",3) && sMessage.len() >= 2) {
        // The user wants to whisp someone, get the name of the player to whisp to.
        CParser p;
        char *pszPlayer = NULL;

        // First, we read the player out of the /w player message
        p.loadStr(sMessage.mid(2,0));
        int nRead = p.parse("% %S% ", &pszPlayer);

        // If there is still something left, we read the rest (it is the message)
        String sText = sMessage.mid(2 + nRead,0);
        if(!sText.isEmpty()) {
            // And send him.
            g_pMeHacky->og_chatWhisp(pszPlayer, sText);
        }

        SAFE_FREE(pszPlayer);
    } else if(sMessage.nicmp("/listchans ",11) && sMessage.len() >= 10) {
        // The user wants to get a list of his channels.
        std::list<String> sChans = g_pMeHacky->og_chatMyChans();

        String sFormattedMessage;
        if(!sChans.empty()) {
            String sMessageFmt = getTranslatedString("Chat_ChannelsList", "ui");
            String sChanPrefix = getTranslatedString("Chat_ChannelsList_ChanPrefix", "ui");
            String sChanSuffix = getTranslatedString("Chat_ChannelsList_ChanSuffix", "ui");
            String sChansList = "";
            for(std::list<String>::iterator i = sChans.begin() ; i != sChans.end() ; ++i) {
                sChansList += sChanPrefix + *i + sChanSuffix;
            }
            sFormattedMessage = sMessageFmt.fmt(sChansList);
        } else {
            sFormattedMessage = getTranslatedString("Chat_ChannelsListEmpty", "ui");
        }

        pRlv->gotSystemMessage(sFormattedMessage);

    } else if(sMessage.nicmp("/delchan ",9) && sMessage.len() >= 10) {
        // The user wants to destroy a channel, get the name of that channel.
        String sChan = sMessage.mid(9, 0).trim();
        if(ERR_OK == g_pMeHacky->og_chatRemChan(sChan)) {
            // Tell the user about the success.
            String sFmt = getTranslatedString("Chat_ChannelRemGood", "ui");
            pRlv->gotSystemMessage(sFmt.fmt(sChan));
        }
    } else if(sMessage.nicmp("/", 1) && sMessage.len() >= 1) {
        // Say that the command doesn't exist, don't show the command-try to the others.
        pRlv->gotSystemMessage(getTranslatedString("Chat_Unknown_cmd", "ui"));
    } else {
        g_pMeHacky->og_chatSendMessage(sMessage);
    }

    return true;
}

/** Construct a HistoryNavigationCmd object.
 *
 * \param in_sTextReceiverName The name of the CEGUI::Window that will receive
 *                             the text of the message at the chosen place in
 *                             the history of chat messages.\n
 *                             This window has also to be the active window
 *                             while the command is being executed.
 * \param in_eMode How to travel trough history.
 *
 * \author Pompei2
 */
FTS::OnlineMenuRlv::HistoryNavigationCmd::HistoryNavigationCmd(const String &in_sTextReceiverName, Mode in_eMode)
    : m_sTextReceiverName(in_sTextReceiverName)
    , m_eMode(in_eMode)
{
}

/// Default destructor.
FTS::OnlineMenuRlv::HistoryNavigationCmd::~HistoryNavigationCmd()
{
}

/// Gives a CEGUI::Window the text at a chosen place in history of chat messages.
/** This function will go at a certain place in history, defined by \a m_eMode
 *  and give the CEGUI::Window described by \a m_sTextReceiverName the text that
 *  is at this place in history.
 *
 * \return true if the window with the name given in the constructor currently
 *         exists and is the active window. False if not (and then nothing is done).
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::HistoryNavigationCmd::exec()
{
    // Get the current runlevel to call appropriate methods.
    Runlevel *pBRlv = RunlevelManager::getSingleton().getCurrRunlevel();
    OnlineMenuRlv *pRlv = dynamic_cast<OnlineMenuRlv *>(pBRlv);

    if(!pRlv || !GUI::getSingleton().getActiveWidget())
        return false;

    if(GUI::getSingleton().getActiveWidget()->getName() != m_sTextReceiverName)
        return false;

    switch(m_eMode) {
    case Back:
        // Go back if possible.
        if(pRlv->m_ilSentMessagesPos != pRlv->m_lSentMessages.begin())
            pRlv->m_ilSentMessagesPos--;
        break;
    case Forth:
    default:
        // Go forth if possible.
        if(pRlv->m_ilSentMessagesPos != pRlv->m_lSentMessages.end())
            pRlv->m_ilSentMessagesPos++;
        break;
    }

    String sText;

    // If we are behind the end, we clear the window's text.
    // If not, we get the text.
    if(pRlv->m_ilSentMessagesPos != pRlv->m_lSentMessages.end()) {
        sText = *pRlv->m_ilSentMessagesPos;
    }

    // Give the window the text.
    try {
        CEGUI::Window *pW = CEGUI::WindowManager::getSingleton()
                                .getWindow(m_sTextReceiverName);
        pW->setText(sText);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return false;
    }

    return true;
}

/// Called on a click on the Channel button in the online main menu.
/** This callback is called when the user clicks the Channel button that
 *  is placed in the online main menu.
 *
 *  Opens a dialog where the user can change the chat channel he resides.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbChan(const CEGUI::EventArgs &)
{
    new OnlineJoinChannelWindow(*this);
    return true;
}

/// Called on a click on the Account button in the online main menu.
/** This callback is called when the user clicks the Account button that
 *  is placed in the online main menu.\n
 *
 *  Opens a dialog where the user can change his account details.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbAcct(const CEGUI::EventArgs &)
{
    new DlgOnlineAcctInfo(DlgOnlineAcctInfo::ModeEdit, g_pMeHacky->getName());
    return true;
}

/// Called on a click on the Find Game button in the online main menu.
/** This callback is called when the user clicks the Find Game button that
 *  is placed in the online main menu.\n
 *
 *  Opens a dialog where the user can search for games and then join one.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbFind(const CEGUI::EventArgs &)
{
    new DlgOnlineFindGame();
    return true;
}

/// Called on a click on the Host Game button in the online main menu.
/** This callback is called when the user clicks the Host Game button that
 *  is placed in the online main menu.\n
 *
 *  Opens a dialog where the user can choose the settings to start a server
 *  and host a game.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbHost(const CEGUI::EventArgs &)
{
    new DlgOnlineNewGame();
    return true;
}

/// Called on a click on the Feedback button in the online main menu.
/** This callback is called when the user clicks the Feedback button that
 *  is placed in the online main menu.\n
 *
 *  Opens a dialog where the user can give us feedback, that is write us
 *  something that will be sent to the server and stored there.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbFeedback(const CEGUI::EventArgs & in_ea)
{
    new OnlineFeedbackWindow();
    return true;
}

/// Called on a click on the Settings button in the online main menu.
/** This callback is called when the user clicks the Settings button that
 *  is placed in the online main menu.\n
 *
 *  Opens a dialog where the user can setup his options. It is basically the
 *  same one that can be opened in the main menu.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbSettings(const CEGUI::EventArgs& in_ea)
{
    new MenuOptions();
    return true;
}

/// Called on a click on the Logout button in the online main menu.
/** This callback is called when the user clicks the Logout button that
 *  is placed in the online main menu.\n
 *
 *  Tries to logout the user on the master server, then goes back to
 *  the main menu.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbLogout(const CEGUI::EventArgs &)
{
    // we want to log off before going back to the mainmenu.
    g_pMeHacky->og_logout();

    // Go back to the main menu.
    RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv());
    return true;
}

/// Called on a click on the Send button in the online main menu.
/** This callback is called when the user clicks the Send button that
 *  is placed in the online main menu. It executes a \a InterpretCurrMsgCmd
 *  command for the menu_online_main/edMessage editbox.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbSend(const CEGUI::EventArgs &)
{
    InterpretCurrMsgCmd cmd("menu_online_main/edMessage", true);
    return cmd.exec();
}

/// Called when a click happens within the users list.
/** This function gets called when a click happens within the users list.
 *  This will, if the click happened with the right mouse button, pop up
 *  a contextmenu.
 *
 * \param in_ea The position of the mouse during the click.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbListClick(const CEGUI::EventArgs & in_ea)
{
    const CEGUI::MouseEventArgs *mea = dynamic_cast<const CEGUI::MouseEventArgs *>(&in_ea);

    if(mea == NULL || mea->button != CEGUI::RightButton)
        return true;

    // Simulate a leftclick, so the item under the mouse gets selected.
    InputManager::getSingleton().simulateMouseClick(MouseButton::Left, (uint16_t)mea->position.d_x, (uint16_t)mea->position.d_y);

    // Get the list item that was right-clicked on.
    try {
        // Get the listbox and empty it.
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli == NULL)
            return true;

        // Place the popup menu at the left of the users list and the cursor's y pos and open it.
        CEGUI::Window *pUsr = CEGUI::WindowManager::getSingleton().getWindow("menu_online_main/lbUsers");
        FTSGetConvertWinMacro(CEGUI::PopupMenu, pm, "menu_online_main/pmUser");
        float fPopupWidth = pm->getPixelRect().d_right - pm->getPixelRect().d_left;
        pm->setXPosition(CEGUI::UDim(0.0f,pUsr->getPixelRect().d_left - fPopupWidth));
        pm->setYPosition(CEGUI::UDim(0.0f,mea->position.d_y));
        GUI::getSingleton().openPopupMenu(pm);

        // Get some info about the user who was clicked on.
        char cMyState = g_pMeHacky->og_chatUserGet(g_pMeHacky->getName());
        bool bItsMe = g_pMeHacky->getName() == cli->getName();

        if(bItsMe) {
            pm->getChild("menu_online_main/pmUser/whisp")->disable();
            pm->getChild("menu_online_main/pmUser/mute")->disable();
            pm->getChild("menu_online_main/pmUser/unmute")->disable();
            pm->getChild("menu_online_main/pmUser/kick")->disable();
            pm->getChild("menu_online_main/pmUser/op")->disable();
            pm->getChild("menu_online_main/pmUser/deop")->disable();
        } else {
            if(cMyState == 2) { // Channel Admin.
                pm->getChild("menu_online_main/pmUser/mute")->enable();
                pm->getChild("menu_online_main/pmUser/unmute")->enable();
                pm->getChild("menu_online_main/pmUser/kick")->enable();
                if(cli->getState() == 0) {
                    pm->getChild("menu_online_main/pmUser/op")->enable();
                    pm->getChild("menu_online_main/pmUser/deop")->disable();
                } else {
                    pm->getChild("menu_online_main/pmUser/op")->disable();
                    pm->getChild("menu_online_main/pmUser/deop")->enable();
                }
            } else if(cMyState == 1) { // Operator.
                pm->getChild("menu_online_main/pmUser/mute")->enable();
                pm->getChild("menu_online_main/pmUser/unmute")->enable();
                if(cli->getState() == 2) { // Operator can't kick admin.
                    pm->getChild("menu_online_main/pmUser/kick")->disable();
                } else {
                    pm->getChild("menu_online_main/pmUser/kick")->enable();
                }
                pm->getChild("menu_online_main/pmUser/op")->disable();
                pm->getChild("menu_online_main/pmUser/deop")->disable();
            } else { // Normal user.
                pm->getChild("menu_online_main/pmUser/mute")->enable();
                pm->getChild("menu_online_main/pmUser/unmute")->enable();
                pm->getChild("menu_online_main/pmUser/kick")->disable();
                pm->getChild("menu_online_main/pmUser/op")->disable();
                pm->getChild("menu_online_main/pmUser/deop")->disable();
            }

            // Disable one of the two muted buttons.
            if(g_pMeHacky->og_haveMuted(cli->getName()))
                pm->getChild("menu_online_main/pmUser/mute")->disable();
            else
                pm->getChild("menu_online_main/pmUser/unmute")->disable();

            pm->getChild("menu_online_main/pmUser/whisp")->enable();
        }

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Get the user currently selected.
/** This method returns a pointer to the user that is currently selected in the
 *  chat users list.\n
 *
 *  It throws an CEGUI::GenericException if there was an error.\n
 *  Returns NULL if nobody is selected.
 *
 * \return A pointer to the currently selected user or NULL if none is selected.
 *
 * \author Pompei2
 */
ChatMembersListItem *FTS::OnlineMenuRlv::getSelectedUser()
{
    // Get the user name that was right-clicked on.
    FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "menu_online_main/lbUsers");

    // Nothing selected ?
    CEGUI::ListboxItem *pLI = pLB->getFirstSelectedItem();
    if(pLI == NULL)
        return NULL;

    // Something is selected!
    ChatMembersListItem *cli = dynamic_cast<ChatMembersListItem *>(pLB->getFirstSelectedItem());
    if((cli) == NULL) {
        throw(CEGUI::InvalidRequestException("Bad cast: The first listbox item in menu_online_main/lbUsers should be of type ChatMembersListItem"));
    }
    return cli;
}

/// Called when the user clicks on the "info" entry in the user popup menu.
/** This function gets called when the user clicks on the "info" entry in
 *  the user popup menu. It will open a frame window that shows informations
 *  about the selected user.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbUserInfo(const CEGUI::EventArgs &)
{
    // Get the user name that was right-clicked on.
    try {
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli)
            new DlgOnlineAcctInfo(DlgOnlineAcctInfo::ModeView, cli->getName());

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Called when the user clicks on the "private message" entry in the user popup menu.
/** This function gets called when the user clicks on the "private message" entry in
 *  the user popup menu. It will add the "/w username" command to the chat window.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbUserWhisp(const CEGUI::EventArgs &)
{
    try {
        // Get the user name that was right-clicked on.
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli == NULL)
            return true;

        // Add the "/w username " to the editbox.
        FTSGetConvertWinMacro(CEGUI::Editbox, pMsg, "menu_online_main/edMessage");
        pMsg->setText("/w " + String(cli->getName()) + " " + String(pMsg->getText()));
        pMsg->setCaratIndex(100000); // Put the carat at the end.

        // Give the chat message editbox the focus.
        pMsg->activate();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Called when the user clicks on the "ignore" entry in the user popup menu.
/** This function gets called when the user clicks on the "ignore" entry in
 *  the user popup menu. It will add the selected user onto my ignore list.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbUserMute(const CEGUI::EventArgs &)
{
    try {
        // Get the user name that was right-clicked on.
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli == NULL)
            return true;

        g_pMeHacky->og_mute(cli->getName());
        cli->recalcText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Called when the user clicks on the "stop ignore" entry in the user popup menu.
/** This function gets called when the user clicks on the "stop ignore" entry in
 *  the user popup menu. It will remove the selected user from my ignore list.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbUserUnmute(const CEGUI::EventArgs &)
{
    try {
        // Get the user name that was right-clicked on.
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli == NULL)
            return true;

        g_pMeHacky->og_unmute(cli->getName());
        cli->recalcText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Called when the user clicks on the "kick" entry in the user popup menu.
/** This function gets called when the user clicks on the "kick" entry in
 *  the user popup menu. It will kick the selected user out of the channel.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbUserKick(const CEGUI::EventArgs &)
{
    try {
        // Get the user name that was right-clicked on.
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli == NULL)
            return true;

        g_pMeHacky->og_chatKick(cli->getName());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Called when the user clicks on the "op" entry in the user popup menu.
/** This function gets called when the user clicks on the "op" entry in
 *  the user popup menu. It will promote the selected user to a channel operator.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbUserOp(const CEGUI::EventArgs &)
{
    try {
        // Get the user name that was right-clicked on.
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli == NULL)
            return true;

        g_pMeHacky->og_chatOp(cli->getName());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Called when the user clicks on the "deop" entry in the user popup menu.
/** This function gets called when the user clicks on the "deop" entry in
 *  the user popup menu. It will make the selected user no more channel operator.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::OnlineMenuRlv::cbUserDeop(const CEGUI::EventArgs &)
{
    try {
        // Get the user name that was right-clicked on.
        ChatMembersListItem *cli = this->getSelectedUser();
        if(cli == NULL)
            return true;

        g_pMeHacky->og_chatDeop(cli->getName());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Join a channel.
/** This function joins a chat channel and sets up all the GUI
 *  stuff after doing so. It also displays a message to the user.
 *
 * \param in_sChan The name of the channel to join.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \note if \a in_sChan is String::EMPTY, the user will be told that
 *       he didn't enter any channel name.
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::join(const String & in_sChan)
{
    // If the channel name is empty, display an error message.
    if(in_sChan == String::EMPTY) {
        FTS18N("Ogm_chat_join_miss", MsgType::Error);

        // And show that.
        this->gotSystemMessage(getTranslatedString("Ogm_chat_join_miss","messages"));
        return -1;
    } else {
        // Join the channel.
        if(ERR_OK != g_pMeHacky->og_chatJoin(in_sChan))
            return -2;

        // Do what you gotta do.
        this->enteringNewChannel(in_sChan);
        return ERR_OK;
    }
}

/// Does all that is needed to update the GUI in case the player is in a new channel.
/** This method updates the player list, the channel name and the channel motto.
 *  Also, it clears the chat message box if so wanted.
 *
 * \param in_sNewChanName The name of the channel I ended up in.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::enteringNewChannel(const String &in_sNewChanName)
{
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    // In case the user wants to clear the chatbox, do this now.
    if(conf.getBool("ClearChatbox")) {
        try {
            FTSGetConvertWinMacro(CEGUI::Listbox, pChat, "menu_online_main/lbChat");
            pChat->resetList();
        } catch(CEGUI::Exception &) {
            // No need to display an error if that fails.
        }
    }

    // Write the channel name onto the edtibox for it.
    try {
        CEGUI::WindowManager::getSingleton()
                .getWindow("menu_online_main/edChanName")->setText(in_sNewChanName);
    } catch(CEGUI::Exception &) {
    }

    // Get the motto.
    String sMotto = g_pMeHacky->og_chatGetMotto();
    if(!sMotto.isEmpty()) {
        this->mottoChange(String::EMPTY, sMotto);
    }

    // Get the new players list.
    this->refreshPlayerList();

    // Say hello.
    String sFmt = getTranslatedString("Chat_Join","ui");
    String sMessage = sFmt.fmt(in_sNewChanName, sMotto.isEmpty() ? "Error" : sMotto);
    this->gotSystemMessage(sMessage);
    return ERR_OK;
}

/// Refresh all players in the list of players in channel.
/** This function removes all players that are in the list of players in channel
 *  then gets a new list of all players in the channels.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::refreshPlayerList()
{
    try {
        // Get the listbox and empty it.
        FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "menu_online_main/lbUsers");
        pLB->resetList();

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    // Now get a list of the players in the channel.
    std::list<String>lPlayers;
    g_pMeHacky->og_chatGetPlayerList(lPlayers);

    for(std::list<String>::iterator i = lPlayers.begin() ; i != lPlayers.end() ; ++i) {
        this->addPlayer(*i);
    }

    return ERR_OK;
}

/// Adds a player to the list of players in channel.
/** This function Adds a player to the list of players in channel. It reads
 *  all necessary data from the master server and then creates an list item
 *  object that represents the player and adds it to the list.
 *
 * \param in_sName The name of the player to add to the list.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::addPlayer(const String & in_sName)
{
    ChatMembersListItem *cli = NULL;
    float fSkillPercent = 0.0f;
    uint8_t cState = 0; // 0 = normal 1 = operator 2 = admin.

    // First, calculate it's skill percent.
    fSkillPercent =
        skillPercent(g_pMeHacky->og_accountGetIntFrom(DSRV_TBL_USR_WINS,  in_sName),
                     g_pMeHacky->og_accountGetIntFrom(DSRV_TBL_USR_DRAWS, in_sName),
                     g_pMeHacky->og_accountGetIntFrom(DSRV_TBL_USR_LOOSES,in_sName));

    cState = g_pMeHacky->og_chatUserGet(in_sName);

    try {
        // Then crate the according list item and set its properties.
        cli = new ChatMembersListItem(in_sName);
        cli->setSkillPercent(fSkillPercent);
        cli->setState(cState);

        // Finally get the listbox and sort the item into it.
        FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "menu_online_main/lbUsers");

        // Sort order: first the admin, then the operators, then the other users
        // operators and other users are sorted by skill points.

        // If it is the admin, insert at the beginning (there is only 1 admin/channel)
        if(cState == DSRV_CHAT_USER_ADMIN) {
            if(pLB->getItemCount() > 0) {
                pLB->insertItem(cli, pLB->getListboxItemFromIndex(0));
            } else {
                pLB->addItem(cli);
            }
        } else {
            // We are operator or user, search the right place.
            size_t iPos = 0;
            for(iPos = 0 ; iPos < pLB->getItemCount() ; ++iPos) {
                ChatMembersListItem *pCurrLI = dynamic_cast<ChatMembersListItem *>(pLB->getListboxItemFromIndex(iPos));
                if(pCurrLI == NULL) {
                    continue;
                }

                // If we are in a section too high for us, continue searching.
                // For example we are simple user but pli is admin.
                if(pCurrLI->getState() > cState)
                    continue;

                // Here we are in the right category, we just need to find the
                // position using the skill percent now.
                if(pCurrLI->getSkillPercent() > fSkillPercent)
                    continue;

                // If we come here, we found the right place.
                pLB->insertItem(cli, pCurrLI);
                break;
            }

            // Found nothing, add it at the end.
            if(iPos >= pLB->getItemCount()) {
                pLB->addItem(cli);
            }
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

/// Removes a player from the list of players in channel.
/** This function Removes a player from the list of players in channel.
 *
 * \param in_sName The name of the player to remove from the list.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::remPlayer(const String & in_sName)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "menu_online_main/lbUsers");

        size_t nItems = pLB->getItemCount();
        for(size_t i = 0 ; i < nItems ; ++i) {
            ChatMembersListItem *cli = dynamic_cast<ChatMembersListItem *>(pLB->getListboxItemFromIndex(i));
            if(cli && cli->getName() == in_sName) {
                pLB->removeItem(cli);
                break;
            }
        }

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

/// Adds a normal message to the chatmessages list.
/** This function adds a formated message to the chat messages listbox.
 *
 * \param in_sFrom  The name of the player who sent this message.
 * \param in_sMsg   The message in person :)
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::gotNormalMessage(const String & in_sFrom, const String & in_sMsg)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "menu_online_main/lbChat");
        (new ChatMsgListItem(in_sFrom, in_sMsg))->addLast(pLB);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

/// Adds a system message to the chatmessages list.
/** This function adds a formated message to the chat messages listbox.
 *
 * \param in_sMsg   The message in person :)
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::gotSystemMessage(const String & in_sMsg)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "menu_online_main/lbChat");
        (new ChatMsgListItem(String::EMPTY, in_sMsg))
            ->setUse(ChatMsgListItem::System)
            ->addLast(pLB);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

/// Adds a whisper message to the chatmessages list and to the whisper messages overlay.
/** This function adds a formated message to the chat messages listbox, and also
 *  lays this message all over the screen, so the user ALWAYS sees it.
 *
 * \param in_sUser  The user who sent this or who you got it from.
 * \param in_sMsg   The message in person :)
 * \param in_bGot   True if you got the message, false if you sent the message away.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 *
 * \todo overlay the message over everything
 */
int FTS::OnlineMenuRlv::gotOrSentWhispMessage(const String &in_sUser,const String & in_sMsg,bool in_bGot)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "menu_online_main/lbChat");

        (new ChatMsgListItem(in_sUser, in_sMsg))
            ->setNickCol(CEGUI::colour(0xFF000000))
            ->setTextCol(CEGUI::colour(0xFF00AA00))
            ->setUse(in_bGot ? ChatMsgListItem::RecvWhisp
                             : ChatMsgListItem::SentWhisp)
            ->addLast(pLB);
        /// \TODO: Whisper->overlay the message over everything.

        // If we get a message, play a sound to catch the player's attention.
        if(in_bGot) {
            ISndSys::getSingleton()
                .getSndObj(SndGroup::Attention, "whisp_recv.ogg")
                ->Play();
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

/// The motto has been changed.
/** This function gets called when the motto has been changed. This
 *  changes the motto in the UI and also displays a message in the chat.
 *
 * \param in_sFrom  The name of the player who changed the motto.
 * \param in_sMotto The new motto.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::OnlineMenuRlv::mottoChange(const String & in_sFrom, const String & in_sMotto)
{
    // If this has been dynamically changed, say something about it in the chat messages.
    if( !in_sFrom.isEmpty() ) {
        String sFmt = getTranslatedString("Chat_MottoSet", "ui");
        this->gotSystemMessage(sFmt.fmt(in_sFrom, in_sMotto));
    }

    // Change the content of the motto editbox.
    try {
        CEGUI::WindowManager::getSingleton()
            .getWindow("menu_online_main/edChanInfo")->setText(in_sMotto);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

bool FTS::OnlineMenuRlv::handleMessage(Packet &in_pack)
{
    // Check what message we have gotten !
    switch(in_pack.getType()) {
    case DSRV_MSG_CHAT_JOINS:
        // Cool, someone joined our channel ! :)
        this->addPlayer(in_pack.get_string());
        return true;

    case DSRV_MSG_CHAT_QUITS:
        // Too bad, someone left the channel. :(
        this->remPlayer(in_pack.get_string());
        return true;

    case DSRV_MSG_CHAT_GETMSG:
    {
        // A message came either in the channel or to us.
        uint8_t cMessageType;
        uint8_t cFlags;
        in_pack.get(cMessageType);
        in_pack.get(cFlags);

        if(cMessageType == DSRV_CHAT_TYPE_SYSTEM) {
            // The message we got came from the system.
            // That means the server needs to tell us something.
            String sMessageID = in_pack.get_string();
            String sMessage = getTranslatedString(sMessageID, "ui");
            if(sMessageID == "Chat_Kicked") {
                String sFrom = in_pack.get_string();
                String sVictim = in_pack.get_string();
                sMessage = sMessage.fmt(sFrom, sVictim);
            }
            this->gotSystemMessage(sMessage);
        } else {
            // The message we got came from some other player.
            String sNick = in_pack.get_string();

            // Skip muted players.
            if(g_pMeHacky->og_haveMuted(sNick))
                return true;

            String sMessage = in_pack.get_string();

            // It may be a whispered message or a normal message.
            if(cMessageType == DSRV_CHAT_TYPE_WHISPER)
                this->gotOrSentWhispMessage(sNick, sMessage, true);
            else
                this->gotNormalMessage(sNick, sMessage);
        }
        return true;
    }

    case DSRV_MSG_CHAT_MOTTO_CHANGED:
    {
        // Someone changed the motto of the current channel.
        String sNick = in_pack.get_string();
        String sMotto = in_pack.get_string();
        this->mottoChange(sNick, sMotto);
        return true;
    }

    case DSRV_MSG_CHAT_KICKED:
    {
        // Someone kicked me out of the channel !!
        String sKicker = in_pack.get_string();
        String sOldChan = in_pack.get_string();
        String sNewChan = g_pMeHacky->og_chatGetCurChannel();
        String sFmt = getTranslatedString("Chat_YouGotKicked", "ui");
        String sMessage = sFmt.fmt(sKicker, sOldChan, sNewChan);

        // Do an update of the GUI.
        this->enteringNewChannel(sNewChan);

        // Show the player that we have been kicked.
        this->gotSystemMessage(sMessage);
        return true;
    }

    case DSRV_MSG_CHAT_OPED:
    {
        // Someone has been Op'ed
        String sOped = in_pack.get_string();
        String sMsg = getTranslatedString("Chat_Oped", "ui").fmt(sOped);

        /// TODO: This could be more optimized,
        ///       but it does it's work quite good for now.
        this->refreshPlayerList();
        this->gotSystemMessage(sMsg);
        return true;
    }

    case DSRV_MSG_CHAT_DEOPED:
    {
        // Someone has been de-Op'ed
        String sDeoped = in_pack.get_string();
        String sMsg = getTranslatedString("Chat_Deoped", "ui").fmt(sDeoped);

        /// TODO: Like above, this could be more optimized,
        ///       but it does it's work quite good for now.
        this->refreshPlayerList();
        this->gotSystemMessage(sMsg);
        return true;
    }

    default:
        return false;
    }

    return false;
}

 /* EOF */
