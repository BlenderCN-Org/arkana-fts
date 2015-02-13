
#include <thread>
#include <chrono>

#include "client.h"
#include "server.h"
#include "db.h"
#include "channel.h"
#include "game.h"
#include "server_log.h"

#define DETAILED_LOG 0

using namespace FTS;
using namespace FTSSrv2;

FTSSrv2::Client::Client(Connection *in_pConnection)
    : m_bLoggedIn(false)
    , m_pMyGame(NULL)
    , m_pMyChannel(NULL)
    , m_pConnection(in_pConnection)
{
}

FTSSrv2::Client::~Client()
{
    delete m_pConnection;
}

int FTSSrv2::Client::getID() const
{
    if( !m_bLoggedIn || m_sNick.empty() ) {
        FTSMSG("Want to get ID of not logged-in client", MsgType::Error);
        return -1;
    }

    return FTSSrv2::Client::getIDByNick(m_sNick);
}

void *FTSSrv2::Client::starter(void *in_pThis)
{
    FTSSrv2::Client *pThis = static_cast<FTSSrv2::Client *>(in_pThis);
    pThis->run();
    return NULL;
}

int FTSSrv2::Client::run()
{
    // Only wait for packets as long as there is a connection up.
    // If we lost the connection, most of the time it means that
    // the user logged out.
    while(m_pConnection->isConnected()) {
        Packet *pPack = m_pConnection->waitForThenGetPacket(false, 500);

        if(!pPack) {
            std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );
            continue;
        }

        // False returned means the connection shall be closed.
        auto rc = this->workPacket( pPack );
        delete pPack;
        if(!rc) {
            m_pConnection->disconnect();
            break;
        }
    }

    // Disconnect the player.
    this->quit();

    delete this;
    return 0;
}

// This closes the connection, logs the user out, leaves the channel if needed.
int FTSSrv2::Client::quit()
{
    int iRet = ERR_OK;
    Lock l (m_mutex);

    // Logout only if needed.
    if(m_bLoggedIn) {
        // Leave the current game.
        if(m_pMyGame) {
            m_pMyGame->playerLeft(this->getNick());
            m_pMyGame = NULL;
        }

        // Kill all games hosted by me.
        Game *pGameHostedByMe = GameManager::getManager()->findGameByHost(m_sNick);
        if(pGameHostedByMe) {
            GameManager::getManager()->remGame(pGameHostedByMe);
        }

        // Leave the current chat room.
        if(m_pMyChannel) {
            m_pMyChannel->quit(this);
            m_pMyChannel = NULL;
        }

        // Call the stored procedure to logout.
        iRet = DataBase::getUniqueDB()
                        ->storedFunctionInt(String("disconnect"),
                                            "\'" + DataBase::getUniqueDB()->escape(m_sNick) + "\', "
                                            "\'" + m_pConnection->getCounterpartIP() + "\',"
                                            "\'" + DataBase::getUniqueDB()->escape(m_sPassMD5) + "\',"
                                            "\'" + String::nr(DSRV_PROC_CONNECT_INGAME) + "\'");

        if(iRet != ERR_OK) {
            FTSMSG("failed (sql logout script returned "+String::nr(iRet)+")", MsgType::Error);
        }

        dynamic_cast<ServerLogger *>(FTS::Logger::getSingletonPtr())->remPlayer();
    }

    // destroy the "session data".
    m_sNick = String::EMPTY;
    m_sPassMD5 = String::EMPTY;
    m_bLoggedIn = false;

    FTSSrv2::ClientsManager::getManager()->unregisterClient(this);

    return iRet;
}

int FTSSrv2::Client::tellToQuit()
{
    // Closing the connection makes the "quitting"-stone rolling.
    Lock l(m_mutex);
    m_pConnection->disconnect();
    return ERR_OK;
}

int FTSSrv2::Client::getIDByNick(const String & in_sNick)
{
    String sQuery;
    MYSQL_RES *pRes = NULL;
    MYSQL_ROW pRow = NULL;

    // Query the id of the user.
    sQuery = "SELECT `"+DataBase::getUniqueDB()->TblUsrField(DSRV_TBL_USR_ID)+"`"
             " FROM `"DSRV_TBL_USR"`"
             " WHERE `"+DataBase::getUniqueDB()->TblUsrField(DSRV_TBL_USR_NICK)+"`"
                "=\'" + DataBase::getUniqueDB()->escape(in_sNick) + "\'";
    if(!DataBase::getUniqueDB()->query(pRes, sQuery)) {
        DataBase::getUniqueDB()->free(pRes);
        FTSMSG("Error during SQL-query in getIDByNick for \"" + in_sNick + "\"", MsgType::Error);
        return -2;
    }

    // Get the query result.
    if(pRes == NULL || NULL == (pRow = mysql_fetch_row(pRes))) {
        FTSMSG("Error during sql fetch row in getID: "+DataBase::getUniqueDB()->getError(), MsgType::Error);
        DataBase::getUniqueDB()->free(pRes);
        return -3;
    }

    int iRet = pRow[0] == NULL ? -4 : atoi(pRow[0]);
    DataBase::getUniqueDB()->free(pRes);

    return iRet;
}

// Returning true keeps the connection up. False would close the connection.
bool FTSSrv2::Client::workPacket(Packet *in_pPacket)
{
#if DETAILED_LOG
    FTSMSGDBG("\n\ngot message 0x",String::nr(in_pPacket->getType(),std::ios::hex), 5);
#endif

    // Preliminary checks.
    master_request_t req = in_pPacket->getType();
    if(
        req != DSRV_MSG_NULL &&
        req != DSRV_MSG_LOGIN &&
        req != DSRV_MSG_SIGNUP &&
        req < DSRV_MSG_MAX
      ) {
        // For all messages else then the above, we may already
        // check if the player is logged in and also get the MD5
        // password hash and check it to be correct.
        int8_t iRet = ERR_OK;
        String sMD5 = in_pPacket->get_string();

        // Check if the user is logged in or not.
        if(!m_bLoggedIn) {
            FTSMSG("failed: not logged in !", MsgType::Error);
            iRet = 10;
        } else {
            // A try to hack ?
            if(m_sPassMD5 != sMD5) {
                FTSMSG("failed: wrong md5 csum ("+sMD5+", correct would be "+m_sPassMD5+")", MsgType::Error);
                iRet = 11;
            }
        }

        // When there was an error, send back the error message and quit.
        if(iRet != ERR_OK) {
            Packet Pack(req);
            Pack.append(iRet);
            m_pConnection->send(&Pack);
            return true;
        }
    }

    // Here begins the real message working.
    switch(in_pPacket->getType()) {
    case DSRV_MSG_NULL:
        //             this->onNull( );
        break;

    case DSRV_MSG_LOGIN:
        {
            String sNick = in_pPacket->get_string();
            String sMD5 = in_pPacket->get_string();

            return this->onLogin(sNick.trim(), sMD5);
        }
    case DSRV_MSG_LOGOUT:
        {
            return this->onLogout();
        }
    case DSRV_MSG_SIGNUP:
        {
            String sNick = in_pPacket->get_string();
            String sMD5 = in_pPacket->get_string();
            String sEMail = in_pPacket->get_string();
            return this->onSignup(sNick.trim(), sMD5, sEMail.trim());
        }
    case DSRV_MSG_FEEDBACK:
        return this->onFeedback(in_pPacket->get_string().trim());
    case DSRV_MSG_PLAYER_SET:
        {
            uint8_t cField;
            in_pPacket->get(cField);

            return this->onPlayerSet(cField, in_pPacket);
        }
    case DSRV_MSG_PLAYER_GET:
        {
            uint8_t cField;
            in_pPacket->get(cField);
            String sNick = in_pPacket->get_string();

            return this->onPlayerGet(cField, sNick.trim(), in_pPacket);
        }
    case DSRV_MSG_PLAYER_SET_FLAG:
        {
            uint32_t cFlag;
            in_pPacket->get(cFlag);
            uint8_t bValue;
            in_pPacket->get(bValue);

            return this->onPlayerSetFlag(cFlag, bValue != 0);
        }
    case DSRV_MSG_GAME_INS:
        return this->onGameIns(in_pPacket);
    case DSRV_MSG_GAME_REM:
        return this->onGameRem();
    case DSRV_MSG_GAME_LST:
        return this->onGameLst();
    case DSRV_MSG_GAME_INFO:
        return this->onGameInfo(in_pPacket->get_string().trim());
    case DSRV_MSG_GAME_START:
        return this->onGameStart();
    case DSRV_MSG_CHAT_SENDMSG:
        return this->onChatSend(in_pPacket);
    case DSRV_MSG_CHAT_IUNAI:
        return this->onChatIuNai();
    case DSRV_MSG_CHAT_JOIN:
        return this->onChatJoin(in_pPacket->get_string().trim());
    case DSRV_MSG_CHAT_LIST:
        return this->onChatList();
    case DSRV_MSG_CHAT_MOTTO_GET:
        return this->onChatMottoGet();
    case DSRV_MSG_CHAT_MOTTO_SET:
        return this->onChatMottoSet(in_pPacket->get_string().trim());
    case DSRV_MSG_CHAT_USER_GET:
        return this->onChatUserGet(in_pPacket->get_string().trim());
    case DSRV_MSG_CHAT_PUBLICS:
        return this->onChatPublics();
    case DSRV_MSG_CHAT_KICK:
        return this->onChatKick(in_pPacket->get_string().trim());
    case DSRV_MSG_CHAT_OP:
        return this->onChatOp(in_pPacket->get_string().trim());
    case DSRV_MSG_CHAT_DEOP:
        return this->onChatDeop(in_pPacket->get_string().trim());
    case DSRV_MSG_CHAT_LIST_MY_CHANS:
        return this->onChatListMyChans();
    case DSRV_MSG_CHAT_DESTROY_CHAN:
        return this->onChatDestroyChan(in_pPacket->get_string().trim());
    default:
        FTSMSG("[WARNING] message with ID '0x"+String::nr(in_pPacket->getType(),-1,'0',std::ios::hex)+"' not yet supported, ignoring.", MsgType::Error);
        break;
    }

    return false;
}

int FTSSrv2::Client::sendPacket(Packet *in_pPacket)
{
#if DETAILED_LOG
    FTSMSGDBG("\n\nsent message 0x"+String::nr(in_pPacket->getType(),-1,'0',std::ios::hex), 5);
#endif

    return m_pConnection->send(in_pPacket);
}

int FTSSrv2::Client::sendChatJoins( const String & in_sPlayer )
{
    Packet p(DSRV_MSG_CHAT_JOINS);

    p.append(in_sPlayer);
    this->sendPacket(&p);

    return ERR_OK;
}

int FTSSrv2::Client::sendChatQuits( const String & in_sPlayer )
{
    Packet p(DSRV_MSG_CHAT_QUITS);

    p.append(in_sPlayer);
    this->sendPacket(&p);

    return ERR_OK;
}

int FTSSrv2::Client::sendChatOped( const String & in_sPlayer )
{
    Packet p(DSRV_MSG_CHAT_OPED);

    p.append(in_sPlayer);
    this->sendPacket(&p);

    return ERR_OK;
}

int FTSSrv2::Client::sendChatDeOped( const String & in_sPlayer )
{
    Packet p(DSRV_MSG_CHAT_DEOPED);

    p.append(in_sPlayer);
    this->sendPacket(&p);

    return ERR_OK;
}

int FTSSrv2::Client::sendChatMottoChanged(const String & in_sFrom, const String & in_sMotto)
{
    Packet p(DSRV_MSG_CHAT_MOTTO_CHANGED);

    p.append(in_sFrom);
    p.append(in_sMotto);

    this->sendPacket(&p);

    return ERR_OK;
}

bool FTSSrv2::Client::onLogin(const String & in_sNick, const String & in_sMD5)
{
    Packet p(DSRV_MSG_LOGIN);
    int iRet = ERR_OK;

    FTSMSGDBG(in_sNick+" is logging in ... ", 1);

    // Call the stored procedure to login.
    String sIP = m_pConnection->getCounterpartIP();
    iRet = DataBase::getUniqueDB()
                    ->storedFunctionInt("connect",
                                        "\'" + DataBase::getUniqueDB()->escape(in_sNick) + "\', " +
                                        "\'" + DataBase::getUniqueDB()->escape(in_sMD5)  + "\', " +
                                        "\'" + DataBase::getUniqueDB()->escape(sIP) + "\'," +
                                        "\'" + String::nr(DSRV_PROC_CONNECT_INGAME) + "\'");

    Lock l(m_mutex);

    if(iRet != ERR_OK) {
        FTSMSG("failed: sql login script returned "+String::nr(iRet), MsgType::Error);
        goto error;
    }

    // save some "session data".
    m_sNick = in_sNick;
    m_sPassMD5 = in_sMD5;
    m_bLoggedIn = true;
    dynamic_cast<ServerLogger *>(FTS::Logger::getSingletonPtr())->addPlayer();
    FTSSrv2::ClientsManager::getManager()->registerClient(this);

    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    p.setType(DSRV_MSG_LOGIN);
    p.append((int8_t)iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    // If there is an error during the login, close the connection.
    if(iRet != ERR_OK)
        return false;

    return true;
}

bool FTSSrv2::Client::onLogout()
{
    Packet p(DSRV_MSG_LOGOUT);
    int iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is logging out ... ", 1);

    if(ERR_OK == (iRet = this->quit())) {
        FTSMSGDBG("success", 1);
    }

    p.append((int8_t)iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);
    return false;
}

bool FTSSrv2::Client::onSignup(const String & in_sNick,
                       const String & in_sMD5,
                       const String & in_sEMail)
{
    Packet p(DSRV_MSG_SIGNUP);
    String sArgs;
    int iRet = ERR_OK;

    FTSMSGDBG("signing up "+in_sNick+" ("+in_sEMail+") ... ", 1);

    // We are logged in but wanna signup ?
    if(m_bLoggedIn) {
        FTSMSG("failed: logged in !", MsgType::Error);
        iRet = 10;
        goto error;
    }

    if(in_sNick.contains(" ") || in_sNick.contains("\t")) {
        FTSMSG("failed: spaces in nick !", MsgType::Error);
        iRet = 11;
        goto error;
    }

    sArgs = "\'" + DataBase::getUniqueDB()->escape(in_sNick) + "\'," +
            "\'" + DataBase::getUniqueDB()->escape(in_sMD5) + "\'," +
            "\'" + DataBase::getUniqueDB()->escape(in_sEMail) + "\'";

    // Call the stored procedure to signup.
    iRet = DataBase::getUniqueDB()->storedFunctionInt("signup", sArgs );
    if(iRet != ERR_OK) {
        FTSMSG("failed: sql signup script returned "+String::nr(iRet), MsgType::Error);
        goto error;
    }

    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    p.append((int8_t)iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onFeedback(const String &in_sMessage)
{
    int8_t iRet = ERR_OK;
    String sQuery = "INSERT INTO `"DSRV_TBL_FEEDBACK"` ("
                        " `"+DataBase::getUniqueDB()->TblFeedbackField(DSRV_TBL_FEEDBACK_NICK)+"`,"
                        " `"+DataBase::getUniqueDB()->TblFeedbackField(DSRV_TBL_FEEDBACK_MSG)+"`,"
                        " `"+DataBase::getUniqueDB()->TblFeedbackField(DSRV_TBL_FEEDBACK_WHEN)+"`) "
                     "VALUES ("
                        " '"+DataBase::getUniqueDB()->escape(this->getNick())+"',"
                        " '"+DataBase::getUniqueDB()->escape(in_sMessage)+"',"
                        " NOW())";

    MYSQL_RES *pRes;
    if(!DataBase::getUniqueDB()->query(pRes, sQuery)) {
        iRet = -1;
    }

    DataBase::getUniqueDB()->free(pRes);

    // Answer.
    Packet p(DSRV_MSG_FEEDBACK);
    p.append(iRet);
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onPlayerSet(uint8_t in_cField, Packet *out_pPacket)
{
    Packet p(DSRV_MSG_PLAYER_SET);
    String sValue, sQuery, sField;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is setting 0x"+String::nr(in_cField,-1,'0',std::ios::hex)+" ... ", 1);

    // Format the value used in the MySQL query string.
    switch(in_cField) {
    case DSRV_TBL_USR_PASS_MD5: // 32
    case DSRV_TBL_USR_MAIL:     // 64
    case DSRV_TBL_USR_JABBER:   // 64
    case DSRV_TBL_USR_CONTACT:  // 255
    case DSRV_TBL_USR_FNAME:    // 32
    case DSRV_TBL_USR_NAME:     // 32
    case DSRV_TBL_USR_BDAY:     // 10
    case DSRV_TBL_USR_CMT:      // 255
        sValue = DataBase::getUniqueDB()->escape(out_pPacket->get_string());
        break;

    case DSRV_TBL_USR_SEX:       // 1 Char
        sValue = String::nr(out_pPacket->get());
        break;

    case DSRV_TBL_USR_CLAN:      // 1 Int
        {
            uint32_t iValue = 0;
            out_pPacket->get(iValue);
            sValue=String::nr(iValue);
        }
        break;

        // Something wrong.
    default:
        FTSMSG("failed: unknown field definer (0x"+String::nr(in_cField,-1,'0',std::ios::hex)+")", MsgType::Error);
        iRet = 1;
        goto error;
    }

    // Create the query string that modifies this field.
    sField = DataBase::TblUsrField(in_cField);
    if(sField.empty()) {
        FTSMSG("failed: unknown field definer (0x"+String::nr(in_cField,-1,'0',std::ios::hex)+")", MsgType::Error);
        iRet = 2;
        goto error;
    }

    sQuery = "UPDATE `"DSRV_TBL_USR"`"
             " SET `" + sField + "` = \'" + sValue + "\'"
             " WHERE `"+DataBase::getUniqueDB()->TblUsrField(DSRV_TBL_USR_NICK)+"`"
                " = \'" + DataBase::getUniqueDB()->escape(m_sNick) + "\'"
             " AND `"+DataBase::getUniqueDB()->TblUsrField(DSRV_TBL_USR_PASS_MD5)+"`"
                " = \'" + DataBase::getUniqueDB()->escape(m_sPassMD5) + "\' "
             " LIMIT 1";

    MYSQL_RES *pRes;
    if(!DataBase::getUniqueDB()->query(pRes, sQuery)) {
        iRet = 3;
        FTSMSG("failed: error during the sql update.", MsgType::Error);
        DataBase::getUniqueDB()->free(pRes);
        goto error;
    }

    DataBase::getUniqueDB()->free(pRes);
    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    p.append(iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onPlayerGet(uint8_t in_cField, const String & in_sNick, Packet *out_pPacket)
{
    Packet p(DSRV_MSG_PLAYER_GET);
    MYSQL_RES *pRes = NULL;
    MYSQL_ROW pRow = NULL;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is getting 0x"+String::nr(in_cField,-1,'0',std::ios::hex)+" from "+in_sNick+" ... ", 1);

    // Do the query to get the field.
    String sArgs = "\'"+DataBase::getUniqueDB()->escape(this->getNick())+"\', "+
                   +"\'"+DataBase::getUniqueDB()->escape(in_sNick)+"\', "+
                    String::nr(in_cField);
    if(!DataBase::getUniqueDB()->storedProcedure(pRes, "getUserPropertyNo", sArgs)) {
        DataBase::getUniqueDB()->free(pRes);
        iRet = 1;
        goto error;
    }

    // Get the query result.
    if(pRes == NULL || NULL == (pRow = mysql_fetch_row(pRes))) {
        FTSMSG("failed: sql fetch row: "+DataBase::getUniqueDB()->getError(), MsgType::Error);
        DataBase::getUniqueDB()->free(pRes);
        iRet = 2;
        goto error;
    }

    FTSMSGDBG("success", 1);
    goto success;

error:
    // Send an error code back.
    p.append(iRet);
    m_pConnection->send(&p);

    return true;

success:
    p.append(iRet);
    p.append(in_cField);

    // And extract the data from the result and add it to the packet.
    switch (in_cField) {
    // 1 Int:
    case DSRV_TBL_USR_WEEKON:
    case DSRV_TBL_USR_TOTALON:
    case DSRV_TBL_USR_WINS:
    case DSRV_TBL_USR_LOOSES:
    case DSRV_TBL_USR_DRAWS:
    case DSRV_TBL_USR_CLAN:
    case DSRV_TBL_USR_FLAGS:
        p.append((uint32_t)atoi(pRow[0]));
        break;

    // 1 Char:
    case DSRV_TBL_USR_SEX:
        p.append((int8_t)atoi(pRow[0]));
        break;

    case DSRV_TBL_USR_MAIL:
    case DSRV_TBL_USR_JABBER:
    case DSRV_TBL_USR_FNAME:
    case DSRV_TBL_USR_NAME:
    case DSRV_TBL_USR_IP:
    case DSRV_TBL_USR_CMT:
    case DSRV_TBL_USR_CONTACT:
    case DSRV_TBL_USR_LOCATION:
    // Dates are also simple strings.
    case DSRV_TBL_USR_SIGNUPD:
    case DSRV_TBL_USR_LASTON:
    case DSRV_TBL_USR_BDAY:
        p.append(String(pRow[0]));
        break;
    // In case of unsupported thing, just give back nothing.
    default:
        p.append((uint8_t)0);
        break;
    }
    DataBase::getUniqueDB()->free(pRes);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onPlayerSetFlag(uint32_t in_cFlag, bool in_bValue)
{
    Packet p(DSRV_MSG_PLAYER_SET_FLAG);
    FTSMSGDBG(m_sNick+" is " + (in_bValue ? "setting" : "clearing") + " his flag 0x"+String::nr(in_cFlag,-1,'0',std::ios::hex)+" ... ", 1);

    // The stored procedure does everything for us.
    String sArgs = "\'"+DataBase::getUniqueDB()->escape(this->getNick())+"\', "+
                   +"\'"+String::nr(in_cFlag)+"\', "+String::b(in_bValue);
    if(ERR_OK != DataBase::getUniqueDB()->storedFunctionInt("setUserFlag", sArgs)) {
        p.append((uint8_t)1);
        FTSMSGDBG("failed", 1);
    } else {
        p.append((uint8_t)ERR_OK);
        FTSMSGDBG("success", 1);
    }

    m_pConnection->send(&p);
    return true;
}

bool FTSSrv2::Client::onGameIns(Packet *out_pPacket)
{
    Packet p(DSRV_MSG_GAME_INS);
    int8_t iRet = ERR_OK;

    String sGameName;

    FTSMSGDBG(m_sNick+" is making a game ... ", 1);

    // Already in a game ?
    if(m_pMyGame) {
        FTSMSG("failed: already in a game", MsgType::Error);
        iRet = 12;
        goto error;
    }

    // Ok, let's get the game's name.
    sGameName = out_pPacket->get_string().trim();

    FTSMSGDBG("name: \""+sGameName+"\" ... ", 1);

    // Check if the game already exists.
    if(GameManager::getManager()->findGame(sGameName) != NULL) {
        FTSMSG("Game already exists.", MsgType::Error);
        iRet = 1;
        goto error;
    }

    // Read the data about the game and create it.
    m_pMyGame = new Game(this->getNick(), this->getCounterpartIP(), sGameName, out_pPacket);
    GameManager::getManager()->addGame(m_pMyGame);

    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    p.append(iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onGameRem()
{
    Packet p(DSRV_MSG_GAME_REM);
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" has aborted/ended his game ... ", 1);

    // Do I even have a game?
    if(!m_pMyGame) {
        FTSMSG("failed: he doesn't have a game?", MsgType::Error);
        iRet = 12;
        goto error;
    }

    GameManager::getManager()->remGame(m_pMyGame);

    // Now I no more have a game!
    m_pMyGame = NULL;

    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    p.append(iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onGameLst()
{
    Packet p(DSRV_MSG_GAME_LST);

    FTSMSGDBG(m_sNick+" gets a list of all games ... ", 1);

    p.append((int8_t)ERR_OK);
    p.append((int16_t)GameManager::getManager()->getNGames());
    GameManager::getManager()->writeListToPacket(&p);

    FTSMSGDBG("success", 1);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onGameInfo(const String & in_sName)
{
    Packet p(DSRV_MSG_GAME_INFO);
    Game *pGame = NULL;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is getting info about the game "+in_sName+" ... ", 1);

    pGame = GameManager::getManager()->findGame(in_sName);
    if(!pGame) {
        FTSMSGDBG("failed: game not found.", 1);
        iRet = 1;
        goto error;
    }

    p.append(iRet);
    pGame->addToInfoPacket(&p);

    FTSMSGDBG("success", 1);
    goto success;

error:
    p.append(iRet);

success:

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onGameStart()
{
    Packet p(DSRV_MSG_GAME_START);
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is starting his game ... ", 1);

    // Do I even have a game?
    if(!m_pMyGame) {
        FTSMSG("failed: he doesn't have a game?", MsgType::Error);
        iRet = 12;
        goto error;
    }

    GameManager::getManager()->startGame(m_pMyGame);

    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    p.append(iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatJoin(const String & in_sChan)
{
    Packet p(DSRV_MSG_CHAT_JOIN);
    Channel *pChannel = NULL;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is joining chan "+in_sChan+" ... ", 1);

    pChannel = ChannelManager::getManager()->findChannel(in_sChan);

    // If the channel doesn't exist, we gotta create it first.
    if(NULL == pChannel) {
        FTSMSGDBG("creating the channel ... ", 1);
        pChannel = ChannelManager::getManager()->createChannel( in_sChan, this );
        if(pChannel == NULL) {
            FTSMSGDBG("error!", 1);
            iRet = -100;
            goto error;
        }
    }

    iRet = ChannelManager::getManager()->joinChannel( pChannel, this );
    if(iRet != ERR_OK)
        goto error;

    FTSMSGDBG("success", 1);
    goto success;

error:

success:

    // Update the location field in the database.
    String sQuery = "UPDATE `"DSRV_TBL_USR"`"
                     " SET `"+DataBase::getUniqueDB()->TblUsrField(DSRV_TBL_USR_LOCATION)+"`"
                        "='chan:"+DataBase::getUniqueDB()->escape(in_sChan)+"'"
                     " WHERE `"+DataBase::getUniqueDB()->TblUsrField(DSRV_TBL_USR_NICK)+"`"
                        "='"+DataBase::getUniqueDB()->escape(this->getNick())+"'"
                     " LIMIT 1";
    MYSQL_RES *pRes;
    DataBase::getUniqueDB()->query(pRes, sQuery);
    DataBase::getUniqueDB()->free(pRes);

    p.append(iRet);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatList()
{
    Packet p(DSRV_MSG_CHAT_LIST);
    std::list<FTSSrv2::Client *> lpUsers;
    int8_t iRet = ERR_OK;
    int32_t nPlayers = -1;

    FTSMSGDBG(m_sNick+" is listing his channel ... ", 1);

    // If we are in no channel, just return an empty list.
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("Hmm, in no channel ?", 1);
        iRet = -3;
        goto error;
    }

    // Get the number of users and the list of users.
    nPlayers = m_pMyChannel->getNUsers();
    lpUsers = m_pMyChannel->getUsers();

    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    // Append the number of users.
    p.append(iRet);
    p.append(nPlayers);

    // And append the users.
    if( iRet == ERR_OK ) {
        for( std::list<FTSSrv2::Client *>::const_iterator i = lpUsers.begin() ;
             i != lpUsers.end() ; i++ ) {
            p.append((*i)->getNick());
        }
    }

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatSend(Packet *out_pPacket)
{
    Packet p(DSRV_MSG_CHAT_SENDMSG);
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is saying something ... ", 1);

    uint8_t cType = out_pPacket->get();
    uint8_t cFlags = out_pPacket->get();

    switch(cType) {
    case DSRV_CHAT_TYPE_NORMAL:
        // Are we currently in a channel ?
        if( m_pMyChannel == NULL ) {
            FTSMSGDBG("Hmm, in no channel ?", 1);
            iRet = -1;
            goto error;
        }

        m_pMyChannel->messageToAll( *this, out_pPacket->get_string(), cFlags );
        goto success;
    case DSRV_CHAT_TYPE_WHISPER:
        {
        FTSSrv2::Client *pTo = NULL;
        String sTo = out_pPacket->get_string().trim();
        FTSMSGDBG("Target is: "+sTo+".", 1);

        pTo = FTSSrv2::ClientsManager::getManager()->findClient(sTo);

        if(pTo == this || pTo == NULL) {
            // User not online, not existent or myself: send a error.
            iRet = 1;
            FTSMSGDBG("Target not existing or online, or it is myself.", 1);
            goto error;
        } else {
            // Send the message to the user.
            String sMessage = out_pPacket->get_string();
            Packet *pRalf = new Packet(DSRV_MSG_CHAT_GETMSG);
            pRalf->append(DSRV_CHAT_TYPE_WHISPER);
            pRalf->append((uint8_t)0);
            pRalf->append(this->getNick());
            pRalf->append(sMessage);

            FTSMSGDBG("He whisps "+sTo+": "+sMessage, 1);

            if(pTo->sendPacket(pRalf) != ERR_OK) {
                SAFE_DELETE(pRalf);
                iRet = 2;
                goto error;
            }

            SAFE_DELETE(pRalf);
            goto success;
        }
        }
        break;
    default:
        FTSMSG("failed: unknown type ("+String::nr(cType)+") !", MsgType::Error);
        break;
    }

    iRet = -12;
    goto error;

error:

success:
    p.append(iRet);
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatIuNai()
{
    Packet p(DSRV_MSG_CHAT_IUNAI);
    String sChannel;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is asking where he is ... ", 1);

    // Are we currently in a channel ?
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("He is nowhere", 1);
    } else {
        sChannel = m_pMyChannel->getName();
        FTSMSGDBG("success, he is in " + sChannel, 1);
    }

    goto success;

success:
    p.append(iRet);
    p.append(sChannel);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatMottoGet()
{
    Packet p(DSRV_MSG_CHAT_MOTTO_GET);
    String sMotto;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is getting his channel motto ... ", 1);

    // Are we currently in a channel ?
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("Hmm, in no channel ?", 1);
        iRet = 1;
        goto error;
    }

    // Get it.
    sMotto = m_pMyChannel->getMotto();

    FTSMSGDBG("success", 1);
    goto success;

error:

success:
    p.append(iRet);
    p.append(sMotto);

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatMottoSet(const String & in_sMotto)
{
    Packet p(DSRV_MSG_CHAT_MOTTO_SET);
    String sMotto;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is setting his channel motto to "+in_sMotto+"... ", 1);

    // Are we currently in a channel ?
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("Hmm, in no channel ?", 1);
        iRet = 1;
        goto error;
    }

    // Set it.
    iRet = m_pMyChannel->setMotto(in_sMotto,m_sNick);

    if( iRet == ERR_OK )
        FTSMSGDBG("success", 1);
    else {
        FTSMSGDBG("error (no rights ?)", 1);
        goto error;
    }

    goto success;

error:

success:
    p.append(iRet);
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatUserGet(const String & in_sUser)
{
    Packet p(DSRV_MSG_CHAT_USER_GET);
    uint8_t cState = 0;
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" is getting the state of the user "+in_sUser+"... ", 1);

    // Are we currently in a channel ?
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("Hmm, in no channel ?", 1);
        iRet = 1;
        goto error;
    }

    // Set it.
    if( m_pMyChannel->getAdmin().ieq(in_sUser) ) {
        cState = 2;
    } else if( m_pMyChannel->isop(in_sUser) ) {
        cState = 1;
    } else {
        cState = 0;
    }

    if( iRet == ERR_OK )
        FTSMSGDBG("success ("+String::nr(cState)+")", 1);

    goto success;

error:

success:
    p.append(iRet);

    if(iRet == ERR_OK) {
        p.append(cState);
    }

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatPublics()
{
    Packet p(DSRV_MSG_CHAT_PUBLICS);
    std::list<Channel *> lpChannels;
    uint32_t nChans = 0;

    FTSMSGDBG(m_sNick+" is listing public channels ... ", 1);

    // Get the number and the list of public channels.
    lpChannels = ChannelManager::getManager()->getPublicChannels();
    nChans = lpChannels.size();

    FTSMSGDBG("success (there are "+String::nr(nChans)+" public channels)", 1);

    // Append the number of public channels.
    p.append((int8_t)ERR_OK);
    p.append(nChans);

    // And append the public channels.
    for( std::list<Channel *>::const_iterator i = lpChannels.begin() ; i != lpChannels.end() ; i++ ) {
        p.append((*i)->getName());
    }

    // And then send the result back to the client.
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatKick(const String & in_sUser)
{
    Packet p(DSRV_MSG_CHAT_KICK);
    int8_t iRet;
    FTSMSGDBG(m_sNick+" wants to kick "+in_sUser+" ... ", 1);

    // Are we currently in a channel ?
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("Hmm, in no channel ?", 1);
        iRet = -1;
        goto error;
    }

    iRet = m_pMyChannel->kick(this, in_sUser);

    if(iRet == ERR_OK) {
        FTSMSGDBG("success", 1);
        goto success;
    } else {
        FTSMSGDBG("failed ("+String::nr(iRet)+")", 1);
        goto error;
    }

error:

success:

    p.append(iRet);
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatOp(const String & in_sUser)
{
    Packet p(DSRV_MSG_CHAT_OP);
    int8_t iRet;
    FTSMSGDBG(m_sNick+" wants to op "+in_sUser+" ... ", 1);

    // Are we currently in a channel ?
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("Hmm, in no channel ?", 1);
        iRet = -1;
        goto error;
    }

    iRet = -2;

    if(m_pMyChannel->getAdmin() == m_sNick) {
        iRet = m_pMyChannel->op(in_sUser);
    }

    if(iRet == ERR_OK) {
        FTSMSGDBG("success", 1);
        goto success;
    } else {
        FTSMSGDBG("failed ("+String::nr(iRet)+")", 1);
        goto error;
    }

error:

success:

    p.append(iRet);
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatDeop(const String & in_sUser)
{
    Packet p(DSRV_MSG_CHAT_DEOP);
    int8_t iRet;
    FTSMSGDBG(m_sNick+" wants to deop "+in_sUser+" ... ", 1);

    // Are we currently in a channel ?
    if( m_pMyChannel == NULL ) {
        FTSMSGDBG("Hmm, in no channel ?", 1);
        iRet = -1;
        goto error;
    }

    iRet = -2;

    if(m_pMyChannel->getAdmin() == m_sNick) {
        iRet = m_pMyChannel->deop(in_sUser);
    }

    if(iRet == ERR_OK) {
        FTSMSGDBG("success", 1);
        goto success;
    } else {
        FTSMSGDBG("failed ("+String::nr(iRet)+")", 1);
        goto error;
    }

error:

success:

    p.append(iRet);
    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatListMyChans()
{
    Packet p(DSRV_MSG_CHAT_LIST_MY_CHANS);
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" wants a list of his chans ... ", 1);

    std::list<String> sChans = ChannelManager::getManager()->getUserChannels(this->getNick());

    p.append(iRet);
    p.append((uint32_t)sChans.size());
    for(std::list<String>::iterator i = sChans.begin() ; i != sChans.end() ; ++i) {
        p.append(*i);
    }

    FTSMSGDBG("got that list, there are "+String::nr(sChans.size())+" of them", 1);

    m_pConnection->send(&p);

    return true;
}

bool FTSSrv2::Client::onChatDestroyChan(const String &in_sChan)
{
    Packet p(DSRV_MSG_CHAT_DESTROY_CHAN);
    int8_t iRet = ERR_OK;

    FTSMSGDBG(m_sNick+" wants to destroy the channel "+in_sChan+" ... ", 1);

    Channel *pChan = ChannelManager::getManager()->findChannel(in_sChan);
    if(!pChan) {
        iRet = -1;
        FTSMSGDBG("failed: does not exist!", 1);
    } else {
        iRet = ChannelManager::getManager()->removeChannel(pChan, this->getNick());
        FTSMSGDBG( iRet == ERR_OK ? "success" : "failed", 1);
    }

    p.append(iRet);
    m_pConnection->send(&p);

    return true;
}

FTSSrv2::ClientsManager::ClientsManager()
{
}

FTSSrv2::ClientsManager::~ClientsManager()
{
    while(!m_mClients.empty()) {
        this->deleteClient(m_mClients.begin()->first);
    }
}

FTSSrv2::Client *FTSSrv2::ClientsManager::createClient(Connection *in_pConnection)
{
    // Check if the client does not yet exist first.
    if(this->findClient(in_pConnection) != NULL)
        return NULL;

    return new FTSSrv2::Client(in_pConnection);
}

void FTSSrv2::ClientsManager::registerClient(FTSSrv2::Client *in_pClient)
{
    // We store them only in lowercase because nicknames are case insensitive.
    // So we can search for some guy more easily.
    Lock l(m_mutex);
    m_mClients[in_pClient->getNick().lower()] = in_pClient;
}

void FTSSrv2::ClientsManager::unregisterClient(FTSSrv2::Client *in_pClient)
{
    Lock l(m_mutex);

    for(std::map<String, FTSSrv2::Client *>::iterator i = m_mClients.begin() ; i != m_mClients.end() ; ++i) {
        if(i->second == in_pClient) {
            m_mClients.erase(i);
            return ;
        }
    }

    return ;
}

FTSSrv2::Client *FTSSrv2::ClientsManager::findClient(const String &in_sName)
{
    Lock l(m_mutex);
    std::map<String, FTSSrv2::Client *>::iterator i = m_mClients.find(in_sName.lower());

    if(i == m_mClients.end()) {
       return NULL;
    }

    return i->second;
}

FTSSrv2::Client *FTSSrv2::ClientsManager::findClient(const Connection *in_pConnection)
{
    Lock l(m_mutex);

    for(std::map<String, FTSSrv2::Client *>::iterator i = m_mClients.begin() ;
        i != m_mClients.end() ; ++i) {

        if(i->second->m_pConnection == in_pConnection) {
            return i->second;
        }
    }

    return NULL;
}

void FTSSrv2::ClientsManager::deleteClient(const String &in_sName)
{
    Lock l(m_mutex);
    std::map<String, FTSSrv2::Client *>::iterator i = m_mClients.find(in_sName.lower());

    if(i == m_mClients.end()) {
        return ;
    }

    FTSSrv2::Client *pClient = i->second;
    pClient->tellToQuit();
    return ;
}

static FTSSrv2::ClientsManager *g_pCM = NULL;

void FTSSrv2::ClientsManager::init()
{
    g_pCM = new FTSSrv2::ClientsManager;
}

FTSSrv2::ClientsManager *FTSSrv2::ClientsManager::getManager()
{
    return g_pCM;
}

void FTSSrv2::ClientsManager::deinit()
{
    SAFE_DELETE(g_pCM);
}
