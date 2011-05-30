#include "game.h"
#include "client.h"
#include "server_log.h"
#include "dummy_types.h"

#include "net/connection.h"

using namespace FTS;
using namespace FTSSrv2;

FTSSrv2::Game::Game(const String &in_sCreater, const String &in_sCreaterIP, const String &in_sGameName, Packet *out_pPacket)
{
    m_mutex.lock();

    m_bStarted = false;

    // Get the IP and player name of the client.
    m_sIP = in_sCreaterIP;
    m_sHost = in_sCreater;

    // Ok, let's collect data about the game ...
    m_sName = in_sGameName;
    out_pPacket->get(m_usPort);
    m_sMapName = out_pPacket->get_string();
    m_sMapDesc = out_pPacket->get_string();
    m_nMapMinPlayers = out_pPacket->get();
    m_nMapMaxPlayers = out_pPacket->get();
    m_sMapSuggPlayers = out_pPacket->get_string();
    m_sMapAuthor = out_pPacket->get_string();
    m_dtMapLastModif = out_pPacket->get_string();
    out_pPacket->get(); // Discard the info if the game is started or not.
    m_bPressBtnToStart = out_pPacket->get() == 1;
    m_gPreview.readFromPacket(out_pPacket);
    m_gIcon.readFromPacket(out_pPacket);

    String sLog = "Making game: \"" + in_sGameName + "\"\n"
                   "  at IP:port = " + in_sCreaterIP + ":" + String::nr(m_usPort);
    FTSMSGDBG(sLog, 1);

    m_mutex.unlock();

    dynamic_cast<ServerLogger *>(Logger::getSingletonPtr())->addGame();

    // Join the game.
    this->playerJoined(in_sCreater);
    this->playerJoined("InexistentTestUser001");
    this->playerJoined("InexistentTestUser002");
    this->playerJoined("InexistentTestUser003");
}

FTSSrv2::Game::~Game( )
{
    dynamic_cast<ServerLogger *>(Logger::getSingletonPtr())->remGame();
}

void FTSSrv2::Game::playerJoined(const String &in_sPlayer)
{
    m_mutex.lock();
    m_lpPlayers.remove(in_sPlayer); // Prohibit double entries.
    m_lpPlayers.push_back(in_sPlayer);
    m_mutex.unlock();
}

void FTSSrv2::Game::playerLeft(const String &in_sPlayer)
{
    m_mutex.lock();
    m_lpPlayers.remove(in_sPlayer);
    m_mutex.unlock();
}

int FTSSrv2::Game::addToInfoPacket(Packet *out_pPacket)
{
    if(!out_pPacket)
        return -1;

    m_mutex.lock();
    out_pPacket->append(m_sIP);
    out_pPacket->append(m_usPort);
    out_pPacket->append(m_sHost);
    out_pPacket->append((uint8_t)m_lpPlayers.size());
    for(std::list<String>::iterator i = m_lpPlayers.begin() ; i != m_lpPlayers.end() ; ++i) {
        out_pPacket->append(*i);
    }
    out_pPacket->append(m_sMapName);
    out_pPacket->append(m_sMapDesc);
    out_pPacket->append(m_nMapMinPlayers);
    out_pPacket->append(m_nMapMaxPlayers);
    out_pPacket->append(m_sMapSuggPlayers);
    out_pPacket->append(m_sMapAuthor);
    out_pPacket->append(m_dtMapLastModif);
    out_pPacket->append(m_bStarted ? (uint8_t)1 : (uint8_t)0);
    out_pPacket->append(m_bPressBtnToStart ? (uint8_t)1 : (uint8_t)0);
    m_gPreview.writeToPacket(out_pPacket);
    m_gIcon.writeToPacket(out_pPacket);
    m_mutex.unlock();

    return ERR_OK;
}

int FTSSrv2::Game::addToLstPacket(Packet *out_pPacket)
{
    if(!out_pPacket)
        return -1;

    m_mutex.lock();
    out_pPacket->append(m_sName);
    out_pPacket->append(m_sMapName);
    out_pPacket->append(m_bStarted ? (uint8_t)1 : (uint8_t)0);
    m_gIcon.writeToPacket(out_pPacket);
    m_mutex.unlock();
    return ERR_OK;
}

FTSSrv2::GameManager::GameManager( void )
{
}

FTSSrv2::GameManager::~GameManager( void )
{
}

int FTSSrv2::GameManager::addGame( FTSSrv2::Game *in_pGame )
{
    m_mutex.lock();

    m_lpGames.push_back(in_pGame);

    m_mutex.unlock();
    return ERR_OK;
}

int FTSSrv2::GameManager::remGame(FTSSrv2::Game *in_pGame)
{
    // If the game has been cancelled, remove it from the list.
    m_mutex.lock();
    m_lpGames.remove(in_pGame);
    m_mutex.unlock();

    SAFE_DELETE(in_pGame);

    return ERR_OK;
}

int FTSSrv2::GameManager::startGame(FTSSrv2::Game *in_pGame)
{
    // If the game has been started, keep it in the list and set it to started state.
    in_pGame->start();
    return ERR_OK;
}

FTSSrv2::Game *FTSSrv2::GameManager::findGame( const String &in_sName )
{
    m_mutex.lock();

    for(std::list<FTSSrv2::Game *>::iterator i = m_lpGames.begin() ; i != m_lpGames.end() ; i++) {
        if((*i)->getName() == in_sName) {
            m_mutex.unlock();
            return *i;
        }
    }

    m_mutex.unlock();
    return NULL;
}

FTSSrv2::Game *FTSSrv2::GameManager::findGameByHost(const String &in_sHost)
{
    m_mutex.lock();

    for(std::list<FTSSrv2::Game *>::iterator i = m_lpGames.begin() ; i != m_lpGames.end() ; i++) {
        if((*i)->getHost() == in_sHost) {
            m_mutex.unlock();
            return *i;
        }
    }

    m_mutex.unlock();
    return NULL;
}

int FTSSrv2::GameManager::writeListToPacket(Packet *in_pPacket)
{
    if(!in_pPacket)
        return -1;

    m_mutex.lock();
    for(std::list<FTSSrv2::Game *>::iterator i = m_lpGames.begin() ; i != m_lpGames.end() ; i++) {
        (*i)->addToLstPacket(in_pPacket);
    }

    m_mutex.unlock();
    return ERR_OK;
}

static FTSSrv2::GameManager *g_pTheGM = NULL;

int FTSSrv2::GameManager::init()
{
    g_pTheGM = new FTSSrv2::GameManager();

    return ERR_OK;
}

FTSSrv2::GameManager *FTSSrv2::GameManager::getManager()
{
    return g_pTheGM;
}

int FTSSrv2::GameManager::deinit()
{
    SAFE_DELETE(g_pTheGM);
    return ERR_OK;
}

 /* EOF */
