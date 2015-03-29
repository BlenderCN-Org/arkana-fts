#ifndef D_CHANNELMANAGER_H
#define D_CHANNELMANAGER_H

#include <list>

#  include "dLib/dString/dString.h"
#  include "utilities/threading.h"
#  include "utilities/Singleton.h"

namespace FTSSrv2 {

    class Channel;
    class Client;

    class ChannelManager : public FTS::Singleton<ChannelManager>
    {
    private:
        std::list<Channel *>m_lpChannels; ///< This list contains all existing channels.
        FTS::Mutex m_mutex; ///< Mutex for accessing me.

    public:
        ChannelManager();
        virtual ~ChannelManager();

        // Singleton-like stuff.
        int init();
        static ChannelManager *getManager();
        static void deinit();

        int loadChannels();
        int saveChannels();

        Channel *createChannel( const FTS::String &in_sName, const Client *in_pCreater, bool in_bPublic = false );
        int removeChannel( Channel *out_pChannel, const FTS::String &in_sWhoWantsIt );

        std::list<Channel *> getPublicChannels();

        int joinChannel( Channel *out_pChannel, Client *out_pClient );
        Channel *findChannel( const FTS::String &in_sName );
        Channel *getDefaultChannel();
        uint32_t countUserChannels( const FTS::String &in_sUserName );
        std::list<FTS::String> getUserChannels( const FTS::String &in_sUserName );

        int getNChannels()
        {
            return ( int ) m_lpChannels.size();
        }
    };

}
#endif