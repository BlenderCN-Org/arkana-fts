/**
 * \file socket_connection_waiter.h
 * \author Pompei2
 * \date 03 Oct 2008
 * \brief This file describes the class that handles a lot of
 *        Network stuff at the server-side.
 **/

#ifndef FTS_SOCKETCONNECTIONWAITER_H
#define FTS_SOCKETCONNECTIONWAITER_H

#include "server.h"
#include "connection_waiter.h"

#include "net/connection.h"

#include <list>

namespace FTSSrv2 {

class SocketConnectionWaiter : public ConnectionWaiter {
public:
    SocketConnectionWaiter();
    ~SocketConnectionWaiter();

    int init(uint16_t in_usPort);
    int deinit();
    bool waitForThenDoConnection(uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT);

protected:
    SOCKET m_listenSocket;       ///< The socket that has been prepared for listening.
    FTS::Mutex m_mutex;           ///< Mutex for the connections list.
};

} // namespace FTSSrv2

#endif /* FTS_SOCKETCONNECTIONWAITER_H */
