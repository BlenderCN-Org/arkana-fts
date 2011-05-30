/**
 * \file socket_connection_waiter.cpp
 * \author Pompei2
 * \date 03 Oct 2008
 * \brief This file implements the class that handles a lot of
 *        Network stuff at the server-side.
 **/

#ifdef D_COMPILES_SERVER

#include "client.h"
#include "socket_connection_waiter.h"
#include "net/packet.h"
#include "server_log.h"
#include "db.h"

using namespace FTS;
using namespace FTSSrv2;

FTSSrv2::SocketConnectionWaiter::SocketConnectionWaiter()
{
}

FTSSrv2::SocketConnectionWaiter::~SocketConnectionWaiter()
{
    this->deinit();
}

int FTSSrv2::SocketConnectionWaiter::init(uint16_t in_usPort)
{
    SOCKADDR_IN serverAddress;

    // Choose our options.
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons((int)in_usPort);

    // Setup the listening socket.
    if((m_listenSocket = socket(AF_INET, SOCK_STREAM, 0 /*IPPROTO_TCP */ )) < 0) {
        FTSMSG("[ERROR] socket: "+String(strerror(errno)), MsgType::Error);
        return -1;
    }

    if(bind(m_listenSocket, (sockaddr *) & serverAddress, sizeof(serverAddress)) < 0) {
        FTSMSG("[ERROR] socket bind: "+String(strerror(errno)), MsgType::Error);
        close(m_listenSocket);
        return -2;
    }

    // Set it to be nonblocking, so we can easily time the wait for a connection.
    TraditionalConnection::setSocketBlocking(m_listenSocket, false);

    if(listen(m_listenSocket, 100) < 0) {
        FTSMSG("[ERROR] socket listen: "+String(strerror(errno)), MsgType::Error);
        close(m_listenSocket);
        return -3;
    }

    FTSMSGDBG("Beginning to listen on port 0x"+String::nr(in_usPort,-1, 0, std::ios::hex), 1);
    return ERR_OK;
}

int FTSSrv2::SocketConnectionWaiter::deinit()
{
    close(m_listenSocket);
    return ERR_OK;
}

bool FTSSrv2::SocketConnectionWaiter::waitForThenDoConnection(uint64_t in_ulMaxWaitMillisec)
{
    SOCKADDR_IN clientAddress;
    socklen_t iClientAddressSize = sizeof(clientAddress);
    SOCKET connectSocket;

    int64_t lMaxWaitMillisecLeft = in_ulMaxWaitMillisec;
    uint32_t uiLastTick = dGetTicks();

    // wait for connections a certain amount of time or infinitely.
    while(true) {
        uint32_t uiNow = dGetTicks();
        lMaxWaitMillisecLeft -= (uiNow - uiLastTick);
        uiLastTick = uiNow;

        // Nothing correct got in time, bye.
        if(lMaxWaitMillisecLeft <= 0)
            return false;

        // Yeah, we got someone !
        if((connectSocket = accept(m_listenSocket, (sockaddr *) & clientAddress, &iClientAddressSize)) != -1) {
            // Build up a class that will work this connection.
            TraditionalConnection *pCon = new TraditionalConnection(connectSocket, clientAddress);
            Client *pCli = ClientsManager::getManager()->createClient(pCon);

            // And start a new thread for him.
            pthread_t thr;
            pthread_create(&thr, 0, Client::starter, pCli);

            return true;

        } else if(errno == EAGAIN || errno == EWOULDBLOCK) {
            // yoyo, wait a bit to avoid megaload of cpu. 1000 microsec = 1 millisec.
            usleep(1000);
            continue;
        } else {
            // Some error ... but continue waiting for a connection.
            FTSMSG("[ERROR] socket accept: "+String(strerror(errno)), MsgType::Error);
            srvFlush(stderr);
            // yoyo, wait a bit to avoid megaload of cpu. 1000 microsec = 1 millisec.
            usleep(1000);
            continue;
        }
    }

    // Should never come here.
    return false;
}

#endif
