/**
 * \file player_og.cpp
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file implements the online gaming part of the class that represents a player,
 *        online or single players are the same !
 **/

#include <CEGUI.h>
#include <SDL_timer.h>
#include <list>
#include <connection.h>
#include <dsrv_constants.h>

#include "game/player.h"

#include "ui/ui_menu_online_main.h"
#include "logging/logger.h"
#include "utilities/utilities.h"
#include "main/runlevels.h"
#include "ui/ui_menu_online.h" // To get back there in og_chatCheckEvents.
#include "dLib/dString/dString.h"
#include "dLib/dConf/configuration.h"

using namespace FTS;

/// ONLINE GAMING - Returns the connection to the master server.
/** This returns a pointer to the connection to the master server.
 *  Wether this connection is up or not doesn't matter.
 *
 * \return pointer to the connection to the master server.
 *
 * \author Pompei2
 */
Connection *Player::og_getConnection()
{
    return m_pcMasterServer;
}

/// ONLINE GAMING - Returns the MD5 checksum of the player, if logged in.
/** This returns a string containing the MD5 hash of the user's password.
 *
 * \return The MD5 Checksum of the player.
 *
 * \author Pompei2
 */
std::string FTS::Player::og_getMD5()
{
    return m_sMD5.str();
}

/// ONLINE GAMING - Connects to the master server.
/** This connects the player to the master server. But this DOES NOT LOG IN !
 *
 * \param in_uiTimeoutMS The time to try to connect to the master server, in milliseconds.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note This is usually only called by the class itself, but could be called
 *       to test an option or so !
 *
 * \TODO Call this from options dialog.
 *
 * \author Pompei2
 */
int FTS::Player::og_connectMaster(uint32_t in_uiTimeoutMS)
{
    // Connect to the master server if we are not yet.
    if(m_pcMasterServer != NULL) {
        if(m_pcMasterServer->isConnected())
            return ERR_OK;
        SAFE_DELETE(m_pcMasterServer);
    }

    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    String sServer = conf.get("MasterServerName");
    int iPort = conf.getInt("MasterServerPort");
    unsigned long timeout= conf.getInt( "ConnectionConnectTimeOut" );
    FTSMSGDBG("  Connecting to the master server "+sServer+":"+String::nr(iPort), 3);
    m_pcMasterServer =  Connection::create(Connection::eConnectionType::D_CONNECTION_TRADITIONAL, sServer.c_str(), iPort, timeout);
    if(!m_pcMasterServer->isConnected()) {
        SAFE_DELETE(m_pcMasterServer);
        return -1;
    }
    m_pcMasterServer->setMaxWaitMillisec( conf.getInt( "ConnectionTimeOut" ) );
    return ERR_OK;
}

/// ONLINE GAMING - Creates an online gaming account.
/** This creates an account on the master server to allow the player
 *  to play online. Later, the player can log-in again with that acc.
 *
 * \param in_sNickname    the nickname of the user to create.
 * \param in_sPassword    the password as raw text, will be encoded by this function.
 * \param in_sEmail       the e-mail address of the user to create.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note
 *
 * \author Pompei2
 */
int FTS::Player::og_accountCreate(const String & in_sNickname,
                                 const String & in_sPassword,
                                 const String & in_sEmail)
{
    // Can't create an account while online.
    if(m_bOnline) {
        FTS18N("Ogm_create_online", MsgType::Error);
        return -1;
    }

    if(!in_sNickname || !in_sPassword || !in_sEmail) {
        FTS18N("Ogm_create_missdata", MsgType::Error);
        return -2;
    }

    if(!in_sEmail.contains("@") || !in_sEmail.contains(".")) {
        FTS18N("Ogm_create_mail", MsgType::Error);
        return -3;
    }

    if(in_sNickname.contains(" ") || in_sNickname.contains("\t")) {
        FTS18N("Ogm_create_nick", MsgType::Error);
        return -3;
    }

    if(ERR_OK != this->og_connectMaster())
        return -4;

    // Encrypt the password.
    char buffMD5[32];

    md5Encode(in_sPassword.c_str(), (int)in_sPassword.len(), buffMD5);

    // Cut all the data to the right size.
    String sNick(in_sNickname);
    String sPMD5(buffMD5, 0, 32);
    String sMail(in_sEmail);

    FTSMSGDBG("  Signing up user "+sNick+" with e-mail "+sMail, 3);

    // Create the signup packet.
    Packet *p = new Packet(DSRV_MSG_SIGNUP);

    p->append(sNick.c_str());
    p->append(sPMD5.c_str());
    p->append(sMail.c_str());
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        return -3;
    }
    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        FTS18N("Ogm_create_err", MsgType::Error);
        return -6;
    }

    // Don't need it anymore.
    SAFE_DELETE(p);

    return ERR_OK;
}

/// ONLINE GAMING - Logs in onto the master server.
/** This logs the player into the master server, using a
 *  nickname and a password.
 *
 *  One can only play online if one is logged in.
 *
 * \param in_sNickname    the nickname with which you want to login.
 * \param in_sPassword    the password that comes with that nickname, in clear text.
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \note
 *
 * \author Pompei2
 */
int FTS::Player::og_login(const String & in_sNickname,
                         const String & in_sPassword)
{
    // I'm already online.
    if(m_bOnline)
        return ERR_OK;

    if(!in_sNickname || !in_sPassword) {
        FTS18N("Ogm_login_miss", MsgType::Error);
        return -1;
    }

    if(ERR_OK != this->og_connectMaster())
        return -2;

    // Encrypt the password.
    char buffMD5[32];

    md5Encode(in_sPassword.c_str(), (int)in_sPassword.len(), buffMD5);

    // Cut all the data to the right size and store the MD5 hash.
    String sNick(in_sNickname);
    m_sMD5 = String(buffMD5, 0, 32);

    FTSMSGDBG("  Logging in with the user "+sNick, 3);

    // Create the login packet.
    Packet *p = new Packet(DSRV_MSG_LOGIN);
    p->append(sNick.c_str());
    p->append(m_sMD5.c_str());

    // And login.
    auto errorCode = m_pcMasterServer->mreq( p );
    if( FTSC_ERR::OK != errorCode ) {
        SAFE_DELETE( p );
        return -3;
    }

    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        FTS18N("Ogm_login_err", MsgType::Error);
        m_pcMasterServer->disconnect();
        return -4;
    }

    m_bOnline = true;
    this->setName(in_sNickname);
    SAFE_DELETE(p);

    return ERR_OK;
}

/// ONLINE GAMING - Logs out of the master server.
/** This logs the player out of the master server, so he can't
 *  play online anymore - until he connects again.
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \note
 *
 * \author Pompei2
 */
int FTS::Player::og_logout()
{
    // I'm not online.
    if(!m_pcMasterServer || !m_pcMasterServer->isConnected())
        m_bOnline = false;

    if(!m_bOnline)
        return ERR_OK;

    // Create the logout packet.
    Packet *p = new Packet(DSRV_MSG_LOGOUT);

    FTSMSGDBG("  Logging out", 3);

    p->append(m_sMD5.c_str());
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        goto logout;
    }

    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        goto logout;
    }

    // Don't need it anymore.
    SAFE_DELETE(p);

logout:
    // TODO: stop the keepalive thread.

    m_sMD5 = String::EMPTY;
    m_bOnline = false;
    SAFE_DELETE(m_pcMasterServer);
    m_sName = String::EMPTY;

    return ERR_OK;
}

/// ONLINE GAMING - Sets a property of the account.
/** This Sets a property (Field) of the account to something.
 *  to see a list of possible fields, either refer to the
 *  dokuwiki, category online gaming/networking, or take a look
 *  at server.h, these are the DSRV_TBL_USR_xxx defines.
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \note Some of these can't be set, like the nickname,
 *       the online time and the lastonline and signup
 *       dates.
 *
 *       DSRV_TBL_USR_WINS, DSRV_TBL_USR_LOOSES, DSRV_TBL_USR_DRAWS,
 *       DSRV_TBL_USR_CLAN and DSRV_TBL_USR_FLAGS
 *       have to be set using og_accountSetInt.
 *
 * \author Pompei2
 */
int FTS::Player::og_accountSet(uint8_t in_cField, const String & in_sValue)
{
    // Can't set something when not online.
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_accountSet");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_PLAYER_SET);

    p->append(m_sMD5.c_str());
    p->append(in_cField);
    p->append(in_sValue.c_str());
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        return -3;
    }
    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        FTS18N("Ogm_set_err", MsgType::Error, String::nr(in_cField), in_sValue);
        return -4;
    }

    // Don't need it anymore.
    SAFE_DELETE(p);
    return ERR_OK;
}

/// ONLINE GAMING - Sets an integer property of the account.
/** This Sets a property (Field) of the account to an integer.
 *  to see a list of possible fields, either refer to the
 *  dokuwiki, category online gaming/networking, or take a look
 *  at server.h, these are the DSRV_TBL_USR_xxx defines.
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \note Some of these can't be set, like the nickname,
 *       the online time and the lastonline and signup
 *       dates.
 *
 *       This function can ONLY set the following defines:
 *       DSRV_TBL_USR_WINS, DSRV_TBL_USR_LOOSES, DSRV_TBL_USR_DRAWS,
 *       DSRV_TBL_USR_CLAN and DSRV_TBL_USR_FLAGS
 *       for all others, see the og_accountSet function.
 *
 * \author Pompei2
 */
int FTS::Player::og_accountSetInt(uint8_t in_cField, int in_iValue)
{
    // Can't set something when not online.
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_accountSetInt");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_PLAYER_SET);

    p->append(m_sMD5.c_str());
    p->append(in_cField);
    p->append(in_iValue);
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        return -3;
    }
    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        FTS18N("Ogm_set_err", MsgType::Error, String::nr(in_cField), String::nr(in_iValue));
        return -4;
    }

    // Don't need it anymore.
    SAFE_DELETE(p);
    return ERR_OK;
}

/// ONLINE GAMING - Sets or clears a property flag of the account.
/** This sets or clears a property flag of the account.
 *  To see a list of possible flags, either refer to the
 *  dokuwiki, category online gaming/networking, or take a look
 *  at server.h, these are the DSRV_PLAYER_FLAG_xxx defines.
 *
 * \param in_cFlag The mask of flags you want to set/clear.
 * \param in_bValue Whether to set (true) or clear (false) the flag(s).
 *
 * \return If successful: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note You can also set/clear several flags at a time, the parameter \a in_cFlag
 *       acts as a mask, OR'ing them will work.
 *
 * \author Pompei2
 */
int FTS::Player::og_accountSetFlag(uint32_t in_cFlag, bool in_bValue)
{
    // Can't set something when not online.
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_accountSetFlag");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_PLAYER_SET_FLAG);

    p->append(m_sMD5.c_str());
    p->append(in_cFlag);
    p->append((uint8_t)(in_bValue ? 1 : 0));
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        return -3;
    }
    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        FTS18N("Ogm_set_flag_err", MsgType::Error, String::nr(in_cFlag), String::b(in_bValue));
        return -4;
    }

    // Don't need it anymore.
    SAFE_DELETE(p);
    return ERR_OK;
}

/// ONLINE GAMING - Gets a property of an account.
/** This Gets a property (Field) of the account from someone.
 *  To see a list of possible fields, either refer to the
 *  dokuwiki, category online gaming/networking, or take a look
 *  at server.h, these are the DSRV_TBL_USR_xxx defines.
 *
 * \return If successful: The string.
 * \return If failed:     An empty string.
 *
 * \note The flags
 *       DSRV_TBL_USR_WEEKON, DSRV_TBL_USR_TOTALON, DSRV_TBL_USR_WINS,
 *       DSRV_TBL_USR_LOOSES, DSRV_TBL_USR_DRAWS, DSRV_TBL_USR_CLAN,
 *       and DSRV_TBL_USR_FLAGS
 *       have to be get using og_accountGetFromInt.
 *
 * \author Pompei2
 */
String FTS::Player::og_accountGetFrom(uint8_t in_cField, const String & in_sNickname)
{
    // Can't get something when not online.
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_accountGetFrom");
        return String::EMPTY;
    }

    // Create the packet.
    Packet p(DSRV_MSG_PLAYER_GET);

    p.append(m_sMD5.c_str());
    p.append(in_cField);
    p.append(in_sNickname.c_str());
    auto mreqError = m_pcMasterServer->mreq( &p );
    if( FTSC_ERR::OK != mreqError ) {
        FTSMSGDBG( "Receive error master request for DSRV_MSG_PLAYER_GET {1}", 3, String::nr( (int) mreqError ) );
        return String::EMPTY;
    }
    auto packetError = p.get();
    if( packetError != ERR_OK ) {
        FTSMSGDBG( "Receive error master request for DSRV_MSG_PLAYER_GET ret = {1}", 3, String::nr( packetError ) );
        FTS18N("Ogm_get_err", MsgType::Error, String::nr(in_cField), in_sNickname);
        return String::EMPTY;
    }

    auto recvField = p.get();
    if( recvField != in_cField ) {
        FTSMSGDBG( "Receive error master request for DSRV_MSG_PLAYER_GET expected field {1} , received {2}", 3, String::nr( in_cField ), String::nr( recvField ) );
        FTS18N("Ogm_get_err", MsgType::Error, String::nr(in_cField), in_sNickname);
        return String::EMPTY;
    }

    // Extract the information.
    return p.get_string();
}

/// ONLINE GAMING - Gets a property of an account.
/** This Gets a property (Field) of the account from someone.
 *  To see a list of possible fields, either refer to the
 *  dokuwiki, category online gaming/networking, or take a look
 *  at server.h, these are the DSRV_TBL_USR_xxx defines.
 *
 * \return If successful: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note This function can ONLY get the following fields:
 *       DSRV_TBL_USR_WEEKON, DSRV_TBL_USR_TOTALON, DSRV_TBL_USR_WINS,
 *       DSRV_TBL_USR_LOOSES, DSRV_TBL_USR_DRAWS, DSRV_TBL_USR_CLAN,
 *       and DSRV_TBL_USR_FLAGS
 *       to get other ones, use og_accountGetFrom function.
 *
 * \author Pompei2
 */
int FTS::Player::og_accountGetIntFrom(uint8_t in_cField, const String & in_sNickname)
{
    // Can't get something when not online.
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_accountGetIntFrom");
        return -2;
    }

    // Create the packet.
    Packet p(DSRV_MSG_PLAYER_GET);

    p.append(m_sMD5.c_str());
    p.append(in_cField);
    p.append(in_sNickname.c_str());
    auto mreqError = m_pcMasterServer->mreq( &p );
    if( FTSC_ERR::OK != mreqError ) {
        FTSMSGDBG( "Receive error master request for DSRV_MSG_PLAYER_GET {1}", 3, String::nr((int)mreqError) );
        return -3;
    }
    auto packetError = p.get();
    if(packetError != ERR_OK) {
        FTSMSGDBG( "Receive error master request for DSRV_MSG_PLAYER_GET ret = {1}", 3, String::nr(packetError) );
        FTS18N("Ogm_get_err", MsgType::Error, String::nr(in_cField), in_sNickname);
        return -4;
    }

    auto recvField = p.get();
    if(recvField != in_cField) {
        FTSMSGDBG( "Receive error master request for DSRV_MSG_PLAYER_GET expected field {1} , received {2}", 3, String::nr( in_cField ), String::nr(recvField) );
        FTS18N("Ogm_get_err", MsgType::Error, String::nr(in_cField), in_sNickname);
        return -5;
    }

    // Extract the information.
    int iRet;
    p.get(iRet);
    return iRet;
}

/// Joins a chat channel
/** This function joins a new chat channel. After joining the channel,
 *  This also gets the players list and all other channel properties.
 *
 * \param in_sChannel The name of the channel to join.
 *
 * \return If successful: ERR_OK
 * \return If failed:     An error code <0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatJoin(const String & in_sChannel)
{
    if( !in_sChannel ) {
        FTS18N("Ogm_chat_join_miss", MsgType::Error);
        return -1;
    }

    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatJoin");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_CHAT_JOIN);
    p->append(m_sMD5.c_str());
    p->append(in_sChannel.c_str() );

    // Send the join message.
    // The server will make us leave the current channel if we are in one.
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        return -3;
    }

    // If this did not work, we should still be in the old channel.
    if(!p || (p->get() != ERR_OK)) {
        FTS18N("Ogm_chat_join_err", MsgType::Error, in_sChannel);
        SAFE_DELETE(p);
        return -4;
    }

    SAFE_DELETE(p);
    return ERR_OK;
}

/// Get a list of chat channel which I'm the administrator of.
/** This function returns a list of names of chat channels where I'm the admin.
 *
 * \return A list of names of chat channels where I'm the admin.
 *
 * \author Pompei2
 */
std::list<String> FTS::Player::og_chatMyChans()
{
    std::list<String> sChans;

    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatMyChans");
        return sChans;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_CHAT_LIST_MY_CHANS);
    p->append(m_sMD5.c_str() );

    // Send the message.
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        return sChans;
    }

    // This did not work? Just report an emtpy list.
    if(!p || (p->get() != ERR_OK)) {
        FTS18N("Ogm_chat_list_my_err", MsgType::Error);
        SAFE_DELETE(p);
        return sChans;
    }

    uint32_t nChans = 0;
    p->get(nChans);

    for(uint32_t i = 0 ; i < nChans ; ++i) {
        sChans.push_back(p->get_string());
    }

    SAFE_DELETE(p);
    return sChans;
}

/// Destroy a channel which I'm the administrator of.
/** This function can destroy a channel, but only if I'm the administrator of
 *  that channel.
 *
 * \param in_sChannel The name of the channel to destroy.
 *
 * \return If successful: ERR_OK
 * \return If failed:     An error code <0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatRemChan(const String & in_sChannel)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatMyChans");
        return -1;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_CHAT_DESTROY_CHAN);
    p->append(m_sMD5.c_str() );
    p->append(in_sChannel.c_str() );

    // Send the message.
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE( p );
        return -2;
    }

    // This did not work? Just report an emtpy list.
    if(!p || (p->get() != ERR_OK)) {
        FTS18N("Ogm_chat_rem_chan_err", MsgType::Error, in_sChannel);
        SAFE_DELETE(p);
        return -3;
    }

    SAFE_DELETE(p);
    return ERR_OK;
}

/// ONLINE GAMING - Returns the current channel of the player, if logged in.
/** This returns a string containing the name of the channel the player is
 *  currently in, or String::EMPTY if he is in no channel.
 *
 * \return The current channel of the player.
 *
 * \author Pompei2
 */
String FTS::Player::og_chatGetCurChannel()
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatGetCurChannel");
        return String::EMPTY;
    }

    Packet *p = new Packet(DSRV_MSG_CHAT_IUNAI);
    p->append(m_sMD5.c_str() );

    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return String::EMPTY;
    }
    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        FTS18N("Ogm_chat_iunai", MsgType::Error);
        return String::EMPTY;
    }

    String sWhere = p->get_string();

    SAFE_DELETE(p);
    return sWhere;
}

/// ONLINE GAMING - sends a chat message to the server.
/** This gets called when the user clicks on send or presses enter and typed
 *  just a normal message with no command.
 *
 * \param in_sMessage The contents of the message to send
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatSendMessage(const String & in_sMessage)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatSendMessage");
        return -2;
    }

    Packet *p = new Packet(DSRV_MSG_CHAT_SENDMSG);
    p->append(m_sMD5.c_str() );

    p->append(DSRV_CHAT_TYPE::NORMAL);
    p->append((uint8_t)0);
    p->append(in_sMessage.c_str() );

    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -1;
    }
    if(!p || (p->get() != ERR_OK)) {
        SAFE_DELETE(p);
        FTS18N("Ogm_chat_sendmsg_err", MsgType::Error);
        return -2;
    }

    SAFE_DELETE(p);
    return ERR_OK;
}

/// ONLINE GAMING - sends a private message to a player.
/** This gets called when the user writes a private message
 *  (aka whispers) to another player.
 *
 * \param in_sPlayer The player to send the message to
 * \param in_sMessage The contents of the message to send
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatWhisp(const String &in_sPlayer, const String & in_sMessage)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatSendMessage");
        return -2;
    }

    Packet *p = new Packet(DSRV_MSG_CHAT_SENDMSG);
    p->append(m_sMD5.c_str() );

    p->append(DSRV_CHAT_TYPE::WHISPER);
    p->append((uint8_t)0);
    p->append(in_sPlayer.c_str() );
    p->append(in_sMessage.c_str() );

    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -1;
    }

    if(p->get() == ERR_OK) {
        // If we are in the online menu, we want to put the message into the
        // chatbox too, so the user sees what he just whispered.
        OnlineMenuRlv *pRlv = dynamic_cast<OnlineMenuRlv *>(RunlevelManager::getSingleton().getCurrRunlevel());
        if(pRlv != NULL) {
            pRlv->gotOrSentWhispMessage(in_sPlayer,in_sMessage,false);
        } else {
            /// \todo If we are not in the online menu, we may be in the game or so,
            /// still show the whispered message.
        }
    } else {
        FTS18N("Ogm_chat_whisp_err", MsgType::Warning, in_sPlayer);
    }

    SAFE_DELETE(p);
    return ERR_OK;
}

/// ONLINE GAMING - Checks if something happened in the channel.
/** This gets called once every frame and looks if there is a message
 *  (FTSPacket) in the queue. If there is and it is packet related to
 *  chatting or the channel, it does the according thing.
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \note Currently, this function makes the game loose all other messages.
 *
 * \author Pompei2
 */
int FTS::Player::og_chatCheckEvents()
{
    if(!m_pcMasterServer->isConnected()) {
        // No more connection ? go back to the login menu.
        m_bOnline = false;
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new LoginMenuRlv());
        return ERR_OK;
    }

    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatCheckEvents");
        return -2;
    }

    // Get up to five packets that are left here for us.
    for(int i = 0; i < 5 ; i++) {
        Packet *pPack = m_pcMasterServer->getReceivedPacketIfAny();
        if(pPack == NULL)
            break;

        // Give it to the runlevel, maybe he wants to handle it.
        Runlevel *pBRlv = RunlevelManager::getSingleton().getCurrRunlevel();
        OnlineRunlevel *pRlv = dynamic_cast<OnlineRunlevel *>(pBRlv);
        if(pRlv != NULL && !pRlv->handleMessage(*pPack)) {
            // No runlevel that is capable to handle the message ...
            // We do nothing with it then right now except a warning ...
            FTSMSGDBG("Awww shit, got the message with id {1} in "
                      "og_chatCheckEvents which did not get handled !\n"
                      "Current runlevel is: {2}\n", MsgType::WarningNoMB,
                      String::nr(pPack->getType(), -1, '0', std::ios::hex),pBRlv->getName());
        }
    }

    return ERR_OK;
}

/// ONLINE GAMING - Refreshes the list of players in the channel.
/** This function gets a list of all players that are in the current channel
 *  and passes them to the argument.
 *
 * \param out_playerList
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatGetPlayerList(std::list<String> &out_playerList)
{
    out_playerList.clear();

    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatGetPlayerList");
        return -1;
    }

    // And get a list of the currently logged in user names.
    Packet *p = new Packet(DSRV_MSG_CHAT_LIST);

    p->append(m_sMD5.c_str() );

    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -2;
    }

    // Silently ignore. But ok, log something.
    if(!p || p->get() != ERR_OK) {
        FTSMSGDBG("Hmmm, error getting the players list", 3);
        SAFE_DELETE(p);
        return -3;
    }

    // Get the answer.
    int32_t nPlayers;
    p->get(nPlayers);

    // Now we get all nicknames and create them.
    for(int32_t i = 0; i < nPlayers; i++) {
        out_playerList.push_back(p->get_string());
    }

    SAFE_DELETE(p);

    return ERR_OK;
}

/// ONLINE GAMING - Refreshes the informations about the current channel.
/** This function gets the motto of the current channel and adapts the
 *  informations in the gui accordingly.
 *
 * \return If successful: ERR_OK
 * \return If failed:     Error code < 0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatRefreshChannelInfo()
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatRefreshChannelInfo");
        return -1;
    }

    // And get a list of the currently logged in user names.
    Packet *p = new Packet(DSRV_MSG_CHAT_MOTTO_GET);

    p->append(m_sMD5.c_str() );

    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -2;
    }

    // Silently ignore. But ok, log something.
    if(!p || p->get() != ERR_OK) {
        FTSMSGDBG("Hmmm, got no answer when asking for the channel", 3);
        return -3;
    }

    // Get the answer.
    String sMotto = p->get_string();

    // Get the current runlevel to call appropriate methods.
    // TODO: Move this around into the rlv itself ??? Its only here cause of connection.
    Runlevel *pBRlv = RunlevelManager::getSingleton().getCurrRunlevel();
    if(pBRlv->getName() != "Online") {
        return -5;
    }
    OnlineMenuRlv *pRlv = dynamic_cast<OnlineMenuRlv *>(pBRlv);
    if(pRlv == NULL) {
        return -5;
    }

    // Now we set the motto in the gui.
    pRlv->mottoChange(String::EMPTY, sMotto);

    SAFE_DELETE(p);

    return ERR_OK;
}

/// Gets the state of a user in the channel.
/** This function gets the state of a user in a chat channel. The state
 *  means wether the user is a normal user, an operator or the admin.
 *
 * \param in_sUser The name of the user to get the state.
 *
 * \return If successful: DSRV_CHAT_USER (0 = normal user, 1 = channel operator, 2 = channel admin).
 * \return If failed:     DSRV_CHAT_USER::UNKNOWN
 *
 * \note The players list CEGUI::Listbox has to be present for this function to work !
 *
 * \author Pompei2
 */
DSRV_CHAT_USER FTS::Player::og_chatUserGet(const String & in_sUser)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatUserGet");
        return DSRV_CHAT_USER::UNKNOWN;
    }

    // Create the packet.
    Packet p(DSRV_MSG_CHAT_USER_GET);
    p.append(m_sMD5.c_str() );
    p.append(in_sUser.c_str() );

    // Send the message.
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( &p ) ) {
        return DSRV_CHAT_USER::UNKNOWN;
    }

    if(p.get() != ERR_OK) {
        FTS18N("Ogm_chat_userget_err", MsgType::Error, in_sUser);
        return DSRV_CHAT_USER::UNKNOWN;
    }

    char cState = p.get();

    return (DSRV_CHAT_USER)cState;
}

/// Sets the motto of a channel (if allowed).
/** This function sets the motto of the current channel, if this user is allowed to
 *  do so. If not, it displays an error message.
 *
 * \param in_sMotto The new motto to set.
 *
 * \return If successful: ERR_OK.
 * \return If failed:     An error code <0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatSetMotto(const String & in_sMotto)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatSetMotto");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_CHAT_MOTTO_SET);
    p->append(m_sMD5.c_str() );
    p->append(in_sMotto.c_str() );

    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -3;
    }

    int8_t errcode = p->get();
    if(!p || errcode != ERR_OK) {
        SAFE_DELETE(p);
        FTS18N("Ogm_chat_generr", MsgType::Error, String::nr(errcode));
        return -4;
    }

    SAFE_DELETE(p);

    return ERR_OK;
}

/// Gets the motto of a channel.
/** This function gets the motto of the current channel.
 *
 * \return If successful: The motto of the current channel.
 * \return If failed:     String::EMPTY
 *
 * \author Pompei2
 */
String FTS::Player::og_chatGetMotto()
{
    Packet *p = new Packet(DSRV_MSG_CHAT_MOTTO_GET);
    p->append(m_sMD5.c_str() );

    // Send the message.
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return String::EMPTY;
    }

    // If this did not work, we should altough be in the current channel,
    // Just leave the motto empty.
    if(!p || (p->get() != ERR_OK)) {
        FTS18N("Ogm_chat_motto_err", MsgType::Error, this->og_chatGetCurChannel());
        SAFE_DELETE(p);
        return String::EMPTY;
    }

    String sMotto = p->get_string();
    SAFE_DELETE(p);

    return sMotto;
}

/// Kick a player out of the channel (if allowed).
/** This kicks a player out of the current channel, if this user is allowed to
 *  do so. If not, it does nothing.
 *
 * \param in_sName The name of the player to kick.
 *
 * \return If successful: ERR_OK.
 * \return If failed:     An error code <0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatKick(const String &in_sName)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatKick");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_CHAT_KICK);
    p->append(m_sMD5.c_str() );
    p->append(in_sName.c_str() );

    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -3;
    }
    int8_t errcode = p->get();
    if(!p || errcode != ERR_OK) {
        SAFE_DELETE(p);
        FTS18N("Ogm_chat_generr", MsgType::Error, String::nr(errcode));
        return -4;
    }

    SAFE_DELETE(p);

    return ERR_OK;
}

/// Make a player channel operator (if allowed).
/** This makes a player operator of the current channel, if this user is allowed to
 *  do so. If not, it does nothing.
 *
 * \param in_sName The name of the player to op.
 *
 * \return If successful: ERR_OK.
 * \return If failed:     An error code <0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatOp(const String &in_sName)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatOp");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_CHAT_OP);
    p->append(m_sMD5.c_str() );
    p->append(in_sName.c_str() );

    // Send the message. This message awaits no answer !
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -3;
    }
    if(!p || p->get() != ERR_OK) {
        SAFE_DELETE(p);
        FTS18N("Ogm_chat_operr", MsgType::Error);
        return -4;
    }

    SAFE_DELETE(p);

    return ERR_OK;
}

/// Remove a player's channel operator status (if allowed).
/** This removes a player's operator status in the current channel, if this user is allowed to
 *  do so. If not, it does nothing.
 *
 * \param in_sName The name of the player to deop.
 *
 * \return If successful: ERR_OK.
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int FTS::Player::og_chatDeop(const String &in_sName)
{
    if( !m_bOnline ) {
        FTS18N("Ogm_offline", MsgType::Error, "og_chatDeop");
        return -2;
    }

    // Create the packet.
    Packet *p = new Packet(DSRV_MSG_CHAT_DEOP);
    p->append(m_sMD5.c_str() );
    p->append(in_sName.c_str() );

    // Send the message. This message awaits no answer !
    if( FTSC_ERR::OK != m_pcMasterServer->mreq( p ) ) {
        SAFE_DELETE(p);
        return -3;
    }
    int8_t errcode = p->get();
    if(!p || errcode != ERR_OK) {
        SAFE_DELETE(p);
        FTS18N("Ogm_chat_deoperr", MsgType::Error);
        return -4;
    }

    SAFE_DELETE(p);

    return ERR_OK;
}

/// Mute a player.
/** This function adds a player to the list of muted players.
 *  If a player is muted, you will not handle messages that you
 *  get from him.
 *
 * \param in_sName The name of the player to mute.
 *
 * \note You can't mute yourself.
 *
 * \author Pompei2
 */
void FTS::Player::og_mute(const String &in_sName)
{
    if(in_sName == m_sName)
        return;

    if(this->og_haveMuted(in_sName))
        return;

    m_sMutedPlayers.push_back(in_sName);
}

/// Unmute a player.
/** This function removes a player from the list of muted players.
 *  If a player is muted, you will not handle messages that you
 *  get from him.
 *
 * \param in_sName The name of the player to unmute.
 *
 * \author Pompei2
 */
void FTS::Player::og_unmute(const String &in_sName)
{
    m_sMutedPlayers.remove(in_sName);
}

/// Check if a player is muted.
/** This function checks if the player is on the muted players list.
 *  If a player is muted, you will not handle messages that you
 *  get from him.
 *
 * \param in_sName The name of the player to mute.
 *
 * \return true if the player is muted, false if the player is not muted.
 *
 * \author Pompei2
 */
bool FTS::Player::og_haveMuted(const String &in_sName)
{
    for(std::list<String>::const_iterator i = m_sMutedPlayers.begin() ; i != m_sMutedPlayers.end() ; i++ ) {
        if((*i) == in_sName)
            return true;
    }

    return false;
}
