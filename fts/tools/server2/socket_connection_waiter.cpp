/**
 * \file socket_connection_waiter.cpp
 * \author Pompei2
 * \date 03 Oct 2008
 * \brief This file implements the class that handles a lot of
 *        Network stuff at the server-side.
 **/

#ifdef D_COMPILES_SERVER
#include <thread>
#include <chrono>

#include "client.h"
#include "socket_connection_waiter.h"
#include "server_log.h"

#if WINDOOF
using socklen_t = int;

inline void close( SOCKET s )
{
    closesocket( s );
    return;
}

#endif

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
    m_port = in_usPort;
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

    FTSMSGDBG("Beginning to listen on port 0x"+String::nr(in_usPort, 0, ' ', std::ios::hex), 1);
    return ERR_OK;
}

int FTSSrv2::SocketConnectionWaiter::deinit()
{
    close(m_listenSocket);
    return ERR_OK;
}

bool FTSSrv2::SocketConnectionWaiter::waitForThenDoConnection(uint64_t in_ulMaxWaitMillisec)
{
    auto startTime = std::chrono::steady_clock::now();
    // wait for connections a certain amount of time or infinitely.
    while(true) {
        // Nothing correct got in time, bye.
        auto nowTime = std::chrono::steady_clock::now();
        if( std::chrono::duration_cast< std::chrono::milliseconds >(startTime - nowTime).count() >= in_ulMaxWaitMillisec )
            return false;

        SOCKADDR_IN clientAddress;
        socklen_t iClientAddressSize = sizeof( clientAddress );
        SOCKET connectSocket;
        if( (connectSocket = accept( m_listenSocket, ( sockaddr * ) & clientAddress, &iClientAddressSize )) != -1 ) {
            // Yeah, we got someone !

            // Build up a class that will work this connection.
            TraditionalConnection *pCon = new TraditionalConnection(connectSocket, clientAddress);
            Client *pCli = ClientsManager::getManager()->createClient(pCon);
            FTSMSGDBG( "Accept connection on port 0x" + String::nr( ( int ) m_port, 0, ' ', std::ios::hex ) + " client<"+ String::nr((const uint32_t)pCli,4, '0', std::ios_base::hex) + "> con<"+ String::nr((const uint32_t)pCon,4,'0', std::ios_base::hex)+ ">", 4 );

            // And start a new thread for him.
            auto thr = std::thread( Client::starter, pCli );
            thr.detach();
            return true;

        } else if(errno == EAGAIN || errno == EWOULDBLOCK) {
            // yoyo, wait a bit to avoid megaload of cpu. 1000 microsec = 1 millisec.
            std::this_thread::sleep_for( std::chrono::milliseconds(1) );
            continue;
        } else {
#if WINDOOF
            if ( connectSocket == INVALID_SOCKET)
            {
                auto err = WSAGetLastError();
                if ( err == WSAEWOULDBLOCK )
                {
                    // yoyo, wait a bit to avoid megaload of cpu. 1000 microsec = 1 millisec.
                    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                    continue;
                }
                
                FTSMSG( "[ERROR] socket accept: " + String::nr( err ), MsgType::Error );
            }

#endif
            // Some error ... but continue waiting for a connection.
            FTSMSG("[ERROR] socket accept: "+String(strerror(errno)), MsgType::Error);
            srvFlush(stderr);
            // yoyo, wait a bit to avoid megaload of cpu. 1000 microsec = 1 millisec.
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
            continue;
        }
    }

    // Should never come here.
    return false;
}

#endif
