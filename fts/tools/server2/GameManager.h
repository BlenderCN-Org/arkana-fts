#ifndef D_GAMEMANAGER_H
#define D_GAMEMANAGER_H

#include <list>
#include "utilities/threading.h"
#include "dLib/dString/dString.h"
#include "utilities/Singleton.h"

namespace FTS {
    class Packet;
}

namespace FTSSrv2 {

    class Game;

    class GameManager : public FTS::Singleton<GameManager>
    {
    private:
        std::list<Game *>m_lpGames; ///< This list contains all existing games.
        FTS::Mutex m_mutex; ///< Mutex for accessing me.

    public:
        GameManager();
        virtual ~GameManager();

        int addGame( Game *in_pGame );
        int remGame( Game *in_pGame );
        int startGame( Game *in_pGame );
        Game *findGame( const FTS::String &in_sName );
        Game *findGameByHost( const FTS::String &in_sHost );

        int writeListToPacket( FTS::Packet *in_pPacket );

        int16_t getNGames()
        {
            return ( int16_t ) m_lpGames.size();
        }

        // Singleton-like stuff.
        static GameManager *getManager();
        static void deinit();
    };

}
#endif