/**
 * \file player.h
 * \author Pompei2
 * \date 11 May 2007
 * \brief This file describes the class that represents a player,
 *        online or single players are the same !
 **/

#ifndef FTS_PLAYER_H
#define FTS_PLAYER_H

#include "main.h"

#include <list>
#include <string>

#include "dLib/dString/dString.h"

namespace FTS {
    class Connection;

/// The FTS player class
/** This class represents a player, that can start a game,
 *  either single or multiplayer, it's the same.
 **/
class Player {
private:
    String m_sName;  ///< The name of this player in game.

    // ----------------- //
    // Gaming variables. //
    // ----------------- //

    /// This is the connection to the game host.
    /// It connects to localhost when this is a singleplayer game.
    Connection *m_pcHost;

    /// The colour of the player, n ARGB format. This may change over time.
    unsigned int m_uiColour;

    // ----------------------------- //
    // The online account variables. //
    // ----------------------------- //

    /// Wether he is logged in the master server or not.
    bool m_bOnline;

    /// If this player is online, this is the connection to the master-server.
    Connection *m_pcMasterServer;

    /// The MD5 key of the player's password. He has to send it with every
    /// administrative packet he sends to the server.
    String m_sMD5;

    /// A list of all players that are muted by me.
    std::list<String> m_sMutedPlayers;

public:
    Player();
    virtual ~Player();

    void setName(const String & in_sName);
    String getName();

    inline void setColour(unsigned int in_uiColour) {m_uiColour = in_uiColour;};
    inline unsigned int getColour() {return m_uiColour;};

    // ----------------------------- //
    // The online account functions. //
    // ----------------------------- //

    Connection *og_getConnection();
    std::string og_getMD5();
    int og_connectMaster(uint32_t in_uiTimeoutMS = 10*1000);
    int og_login(const String & in_sNickname,
                 const String & in_sPassword);
    int og_logout();
    int og_accountCreate(const String & in_sNickname,
                         const String & in_sPassword,
                         const String & in_sEmail);
    int og_accountSet(uint8_t in_cField, const String & in_sValue);
    int og_accountSetInt(uint8_t in_cField, int in_iValue);
    int og_accountSetFlag(uint32_t in_cFlag, bool in_bValue);
    String og_accountGetFrom(uint8_t in_cField,
                                  const String & in_sNickname);
    int og_accountGetIntFrom(uint8_t in_cField, const String & in_sNickname);

    int og_chatJoin(const String & in_sChannel);
    std::list<String> og_chatMyChans();
    int og_chatRemChan(const String & in_sChannel);
    String og_chatGetCurChannel();
    int og_chatSendMessage(const String & in_sMessage);
    int og_chatWhisp(const String &in_sPlayer, const String & in_sMessage);
    int og_chatCheckEvents();
    int og_chatGetPlayerList(std::list<String> &out_playerList);
    int og_chatRefreshChannelInfo();
    uint8_t og_chatUserGet(const String & in_sUser);
    int og_chatSetMotto(const String & in_sMotto);
    String og_chatGetMotto();
    int og_chatKick(const String &in_sName);
    int og_chatOp(const String &in_sName);
    int og_chatDeop(const String &in_sName);

    void og_mute(const String &in_sName);
    void og_unmute(const String &in_sName);
    bool og_haveMuted(const String &in_sName);

    //      int join( CFTSGame *pGame );
    //      int host( .. ); = start server + join.
};

extern Player* g_pMeHacky;

} // namespace FTS

#endif /* FTS_PLAYER_H */

 /* EOF */
