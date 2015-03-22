/**
 * \file connection.h
 * \author Pompei2
 * \date 11 May 2007
 * \brief This file describes the class that represents a network
 *        connection that can send packets.
 **/

#ifndef FTS_CONNECTION_H
#define FTS_CONNECTION_H

#include "main.h"

#if WINDOOF
#  include <Winsock2.h>
#else
#  include <arpa/inet.h>
#endif

#include <list>

// windows compatibility.
#if !WINDOOF
using SOCKET = int;
#endif
using SOCKADDR_IN = sockaddr_in;

#ifndef SOCKET_ERROR
#  define SOCKET_ERROR -1
#endif

#include "packet.h"
#include "utilities/threading.h"

#define FTSC_TIME_OUT      1000    ///< time out value in milliseconds
#define FTSC_MAX_QUEUE_LEN 32      ///< The longest queue we shall have. If queue gets longer, drop it.

#define FTSC_ERR_NOT_CONNECTED -1; ///< The connection is not connected.
#define FTSC_ERR_SEND          -2; ///< Socket error on send()
#define FTSC_ERR_SELECT        -3; ///< Socket error on select
#define FTSC_ERR_TIMEOUT       -4; ///< select() w/ has timed out
#define FTSC_ERR_RECEIVE       -5; ///< Socket error on recv
#define FTSC_ERR_WRONG_RSP     -6; ///< Response doesn't match the request
#define FTSC_ERR_WRONG_REQ     -7; ///< Invalid request

namespace FTS {
    class RawDataContainer;

/// The FTS connection class
/** This class represents an abstract connection.
 *  It may be implemented as a connection over tcp/ip, over serial,
 *  over pipes or whatever you want.
 **/
class Connection {
public:
    virtual ~Connection() {};

    typedef enum {
        D_CONNECTION_TRADITIONAL  = 0x0,
        D_CONNECTION_ONDEMAND_CLI = 0x1,
        D_CONNECTION_ONDEMAND_SRV = 0x2
    } eConnectionType;

    virtual eConnectionType getType() const = 0;

    virtual bool isConnected() = 0;
    virtual void disconnect() = 0;

    virtual String getCounterpartIP() const = 0;

    virtual Packet *waitForThenGetPacket(bool in_bUseQueue = true, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT) = 0;
    virtual Packet *getPacketIfPresent(bool in_bUseQueue = true) = 0;
    virtual Packet *getReceivedPacketIfAny();
    virtual int send(Packet *in_pPacket) = 0;
    virtual int mreq(Packet *in_pPacket, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT) = 0;

    virtual void waitAntiFlood() = 0;

protected:
    std::list<Packet *>m_lpPacketQueue; ///< A queue of packets that have been received but not consumed. Most recent are at the back.
    Mutex m_mutex;                      ///< The mutex to protect myself.

    Connection() {};
    virtual Packet *getFirstPacketFromQueue(master_request_t in_req = DSRV_MSG_NONE);
    virtual void queuePacket(Packet *in_pPacket);
};

int getHTTPFile(FTS::RawDataContainer &out_data, const String &in_sServer, const String &in_sPath, uint64_t in_ulMaxWaitMillisec);
FTS::RawDataContainer *getHTTPFile(const String &in_sServer, const String &in_sPath, uint64_t in_ulMaxWaitMillisec);
int downloadHTTPFile(const String &in_sServer, const String &in_sPath, const String &in_sLocal, uint64_t in_ulMaxWaitMillisec);

/// A Traditional TCP/IP implementation of the connection class.
/**
 * This class is an implementation of the Connection class and implements
 * a network connection over TCP/IP using sockets.\n
 * \n
 * The connection is done one time
 * and then never done again. At the end, the connection is closed.\n
 * \n
 * Read more details
 * about it in our DokuWiki design documents->networking section, direct link:
 * http://wiki.arkana-fts.org/doku.php?id=design_documents:networking:src:connection_types
 **/
class TraditionalConnection : public Connection {
    friend class OnDemandHTTPConnection;
    friend int FTS::getHTTPFile(FTS::RawDataContainer &out_data, const String &in_sServer, const String &in_sPath, uint64_t in_ulMaxWaitMillisec);

public:
    TraditionalConnection(const String &in_sName, uint16_t in_usPort, uint64_t in_ulTimeoutInMillisec);
    TraditionalConnection(SOCKET in_sock, SOCKADDR_IN in_sa);
    virtual ~TraditionalConnection();

    eConnectionType getType() const {return D_CONNECTION_TRADITIONAL;};

    virtual bool isConnected();
    virtual void disconnect();

    virtual String getCounterpartIP() const;

    virtual Packet *waitForThenGetPacket(bool in_bUseQueue = true, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT);
    virtual Packet *getPacketIfPresent(bool in_bUseQueue = true);

    virtual Packet *waitForThenGetPacketWithReq(master_request_t in_req, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT);
    virtual Packet *getPacketWithReqIfPresent(master_request_t in_req);

    virtual int send(Packet *in_pPacket);
    virtual int mreq(Packet *in_pPacket, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT);

    virtual void waitAntiFlood();
    static int setSocketBlocking(SOCKET out_socket, bool in_bBlocking);

protected:
    bool m_bConnected;           ///< Wether the connection is up or not.
    SOCKET m_sock;               ///< The connection socket.
    SOCKADDR_IN m_saCounterpart; ///< This is the address of our counterpart.
    unsigned long m_ulLastcall;  ///< The last time a networking function has been called.

    int connectByName( String in_sName, uint16_t in_usPort, uint64_t in_ulTimeoutInMillisec);
    virtual Packet *getPacket(bool in_bUseQueue, uint64_t in_ulMaxWaitMillisec);
    virtual int get_lowlevel(void *out_pBuf, uint32_t in_uiLen, uint64_t in_ulMaxWaitMillisec);
    virtual String getLine(const String in_sLineEnding, uint64_t in_ulMaxWaitMillisec);

    virtual int send(const void *in_pData, uint32_t in_uiLen);
};

#if 0
/// A new style on-demand TCP/IP implementation of the connection class, client side.
/**
 * This class is an implementation of the Connection class and implements
 * a network connection over TCP/IP using sockets. It implements the client side.\n
 * \n
 * The connection is not kept up like in the traditional fashion. It is rather
 * created right before sending a packet, then it waits for an answer and then
 * closes the connection again.\n
 * The user of this class shall not notice this fact if he only uses the \a mreq method!\n
 * \n
 * Read more details
 * about it in our DokuWiki design documents->networking section, direct link:
 * http://wiki.arkana-fts.org/doku.php?id=design_documents:networking:src:connection_types
 **/
class OnDemandHTTPConnection : public Connection {
public:
    OnDemandHTTPConnection(const String &in_sName, const String &in_sPath, uint16_t in_iPort);
    virtual ~OnDemandHTTPConnection() {};

    eConnectionType getType() const {return D_CONNECTION_ONDEMAND_CLI;};

    virtual bool isConnected();
    virtual void disconnect();

    virtual String getCounterpartIP() const {return m_sLastCounterpartIP;};

//     virtual int send(Packet *in_pPacket) {return ERR_OK;};
    virtual int mreq(Packet *out_pPacket, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT);

//     virtual Packet *waitForThenGetPacket(bool in_bUseQueue = true, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT) {return NULL;};
//     virtual Packet *getPacketIfPresent(bool in_bUseQueue = true) {return NULL;};
//
//     virtual Packet *waitForThenGetPacketWithReq(master_request_t in_req, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT) {return NULL;};
//     virtual Packet *getPacketWithReqIfPresent(master_request_t in_req) {return NULL;};

    virtual void waitAntiFlood() {};

protected:
    bool m_bConnected;            ///< Wether the connection is up or not.
    unsigned long m_ulLastcall;   ///< The last time a networking function has been called.
    String m_sLastCounterpartIP; ///< The IP the counterpart had last time we connected.

    String m_sServer;    ///< The name of the server we always connect to.
    uint16_t m_usPort;    ///< The port number we connect to at the server.

    String m_sHTTPHeader; ///< The HTTP header that needs to be prepended.

//     virtual int send(const void *in_pData, uint32_t in_uiLen) {return ERR_OK;};
};
#endif

#if 0
/** This is a refinement of the CFTSOnDemandConnection class, it is for sending packets
 *  in the on-demand style over network, but passing trough a HTTP server.
 *
 *  This is done by sending the packet as a HTTP-request to a certain cgi-script at the server
 *  which one then sends the content of the cleaned out packet (without the HTTP-request)
 *  to the real server.
 *
 *  The real server then works with this packet, builds up an answer and sends it back to the
 *  CGI-script which will send it back to the client by HTTP.
 *  But to bypass firewalls and similar, the packet being sent back also includes some special
 *  prefix making everyone think it's a HTML page. The script handles this.
 *
 *  This class implements the functions to work at the client-side (write the message to the
 *  cgi-script at the server, filter out the HTML-prefix from the answer).
 **/
class HTTPConnectionClient : public Connection {
public:
    HTTPConnectionClient(String in_sName, int in_iPort, time_t in_nTimeout);
    virtual ~HTTPConnectionClient();

    eConnectionType getType() const {return D_CONNECTION_ONDEMAND;};

    virtual bool isConnected() const;
    virtual void disconnect();

    virtual String getCounterpartIP() const;

    virtual int send(Packet *in_pPacket);
    virtual int mreq(Packet *in_pPacket);

    virtual Packet *waitForThenGetPacket(bool in_bUseQueue = true, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT);
    virtual Packet *getPacketIfPresent(bool in_bUseQueue = true);

    virtual Packet *waitForThenGetPacketWithReq(master_request_t in_req, uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT);
    virtual Packet *getPacketWithReqIfPresent(master_request_t in_req);

    virtual void waitAntiFlood();

protected:
    bool m_bConnected;           ///< Wether the connection is up or not.
    unsigned long m_ulLastcall;  ///< The last time a networking function has been called.
};
#endif

}

#endif /* FTS_CONNECTION_H */

 /* EOF */
