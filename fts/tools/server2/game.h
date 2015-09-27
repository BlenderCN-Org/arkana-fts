#ifndef D_GAME_H
#define D_GAME_H

#include "server.h"
#include "dummy_types.h"
#include "utilities/threading.h"
#include "dLib/dString/dString.h"

#include <list>
#include "GameManager.h"

namespace FTS {
    class Packet;
}

namespace FTSSrv2 {
    class Client;

class Game {
private:
    FTS::String m_sName;    ///< The name of the game you want to create.
    FTS::String m_sIP;      ///< The IPv4 address of the host (xxx.xxx.xxx.xxx)
    uint16_t m_usPort;      ///< The portnumber on what you can connect to the host.
    FTS::String m_sHost;    ///< The player that crated this game.

    FTS::String m_sMapName;         ///< The name of the map.
    FTS::String m_sMapDesc;         ///< A description of the map.
    uint8_t m_nMapMinPlayers;       ///< The minimum number of players needed to play this map.
    uint8_t m_nMapMaxPlayers;       ///< The maximum number of players allowed to play this map.
    FTS::String m_sMapSuggPlayers;  ///< The suggested number of players (for example: "2v2 and 3v3").
    FTS::String m_sMapAuthor;       ///< This is a string containing the name of the author of the map.
    FTS::String m_dtMapLastModif;   ///< When this map has been last modified.
    FTSSrv2::Graphic m_gPreview;            ///< The preview image that is associated with this map.
    FTSSrv2::Graphic m_gIcon;               ///< The icon image that is associated with this map.

    bool m_bStarted;            ///< Whether this game is already started or not.
    bool m_bPressBtnToStart;    ///< Whether to press a button to start the game or not.

    std::list<FTS::String>m_lpPlayers;   ///< The players that are currently in this game.

    FTS::Mutex m_mutex;  ///< A mutex to protect myself.

public:
    Game(const FTS::String &in_sCreater, const FTS::String &in_sCreaterIP, const FTS::String &in_sGameName, FTS::Packet *out_pPacket);
    virtual ~Game();

    void playerJoined(const FTS::String &in_sPlayer);
    void playerLeft(const FTS::String &in_sPlayer);

    inline FTS::String getName() const { return m_sName; }
    inline void start() { m_bStarted = true; }
    inline FTS::String getHost() const { return m_sHost; }

    int addToInfoPacket(FTS::Packet *out_pPacket);
    int addToLstPacket(FTS::Packet *out_pPacket);
};

} // namespace FTSSrv2

#endif // D_CHANNEL_H

/* EOF */
