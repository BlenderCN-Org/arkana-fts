#ifndef D_CHANNEL_H
#define D_CHANNEL_H

#include "utilities/threading.h"
#include "dLib/dString/dString.h"

#include <list>

namespace FTS {
    class Packet;
}

namespace FTSSrv2 {
    class Client;

class Channel {
private:
    int m_iID;        ///< The MySQL ID of the row, needed when saving.
    bool m_bPublic;   ///< Whether this is publicly visible or not.
    FTS::String m_sName;  ///< The name of this channel.
    FTS::String m_sMotto; ///< The motto of this channel.
    FTS::String m_sAdmin; ///< The name of the admin of this channel.

    std::list<FTS::String>m_lsOperators; ///< The names of the operators of this channel.
    std::list<Client *>m_lpUsers;   ///< The users that are currently in this channel.

    FTS::Mutex m_mutex; ///< Mutex for accessing me.

public:
    Channel(int in_iID,
            bool in_bPublic,
            const FTS::String &in_sName,
            const FTS::String &in_sMotto,
            const FTS::String &in_sAdmin);
    virtual ~Channel();

    int join(Client *in_pUser);
    int quit(Client *in_pUser);
    int save();
    int destroyDB(const FTS::String &in_sWhoWantsIt);

    int op(const FTS::String & in_sUser, bool in_bOninit = false);
    int deop(const FTS::String & in_sUser);
    bool isop(const FTS::String & in_sUser);

    int messageToAll(const Client &in_From, const FTS::String &in_sMessage, uint8_t in_cFlags);

    FTS::Packet *makeSystemMessagePacket(const FTS::String &in_sMessageID);
    int sendPacketToAll(FTS::Packet *in_pPacket);

    int kick(const Client *in_pFrom, const FTS::String &in_sUser);

    inline FTS::String getMotto() const { return m_sMotto; }
    int setMotto(const FTS::String &in_sMotto, const FTS::String &in_sClient);

    inline FTS::String getName() const { return m_sName; }
    inline FTS::String getAdmin() const { return m_sAdmin; }
    inline void setAdmin(const FTS::String &in_sNewAdmin) { m_sAdmin = in_sNewAdmin; }
    inline std::list<Client *> getUsers() const { return m_lpUsers; }
    inline int getNUsers() const { return this->getUsers().size(); }
    inline bool isPublic() const { return m_bPublic; }

    Client *getUserIfPresent(const FTS::String &in_sUsername);
};

class ChannelManager {
private:
    std::list<Channel *>m_lpChannels; ///< This list contains all existing channels.
    FTS::Mutex m_mutex; ///< Mutex for accessing me.

public:
    ChannelManager();
    virtual ~ChannelManager();

    // Singleton-like stuff.
    static int init();
    static ChannelManager *getManager();
    static int deinit();

    int loadChannels();
    int saveChannels();

    Channel *createChannel(const FTS::String &in_sName, const Client *in_pCreater, bool in_bPublic = false);
    int removeChannel(Channel *out_pChannel, const FTS::String &in_sWhoWantsIt);

    std::list<Channel *> getPublicChannels();

    int joinChannel(Channel *out_pChannel, Client *out_pClient);
    Channel *findChannel(const FTS::String &in_sName);
    Channel *getDefaultChannel();
    uint32_t countUserChannels(const FTS::String &in_sUserName);
    std::list<FTS::String> getUserChannels(const FTS::String &in_sUserName);

    int getNChannels() {
        return m_lpChannels.size();
    }
};

} // namespace FTSSrv2

#endif // D_CHANNEL_H

/* EOF */
