#ifndef D_CLIENTSMANAGER_H
#define D_CLIENTSMANAGER_H

#  include <list>
#  include <map>

#  include "utilities/threading.h"
#  include "utilities/Singleton.h"

namespace FTS {
    class Packet;
    class Connection;
}

namespace FTSSrv2 {
    class Client;

    class ClientsManager : public FTS::Singleton<ClientsManager>
    {
    private:
        std::map<FTS::String, Client *>m_mClients;
        FTS::Mutex m_mutex;
    protected:
    public:
        ClientsManager();
        virtual ~ClientsManager();

        static ClientsManager *getManager();
        static void deinit();

        Client *createClient( FTS::Connection *in_pConnection );
        void registerClient( Client *in_pClient );
        void unregisterClient( Client *in_pClient );
        Client *findClient( const FTS::String &in_sName );
        Client *findClient( const FTS::Connection *in_pConnection );
        void deleteClient( const FTS::String &in_sName );
    };

}

#endif
