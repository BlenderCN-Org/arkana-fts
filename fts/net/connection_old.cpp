/**
 * \file connection.cpp
 * \author Pompei2
 * \date 11 May 2007
 * \brief This file implements the class that represents a network
 *        connection that can send packets.
 **/

#include "net/connection.h"
#include "net/packet.h"

#ifndef D_COMPILES_SERVER
#  include "logging/logger.h"
#  include "utilities/utilities.h"
#else
#  include "server.h"
#endif

#if !WINDOOF
#  include <sys/select.h>
#  include <poll.h>
#endif

#if defined(DEBUG) && FTS_DEBUG_MEM && !defined(D_COMPILES_SERVER)
#  define new new(__FILE__,__LINE__)
#endif

#if 0
/** Creates the connection object and connect.
 *
 * \author Klaus Beyer (kabey)
 */
CFTSConnection::CFTSConnection(String in_sName, int in_iPort,
                               time_t in_nTimeout)
{
    m_ulLastcall = 0;
    connectByName(in_sName, in_iPort, in_nTimeout);
}

/*! ctor. Uses a existing connection described by the 2 parameter.
 *
 * \author Klaus.Beyer (kabey)
 *
 * \param[in] in_sock the socket to use for the connection
 * \param[in] in_sa   address of counterpart to which the connection goes.
 *
 */
CFTSConnection::CFTSConnection(SOCKET in_sock, SOCKADDR_IN in_sa)
{
    m_ulLastcall = 0;
    m_sock = in_sock;
    m_saCounterpart = in_sa;
    m_bConnected = true;
}

/// Default destructor
/** Closes the connection.
 *
 * \author Pompei2
 */
CFTSConnection::~CFTSConnection()
{
    this->disconnect();
}

/// Check if i'm connected.
/** This checks if this connection is currently up or down.
 *
 * \return true if this connection is up, false if it's down.
 *
 * \note This function doesn't currently check if the connection
 *       is up right now, it just looks at the state the connection
 *       was during the last send/recv operation.
 *
 * \author Pompei2
 */
bool CFTSConnection::isConnected(void) const
{
    return m_bConnected;
}

/// Return the address of the counterpart.

/** This returns a pointer to the place in memory where the
 *  counterpart's address is stored as an SOCKADDR_IN structure.
 *
 * \return a pointer to the counterpart's address.
 *
 * \author Pompei2
 */
const SOCKADDR_IN *CFTSConnection::getCounterpart(void) const
{
    return &m_saCounterpart;
}

/// Connects to another pc by it's name.
/** This resolves the name to an IP address and then connects to it.
 *
 * \param in_sName    The name of the computer to conenct to.
 * \param in_iPort    The port you want to use for the connection.
 * \param in_nTimeout The number of seconds to try to connect before displaying an error.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int CFTSConnection::connectByName(String in_sName, int in_iPort,
                                  time_t in_nTimeout)
{
    hostent *serverInfo = NULL;
    int iRet = -1;

    m_mutex.lock();

    // Setup the connection socket.
#if WINDOOF
    if((m_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
#else
    if((m_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
        FTS18N("Net_mksock", FTS_ERROR, strerror(errno), errno);
        m_mutex.unlock();
        return -1;
    }

    // Get some information we need to connect to the server.
    if(NULL == (serverInfo = gethostbyname(in_sName.c_str()))) {
        switch (h_errno) {
        case -1:
            FTS18N("Net_hostname", FTS_ERROR, in_sName.c_str(),
                   strerror(errno), errno);
            break;
        default:
            FTS18N("Net_hostname", FTS_ERROR, in_sName.c_str(),
                   "Unknown hostname", h_errno);
            break;
        }
        close(m_sock);
        m_mutex.unlock();
        return -2;
    }

    // Prepare to connect.
    m_saCounterpart.sin_family = serverInfo->h_addrtype;
    memcpy((char *)&m_saCounterpart.sin_addr.s_addr,
           serverInfo->h_addr_list[0], serverInfo->h_length);
    m_saCounterpart.sin_port = htons(in_iPort);

    // Set the socket non-blocking so we can cancel it if it can't connect.
    m_mutex.unlock();
    this->setSocketBlocking(m_sock, false);
    m_mutex.lock();

    time_t tBegin = time(NULL);

    // Try to connect to the server.
    do {
        iRet = connect(m_sock, (struct sockaddr *)&m_saCounterpart, sizeof(m_saCounterpart));

        // It was successful.
        if(iRet == 0) {
            m_bConnected = true;
            m_mutex.unlock();
            return ERR_OK;

            // Already connected.
#if WINDOOF
        } else if(WSAGetLastError() == WSAEISCONN) {
#else
        } else if(errno == EISCONN) {
#endif
            m_bConnected = true;
            m_mutex.unlock();
            return ERR_OK;

            // There was another error then the retry/in proggress/busy error.
#if WINDOOF
        } else if(WSAGetLastError() != WSAEWOULDBLOCK &&
                  WSAGetLastError() != WSAEALREADY &&
                  WSAGetLastError() != WSAEINVAL) {
            FTS18N("Net_connect", FTS_ERROR, in_sName.c_str(), "Address not found (maybe you're not connected to the internet)",
                   WSAGetLastError());
#else
        } else if(errno != EINPROGRESS &&
                  errno != EALREADY &&
                  errno != EAGAIN) {
            FTS18N("Net_connect", FTS_ERROR, in_sName.c_str(), strerror(errno), errno);
#endif
            close(m_sock);
            m_mutex.unlock();
            return -3;
        }

        // Retry as long as we have time to.
    } while((time(NULL) - tBegin) < in_nTimeout);

    //     close( m_sock );
#if WINDOOF
    FTS18N( "Net_connect", FTS_ERROR, in_sName.c_str( ), "Timed out (maybe the counterpart is down)", WSAGetLastError( ) );
#else
    FTS18N( "Net_connect", FTS_ERROR, in_sName.c_str( ), strerror( errno ), errno );
#endif
    m_mutex.unlock();
    return -4;
}

/// Closes the connection.
/** This safely closes the connection with the counterpart by closing the socket.
 *
 * \note You don't need to call this, as it gets called by the destructor.
 *
 * \author Pompei2
 */
void CFTSConnection::disconnect(void)
{
    m_mutex.lock();
    if(m_bConnected) {
        close(m_sock);
        m_bConnected = false;
    }

    // We need to check empty the queue ourselves.
    if(!m_lpPacketQueue.empty()) {
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            CFTSPacket *p = *i;

            SAFE_DELETE(p);
        }
    }

    m_mutex.unlock();
}

/// Sends a packet.
/** This sends a packet to the pc this connection is with.
 *
 * \param in_pPacket A pointer to the packet to send.
 *
 * \return If successful: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \todo Add a select here too. ?????
 *
 * \author Pompei2
 */
int CFTSConnection::send(CFTSPacket * in_pPacket)
{
    if(!m_bConnected)
        return -1;

    errno = 0;

#ifdef DEBUG
    FTSMSG("Sending packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
           (int)in_pPacket->getType(),
           (int)in_pPacket->getPayloadLen());
#endif

    int iSent = 0;
    int iToSend = in_pPacket->getTotalLen();
    char *buf = in_pPacket->m_pData;

    m_mutex.lock();
    do {
        iSent = ::send(m_sock, buf, iToSend, 0);
#if WINDOOF
        if(iSent == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                     WSAGetLastError() == WSATRY_AGAIN ||
                                     WSAGetLastError() == WSAEWOULDBLOCK))
#else
        if(iSent < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
#endif
            continue;
        if(iSent < 0) {
            FTS18N("Net_send", FTS_ERROR, strerror(errno), errno);
            m_mutex.unlock();
//             this->disconnect();
            return -1;
        }
        iToSend -= iSent;
        buf += iSent;
    } while(iToSend > 0);

    m_ulLastcall = SDL_GetTicks();
    m_mutex.unlock();
    return ERR_OK;
}

/// Receives a packet.
/** This first (by default) looks in the message queue, if there is any message,
 *  it returns that message and removes it from the queue. If the queue is empty
 *  or shall not be used, it either just checks if there is an incoming message
 *  or waits a certain amount for one (depending on in_bBlock).
 *
 * \param in_bBlock wether to block and wait until something comes
 *                  or just return NULL if there is no package.
 * \param in_bUseQueue Use the queue or just ignore it ?
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note The user has to free the returned value !
 *
 * \author Pompei2
 */
CFTSPacket *CFTSConnection::recv(bool in_bBlock, bool in_bUseQueue)
{
    if(!m_bConnected) {
        FTS18N("InvParam", FTS_HORROR, "CFTSConnection::recv");
        return NULL;
    }

    m_mutex.lock();

    // First check the queue (if wanted).
    if(in_bUseQueue && !m_lpPacketQueue.empty()) {
        // there is something, return it.
        CFTSPacket *p = m_lpPacketQueue.front();
        m_lpPacketQueue.pop_front();
#ifdef DEBUG
        FTSMSG("Recv packet from queue with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               p->getType(),
               p->getPayloadLen());
        String s = String::sformat("Queue is now: (len:%d)", m_lpPacketQueue.size());
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            CFTSPacket *pPack = *i;
            s += String::sformat("(0x%x,%d) ", (int)pPack->getType(), (int)pPack->getPayloadLen());
        }
        s += "End.";
        FTSMSG(s.c_str(), FTS_DEBUG, 4);
#endif
        m_mutex.unlock();
        return p;
    }
    m_mutex.unlock();

    // If this is non-blocking, set a timeout of 1 milisecond.
    size_t iTimeout = in_bBlock ? FTSC_TIME_OUT : 1000;
    int serr = 0;

#if WINDOOF
    fd_set fdr;
    timeval tv = {0, iTimeout};

    FD_ZERO( &fdr );
    FD_SET( m_sock, &fdr );

    serr = ::select(1, &fdr,NULL, NULL, &tv);
#else
    do {
        pollfd pfd;
        pfd.fd = m_sock;
        pfd.events = 0 | POLLIN;
        pfd.revents = 0;

        serr = ::poll( &pfd, 1, iTimeout);
    } while( serr == SOCKET_ERROR && errno == EINTR );
#endif

    if( serr == SOCKET_ERROR ) {
        FTS18N( "Net_select", FTS_ERROR, strerror(errno), errno );
        this->disconnect();
        return NULL;
    }

    if( serr == 0 ) {
        return NULL;
    }

    CFTSPacket *p = new CFTSPacket(DSRV_MSG_NULL);
    int read = 0;
    int to_read = sizeof(fts_packet_hdr_t);
    char *buf = p->m_pData;

    m_mutex.lock();
    do {
        read = ::recv(m_sock, buf, to_read, 0);
#if WINDOOF
        if(read == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                    WSAGetLastError() == WSATRY_AGAIN ||
                                    WSAGetLastError() == WSAEWOULDBLOCK))
#else
        if(read < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
#endif
            continue;
        if(read <= 0) {
            FTS18N("Net_recv", FTS_ERROR);
            m_mutex.unlock();
            this->disconnect();
            SAFE_DELETE(p);
            return NULL;
        }
        to_read -= read;
        buf += read;
    } while(to_read);
    m_mutex.unlock();

    if( p->getPayloadLen() == 0 ) {
        FTS18N( "Net_packet_len", FTS_ERROR, p->getPayloadLen() );
        SAFE_DELETE(p);
        return NULL;
    }
    to_read = p->getPayloadLen();
    p->m_pData = (char*)MyRealloc(p->m_pData,p->getTotalLen());
    buf = &p->m_pData[sizeof(fts_packet_hdr_t)];

    m_mutex.lock();
    do {
       read = ::recv(m_sock, buf, to_read, 0);
#if WINDOOF
        if(read == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                    WSAGetLastError() == WSATRY_AGAIN ||
                                    WSAGetLastError() == WSAEWOULDBLOCK))
#else
        if(read < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
#endif
            continue;
        if(read <= 0) {
            FTS18N("Net_recv", FTS_ERROR);
            m_mutex.unlock();
            this->disconnect();
            SAFE_DELETE(p);
            return NULL;
        }
        to_read -= read;
        buf += read;
    } while(to_read);
    m_mutex.unlock();

    // All is good, check the package ID.
    if(p->isValid()) {
#ifdef DEBUG
        FTSMSG("Recv packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               (int)p->getType(),
               (int)p->getPayloadLen());
#endif
        return p;
    }
    // Invalid packet received.
    FTS18N("Net_packet", FTS_ERROR, "No FTSS Header");
    SAFE_DELETE(p);
    return NULL;
}

/// Receives a packet.
/** This waits until a packet OF A CERTAIN TYPE comes in and then accepts it.
 *  This queues all packets that are not of the given type, so they are returned
 *  by later calls to recv. Also, this firsts looks if such a message is in the
 *  queue and, if yes, returns that message.
 *
 * \param in_cID The ID of the packet to get.
 * \param in_bBlock Whether to block on the recv call or not.
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note This function MAY return old messages, if there is one corresponding in the queue.\n
 *       The user has to free the returned value !\n
 *       If this function gets other non-corresponding messages, it queues them.
 *
 * \author Pompei2
 */
CFTSPacket *CFTSConnection::recv(master_request_t in_cID, bool in_bBlock)
{
    CFTSPacket *p = NULL;

    m_mutex.lock();
    // We need to check the queue ourselves to avoid infinite recursion:
    if(!m_lpPacketQueue.empty()) {
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            p = *i;
            // If it is the packet we want, remove it from the list and return it.
            if(((fts_packet_hdr_t *)p->m_pData)->req_id == in_cID) {
                m_lpPacketQueue.erase(i);
                break;
            }

            p = NULL;
        }
    }

    if(p) {
#ifdef DEBUG
        FTSMSG("Recv packet from queue with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               (int)p->getType(),
               (int)p->getPayloadLen());
        String s = String::sformat("Queue is now: (len:%d)", m_lpPacketQueue.size());
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            s += String::sformat("(0x%x,%d) ", (int)(*i)->getType(), (int)(*i)->getPayloadLen());
        }
        s += "End.";
        FTSMSG(s.c_str(), FTS_DEBUG, 4);
#endif
        m_mutex.unlock();
        return p;
    }
    m_mutex.unlock();

    do {
        // We don't want recv to handle the queue as we would again add.
        // messages to the queue that would cause infinite recursion.
        p = this->recv(in_bBlock, false);

        m_mutex.lock();
        if(!p) {
            m_mutex.unlock();
            return NULL;
        }

        // Check if this is the packet we want.
        if(((fts_packet_hdr_t *)p->m_pData)->req_id == in_cID) {
#ifndef D_COMPILES_SERVER
            FTSMSG("Accepted packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
                   (int)p->getType(),
                   (int)p->getPayloadLen());
#endif
            m_mutex.unlock();
            return p;
        }

        // If it is not the packet we want, queue this packet.
        m_lpPacketQueue.push_back(p);

        // Don't make the queue too big.
        while(m_lpPacketQueue.size() > FTSC_MAX_QUEUE_LEN) {
            CFTSPacket *pPack = m_lpPacketQueue.front();
#ifdef DEBUG
            FTSMSG("Queue full, dropping packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
                   (int)pPack->getType(),
                   (int)pPack->getPayloadLen());
#endif
            m_lpPacketQueue.pop_front();
            SAFE_DELETE(pPack);
        }

#ifdef DEBUG
        FTSMSG("Queued packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               (int)p->getType(),
               (int)p->getPayloadLen());
        String s = String::sformat("Queue is now: (len:%d)", m_lpPacketQueue.size());
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            CFTSPacket *pPack = *i;
            s += String::sformat("(0x%x,%d) ", (int)pPack->getType(), (int)pPack->getPayloadLen());
        }
        s += "End.";
        FTSMSG(s.c_str(), FTS_DEBUG, 4);
#endif
        m_mutex.unlock();
    } while(true);

    return NULL;
}

/// Sleeps until we can send again without flooding.
/** This function sleeps the thread long enough so that when this function
 *  returns, one can send packets again without any danger of being kicked
 *  off of the server because of flooding.
 *
 * \TODO: DAMN, because of tcp storing packets and sending them altogether, this seems not to work !
 *
 * \author Pompei2
 */
void CFTSConnection::waitAntiFlood(void)
{
    /*    unsigned long ulNow = SDL_GetTicks( );
     *
     * // The first message will never be flood :)
     * if( m_ulLastcall == 0 ) {
     * m_ulLastcall = ulNow;
     * return ;
     * }
     *
     * // If this is true, he sends data too fast.
     * if( (SDL_GetTicks( ) - m_ulLastcall) < (D_NETPACKET_LEN/10) * D_ANTIFLOOD_DELAY_PER_TEN_BYTE ) {
     * unsigned long ulTimeDiff = (ulNow - m_ulLastcall);
     * unsigned long ulIdeal = ((D_NETPACKET_LEN/10) * D_ANTIFLOOD_DELAY_PER_TEN_BYTE);
     *
     * // So wait the needed amount of time.
     * dSleep( ulIdeal - ulTimeDiff );
     *
     * FTSMSG( "Sleeping %d msec to avoid flooding\n", FTS_NOMSG, ulIdeal - ulTimeDiff );
     * }
     */
    return;
}

/*! The request packet is send to the master server. The function waits until the whole
 * response is received or time out is elapsed. The response is checked for the
 * right ID in the header.
 *
 * @author Klaus.Beyer
 *
 * @param[in] in_pPacket The packet to send.
 *
 * @return CFTSPacket * Packet containing the response.
 * @retval NULL means an net error. Use get_error() to see what happens.
 *
 * @note The internal time out is currently set by the const DSRV_TIMEOUT and
 *       is fix for now.
 */
int CFTSConnection::mreq(CFTSPacket * in_pPacket)
{
    master_request_t req;

    if(!m_bConnected) {
        return FTSC_ERR_NOT_CONNECTED;
    }

    req = in_pPacket->getType();
    if(req == DSRV_MSG_NULL) {
        return FTSC_ERR_WRONG_REQ;
    }

    if( this->send( in_pPacket ) != ERR_OK ) {
      FTS18N( "Net_send", FTS_ERROR, strerror( errno ), errno );
        return FTSC_ERR_SEND;
    }

    CFTSPacket *p = this->recv(req, true);
    if(p == NULL) {
        return FTSC_ERR_RECEIVE;
    }

    if(p->getType() != req) {
        FTS18N("Net_packet", FTS_ERROR,
               String::sformat("got id %d, wanted %d",
                                ((fts_packet_hdr_t *) p->m_pData)->req_id,req).c_str());
        SAFE_DELETE(p);
        return FTSC_ERR_WRONG_RSP;
    }

    m_mutex.lock();
    m_ulLastcall = SDL_GetTicks();
    m_mutex.unlock();

    // Transfer the receive buffer to the in packet

    // Free the send data buffer
    SAFE_FREE(in_pPacket->m_pData);

    // Put the receive buffer pointer in to the in packet
    in_pPacket->m_pData = p->m_pData;

    // In order that the delete works, set data pointer to NULL
    p->m_pData = NULL;

    // delete the interims packet
    SAFE_DELETE(p);
    in_pPacket->rewind();
    return ERR_OK;
}

/*! Change a sockets blocking mode.
 *
 * @param[in] in_socket      socket to change the blocking mode
 * @param[in] in_bBlocking   true: enable blocking; false: disable blocking
 *
 * @return 0 on success ; -1 on error.
 *
 * @note
 *
 */
int CFTSConnection::setSocketBlocking(SOCKET in_socket, bool in_bBlocking)
{
#if WINDOOF
    u_long ulMode = in_bBlocking ? 0 : 1;

    return ioctlsocket(in_socket, FIONBIO, &ulMode);
#else
    int flags;

    if(in_bBlocking) {
        if((flags = fcntl(in_socket, F_GETFL, 0)) < 0) {
            fprintf(stderr, "[ERROR] socket fcntl get: %d: %s\n", errno,
                    strerror(errno));
            return -1;
        }

        if(fcntl(in_socket, F_SETFL, flags & (~O_NONBLOCK)) < 0) {
            fprintf(stderr, "[ERROR] socket fcntl set: %d: %s\n", errno,
                    strerror(errno));
            return -1;
        }
    } else {
        if((flags = fcntl(in_socket, F_GETFL, 0)) < 0) {
            fprintf(stderr, "[ERROR] socket fcntl get: %d: %s\n", errno,
                    strerror(errno));
            return -1;
        }

        if(fcntl(in_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
            fprintf(stderr, "[ERROR] socket fcntl set: %d: %s\n", errno,
                    strerror(errno));
            return -1;
        }
    }
    return 0;
#endif
}

/** Change the blocking mode of this connection's socket.
 *
 * \param in_bBlocking true: enable blocking; false: disable blocking
 *
 * \return 0 on success ; -1 on error.
 *
 * \note
 *
 */
int CFTSConnection::setBlocking(bool in_bBlocking)
{
    m_mutex.lock();
    int iRet = CFTSConnection::setSocketBlocking(m_sock, in_bBlocking);
    m_mutex.unlock();
    return iRet;
}
#else
/** Creates the connection object and connect.
 *
 * \author Klaus Beyer (kabey)
 */
CFTSTraditionalConnection::CFTSTraditionalConnection(String in_sName, int in_iPort, time_t in_nTimeout)
{
    m_ulLastcall = 0;
    m_eType =
    connectByName(in_sName, in_iPort, in_nTimeout);
}

/*! ctor. Uses a existing connection described by the 2 parameter.
 *
 * \author Klaus.Beyer (kabey)
 *
 * \param[in] in_sock the socket to use for the connection
 * \param[in] in_sa   address of counterpart to which the connection goes.
 *
 */
CFTSTraditionalConnection::CFTSTraditionalConnection(SOCKET in_sock, SOCKADDR_IN in_sa)
{
    m_ulLastcall = 0;
    m_sock = in_sock;
    m_saCounterpart = in_sa;
    m_bConnected = true;
}

/// Default destructor
/** Closes the connection.
 *
 * \author Pompei2
 */
CFTSTraditionalConnection::~CFTSTraditionalConnection()
{
    this->disconnect();
}

/// Check if i'm connected.
/** This checks if this connection is currently up or down.
 *
 * \return true if this connection is up, false if it's down.
 *
 * \note This function doesn't currently check if the connection
 *       is up right now, it just looks at the state the connection
 *       was during the last send/recv operation.
 *
 * \author Pompei2
 */
bool CFTSTraditionalConnection::isConnected(void) const
{
    return m_bConnected;
}

/// Closes the connection.
/** This safely closes the connection with the counterpart by closing the socket.
 *
 * \note You don't need to call this, as it gets called by the destructor.
 *
 * \author Pompei2
 */
void CFTSTraditionalConnection::disconnect(void)
{
    m_mutex.lock();
    if(m_bConnected) {
        close(m_sock);
        m_bConnected = false;
    }

    // We need to check empty the queue ourselves.
    if(!m_lpPacketQueue.empty()) {
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            CFTSPacket *p = *i;

            SAFE_DELETE(p);
        }
    }

    m_mutex.unlock();
}

/// Return the IP address of the counterpart.
/** This returns a string containing the counterpart's IPv4 address
 *  in the format "xxx.xxx.xxx.xxx".
 *
 * \return a string containing counterpart's IPv4 address.
 *
 * \author Pompei2
 */
String CFTSTraditionalConnection::getCounterpartIP(void) const
{
    return _S(inet_ntoa(m_saCounterpart.sin_addr));
}

/// Connects to another pc by it's name.
/** This resolves the name to an IPv4 address and then connects to it.
 *
 * \param in_sName    The name of the computer to conenct to.
 * \param in_iPort    The port you want to use for the connection.
 * \param in_nTimeout The number of seconds to try to connect before displaying an error.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int CFTSTraditionalConnection::connectByName(String in_sName, int in_iPort, time_t in_nTimeout)
{
    hostent *serverInfo = NULL;
    int iRet = -1;

    m_mutex.lock();

    // Setup the connection socket.
#if WINDOOF
    if((m_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
#else
    if((m_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
        FTS18N("Net_TCPIP_mksock", FTS_ERROR, strerror(errno), errno);
        m_mutex.unlock();
        return -1;
    }

    // Get some information we need to connect to the server.
    if(NULL == (serverInfo = gethostbyname(in_sName.c_str()))) {
        switch (h_errno) {
        case -1:
            FTS18N("Net_TCPIP_hostname", FTS_ERROR, in_sName.c_str(),
                   strerror(errno), errno);
            break;
        default:
            FTS18N("Net_TCPIP_hostname", FTS_ERROR, in_sName.c_str(),
                   "Unknown hostname", h_errno);
            break;
        }
        close(m_sock);
        m_mutex.unlock();
        return -2;
    }

    // Prepare to connect.
    m_saCounterpart.sin_family = serverInfo->h_addrtype;
    memcpy((char *)&m_saCounterpart.sin_addr.s_addr,
           serverInfo->h_addr_list[0], serverInfo->h_length);
    m_saCounterpart.sin_port = htons(in_iPort);

    // Set the socket non-blocking so we can cancel it if it can't connect.
    m_mutex.unlock();
    this->setSocketBlocking(m_sock, false);
    m_mutex.lock();

    time_t tBegin = time(NULL);

    // Try to connect to the server.
    do {
        iRet = connect(m_sock, (struct sockaddr *)&m_saCounterpart, sizeof(m_saCounterpart));

        // It was successful.
        if(iRet == 0) {
            m_bConnected = true;
            m_mutex.unlock();
            return ERR_OK;

            // Already connected.
#if WINDOOF
        } else if(WSAGetLastError() == WSAEISCONN) {
#else
        } else if(errno == EISCONN) {
#endif
            m_bConnected = true;
            m_mutex.unlock();
            return ERR_OK;

            // There was another error then the retry/in proggress/busy error.
#if WINDOOF
        } else if(WSAGetLastError() != WSAEWOULDBLOCK &&
                  WSAGetLastError() != WSAEALREADY &&
                  WSAGetLastError() != WSAEINVAL) {
            FTS18N("Net_TCPIP_connect", FTS_ERROR, in_sName.c_str(), "Address not found (maybe you're not connected to the internet)",
                   WSAGetLastError());
#else
        } else if(errno != EINPROGRESS &&
                  errno != EALREADY &&
                  errno != EAGAIN) {
            FTS18N("Net_TCPIP_connect", FTS_ERROR, in_sName.c_str(), strerror(errno), errno);
#endif
            close(m_sock);
            m_mutex.unlock();
            return -3;
        }

        // Retry as long as we have time to.
    } while((time(NULL) - tBegin) < in_nTimeout);

    //     close( m_sock );
#if WINDOOF
    FTS18N( "Net_TCPIP_connect", FTS_ERROR, in_sName.c_str( ), "Timed out (maybe the counterpart is down)", WSAGetLastError( ) );
#else
    FTS18N( "Net_TCPIP_connect", FTS_ERROR, in_sName.c_str( ), strerror( errno ), errno );
#endif
    m_mutex.unlock();
    return -4;
}

/// Sends a packet.
/** This sends a packet to the pc this connection is with.
 *
 * \param in_pPacket A pointer to the packet to send.
 *
 * \return If successful: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \todo Add a select here too. ?????
 *
 * \author Pompei2
 */
int CFTSTraditionalConnection::send(CFTSPacket * in_pPacket)
{
    if(!m_bConnected)
        return -1;

    errno = 0;

#ifdef DEBUG
    FTSMSG("Sending packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
           (int)in_pPacket->getType(),
           (int)in_pPacket->getPayloadLen());
#endif

    int iSent = 0;
    int iToSend = in_pPacket->getTotalLen();
    char *buf = in_pPacket->m_pData;

    m_mutex.lock();
    do {
        iSent = ::send(m_sock, buf, iToSend, 0);
#if WINDOOF
        if(iSent == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                     WSAGetLastError() == WSATRY_AGAIN ||
                                     WSAGetLastError() == WSAEWOULDBLOCK))
#else
        if(iSent < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
#endif
            continue;
        if(iSent < 0) {
            FTS18N("Net_TCPIP_send", FTS_ERROR, strerror(errno), errno);
            m_mutex.unlock();
//             this->disconnect();
            return -1;
        }
        iToSend -= iSent;
        buf += iSent;
    } while(iToSend > 0);

    m_ulLastcall = SDL_GetTicks();
    m_mutex.unlock();
    return ERR_OK;
}

/// Receives a packet.
/** This first (by default) looks in the message queue, if there is any message,
 *  it returns that message and removes it from the queue. If the queue is empty
 *  or shall not be used, it either just checks if there is an incoming message
 *  or waits a certain amount for one (depending on in_bBlock).
 *
 * \param in_bBlock wether to block and wait until something comes
 *                  or just return NULL if there is no package.
 * \param in_bUseQueue Use the queue or just ignore it ?
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note The user has to free the returned value !
 *
 * \author Pompei2
 */
CFTSPacket *CFTSTraditionalConnection::recv(bool in_bBlock, bool in_bUseQueue)
{
    if(!m_bConnected) {
        FTS18N("InvParam", FTS_HORROR, "CFTSConnection::recv");
        return NULL;
    }

    m_mutex.lock();

    // First check the queue (if wanted).
    if(in_bUseQueue && !m_lpPacketQueue.empty()) {
        // there is something, return it.
        CFTSPacket *p = m_lpPacketQueue.front();
        m_lpPacketQueue.pop_front();
#ifdef DEBUG
        FTSMSG("Recv packet from queue with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               p->getType(),
               p->getPayloadLen());
        String s = String::sformat("Queue is now: (len:%d)", m_lpPacketQueue.size());
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            CFTSPacket *pPack = *i;
            s += String::sformat("(0x%x,%d) ", (int)pPack->getType(), (int)pPack->getPayloadLen());
        }
        s += "End.";
        FTSMSG(s.c_str(), FTS_DEBUG, 4);
#endif
        m_mutex.unlock();
        return p;
    }
    m_mutex.unlock();

    // If this is non-blocking, set a timeout of 1 milisecond.
    size_t iTimeout = in_bBlock ? FTSC_TIME_OUT : 1;
    int serr = 0;

#if WINDOOF
    fd_set fdr;
    timeval tv = {0, iTimeout};

    FD_ZERO( &fdr );
    FD_SET( m_sock, &fdr );

    serr = ::select(1, &fdr,NULL, NULL, &tv);
#else
    do {
        pollfd pfd;
        pfd.fd = m_sock;
        pfd.events = 0 | POLLIN;
        pfd.revents = 0;

        serr = ::poll( &pfd, 1, iTimeout );
    } while( serr == SOCKET_ERROR && errno == EINTR );
#endif

    if( serr == SOCKET_ERROR ) {
        FTS18N( "Net_TCPIP_select", FTS_ERROR, strerror(errno), errno );
        this->disconnect();
        return NULL;
    }

    if( serr == 0 ) {
        return NULL;
    }

    CFTSPacket *p = new CFTSPacket(DSRV_MSG_NULL);
    int read = 0;
    int to_read = sizeof(fts_packet_hdr_t);
    char *buf = p->m_pData;

    m_mutex.lock();
    do {
        read = ::recv(m_sock, buf, to_read, 0);
#if WINDOOF
        if(read == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                    WSAGetLastError() == WSATRY_AGAIN ||
                                    WSAGetLastError() == WSAEWOULDBLOCK))
#else
        if(read < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
#endif
            continue;
        if(read <= 0) {
            FTS18N("Net_TCPIP_recv", FTS_ERROR);
            m_mutex.unlock();
            this->disconnect();
            SAFE_DELETE(p);
            return NULL;
        }
        to_read -= read;
        buf += read;
    } while(to_read);
    m_mutex.unlock();

    if( p->getPayloadLen() == 0 ) {
        FTS18N( "Net_packet_len", FTS_ERROR, p->getPayloadLen() );
        SAFE_DELETE(p);
        return NULL;
    }
    to_read = p->getPayloadLen();
    p->m_pData = (char*)MyRealloc(p->m_pData,p->getTotalLen());
    buf = &p->m_pData[sizeof(fts_packet_hdr_t)];

    m_mutex.lock();
    do {
       read = ::recv(m_sock, buf, to_read, 0);
#if WINDOOF
        if(read == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                    WSAGetLastError() == WSATRY_AGAIN ||
                                    WSAGetLastError() == WSAEWOULDBLOCK))
#else
        if(read < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
#endif
            continue;
        if(read <= 0) {
            FTS18N("Net_TCPIP_recv", FTS_ERROR);
            m_mutex.unlock();
            this->disconnect();
            SAFE_DELETE(p);
            return NULL;
        }
        to_read -= read;
        buf += read;
    } while(to_read);
    m_mutex.unlock();

    // All is good, check the package ID.
    if(p->isValid()) {
#ifdef DEBUG
        FTSMSG("Recv packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               (int)p->getType(),
               (int)p->getPayloadLen());
#endif
        return p;
    }
    // Invalid packet received.
    FTS18N("Net_packet", FTS_ERROR, "No FTSS Header");
    SAFE_DELETE(p);
    return NULL;
}

/// Receives a packet.
/** This waits until a packet OF A CERTAIN TYPE comes in and then accepts it.
 *  This queues all packets that are not of the given type, so they are returned
 *  by later calls to recv. Also, this firsts looks if such a message is in the
 *  queue and, if yes, returns that message.
 *
 * \param in_cID The ID of the packet to get.
 * \param in_bBlock Whether to block on the recv call or not.
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note This function MAY return old messages, if there is one corresponding in the queue.\n
 *       The user has to free the returned value !\n
 *       If this function gets other non-corresponding messages, it queues them.
 *
 * \author Pompei2
 */
CFTSPacket *CFTSTraditionalConnection::recv(master_request_t in_cID, bool in_bBlock)
{
    CFTSPacket *p = NULL;

    m_mutex.lock();
    // We need to check the queue ourselves to avoid infinite recursion:
    if(!m_lpPacketQueue.empty()) {
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            p = *i;
            // If it is the packet we want, remove it from the list and return it.
            if(((fts_packet_hdr_t *)p->m_pData)->req_id == in_cID) {
                m_lpPacketQueue.erase(i);
                break;
            }

            p = NULL;
        }
    }

    if(p) {
#ifdef DEBUG
        FTSMSG("Recv packet from queue with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               (int)p->getType(),
               (int)p->getPayloadLen());
        String s = String::sformat("Queue is now: (len:%d)", m_lpPacketQueue.size());
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            s += String::sformat("(0x%x,%d) ", (int)(*i)->getType(), (int)(*i)->getPayloadLen());
        }
        s += "End.";
        FTSMSG(s.c_str(), FTS_DEBUG, 4);
#endif
        m_mutex.unlock();
        return p;
    }
    m_mutex.unlock();

    do {
        // We don't want recv to handle the queue as we would again add.
        // messages to the queue that would cause infinite recursion.
        p = this->recv(in_bBlock, false);

        m_mutex.lock();
        if(!p) {
            m_mutex.unlock();
            return NULL;
        }

        // Check if this is the packet we want.
        if(((fts_packet_hdr_t *)p->m_pData)->req_id == in_cID) {
#ifndef D_COMPILES_SERVER
            FTSMSG("Accepted packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
                   (int)p->getType(),
                   (int)p->getPayloadLen());
#endif
            m_mutex.unlock();
            return p;
        }

        // If it is not the packet we want, queue this packet.
        m_lpPacketQueue.push_back(p);

        // Don't make the queue too big.
        while(m_lpPacketQueue.size() > FTSC_MAX_QUEUE_LEN) {
            CFTSPacket *pPack = m_lpPacketQueue.front();
#ifdef DEBUG
            FTSMSG("Queue full, dropping packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
                   (int)pPack->getType(),
                   (int)pPack->getPayloadLen());
#endif
            m_lpPacketQueue.pop_front();
            SAFE_DELETE(pPack);
        }

#ifdef DEBUG
        FTSMSG("Queued packet with ID 0x%x, payload len: %d", FTS_DEBUG, 4,
               (int)p->getType(),
               (int)p->getPayloadLen());
        String s = String::sformat("Queue is now: (len:%d)", m_lpPacketQueue.size());
        for(std::list<CFTSPacket *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            CFTSPacket *pPack = *i;
            s += String::sformat("(0x%x,%d) ", (int)pPack->getType(), (int)pPack->getPayloadLen());
        }
        s += "End.";
        FTSMSG(s.c_str(), FTS_DEBUG, 4);
#endif
        m_mutex.unlock();
    } while(true);

    return NULL;
}

/*! The request packet is send to the master server. The function waits until the whole
 * response is received or time out is elapsed. The response is checked for the
 * right ID in the header.
 *
 * @author Klaus.Beyer
 *
 * @param[in] in_pPacket The packet to send.
 *
 * @return CFTSPacket * Packet containing the response.
 * @retval NULL means an net error. Use get_error() to see what happens.
 *
 * @note The internal time out is currently set by the const DSRV_TIMEOUT and
 *       is fix for now.
 */
int CFTSTraditionalConnection::mreq(CFTSPacket * in_pPacket)
{
    master_request_t req;

    if(!m_bConnected) {
        return FTSC_ERR_NOT_CONNECTED;
    }

    req = in_pPacket->getType();
    if(req == DSRV_MSG_NULL) {
        return FTSC_ERR_WRONG_REQ;
    }

    if( this->send( in_pPacket ) != ERR_OK ) {
      FTS18N( "Net_TCPIP_send", FTS_ERROR, strerror( errno ), errno );
        return FTSC_ERR_SEND;
    }

    CFTSPacket *p = this->recv(req, true);
    if(p == NULL) {
        return FTSC_ERR_RECEIVE;
    }

    if(p->getType() != req) {
        FTS18N("Net_packet", FTS_ERROR,
               String::sformat("got id %d, wanted %d",
                                ((fts_packet_hdr_t *) p->m_pData)->req_id,req).c_str());
        SAFE_DELETE(p);
        return FTSC_ERR_WRONG_RSP;
    }

    m_mutex.lock();
    m_ulLastcall = SDL_GetTicks();
    m_mutex.unlock();

    // Transfer the receive buffer to the in packet

    // Free the send data buffer
    SAFE_FREE(in_pPacket->m_pData);

    // Put the receive buffer pointer in to the in packet
    in_pPacket->m_pData = p->m_pData;

    // In order that the delete works, set data pointer to NULL
    p->m_pData = NULL;

    // delete the interims packet
    SAFE_DELETE(p);
    in_pPacket->rewind();
    return ERR_OK;
}

/// Sleeps until we can send again without flooding.
/** This function sleeps the thread long enough so that when this function
 *  returns, one can send packets again without any danger of being kicked
 *  off of the server because of flooding.
 *
 * \TODO: SHIT, because of tcp storing packets and sending them altogether, this seems not to work !
 *
 * \author Pompei2
 */
void CFTSTraditionalConnection::waitAntiFlood(void)
{
    /*    unsigned long ulNow = SDL_GetTicks( );
     *
     * // The first message will never be flood :)
     * if( m_ulLastcall == 0 ) {
     * m_ulLastcall = ulNow;
     * return ;
     * }
     *
     * // If this is true, he sends data too fast.
     * if( (SDL_GetTicks( ) - m_ulLastcall) < (D_NETPACKET_LEN/10) * D_ANTIFLOOD_DELAY_PER_TEN_BYTE ) {
     * unsigned long ulTimeDiff = (ulNow - m_ulLastcall);
     * unsigned long ulIdeal = ((D_NETPACKET_LEN/10) * D_ANTIFLOOD_DELAY_PER_TEN_BYTE);
     *
     * // So wait the needed amount of time.
     * dSleep( ulIdeal - ulTimeDiff );
     *
     * FTSMSG( "Sleeping %d msec to avoid flooding\n", FTS_NOMSG, ulIdeal - ulTimeDiff );
     * }
     */
    return;
}

/*! Change a sockets blocking mode.
 *
 * @param[in] in_socket      socket to change the blocking mode
 * @param[in] in_bBlocking   true: enable blocking; false: disable blocking
 *
 * @return 0 on success ; -1 on error.
 *
 * @note
 *
 */
int CFTSTraditionalConnection::setSocketBlocking(SOCKET in_socket, bool in_bBlocking)
{
#if WINDOOF
    u_long ulMode = in_bBlocking ? 0 : 1;

    return ioctlsocket(in_socket, FIONBIO, &ulMode);
#else
    int flags;

    if(in_bBlocking) {
        if((flags = fcntl(in_socket, F_GETFL, 0)) < 0) {
            FTS18N("Net_TCPIP_fcntl_get", FTS_ERROR, strerror(errno), errno);
            return -1;
        }

        if(fcntl(in_socket, F_SETFL, flags & (~O_NONBLOCK)) < 0) {
            FTS18N("Net_TCPIP_fcntl_set", FTS_ERROR, strerror(errno), errno);
            return -1;
        }
    } else {
        if((flags = fcntl(in_socket, F_GETFL, 0)) < 0) {
            FTS18N("Net_TCPIP_fcntl_get", FTS_ERROR, strerror(errno), errno);
            return -1;
        }

        if(fcntl(in_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
            FTS18N("Net_TCPIP_fcntl_set", FTS_ERROR, strerror(errno), errno);
            return -1;
        }
    }
    return 0;
#endif
}

/** Change the blocking mode of this connection's socket.
 *
 * \param in_bBlocking true: enable blocking; false: disable blocking
 *
 * \return 0 on success ; -1 on error.
 *
 * \note
 *
 */
int CFTSTraditionalConnection::setBlocking(bool in_bBlocking)
{
    m_mutex.lock();
    int iRet = CFTSTCPIPConnection::setSocketBlocking(m_sock, in_bBlocking);
    m_mutex.unlock();
    return iRet;
}

#endif
