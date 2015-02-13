/**
 * \file connection.cpp
 * \author Pompei2
 * \date 11 May 2007
 * \brief This file implements the class that represents a network
 *        connection that can send packets.
 **/

#include "net/connection.h"
#include "net/packet.h"

#include "logging/logger.h"
#include "utilities/DataContainer.h"
#include "utilities/utilities.h"
#include "logging/Chronometer.h"

#if !WINDOOF
#  include <unistd.h>
#  include <poll.h>
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <resolv.h>
#  include <netdb.h>
#  include <fcntl.h>
#else

// Gotta work this around with a function cuz a define would be too risked.
inline void close(SOCKET s)
{
    closesocket(s);
    return;
}
#  endif

#include <cerrno>
#include <cassert>

using namespace FTS;

/// TODO: Add better logging of what travels trough the net maybe ?
#define NETLOG 0
#if NETLOG && defined(DEBUG)
static String g_netlogcmt;
void netlog(const String &in_s)
{
#ifndef D_COMPILES_SERVER
    if(g_netlogcmt.isEmpty())
        FTSMSGDBG(in_s+"\n", 5);
    else
        FTSMSGDBG("("+g_netlogcmt+") - "+in_s+"\n", 5);
#else /* D_COMPILES_SERVER */
    if(g_netlogcmt.isEmpty())
        CSLog::netlog(in_s + "\n");
    else
        CSLog::netlog("(" + g_netlogcmt + ") - " + in_s + "\n");
#endif /* D_COMPILES_SERVER */
}

void netlog2(const String &in_s, uint32_t in_uiLen, const char *in_pBuf)
{
    const String sHex = String::hexFromData(in_pBuf, in_uiLen);
    String sMsg = in_s + ": "+String::nr(in_uiLen)+" Bytes: "+sHex+" (\"";
    for(uint32_t i = 0 ; i < in_uiLen ; i++) {
        // Replace control characters by a space for output.
        if(in_pBuf[i] < 32) {
            sMsg += " ";
        } else {
            sMsg += in_pBuf[i];
        }
    }

    sMsg += "\"";
    netlog(sMsg);
}
#else
#  define netlog(a)
#  define netlog2(a, b, c)
#endif

/// Retrieves the packet in front of the queue or the first packet with a special ID.
/** This takes out either the packet that is in front of the message queue (if \a in_req
 *  is DSRV_MSG_NONE) or the first packet whose request id is \a in_req and
 *  returns it to the caller. If there is no message in the queue or no message with
 *  the request id being \a in_req, it just returns NULL.
 *
 * \return If successfull:    A pointer to the packet.
 * \return If queue is empty or no request was found: NULL
 *
 * \note The user has to free the returned value !
 *
 * \author Pompei2
 */
Packet *FTS::Connection::getFirstPacketFromQueue(master_request_t in_req)
{
    Lock l(m_mutex);

    if(m_lpPacketQueue.empty()) {
        return NULL;
    }

    Packet *p = NULL;

    // Just get the first one ?
    if(in_req == DSRV_MSG_NONE) {
        // Just get the first packet of the queue.
        p = m_lpPacketQueue.front();
        m_lpPacketQueue.pop_front();
    } else {
        // Search the list for the first packet with the corresponding request id.
        for(std::list<Packet *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            p = *i;
            // If it is the packet we want, remove it from the list and return it.
            if(((fts_packet_hdr_t *)p->m_pData)->req_id == in_req) {
                m_lpPacketQueue.erase(i);
                break;
            }

            p = NULL;
        }

        // If we found none with that req_id, return NULL.
        if(p == NULL) {
            return NULL;
        }
    }

#ifdef DEBUG
    FTSMSGDBG("Recv packet from queue with ID 0x{1}, payload len: {2}", 4,
              String::nr(p->getType(), -1, ' ', std::ios::hex), String::nr(p->getPayloadLen()));
    String s = "Queue is now: (len:"+String::nr(m_lpPacketQueue.size())+")";
    for(std::list<Packet *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
        Packet *pPack = *i;
        s += "(0x" + String::nr(pPack->getType(), -1, ' ', std::ios::hex) + "," + String::nr(pPack->getPayloadLen()) + ")";
    }
    s += "End.";
    FTSMSGDBG(s, 4);
#endif

    return p;
}

/// Adds a packet to the queue.
/** This first adds a packet at the end of the queue, then checks if the queue is
 *  too big. If it is, it drops out the first packet in the front of the queue.
 *  Also displays debugging messages about what it does.
 *
 * \author Pompei2
 */
void FTS::Connection::queuePacket(Packet *in_pPacket)
{
    if(!in_pPacket)
        return ;

    Lock l(m_mutex);
    m_lpPacketQueue.push_back(in_pPacket);

    // Don't make the queue too big.
    while(m_lpPacketQueue.size() > FTSC_MAX_QUEUE_LEN) {
        Packet *pPack = m_lpPacketQueue.front();
#ifdef DEBUG
        FTSMSGDBG("Queue full, dropping packet with ID 0x{1}, payload len: {2}",4,
                  String::nr(pPack->getType(), -1, ' ', std::ios::hex), String::nr(pPack->getPayloadLen()));
#endif
        m_lpPacketQueue.pop_front();
        SAFE_DELETE(pPack);
    }

#ifdef DEBUG
    FTSMSGDBG("Queued packet with ID 0x{1}, payload len: {2}", 4,
              String::nr(in_pPacket->getType(),-1,' ',std::ios::hex), String::nr(in_pPacket->getPayloadLen()));
    String s = "Queue is now: (len:"+String::nr(m_lpPacketQueue.size())+")";
    for(std::list<Packet *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
        Packet *pPack = *i;
        s += "(0x" + String::nr(pPack->getType(), -1, ' ', std::ios::hex) + "," + String::nr(pPack->getPayloadLen()) + ")";
    }
    s += "End.";
    FTSMSGDBG(s, 4);
#endif
}

/// Creates the connection object and connect.
/** This creates the connection object and tries to connect to the specified
 *  server. You can check if the connection succeeded by calling the
 *  isConnected method.
 *
 * \param in_sName The name of the machine to connect to. This may be a name like
 *                 srv.bla.org or an IP-address like 127.0.0.1
 * \param in_usPort The port to connect to.
 * \param in_nTimeout The maximum number of milliseconds (1/1000 seconds)
 *                    to wait for a connection.
 *
 * \author Klaus Beyer (kabey)
 *
 * \note modified by Pompei2
 */
FTS::TraditionalConnection::TraditionalConnection(const String &in_sName, uint16_t in_usPort, uint64_t in_ulTimeoutInMillisec)
    : m_bConnected(false)
    , m_sock(0)
{
    memset(&m_saCounterpart, 0, sizeof(m_saCounterpart));
    connectByName(in_sName, in_usPort, in_ulTimeoutInMillisec);
}

/*! ctor. Uses a existing connection described by the 2 parameter.
 *
 * \author Klaus.Beyer (kabey)
 *
 * \param[in] in_sock the socket to use for the connection
 * \param[in] in_sa   address of counterpart to which the connection goes.
 *
 */
FTS::TraditionalConnection::TraditionalConnection(SOCKET in_sock, SOCKADDR_IN in_sa)
    : m_bConnected(true)
    , m_sock(in_sock)
    , m_saCounterpart(in_sa)
{
}

/// Default destructor
/** Closes the connection.
 *
 * \author Pompei2
 */
FTS::TraditionalConnection::~TraditionalConnection()
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
bool FTS::TraditionalConnection::isConnected()
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
void FTS::TraditionalConnection::disconnect()
{
    Lock l(m_mutex);
    if(m_bConnected) {
        close(m_sock);
        m_bConnected = false;
    }

    // We need to check empty the queue ourselves.
    if(!m_lpPacketQueue.empty()) {
        for(std::list<Packet *>::iterator i = m_lpPacketQueue.begin() ; i != m_lpPacketQueue.end() ; i++) {
            Packet *p = *i;

            SAFE_DELETE(p);
        }
    }
}

/// Return the IP address of the counterpart.
/** This returns a string containing the counterpart's IPv4 address
 *  in the format "xxx.xxx.xxx.xxx".
 *
 * \return a string containing counterpart's IPv4 address.
 *
 * \author Pompei2
 */
String FTS::TraditionalConnection::getCounterpartIP() const
{
    return String(inet_ntoa(m_saCounterpart.sin_addr));
}

/// Connects to another pc by it's name.
/** This resolves the name to an IPv4 address and then connects to it.
 *  If we are already connected, it first closes the old connection.
 *
 * \param in_sName    The name of the computer to conenct to.
 * \param in_iPort    The port you want to use for the connection.
 * \param in_nTimeout The number of milliseconds to try to connect before displaying an error.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FTS::TraditionalConnection::connectByName(String in_sName, uint16_t in_usPort, uint64_t in_ulTimeoutInMillisec)
{
    if(this->isConnected()) {
        this->disconnect();
    }

    hostent *serverInfo = NULL;
    int iRet = -1;

    { Lock l(m_mutex);

        // Setup the connection socket.
#if WINDOOF
        if((m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
#else
        if((m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
#endif
            FTS18N("Net_TCPIP_mksock", MsgType::Error, strerror(errno), String::nr(errno));
            return -1;
        }

        // Get some information we need to connect to the server.
        if(NULL == (serverInfo = gethostbyname(in_sName.c_str()))) {
            switch (h_errno) {
            case -1:
                FTS18N("Net_TCPIP_hostname",MsgType::Error,in_sName,strerror(errno),String::nr(errno));
                break;
            default:
                FTS18N("Net_TCPIP_hostname",MsgType::Error,in_sName,"Unknown hostname",String::nr(h_errno));
                break;
            }
            close(m_sock);
            return -2;
        }

        // Prepare to connect.
        m_saCounterpart.sin_family = serverInfo->h_addrtype;
        memcpy((char *)&m_saCounterpart.sin_addr.s_addr,
               serverInfo->h_addr_list[0], serverInfo->h_length);
        m_saCounterpart.sin_port = htons(in_usPort);
    }

    // Set the socket non-blocking so we can cancel it if it can't connect.
    this->setSocketBlocking(m_sock, false);

    Lock l(m_mutex);

    Chronometer chron;

    // Try to connect to the server.
    do {
        iRet = connect(m_sock, (struct sockaddr *)&m_saCounterpart, sizeof(m_saCounterpart));

        // It was successful.
        if(iRet == 0) {
            m_bConnected = true;
            return ERR_OK;

            // Already connected.
#if WINDOOF
        } else if(WSAGetLastError() == WSAEISCONN) {
#else
        } else if(errno == EISCONN) {
#endif
            m_bConnected = true;
            return ERR_OK;

            // Need to wait for the socket to be available.
#if WINDOOF
#else
        } else if(errno == EINPROGRESS) {
            int serr = 0;
//             do {
                pollfd pfd;
                pfd.fd = m_sock;
                pfd.events = 0 | POLLOUT;
                pfd.revents = 0;

                // Wait an amount of time or wait infinitely
                if(in_ulTimeoutInMillisec == ((uint64_t)(-1)))
                    serr = ::poll( &pfd, 1, -1 );
                else
                    serr = ::poll( &pfd, 1, (int)(in_ulTimeoutInMillisec) );
//             } while( serr == SOCKET_ERROR && errno == EINTR && in_ulTimeoutInMillisec == ((uint64_t)(-1)));
            if(serr == SOCKET_ERROR) {
                FTS18N("Net_TCPIP_select", MsgType::Error, strerror(errno), String::nr(errno));
                close(m_sock);
                return -4;
            }

            int result = 0;
            socklen_t len = sizeof(int);
            getsockopt(m_sock, SOL_SOCKET, SO_ERROR, &result, &len);
            // Connection succeeded.
            if(result == 0 || result == EISCONN) {
                m_bConnected = true;
                return ERR_OK;
            } else {
                FTS18N("Net_TCPIP_connect", MsgType::Error, in_sName, String::nr(in_usPort), strerror(result), String::nr(result));
                close(m_sock);
                return -5;
            }
#endif

            // There was another error then the retry/in proggress/busy error.
#if WINDOOF
        } else if(WSAGetLastError() != WSAEWOULDBLOCK &&
                  WSAGetLastError() != WSAEALREADY &&
                  WSAGetLastError() != WSAEINVAL) {
                      FTS18N("Net_TCPIP_connect", MsgType::Error, in_sName, String::nr(in_usPort),
                   "Address not found (maybe you're not connected to the internet)",
                   String::nr(WSAGetLastError()));
#else
        } else if(/*errno != EINPROGRESS &&*/
                  errno != EALREADY &&
                  errno != EAGAIN) {
            FTS18N("Net_TCPIP_connect", MsgType::Error, in_sName, String::nr(in_usPort), strerror(errno), String::nr(errno));
#endif
            close(m_sock);
            return -3;
        }

        // Retry as long as we have time to.
    } while(chron.measure() < ((double)in_ulTimeoutInMillisec)/1000.0);

    //     close( m_sock );
#if WINDOOF
    FTS18N( "Net_TCPIP_connect_to", MsgType::Error, in_sName, String::nr(in_usPort), "Timed out (maybe the counterpart is down)", String::nr(WSAGetLastError( )) );
#else
    FTS18N( "Net_TCPIP_connect_to", MsgType::Error, in_sName, String::nr(in_usPort), strerror( errno ), String::nr(errno) );
#endif
    return -4;
}

/// lowlevel data receiving method.
/** This tries to receive some amount of data over the network. If it does get
 *  nothing (or not enough) within the time it has been accorded, it returns.
 *
 * \param out_pBuf The (allocated) buffer where to write the data.
 * \param in_uiLen The length of the buffer to get.
 * \param in_ulMaxWaitMillisec The amount of milliseconds (1/1000 seconds) to
 *                             wait for a message to come.\n
 *
 * \return If successfull:  0
 * \return If failed:      <0
 * \return If timed out:    1
 *
 * \note The user has to allocate the buffer big enough!
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved. Anyway, on most PC's
 *       an exactitude of more the 10ms is nearly never possible.
 * \internal This method is only for internal use!
 *
 * \author Pompei2
 */
int FTS::TraditionalConnection::get_lowlevel(void *out_pBuf, uint32_t in_uiLen, uint64_t in_ulMaxWaitMillisec)
{
    int read = 0;
    uint32_t to_read = in_uiLen;
    int8_t *buf = (int8_t *)out_pBuf;

    Chronometer timeout;

    do {
        { Lock l(m_mutex);
#if WINDOOF
            read = ::recv(m_sock, (char *)buf, to_read, 0);
            if(read == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                        WSAGetLastError() == WSATRY_AGAIN ||
                                        WSAGetLastError() == WSAEWOULDBLOCK)) {
                // Only check for timeouts when waiting for data!
                if(timeout.measure()*1000.0 > in_ulMaxWaitMillisec) {
                    netlog("Dropping due to timeout (allowed "+String::nr(in_ulMaxWaitMillisec)+" ms)!");
                    return ERR_OK;
                }
                continue;
            }
#else
            read = ::recv(m_sock, buf, to_read, 0);
            if(read < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
                // Only check for timeouts when waiting for data!
                if(timeout.measure()*1000.0 > in_ulMaxWaitMillisec) {
                    netlog("Dropping due to timeout (allowed "+String::nr(in_ulMaxWaitMillisec)+" ms)!");
                    return ERR_OK;
                }
                continue;
            }
#endif
        }

        if(read <= 0) {
            FTS18N("Net_TCPIP_recv", MsgType::Error);
            this->disconnect();
            return -1;
        }

        // If we get some data, we re-init the timeout (give full time again)
        timeout.reset();
        to_read -= read;
        buf += read;
    } while(to_read);

#if defined(DEBUG) && NETLOG
    netlog2("recv", in_uiLen, (const char *)out_pBuf);
#endif

    return ERR_OK;
}

/** This tries to receive data over the network until some ending string. If it
 *  does get nothing (or not enough) within the time it has been accorded, it
 *  returns what it got so-far.
 *
 * \param in_sLineEnding The method will stop getting data when it gets this
 *                       exact data-string.
 * \param in_ulMaxWaitMillisec The amount of milliseconds (1/1000 seconds) to
 *                             wait for a message to come.\n
 *
 * \return The data it got.
 *
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved. Anyway, on most PC's
 *       an exactitude of more the 10ms is nearly never possible.
 * \internal This method is only for internal use!
 *
 * \author Pompei2
 */
String FTS::TraditionalConnection::getLine(const String in_sLineEnding, uint64_t in_ulMaxWaitMillisec)
{
    String sLine;
    uint8_t byte[2] = {0};
    while(this->get_lowlevel(&byte[0], 1, in_ulMaxWaitMillisec) == ERR_OK) {
        sLine += byte;

        // Got an end of line?
        if(sLine.right(in_sLineEnding.len()) == in_sLineEnding) {
            return sLine;
        }
    }

    // Connection lost or timed out before EOL.
    return sLine;
}

/// (Waits for and then) receives any packet.
/** This first (by default) looks in the message queue, if there is any message,
 *  it returns that message and removes it from the queue. If the queue is empty
 *  or shall not be used, it waits a certain amount of time, infilitely or not
 *  at all for a message to come over the net.
 *
 * \param in_bUseQueue Use the queue or just ignore it ?
 * \param in_ulMaxWaitMillisec The amount of milliseconds (1/1000 seconds) to
 *                             wait for a message to come.\n
 *                             0 means to not wait at all, instantly returning NULL
 *                             if there is nothing available.\n
 *                             ((uint64_t)(-1)) means wait for an infinite time (dangerous).
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note The user has to free the returned value!
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved. Anyway, on most PC's
 *       an exactitude of more the 10ms is nearly never possible.
 * \note This method eats all network incoming data until it finds the FTSS identifier!
 * \internal This method is only for internal use!
 *
 * \author Pompei2
 */
Packet *FTS::TraditionalConnection::getPacket(bool in_bUseQueue, uint64_t in_ulMaxWaitMillisec)
{
    if(!m_bConnected) {
        FTS18N("InvParam", MsgType::Horror, "FTS::Connection::recv");
        return NULL;
    }

    // First, check the queue if wanted.
    if(in_bUseQueue) {
        Packet *p = this->getFirstPacketFromQueue();
        if(p)
            return p;
    }

    int serr = 0;

    Chronometer timeout;

#if WINDOOF
    fd_set fdr;
    timeval tv = {0, (size_t)in_ulMaxWaitMillisec*1000};

    FD_ZERO( &fdr );
    FD_SET( m_sock, &fdr );

    // Wait an amount of time or wait infinitely
    if(in_ulMaxWaitMillisec == ((uint64_t)(-1)))
        serr = ::select(1, &fdr,NULL, NULL, NULL);
    else
        serr = ::select(1, &fdr,NULL, NULL, &tv);
#else
    do {
        pollfd pfd;
        pfd.fd = m_sock;
        pfd.events = 0 | POLLIN;
        pfd.revents = 0;

        // Wait an amount of time or wait infinitely
        if(in_ulMaxWaitMillisec == ((uint64_t)(-1)))
            serr = ::poll( &pfd, 1, -1 );
        else
            serr = ::poll( &pfd, 1, (int)(in_ulMaxWaitMillisec) );
    } while( serr == SOCKET_ERROR && errno == EINTR );
#endif

    if( serr == SOCKET_ERROR ) {
        FTS18N( "Net_TCPIP_select", MsgType::Error, strerror(errno), String::nr(errno) );
        this->disconnect();
        return NULL;
    }

    if( serr == 0 ) {
        return NULL;
    }

    if(timeout.measure()*1000.0 > in_ulMaxWaitMillisec) {
        netlog("Dropping due to timeout (allowed "+String::nr(in_ulMaxWaitMillisec)+" ms)!");
        return NULL;
    }

    // First, ignore everything until the "FTSS" identifier.
    int8_t buf;

    while(true) {
        // Get the first "F"
#if defined(DEBUG) && NETLOG
        g_netlogcmt = "getPacket: first F";
#endif
        do {
            if(ERR_OK != this->get_lowlevel(&buf, 1, (uint64_t)(in_ulMaxWaitMillisec - 1000.0*timeout.measure()) )) {
#if defined(DEBUG) && NETLOG
                g_netlogcmt = NULL;
#endif
                return NULL;
            }
        } while(buf != 'F');

        // Check for the next "T". If it isn't one, restart.
#if defined(DEBUG) && NETLOG
        g_netlogcmt = "getPacket: first T";
#endif
        if(ERR_OK != this->get_lowlevel(&buf, 1, (uint64_t)(in_ulMaxWaitMillisec - 1000.0*timeout.measure()) )) {
#if defined(DEBUG) && NETLOG
                g_netlogcmt = NULL;
#endif
            return NULL;
        }

        if(buf != 'T')
            continue;

        // Check for the next "S". If it isn't one, restart.
#if defined(DEBUG) && NETLOG
        g_netlogcmt = "getPacket: first S";
#endif
        if(ERR_OK != this->get_lowlevel(&buf, 1, (uint64_t)(in_ulMaxWaitMillisec - 1000.0*timeout.measure()) )) {
#if defined(DEBUG) && NETLOG
                g_netlogcmt = NULL;
#endif
            return NULL;
        }

        if(buf != 'S')
            continue;

        // Check for the next "S". If it isn't one, restart.
#if defined(DEBUG) && NETLOG
        g_netlogcmt = "getPacket: 2nd   S";
#endif
        if(ERR_OK != this->get_lowlevel(&buf, 1, (uint64_t)(in_ulMaxWaitMillisec - 1000.0*timeout.measure()) )) {
#if defined(DEBUG) && NETLOG
                g_netlogcmt = NULL;
#endif
            return NULL;
        }

        if(buf != 'S')
            continue;
        break;
    }

    // We already got the "FTSS" header, now get the rest of the header.
#if defined(DEBUG) && NETLOG
    g_netlogcmt = "getPacket: header rest";
#endif
    Packet *p = new Packet(DSRV_MSG_NULL);
    serr = this->get_lowlevel(&p->m_pData[4], sizeof(fts_packet_hdr_t)-4, (uint64_t)(in_ulMaxWaitMillisec - 1000.0*timeout.measure()) );
#if defined(DEBUG) && NETLOG
    g_netlogcmt = NULL;
#endif
    if( serr != ERR_OK ) {
        SAFE_DELETE( p );
        return NULL;
    }

    // Now, prepare to get the packet's data.
    if( p->getPayloadLen() <= 0 ) {
        FTS18N( "Net_packet_len", MsgType::Error, String::nr(p->getPayloadLen()) );
        SAFE_DELETE(p);
        return NULL;
    }

    p->m_pData = (int8_t*)realloc(p->m_pData,p->getTotalLen());
    assert(p->m_pData != NULL );
    // And get it.
#if defined(DEBUG) && NETLOG
    g_netlogcmt = "getPacket: data";
#endif
    serr = this->get_lowlevel(&p->m_pData[sizeof(fts_packet_hdr_t)], p->getPayloadLen(), (uint64_t)(in_ulMaxWaitMillisec - 1000.0*timeout.measure()) );
#if defined(DEBUG) && NETLOG
    g_netlogcmt = NULL;
#endif
    if(serr != ERR_OK) {
        SAFE_DELETE( p );
        return NULL;
    }

    // All is good, check the package ID.
    if(p->isValid()) {
#if defined(DEBUG) && !defined(D_COMPILES_SERVER)
        FTSMSGDBG("Recv packet with ID 0x{1}, payload len: {2}", 4,
                  String::nr(p->getType(), -1, ' ', std::ios::hex), String::nr(p->getPayloadLen()));
#endif
        return p;
    }

    // Invalid packet received.
    FTS18N("Net_packet", MsgType::Error, "No FTSS Header/Invalid request");
    SAFE_DELETE(p);
    return NULL;
}

/// Waits for and then receives any packet.
/** This first (by default) looks in the message queue, if there is any message,
 *  it returns that message and removes it from the queue. If the queue is empty
 *  or shall not be used, it waits a certain amount of time for a message to come
 *  over the net. (or waits indefinitely if \a in_nMaxWaitMillisec == 0).
 *
 * \param in_bUseQueue Use the queue or just ignore it ?
 * \param in_nMaxWaitMillisec The amount of milliseconds (1/1000 seconds) to
 *                            wait for a message to come, or 0, that means wait
 *                            for an infinite time (this is dangerous).
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note The user has to free the returned value !
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved. Anyway, on most PC's
 *       an exactitude of more the 10ms is nearly never possible.
 *
 * \author Pompei2
 */
Packet *FTS::TraditionalConnection::waitForThenGetPacket(bool in_bUseQueue, uint64_t in_nMaxWaitMillisec)
{
    return this->getPacket(in_bUseQueue, in_nMaxWaitMillisec == 0 ? ((uint64_t)(-1)) : in_nMaxWaitMillisec);
}

/// Checks for and then receives any packet.
/** This first (by default) looks in the message queue, if there is any message,
 *  it returns that message and removes it from the queue. If the queue is empty
 *  or shall not be used, it checks if a message is coming over the net. If ther is,
 *  it retrieves it. If there is none, it returns NULL immediately.
 *
 * \param in_bUseQueue Use the queue or just ignore it ?
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note The user has to free the returned value !
 *
 * \author Pompei2
 */
Packet *FTS::TraditionalConnection::getPacketIfPresent(bool in_bUseQueue)
{
    return this->getPacket(in_bUseQueue, 0);
}

/// Waits for and then receives a certain packet.
/** This first looks in the message queue, if there is a certain message,
 *  it returns that message and removes it from the queue. If the queue is empty,
 *  it waits a certain amount of time for a certain message to come over the net.
 *  (or waits indefinitely if \a in_nMaxWaitMillisec == 0).\n
 *  If a message with the wrong request ID comes during this time, it is queued and
 *  waited for the next message.\n
 *  A _certain_ message means a message with a special request ID (as in \a in_req ).
 *
 * \param in_req The request ID of the message to wait for (DSRV_MSG_XXX).
 * \param in_nMaxWaitMillisec The amount of milliseconds (1/1000 seconds) to
 *                            wait for a message to come, or 0, that means wait
 *                            for an infinite time (this is dangerous).
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note The user has to free the returned value !
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved.
 *       Anyway, on most PC's an exactitude of more the 10ms is nearly never possible.
 * \note If the queue is full, the first messages put in it are dropped!
 *
 * \author Pompei2
 */
Packet *FTS::TraditionalConnection::waitForThenGetPacketWithReq(master_request_t in_req, uint64_t in_ulMaxWaitMillisec)
{
    // Check for valid packet request ID's.
    if(in_req == DSRV_MSG_NONE || in_req > DSRV_MSG_MAX)
        return NULL;

    // We need to check the queue ourselves to avoid infinite recursion:
    Packet *p = this->getFirstPacketFromQueue(in_req);
    if(p)
        return p;

    // Nothing in the queue, wait for a message.
    do {
        // We don't want recv to handle the queue as we would again add
        // messages to the queue that would cause infinite recursion.
        p = this->waitForThenGetPacket(false, in_ulMaxWaitMillisec);

        // Nothing got in time, bye.
        if(!p)
            return NULL;

        // Check if this is the packet we want.
        if(((fts_packet_hdr_t *)p->m_pData)->req_id == in_req) {
#ifndef D_COMPILES_SERVER
            FTSMSGDBG("Accepted packet with ID 0x{1}, payload len: {2}", 4,
                      String::nr(p->getType(), -1, ' ', std::ios::hex), String::nr(p->getPayloadLen()));
#endif
            return p;
        }

        // If it is not the packet we want, queue this packet.
        this->queuePacket(p);
    } while(true);

    return NULL;
}

/// Checks for and then receives a certain packet.
/** This first looks in the message queue, if there is a certain message,
 *  it returns that message and removes it from the queue. If the queue is empty,
 *  it checks if a certain message is coming over the net. If there is,
 *  it retrieves it. If there is none, it returns NULL immediately.\n
 *  If there is a message with the wrong request ID, it is queued and tried again.\n
 *  A _certain_ message means a message with a special request ID (as in \a in_req ).
 *
 * \param in_req The request ID of the message to wait for (DSRV_MSG_XXX).
 *
 * \return If successfull: A pointer to the packet.
 * \return If failed:      NULL
 *
 * \note The user has to free the returned value !
 * \note If the queue is full, the first messages put in it are dropped!
 *
 * \author Pompei2
 */
Packet *FTS::TraditionalConnection::getPacketWithReqIfPresent(master_request_t in_req)
{
    // Check for valid packet request ID's.
    if(in_req == DSRV_MSG_NONE || in_req > DSRV_MSG_MAX)
        return NULL;

    // We need to check the queue ourselves to avoid infinite recursion:
    Packet *p = this->getFirstPacketFromQueue(in_req);
    if(p)
        return p;

    // Nothing in the queue, check for a message.
    while((p = this->getPacketIfPresent(true)) != NULL) {

        // Check if this is the packet we want.
        if(((fts_packet_hdr_t *)p->m_pData)->req_id == in_req) {
#ifndef D_COMPILES_SERVER
            FTSMSGDBG("Accepted packet with ID 0x{1}, payload len: {2}", 4,
                      String::nr(p->getType(), -1, ' ', std::ios::hex), String::nr(p->getPayloadLen()));
#endif
            return p;
        }

        // If it is not the packet we want, queue this packet and try again
        this->queuePacket(p);
    }

    // No packet on the wire.
    return NULL;
}

/// Sends some data.
/** This sends some data to the pc this connection is with.
 *
 * \param in_pData A pointer to the data to send.
 * \param in_uiLen The length of the data to send.
 *
 * \return If successful: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \todo Add a select here too. ?????
 *
 * \author Pompei2
 */
int FTS::TraditionalConnection::send(const void *in_pData, uint32_t in_uiLen)
{
    if(!m_bConnected)
        return -1;

    errno = 0;

    int iSent = 0;
    uint32_t uiToSend = in_uiLen;
    const int8_t *buf = (const int8_t *)in_pData;

    Lock l(m_mutex);
    do {
#if WINDOOF
        iSent = ::send(m_sock, (const char *)buf, uiToSend, 0);
        if(iSent == SOCKET_ERROR && (WSAGetLastError() == WSAEINTR ||
                                     WSAGetLastError() == WSATRY_AGAIN ||
                                     WSAGetLastError() == WSAEWOULDBLOCK))
#else
        iSent = ::send(m_sock, buf, uiToSend, 0);
        if(iSent < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
#endif
            continue;
        if(iSent < 0) {
            FTS18N("Net_TCPIP_send", MsgType::Error, strerror(errno), String::nr(errno));
//             this->disconnect();
            return -1;
        }
        uiToSend -= iSent;
        buf += iSent;
    } while(uiToSend > 0);

#if defined(DEBUG)
    netlog2("send", in_uiLen, (const char *)in_pData);
#endif

    return ERR_OK;
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
int FTS::TraditionalConnection::send(Packet * in_pPacket)
{
    if(!m_bConnected || in_pPacket == NULL)
        return -1;

#if defined(DEBUG) && !defined(D_COMPILES_SERVER)
    FTSMSGDBG("Sending packet with ID 0x{1}, payload len: {2}", 4,
              String::nr(in_pPacket->getType(), -1, ' ', std::ios::hex),
              String::nr(in_pPacket->getPayloadLen()));
#endif

    if(this->send(in_pPacket->m_pData, in_pPacket->getTotalLen()) != ERR_OK)
        return -1;

    return ERR_OK;
}

/*! The request packet is send to the master server. The function waits until the whole
 * response is received or time out is elapsed. The response is checked for the
 * right ID in the header.\n
 * The input packet is destroyed and replaced by the response packet.
 *
 * @author Klaus.Beyer
 *
 * @param[out] out_pPacket The packet to send. Will be replaced by the response.
 * @param[in] in_ulMaxWaitMillisec The maximum time to wait for the response (in milliseconds).
 *
 * @return If successfull: ERR_OK
 * @return If failed:      Error code < 0
 *
 * @note Adapted by Pompei2.
 */
int FTS::TraditionalConnection::mreq(Packet *out_pPacket, uint64_t in_ulMaxWaitMillisec)
{
    master_request_t req;

    if(!m_bConnected) {
        return FTSC_ERR_NOT_CONNECTED;
    }

    req = out_pPacket->getType();
    if(req == DSRV_MSG_NULL || req == DSRV_MSG_NONE || req > DSRV_MSG_MAX ) {
        return FTSC_ERR_WRONG_REQ;
    }

    if(this->send(out_pPacket) != ERR_OK) {
        FTS18N( "Net_TCPIP_send", MsgType::Error, strerror( errno ), String::nr(errno) );
        return FTSC_ERR_SEND;
    }

    Packet *p = this->waitForThenGetPacketWithReq(req, in_ulMaxWaitMillisec);
    if(p == NULL) {
        return FTSC_ERR_RECEIVE;
    }

    if(p->getType() != req) {
        master_request_t id = ((fts_packet_hdr_t *) p->m_pData)->req_id;
        FTS18N("Net_packet", MsgType::Error, "got id "+String::nr(id)+", wanted "+String::nr(req));
        SAFE_DELETE(p);
        return FTSC_ERR_WRONG_RSP;
    }

    // Transfer the receive buffer to the in packet

    // Free the send data buffer
    SAFE_FREE(out_pPacket->m_pData);

    // Put the receive buffer pointer in to the in packet
    out_pPacket->m_pData = p->m_pData;

    // In order that the delete works, set data pointer to NULL
    p->m_pData = NULL;

    // delete the interim packet
    SAFE_DELETE(p);
    out_pPacket->rewind();
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
void FTS::TraditionalConnection::waitAntiFlood()
{
    /*    unsigned long ulNow = dGetTicks( );
     *
     * // The first message will never be flood :)
     * if( m_ulLastcall == 0 ) {
     * m_ulLastcall = ulNow;
     * return ;
     * }
     *
     * // If this is true, he sends data too fast.
     * if( (dGetTicks( ) - m_ulLastcall) < (D_NETPACKET_LEN/10) * D_ANTIFLOOD_DELAY_PER_TEN_BYTE ) {
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
int FTS::TraditionalConnection::setSocketBlocking(SOCKET in_socket, bool in_bBlocking)
{
#if WINDOOF
    u_long ulMode = in_bBlocking ? 0 : 1;

    return ioctlsocket(in_socket, FIONBIO, &ulMode);
#else
    int flags;

    if(in_bBlocking) {
        if((flags = fcntl(in_socket, F_GETFL, 0)) < 0) {
            FTS18N("Net_TCPIP_fcntl_get", MsgType::Error, strerror(errno), String::nr(errno));
            return -1;
        }

        if(fcntl(in_socket, F_SETFL, flags & (~O_NONBLOCK)) < 0) {
            FTS18N("Net_TCPIP_fcntl_set", MsgType::Error, strerror(errno), String::nr(errno));
            return -1;
        }
    } else {
        if((flags = fcntl(in_socket, F_GETFL, 0)) < 0) {
            FTS18N("Net_TCPIP_fcntl_get", MsgType::Error, strerror(errno), String::nr(errno));
            return -1;
        }

        if(fcntl(in_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
            FTS18N("Net_TCPIP_fcntl_set", MsgType::Error, strerror(errno), String::nr(errno));
            return -1;
        }
    }
    return 0;
#endif
}

/// Gets a file via HTTP.
/** This sends an HTTP server the request to get a file and then gets that file
 *  from the server.
 *
 * \param out_data Where to store the data of the file.
 * \param in_sServer The server address to connect to, ex: arkana-fts.org
 * \param in_sPath The path to the file on the server, ex: /path/to/file.ex
 * \param out_uiFileSize Will be set to the size of the data that will be returned.
 *
 * \return If successfull:  ERR_OK.
 * \return If failed:       An error code < 0
 *
 * \note The DataContainer object you give will be resized to match the file's
 *       size and the content will then be overwritten.
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved. Anyway, on most PC's
 *       an exactitude of more the 10ms is nearly never possible.
 * \note Although the \a out_uiFileSize is a 64-bit unsigned integer, the
 *       support for large files (>4GB) is not implemented yet.
 *
 * \author Pompei2
 */
int FTS::getHTTPFile(FTS::RawDataContainer &out_data, const String &in_sServer, const String &in_sPath, uint64_t in_ulMaxWaitMillisec)
{
    // We connect using the traditional connection.
    TraditionalConnection tradConn(in_sServer, 80, in_ulMaxWaitMillisec);
    if(!tradConn.isConnected())
        return -1;

    // Send the request to get that file.
    String sToSend = "GET http://" + in_sServer + in_sPath + " HTTP/1.0\r\n\r\n";
    tradConn.send(sToSend.c_str(), sToSend.len());

    // The first line we get is very interesting: the status.
    String sLine = tradConn.getLine("\r\n", in_ulMaxWaitMillisec);
    String sCode = sLine.mid(9,sLine.len()-12);
    if(sCode != "200") {
        // 200 means OK.
        return -2;
    }

    // The following loop reads out the HTTP header and gets the data size.
    uint64_t uiFileSize = 0;
    while(true) {
        String sHttpLine = tradConn.getLine("\r\n", in_ulMaxWaitMillisec);

        // Reached the end.
        if(sHttpLine == String::EMPTY || sHttpLine == "\r\n") {
            break;
        }

        // This line is interesting.
        if(sHttpLine.left(16) == "Content-Length: ") {
            long long int i = 0;
            if( sscanf(sHttpLine.c_str(), "Content-Length: %lld", &i) == 1 ) {
                uiFileSize = static_cast<uint64_t>(i);
            }
        }
    }

    // Did not get any header about the data length.
    if(uiFileSize == 0)
        return -3;

    // Get the memory to write the data in.
    out_data.resize(uiFileSize);

    // Get the data.
    if(tradConn.get_lowlevel(out_data.getData(), static_cast<uint32_t>(uiFileSize), in_ulMaxWaitMillisec) != ERR_OK) {
        return -4;
    }

    return ERR_OK;
}

/// Gets a file via HTTP.
/** This sends an HTTP server the request to get a file and then gets that file
 *  from the server.
 *
 * \param in_sServer The server address to connect to, ex: arkana-fts.org
 * \param in_sPath The path to the file on the server, ex: /path/to/file.ex
 * \param out_uiFileSize Will be set to the size of the data that will be returned.
 *
 * \return If successfull:  A new DataContainer containing the file.
 * \return If failed:       NULL
 *
 * \note The DataContainer object you get should be deleted by you.
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved. Anyway, on most PC's
 *       an exactitude of more the 10ms is nearly never possible.
 * \note Although the \a out_uiFileSize is a 64-bit unsigned integer, the
 *       support for large files (>4GB) is not implemented yet.
 *
 * \author Pompei2
 */
FTS::RawDataContainer *FTS::getHTTPFile(const String &in_sServer, const String &in_sPath, uint64_t in_ulMaxWaitMillisec)
{
    // Get the memory to write the data in.
    FTS::RawDataContainer *pdc = new FTS::RawDataContainer(0);

    if(ERR_OK != FTS::getHTTPFile(*pdc, in_sServer, in_sPath, in_ulMaxWaitMillisec))
        SAFE_DELETE(pdc);

    return pdc;
}

/// Downloads a file via HTTP and saves it locally.
/** This sends an HTTP server the request to get a file and then gets that file
 *  from the server. Then it stores the file locally.
 *
 * \param in_sServer The server address to connect to, ex: arkana-fts.org
 * \param in_sPath The path to the file on the server, ex: /path/to/file.ex
 * \param in_sLocal The path to the file to create/overwrite to save the data to
 *                  . This path is on the local machine.
 * \param in_ulMaxWaitMillisec The amount of milliseconds (1/1000 seconds) to
 *                             wait for a message to come.
 *
 * \return If successfull:  ERR_OK
 * \return If failed:       An error code < 0
 *
 * \note If the file pointed to by \a in_sLocal already exists, it will be overwritten.
 * \note On linux, only an exactitude of 1-10 millisecond may be achieved. Anyway, on most PC's
 *       an exactitude of more the 10ms is nearly never possible.
 *
 * \author Pompei2
 */
int FTS::downloadHTTPFile(const String &in_sServer, const String &in_sPath, const String &in_sLocal, uint64_t in_ulMaxWaitMillisec)
{
    FTS::DataContainer *pData = FTS::getHTTPFile(in_sServer, in_sPath, in_ulMaxWaitMillisec);
    if(pData == NULL)
        return -1;

    FILE *pFile = fopen(in_sLocal.c_str(), "w+b");
    if(!pFile) {
        FTS18N("File_FopenW", MsgType::Error, in_sLocal, strerror(errno));
        SAFE_DELETE(pData);
        return -2;
    }

    bool bSuccess = fwrite(pData->getData(), static_cast<size_t>(pData->getSize()), 1, pFile) == 1;
    if(!bSuccess)
        FTS18N("File_Write", MsgType::Error, in_sLocal, strerror(errno));

    SAFE_DELETE(pData);
    SAFE_FCLOSE(pFile);

    return bSuccess ? ERR_OK : -3;
}

#if 0
/// Constructs an OnDemand connection object, does not do any connection!
/** This just constructs the connection object without doing any connection
 *  yet! The connection is only done when sending a packet. See our dokuwiki
 *  for more informations.
 *
 * \author Pompei2
 */
CFTSOnDemandHTTPConnection::CFTSOnDemandHTTPConnection(const String &in_sName, const String &in_sPath, uint16_t in_usPort)
        : m_bConnected(false),
          m_ulLastcall(0),
          m_sLastCounterpartIP(String::EMPTY),
          m_sServer(in_sName),
          m_usPort(in_usPort)
{
    // We connect using the traditional connection to check if the counterpart exists.
    TraditionalConnection tradConn(m_sServer, m_usPort, 0);
    m_sLastCounterpartIP = tradConn.getCounterpartIP();
    m_bConnected = tradConn.isConnected();

    m_sHTTPHeader = "POST http://" + in_sName + in_sPath + " HTTP/0.9\r\nContent-length: %d\r\n\r\n";
}

/// Check if the counterpart still exists.
/** This tries to build up a connection with the counterpart to check
 *  if it still exists and accepts connections. After that the connection
 *  is closed again.
 *
 * \author Pompei2
 */
bool CFTSOnDemandHTTPConnection::isConnected()
{
    // We connect using the traditional connection to check if the counterpart exists.
    TraditionalConnection tradConn(m_sServer, m_usPort, 0);
    m_sLastCounterpartIP = tradConn.getCounterpartIP();
    return (m_bConnected = tradConn.isConnected());
}

/// Forget about the connection info => make it impossible to connect again.
/** This does not literally disconnect us, as we are not connected. It just
 *  removes all connection information so we will never be able to connect again.
 *
 * \author Pompei2
 */
void CFTSOnDemandHTTPConnection::disconnect()
{
    m_sServer = String::EMPTY;
    m_usPort = 0;
    m_bConnected = false;
    m_sLastCounterpartIP = String::EMPTY;
}

/// TODO: make the send->recv oursleve and handle the queue (also in the disconnect method)
/// Only if needed.
int CFTSOnDemandHTTPConnection::mreq(Packet *out_pPacket, uint64_t in_ulMaxWaitMillisec)
{
    // Connect using a temporary traditional connection.
    uint64_t tBegin = dGetTicks();
    TraditionalConnection tradConn(m_sServer, m_usPort, in_ulMaxWaitMillisec);
    uint64_t tLost = dGetTicks() - tBegin;

    // Update our knowledge.
    m_bConnected = tradConn.isConnected();
    if(!m_bConnected)
        return -1;

    m_sLastCounterpartIP = tradConn.getCounterpartIP();

    // Make the request
//     return tradConn.mreq(in_pPacket, in_ulMaxWaitMillisec - tLost);

    // Check the request type.
    master_request_t req = out_pPacket->getType();
    if(req == DSRV_MSG_NULL || req == DSRV_MSG_NONE || req > DSRV_MSG_MAX ) {
        return FTSC_ERR_WRONG_REQ;
    }

    // Now we build up a packet containing the HTTP header.
    String sHTTPHeader = m_sHTTPHeader.fmt(out_pPacket->getTotalLen()));
    uint32_t sendLen = sHTTPHeader.len() + out_pPacket->getTotalLen();
    int8_t *pSendBuff = (int8_t *)Alloc(sendLen);
    if(!pSendBuff)
        return -10;

    memcpy(pSendBuff, sHTTPHeader.c_str(), sHTTPHeader.len());
    memcpy(&pSendBuff[sHTTPHeader.len()], out_pPacket->m_pData, out_pPacket->getTotalLen());

    // Send that packet to the server.
    if(tradConn.send(pSendBuff, sendLen) != ERR_OK) {
        FTS18N( "Net_TCPIP_send", MsgType::Error, strerror( errno ), errno );
        return FTSC_ERR_SEND;
    }

    // Get an answer from the server.
    Packet *p = tradConn.waitForThenGetPacketWithReq(req, in_ulMaxWaitMillisec - tLost);
    if(p == NULL) {
        return FTSC_ERR_RECEIVE;
    }

    // No need to remove the HTTP header as it has already been eaten up
    // by the get method.

    // Now check if the request Id corresponds.
    if(p->getType() != req) {
        FTS18N("Net_packet", MsgType::Error, "got id "+_S(((fts_packet_hdr_t *) p->m_pData)->req_id)+", wanted "+_S(req));
        SAFE_DELETE(p);
        return FTSC_ERR_WRONG_RSP;
    }

    m_mutex.lock();
    m_ulLastcall = dGetTicks();
    m_mutex.unlock();

    // Transfer the receive buffer to the in packet

    // Free the send data buffer
    SAFE_FREE(out_pPacket->m_pData);

    // Put the receive buffer pointer in to the in packet
    out_pPacket->m_pData = p->m_pData;

    // In order that the delete works, set data pointer to NULL
    p->m_pData = NULL;

    // delete the interim packet
    SAFE_DELETE(p);
    out_pPacket->rewind();
    return ERR_OK;
}
#endif
