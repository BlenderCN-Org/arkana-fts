
/**
 * \file ClientsManager.cpp
 * \author Pompei2
 * \date 03 Oct 2008 
 * \brief Singleton to manage all clients known by the server. 
 * \note Extracted from client.cpp
 **/

#include "ClientsManager.h"
#include "client.h"
#include "net/connection.h"

using namespace FTS;
using namespace FTSSrv2;


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
    if(this->findClient(in_pConnection) != nullptr)
        return nullptr;

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

    for(const auto& i : m_mClients) {
        if(i.second == in_pClient) {
            m_mClients.erase(i.first);
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
       return nullptr;
    }

    return i->second;
}

FTSSrv2::Client *FTSSrv2::ClientsManager::findClient(const Connection *in_pConnection)
{
    Lock l(m_mutex);

    for(const auto& i : m_mClients) {

        if(i.second->m_pConnection == in_pConnection) {
            return i.second;
        }
    }

    return nullptr;
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

FTSSrv2::ClientsManager *FTSSrv2::ClientsManager::getManager()
{
    return Singleton::getSingletonPtr();
}

void FTSSrv2::ClientsManager::deinit()
{
    delete Singleton::getSingletonPtr() ;
}


