#ifndef FTS_UI_MENUONLINEMAINCB_H
#define FTS_UI_MENUONLINEMAINCB_H

#include "main.h"
#include "main/runlevels.h"
#include "utilities/command.h"

namespace FTS {

class ChatMembersListItem;

class OnlineMenuRlv : public OnlineRunlevel {
private:
    /// The root of the CEGUI menu.
    CEGUI::Window *m_pRoot;

    /// A list of all chat messages that have been entered by the player.
    std::list<String>m_lSentMessages;
    /// Where in the list of sent messages I currently am, while browsing the history.
    std::list<String>::const_iterator m_ilSentMessagesPos;

    bool cbChan(const CEGUI::EventArgs& in_ea);
    bool cbAcct(const CEGUI::EventArgs& in_ea);
    bool cbFind(const CEGUI::EventArgs& in_ea);
    bool cbHost(const CEGUI::EventArgs& in_ea);
    bool cbSend(const CEGUI::EventArgs& in_ea);
    bool cbFeedback(const CEGUI::EventArgs& in_ea);
    bool cbSettings(const CEGUI::EventArgs& in_ea);
    bool cbLogout(const CEGUI::EventArgs& in_ea);

    bool cbListClick(const CEGUI::EventArgs& in_ea);
    bool cbUserInfo(const CEGUI::EventArgs& in_ea);
    bool cbUserWhisp(const CEGUI::EventArgs& in_ea);
    bool cbUserMute(const CEGUI::EventArgs& in_ea);
    bool cbUserUnmute(const CEGUI::EventArgs& in_ea);
    bool cbUserKick(const CEGUI::EventArgs& in_ea);
    bool cbUserOp(const CEGUI::EventArgs& in_ea);
    bool cbUserDeop(const CEGUI::EventArgs& in_ea);

    ChatMembersListItem *getSelectedUser();
    int refreshPlayerList();
    int enteringNewChannel(const String &in_sNewChanName);
public:
    OnlineMenuRlv();
    virtual ~OnlineMenuRlv();

    virtual bool load();
    virtual bool unload();
    virtual void render2D(const Clock&);
    virtual bool update(const Clock&);
    virtual String getName();

    int join(const String & in_sChan);
    int addPlayer(const String & in_sName);
    int remPlayer(const String & in_sName);
    int gotSystemMessage(const String & in_sMessage);
    int gotNormalMessage(const String & in_sFrom,const String & in_sMessage);
    int gotOrSentWhispMessage(const String &in_sUser,const String & in_sMessage,bool in_bGot);
    int mottoChange(const String & in_sFrom, const String & in_sMotto);

    bool handleMessage(Packet &in_pack);

    /// This command will interpret a (chat-) message when being executed.
    /// That means it checks if the chat message contains a command (like /join)
    /// and executes that command. If it doesn't, The message will be sent to
    /// the current channel.
    class InterpretCurrMsgCmd : public CommandBase {
        /// The name of the CEGUI::Window that will hold the
        /// text of the message to execute when this command
        /// gets executed.
        String m_sTextHolderName;
        /// Whether to clear the window that I get the text from upon execution
        /// or leave the text as-is.
        bool m_bClearWindow;
    public:
        InterpretCurrMsgCmd(const String &in_sTextHolderName, bool in_bClearWindow);
        virtual ~InterpretCurrMsgCmd();
        virtual bool exec();
    };
    /// This command will travel trough the chat messages history and set a
    /// window's text to the chosen point in history.
    class HistoryNavigationCmd : public CommandBase {
    public:
        /// This decides how to travel trough history.
        enum Mode {
            Back,   ///< Go back in history.
            Forth   ///< Go forth in history.
        };
    private:
        /// The name of the CEGUI::Window that will get its text set to the
        /// message in history. This window has also to be the active window
        /// while the command is being executed.
        String m_sTextReceiverName;
        /// The action I have to do with the history.
        Mode m_eMode;
    public:
        HistoryNavigationCmd(const String &in_sTextReceiverName, Mode in_eMode);
        virtual ~HistoryNavigationCmd();
        virtual bool exec();
    };
};

} // namespace FTS

#endif                          /* FTS_UI_MENUONLINEMAINCB_H */

 /* EOF */
