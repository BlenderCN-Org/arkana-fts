#ifndef D_CLIENT_H
#  define D_CLIENT_H

#  include <list>
#  include <map>

#  include "utilities/threading.h"
#  include "net/connection.h"
#include "ClientsManager.h"

namespace FTS {
    class Packet;
    class Connection;
}


namespace FTSSrv2 {
    class Game;
    class Channel;

class Client {
private:
    bool m_bLoggedIn;           ///< Whether the user is logged in or not.
    FTS::String m_sNick;            ///< The nickname of the user that is logged on.
    FTS::String m_sPassMD5;         ///< The MD5 encoded password of the user that is logged on.

    Game *m_pMyGame;           ///< The game I have started or I am in. NULL else.
    Channel *m_pMyChannel;     ///< The chat channel the player is currently in.

    FTS::Connection *m_pConnection; ///< The connection to my client.

public:
    friend class ClientsManager;

    Client(FTS::Connection *in_pConnection);
    virtual ~Client();

    static void starter(Client *in_pThis);
    int run();
    bool workPacket(FTS::Packet *in_pPacket);
    int quit();
    int tellToQuit();

    inline FTS::String getNick() const { return m_sNick; };
    inline bool isLoggedIn() const { return m_bLoggedIn; };

    inline FTS::String getCounterpartIP() const { return m_pConnection->getCounterpartIP(); };

    inline Channel *getMyChannel() const { return m_pMyChannel; };
    inline void setMyChannel(Channel *in_pChannel) { m_pMyChannel = in_pChannel; };
    inline Game *getMyGame() const { return m_pMyGame; };
    inline void setMyGame(Game *in_pGame) { m_pMyGame = in_pGame; };

    int getID() const;
    static int getIDByNick(const FTS::String &in_sNick);

    bool sendPacket(FTS::Packet *in_pPacket);
    int sendChatJoins(const FTS::String &in_sPlayer);
    int sendChatQuits(const FTS::String &in_sPlayer);
    int sendChatOped(const FTS::String &in_sPlayer);
    int sendChatDeOped(const FTS::String &in_sPlayer);
    int sendChatMottoChanged(const FTS::String &in_sFrom, const FTS::String &in_sMotto);

private:

    //     int onNull( void );
    bool onLogin(const FTS::String &in_sNick, const FTS::String &in_sMD5);
    bool onLogout();

    bool onSignup(const FTS::String &in_sNick, const FTS::String &in_sMD5, const FTS::String &in_sEMail);
    bool onFeedback(const FTS::String &in_sMessage);
    bool onPlayerSet(uint8_t in_cField, FTS::Packet *out_pPacket);
    bool onPlayerGet(uint8_t in_cField, const FTS::String &in_sNick, FTS::Packet *out_pPacket);
    bool onPlayerSetFlag(uint32_t in_cFlag, bool in_bValue);

    bool onGameIns(FTS::Packet *out_pPacket);
    bool onGameRem();
    bool onGameLst();
    bool onGameInfo(const FTS::String &in_sName);
    bool onGameStart();

    bool onChatSend(FTS::Packet *out_pPacket);
    bool onChatIuNai();
    bool onChatJoin(const FTS::String &in_sChan);
    bool onChatList();
    bool onChatMottoGet();
    bool onChatMottoSet(const FTS::String &in_sMotto);
    bool onChatUserGet(const FTS::String &in_sUser);
    bool onChatPublics();
    bool onChatKick(const FTS::String &in_sUser);
    bool onChatOp(const FTS::String &in_sUser);
    bool onChatDeop(const FTS::String &in_sUser);
    bool onChatListMyChans();
    bool onChatDestroyChan(const FTS::String &in_sChan);
};


} // namespace FTSSrv2

#endif                          /* D_CLIENT_H */
