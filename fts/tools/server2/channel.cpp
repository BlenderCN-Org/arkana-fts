#include "channel.h"
#include "client.h"
#include "db.h"
#include "server_log.h"

#include "net/connection.h"

using namespace FTS;
using namespace FTSSrv2;

FTSSrv2::Channel::Channel( int in_iID,
                    bool in_bPublic,
                    const String & in_sName,
                    const String & in_sMotto,
                    const String & in_sAdmin )
    : m_iID(in_iID)
    , m_bPublic(in_bPublic)
    , m_sName(in_sName)
    , m_sMotto(in_sMotto)
    , m_sAdmin(in_sAdmin)
{
    m_lsOperators.clear( );
    m_lpUsers.clear( );

    FTSMSGDBG("Created channel "+m_sName+" ("+String(m_bPublic ? "public" : "private")+", admin: "+m_sAdmin+")", 1);
}

FTSSrv2::Channel::~Channel( )
{
    // Kick all users that are still in the channel.
    while(!m_lpUsers.empty()) {
        this->kick(NULL, m_lpUsers.front()->getNick());
    }

    m_lsOperators.clear();
}

int FTSSrv2::Channel::join(Client *in_pUser)
{
    String sJoinName = in_pUser->getNick();

    // Add the user to the users list.
    {
        Lock l(m_mutex);
        m_lpUsers.push_back(in_pUser);

        // Tell everybody that somebody joined.
        for(std::list<Client *>::const_iterator i = m_lpUsers.begin() ; i != m_lpUsers.end() ; ++i) {
            // But don't tell myself !
            if( in_pUser == (*i) )
                continue;

            (*i)->sendChatJoins( sJoinName );
        }
    }

    in_pUser->setMyChannel(this);

    FTSMSGDBG(in_pUser->getNick() + " joined " + m_sName, 1);
    return ERR_OK;
}

int FTSSrv2::Channel::quit(Client *in_pUser)
{
    String sQuitName = in_pUser->getNick();

    // Remove the user from the list.
    {
        Lock l(m_mutex);
        m_lpUsers.remove(in_pUser);

        // Tell everybody that somebody left.
        for(std::list<Client *>::const_iterator i = m_lpUsers.begin() ; i != m_lpUsers.end() ; ++i) {
            (*i)->sendChatQuits(sQuitName);
        }
    }

    in_pUser->setMyChannel(NULL);

    FTSMSGDBG(in_pUser->getNick() + " left " + m_sName, 1);
    return ERR_OK;
}

int FTSSrv2::Channel::save()
{
    String sQuery;
    Lock l(m_mutex);

    // Create this channel from scratch.
    if( m_iID < 0 ) {
        sQuery = "\'" + DataBase::getUniqueDB()->escape(m_sName) + "\', " +
                 "\'" + DataBase::getUniqueDB()->escape(m_sMotto) + "\', " +
                 "\'" + DataBase::getUniqueDB()->escape(m_sAdmin) + "\', " +
                 String(m_bPublic ? "1" : "0");

        m_iID = DataBase::getUniqueDB()->storedFunctionInt( "channelCreate", sQuery );
        if( m_iID < 0 ) {
            FTSMSG("Error saving the channel:mysql stored function returned "+String::nr(m_iID), MsgType::Error);
            return -1;
        }

    // Or just save modifications ?
    } else {
        sQuery = "UPDATE `"DSRV_TBL_CHANS"`"
                 " SET `"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_NAME)+"`"
                        "=\'" + DataBase::getUniqueDB()->escape(m_sName) + "\',"
                      "`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_MOTTO)+"`"
                        "=\'" + DataBase::getUniqueDB()->escape(m_sMotto) + "\',"
                      "`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_ADMIN)+"`"
                        "=\'" + DataBase::getUniqueDB()->escape(m_sAdmin) + "\',"
                      "`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_PUBLIC)+"`"
                        "=" + (m_bPublic ? String("1") : String("0")) +
                 " WHERE `"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_ID)+"`"
                        "=" + String::nr(m_iID) +
                 " LIMIT 1";

        MYSQL_RES *pDummy;
        DataBase::getUniqueDB()->query(pDummy, sQuery);
        DataBase::getUniqueDB()->free(pDummy);
    }

    // Now we have to update the operators table.
    for(std::list<String>::const_iterator i = m_lsOperators.begin() ; i != m_lsOperators.end() ; ++i) {
        sQuery = "\'" + DataBase::getUniqueDB()->escape((*i)) + "\'," + String::nr(m_iID);
        DataBase::getUniqueDB()->storedFunctionInt( "channelAddOp", sQuery );
    }

    return ERR_OK;
}

int FTSSrv2::Channel::destroyDB(const String &in_sWhoWantsIt)
{
    // Not yet in database, ignore!
    if(m_iID < 0)
        return ERR_OK;

    String sQuery = "\'" + DataBase::getUniqueDB()->escape(in_sWhoWantsIt) + "\', " +
                     "\'" + DataBase::getUniqueDB()->escape(m_sName) + "\'";

    return DataBase::getUniqueDB()->storedFunctionInt("channelDestroy", sQuery);
}

int FTSSrv2::Channel::op(const String & in_sUser, bool in_bOninit)
{
    // Don't check all these things during the startup.
    if(!in_bOninit) {
        // Check if the user is in the channel.
        if(NULL == this->getUserIfPresent(in_sUser)) {
            FTSMSGDBG(in_sUser+" does not exist in this channel", 1);
            return -31;
        }

        // Do not add the user to the operators list if he already is.
        if(this->isop(in_sUser)) {
            FTSMSGDBG(in_sUser+" is already operator in this channel", 1);
            return -32;
        }
    }

    if(in_sUser == m_sAdmin) {
        FTSMSGDBG(in_sUser+" is the channel admin!", 1);
        return -33;
    }

    // add to the operators list.
    {
        Lock l(m_mutex);
        m_lsOperators.push_back(in_sUser);

        if(!in_bOninit) {
            // Tell everybody that somebody op's, including to me.
            for(std::list<Client *>::const_iterator i = m_lpUsers.begin() ; i != m_lpUsers.end() ; ++i) {
                (*i)->sendChatOped( in_sUser );
            }
        }
    }

    // Store the changes in the database.
    this->save();

    FTSMSGDBG(in_sUser+" op in "+m_sName, 1);
    return ERR_OK;
}

int FTSSrv2::Channel::deop( const String & in_sUser )
{
    // Check if the user is op.
    if(!this->isop(in_sUser)) {
        FTSMSGDBG(in_sUser+" is not operator in this channel", 1);
        return -1;
    }

    // remove from the operators list.
    {
        Lock l(m_mutex);
        m_lsOperators.remove(in_sUser);

        // Tell everybody that somebody Deop's, including to me.
        for(std::list<Client *>::const_iterator i = m_lpUsers.begin() ; i != m_lpUsers.end() ; ++i) {
            (*i)->sendChatDeOped( in_sUser );
        }
    }

    // Store the changes in the database.
    this->save();

    FTSMSGDBG(in_sUser+" deop in "+m_sName, 1);
    return ERR_OK;
}

bool FTSSrv2::Channel::isop( const String & in_sUser )
{
    Lock l(m_mutex);
    for(std::list<String>::const_iterator i = m_lsOperators.begin() ; i != m_lsOperators.end() ; ++i) {
        if(in_sUser.ieq(*i)) {
            return true;
        }
    }

    return false;
}

int FTSSrv2::Channel::messageToAll( const Client &in_From, const String & in_sMessage, uint8_t in_cFlags )
{
    Packet *p = new Packet(DSRV_MSG_CHAT_GETMSG);
    p->append(DSRV_CHAT_TYPE_NORMAL);
    p->append(in_cFlags);
    p->append(in_From.getNick());
    p->append(in_sMessage);

    FTSMSGDBG(in_From.getNick()+" says "+in_sMessage, 1);

    this->sendPacketToAll(p);
    SAFE_DELETE(p);
    return ERR_OK;
}

Packet *FTSSrv2::Channel::makeSystemMessagePacket( const String &in_sMessageID )
{
    Packet *p = new Packet(DSRV_MSG_CHAT_GETMSG);
    p->append(DSRV_CHAT_TYPE_SYSTEM);
    p->append((uint8_t)0);
    p->append(in_sMessageID);

    return p;
}

int FTSSrv2::Channel::sendPacketToAll( Packet *in_pPacket )
{
    Lock l(m_mutex);
    for(std::list<Client *>::const_iterator i = m_lpUsers.begin() ; i != m_lpUsers.end() ; ++i) {
        (*i)->sendPacket(in_pPacket);
    }

    return ERR_OK;
}

int FTSSrv2::Channel::setMotto( const String & in_sMotto, const String & in_sUser )
{
    // Only op or admin can do this !
    if( !this->isop(in_sUser) && in_sUser != m_sAdmin )
        return -20;

    {
        Lock l(m_mutex);
        m_sMotto = in_sMotto;

        // Tell everybody that somebody changed the motto, including to me.
        for(std::list<Client *>::const_iterator i = m_lpUsers.begin() ; i != m_lpUsers.end() ; ++i) {
            (*i)->sendChatMottoChanged( in_sUser, in_sMotto );
        }
    }

    return this->save( );
}

int FTSSrv2::Channel::kick( const Client *in_pFrom, const String & in_sUser )
{
    String sKicker;
    Client *pKicked = NULL;

    // If the system wants to kick him out.
    if(in_pFrom == NULL) {
        sKicker = "Arkana-FTS Server";
    } else {
        sKicker = in_pFrom->getNick();

        // Only op or admin can do this !
        if(!this->isop(sKicker) && sKicker != m_sAdmin)
            return -20;

        // The admin can't be kicked.
        if(in_sUser == m_sAdmin) {
            return -21;
        }

        // Only admins can kick operators.
        if(sKicker != m_sAdmin && this->isop(in_sUser)) {
            return -22;
        }
    }

    // Find the user that we want to kick.
    pKicked = this->getUserIfPresent(in_sUser);

    // User not in the channel ?
    if(pKicked == NULL)
        return -21;

    // Kick him out to the default channel.
    FTSSrv2::ChannelManager::getManager()->joinChannel(FTSSrv2::ChannelManager::getManager()->getDefaultChannel(), pKicked);

    // Tell him about it.
    Packet *p = new Packet(DSRV_MSG_CHAT_KICKED);
    p->append(sKicker);
    p->append(this->getName());
    pKicked->sendPacket(p);
    SAFE_DELETE(p);

    // And tell all others in this channel about it.
    p = this->makeSystemMessagePacket("Chat_Kicked");
    p->append(sKicker);
    p->append(pKicked->getNick());
    this->sendPacketToAll(p);
    SAFE_DELETE(p);

    return ERR_OK;
}

Client *FTSSrv2::Channel::getUserIfPresent(const String &in_sUsername)
{
    Lock l(m_mutex);
    Client *pCli = NULL;
    for(std::list<Client *>::const_iterator i = m_lpUsers.begin() ; i != m_lpUsers.end() ; ++i) {
        pCli = *i;
        if(pCli->getNick().ieq(in_sUsername)) {
            return pCli;
        }
    }

    return NULL;
}

FTSSrv2::ChannelManager::ChannelManager()
{
    m_lpChannels.clear();
}

FTSSrv2::ChannelManager::~ChannelManager()
{
    this->saveChannels( );

    for( std::list<FTSSrv2::Channel *>::const_iterator i = m_lpChannels.begin() ;
         i != m_lpChannels.end() ; i++ ) {
        delete (*i);
    }

    m_lpChannels.clear();
}

static FTSSrv2::ChannelManager *g_pTheCM = NULL;

int FTSSrv2::ChannelManager::init()
{
    g_pTheCM = new FTSSrv2::ChannelManager();

    // First, load all the channels from the database.
    if(ERR_OK != g_pTheCM->loadChannels()) {
        SAFE_DELETE(g_pTheCM);
        return -1;
    }

    // Then, check if the main channel is existing.
    if(g_pTheCM->getDefaultChannel() == NULL) {
        // If not, create it with Pompei2 as admin.
        // We do create it manually here because it's a special case.
        g_pTheCM->m_lpChannels.push_back(new FTSSrv2::Channel(-1, true,
                                                      DSRV_DEFAULT_CHANNEL_NAME,
                                                      DSRV_DEFAULT_CHANNEL_MOTTO,
                                                      DSRV_DEFAULT_CHANNEL_ADMIN));

    } else if(g_pTheCM->getDefaultChannel()->getAdmin() != DSRV_DEFAULT_CHANNEL_ADMIN) {
        // Or if somehow another admin is entered in it, set it to the default admin!
        g_pTheCM->getDefaultChannel()->setAdmin(DSRV_DEFAULT_CHANNEL_ADMIN);
    }

    // Same for the dev's channel.
    if(g_pTheCM->findChannel(DSRV_DEVS_CHANNEL_NAME) == NULL) {
        // If not, create it with Pompei2 as admin.
        // We do create it manually here because it's a special case.
        g_pTheCM->m_lpChannels.push_back(new FTSSrv2::Channel(-1, true,
                                                      DSRV_DEVS_CHANNEL_NAME,
                                                      DSRV_DEVS_CHANNEL_MOTTO,
                                                      DSRV_DEVS_CHANNEL_ADMIN));

    } else if(g_pTheCM->findChannel(DSRV_DEVS_CHANNEL_NAME)->getAdmin() != DSRV_DEVS_CHANNEL_ADMIN) {
        // Or if somehow another admin is entered in it, set it to the default admin!
        g_pTheCM->findChannel(DSRV_DEVS_CHANNEL_NAME)->setAdmin(DSRV_DEVS_CHANNEL_ADMIN);
    }

    return ERR_OK;
}

FTSSrv2::ChannelManager *FTSSrv2::ChannelManager::getManager()
{
    return g_pTheCM;
}

int FTSSrv2::ChannelManager::deinit()
{
    SAFE_DELETE(g_pTheCM);
    return ERR_OK;
}

int FTSSrv2::ChannelManager::loadChannels(void)
{
    MYSQL_RES *pRes = NULL;
    MYSQL_ROW pRow = NULL;

    // Do the query to get the field.
    String sQuery = "SELECT  `"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_ID)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_PUBLIC)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_NAME)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_MOTTO)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_ADMIN)+"`"
                     " FROM `"DSRV_TBL_CHANS"`";

    if(!DataBase::getUniqueDB()->query(pRes, sQuery)) {
        DataBase::getUniqueDB()->free(pRes);
        return -1;
    }

    // Invalid record ? forget about it!
    if(pRes == NULL || mysql_num_fields(pRes) < 5) {
        DataBase::getUniqueDB()->free(pRes);
        return -2;
    }

    // Create every single channel.
    while(NULL != (pRow = mysql_fetch_row(pRes))) {
        int iChannelID = atoi(pRow[0]);
        bool bPublic = (pRow[1] == NULL ? false : (pRow[1][0] == '0' ? false : true));
        String sChanName = pRow[2];
        String sChanMotto = pRow[3];
        String sChanAdmin = pRow[4];

        FTSSrv2::Channel *pChan = new FTSSrv2::Channel(iChannelID, bPublic, sChanName, sChanMotto, sChanAdmin);
        m_lpChannels.push_back(pChan);
    }

    DataBase::getUniqueDB()->free(pRes);

    // Now read all channel operators.
    sQuery = "SELECT  `"+DataBase::getUniqueDB()->TblChanOpsField(DSRV_VIEW_CHANOPS_NICK)+"`"
                    ",`"+DataBase::getUniqueDB()->TblChanOpsField(DSRV_VIEW_CHANOPS_CHAN)+"`"
             " FROM `"DSRV_VIEW_CHANOPS"`";
    if(!DataBase::getUniqueDB()->query(pRes, sQuery)) {
        DataBase::getUniqueDB()->free(pRes);
        return -3;
    }

    // Invalid record ? forget about it!
    if(pRes == NULL || mysql_num_fields(pRes) < 2) {
        DataBase::getUniqueDB()->free(pRes);
        return -4;
    }

    // Setup every operator<->channel connection.
    // But first just put all assocs. in a list because we need to free the DB.
    std::list<std::pair<FTSSrv2::Channel *, String> > operators;
    while(NULL != (pRow = mysql_fetch_row(pRes))) {
        FTSSrv2::Channel *pChan = this->findChannel(pRow[1]);

        if(!pChan)
            continue;

        operators.push_back(std::make_pair(pChan, pRow[0]));
    }

    DataBase::getUniqueDB()->free(pRes);

    // Now we execute that action (only now as the DB result needs to be freed).
    for(std::list<std::pair<FTSSrv2::Channel *, String> >::iterator i = operators.begin() ; i != operators.end() ; ++i) {
        i->first->op(i->second, true);
    }

    return ERR_OK;
}

int FTSSrv2::ChannelManager::saveChannels( void )
{
    Lock l(m_mutex);
    for( std::list<FTSSrv2::Channel *>::const_iterator i = m_lpChannels.begin() ;
         i != m_lpChannels.end() ; i++ ) {
        (*i)->save( );
    }

    return ERR_OK;
}

FTSSrv2::Channel *FTSSrv2::ChannelManager::createChannel(const String & in_sName, const Client *in_pCreater, bool in_bPublic)
{
    // Every user can only create a limited amount of channels!
    if(this->countUserChannels(in_pCreater->getNick()) >= DSRV_MAX_CHANS_PER_USER) {
        return NULL;
    }

    Lock l(m_mutex);
    FTSSrv2::Channel *pChannel = new FTSSrv2::Channel(-1, in_bPublic, in_sName,
                                      DSRV_DEFAULT_MOTTO,
                                      in_pCreater->getNick());

    m_lpChannels.push_back(pChannel);
    pChannel->save(); // Update the database right now!

    return pChannel;
}

int FTSSrv2::ChannelManager::removeChannel(FTSSrv2::Channel *out_pChannel, const String &in_sWhoWantsIt)
{
    std::unique_ptr<Lock> l(new Lock(m_mutex));
    for(std::list<FTSSrv2::Channel *>::iterator i = m_lpChannels.begin() ; i != m_lpChannels.end() ; ++i) {
        if(*i == out_pChannel) {
            // Found! remove it from DB and manager, if we have the rights!
            if(ERR_OK == out_pChannel->destroyDB(in_sWhoWantsIt)) {
                m_lpChannels.erase(i);
                // The order here is important, as deleting the channel might kick players, that will be locking the mutex.
                l.reset();
                SAFE_DELETE(out_pChannel);
                return ERR_OK;
            } else {
                // Found, but no right to remove it.
                return -2;
            }
        }
    }

    // Not found.
    return -1;
}

std::list<FTSSrv2::Channel *> FTSSrv2::ChannelManager::getPublicChannels()
{
    std::list<FTSSrv2::Channel *>lpPubChannels;

    Lock l(m_mutex);
    for( std::list<FTSSrv2::Channel *>::const_iterator i = m_lpChannels.begin() ;
         i != m_lpChannels.end() ; i++ ) {
        if( (*i)->isPublic( ) ) {
            lpPubChannels.push_back(*i);
        }
    }

    return lpPubChannels;
}

int FTSSrv2::ChannelManager::joinChannel( FTSSrv2::Channel *out_pChannel, Client *out_pClient )
{
    if(out_pChannel == NULL)
        return -1;

    FTSSrv2::Channel *pOldChan = out_pClient->getMyChannel();

    Lock l(m_mutex);
    // Leave the old channel.
    if(pOldChan) {
        pOldChan->quit(out_pClient);
    }

    // Join the new channel.
    out_pChannel->join(out_pClient);

    return ERR_OK;
}

FTSSrv2::Channel *FTSSrv2::ChannelManager::findChannel(const String & in_sName)
{
    Lock l(m_mutex);
    for(std::list<FTSSrv2::Channel *>::const_iterator i = m_lpChannels.begin() ; i != m_lpChannels.end() ; ++i) {
        // "Pompei2's ChanNel" is the same as "pOmpei2's chAnnEl"
        if((*i)->getName().ieq(in_sName)) {
            return *i;
        }
    }

    return NULL;
}

uint32_t FTSSrv2::ChannelManager::countUserChannels(const String &in_sUserName)
{
    uint32_t nChans = 0;

    Lock l(m_mutex);
    for(std::list<FTSSrv2::Channel *>::const_iterator i = m_lpChannels.begin() ; i != m_lpChannels.end() ; ++i) {
        if((*i)->getAdmin().ieq(in_sUserName)) {
            nChans++;
        }
    }

    return nChans;
}

std::list<String> FTSSrv2::ChannelManager::getUserChannels(const String &in_sUserName)
{
    std::list<String>sChans;

    Lock l(m_mutex);
    for(std::list<FTSSrv2::Channel *>::const_iterator i = m_lpChannels.begin() ; i != m_lpChannels.end() ; ++i) {
        if((*i)->getAdmin().ieq(in_sUserName)) {
            sChans.push_back((*i)->getName());
        }
    }

    return sChans;
}

FTSSrv2::Channel *FTSSrv2::ChannelManager::getDefaultChannel(void)
{
    return this->findChannel(DSRV_DEFAULT_CHANNEL_NAME);
}
