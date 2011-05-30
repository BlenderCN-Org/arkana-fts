/**
 * \file connection.h
 * \author Pompei2
 * \date 11 May 2007
 * \brief This file describes the class that represents a network
 *        connection that can send packets.
 **/

#ifndef FTS_CONNECTION_H
#define FTS_CONNECTION_H

#ifdef D_COMPILES_SERVER
#  include "server.h"
#else
#  include "main/main.h"
#endif

// windows compatibility.
#define SOCKET int
#define SOCKADDR_IN sockaddr_in

#ifndef SOCKET_ERROR
#  define SOCKET_ERROR -1
#endif

#include "packet.h"
#include "utilities/threading.h"
#include <list>

#define FTSC_TIME_OUT      1000    ///< time out value in milli seconds
#define FTSC_MAX_QUEUE_LEN 32      ///< The longest queue we shall have. If queue gets longer, drop it.

#define FTSC_ERR_NOT_CONNECTED -1; ///< The connection is not connected.
#define FTSC_ERR_SEND          -2; ///< Socket error on send()
#define FTSC_ERR_SELECT        -3; ///< Socket error on select
#define FTSC_ERR_TIMEOUT       -4; ///< select() w/ has timed out
#define FTSC_ERR_RECEIVE       -5; ///< Socket error on recv
#define FTSC_ERR_WRONG_RSP     -6; ///< Response doesn't match the request
#define FTSC_ERR_WRONG_REQ     -7; ///< Invalid request

#ifndef SOCKET_ERROR
#  define SOCKET_ERROR -1
#endif

#if 0
/// The FTS packet class
/** This class represents a packet that can be sent over a connection.
 *  This connection is described by another class.
 **/
class CFTSConnection {
private:
    bool m_bConnected;           ///< Wether the connection is up or not.
    SOCKET m_sock;               ///< The connection socket.
    SOCKADDR_IN m_saCounterpart; ///< This is the address of our counterpart.
    unsigned long m_ulLastcall;  ///< The last time a networking function has been called.
    std::list<CFTSPacket *>m_lpPacketQueue; ///< A queue of packets that have been received but not consumed. Most recent are at the back.
    CFTSMutex m_mutex;           ///< The mutex to protect myself.

public:
    friend class CFTSPacket;

    CFTSConnection(SOCKET in_sock, SOCKADDR_IN in_sa);
    CFTSConnection(String in_sName, int in_iPort, time_t in_nTimeout);
    virtual ~CFTSConnection();

    bool isConnected(void) const;
    const SOCKADDR_IN *getCounterpart(void) const;

    void disconnect(void);

    int send(CFTSPacket * in_pPacket);
    CFTSPacket *recv(bool in_bBlock = true, bool in_bUseQueue = true);
    CFTSPacket *recv(master_request_t in_cID, bool in_bBlock = true);
    int mreq(CFTSPacket * in_pPacket);
    void waitAntiFlood(void);
    static int setSocketBlocking(SOCKET out_socket, bool in_bBlocking);
    int setBlocking(bool in_bBlocking);

protected:
    int connectByName( String in_sName, int in_iPort, time_t in_nTimeout );
};
#else
/// The FTS connection class
/** This class represents an abstract connection.
 *  It may be implemented as a connection over tcp/ip, over serial,
 *  over pipes or whatever you want.
 **/
class CFTSConnection {
public:
    friend class CFTSPacket;

    virtual ~CFTSConnection() {};

    enum {
        D_CONNECTION_TRADITIONAL = 0x0,
        D_CONNECTION_ONDEMAND    = 0x1
    } eConnectionType;

    virtual eConnectionType getType() const = 0;

    virtual CFTSPacket *buildPacket() = 0;
    virtual CFTSPacket *buildPacket(char in_cType) = 0;

    virtual bool isConnected(void) const = 0;
    virtual void disconnect(void) = 0;

    virtual String getCounterpartIP(void) const = 0;

    virtual int send(CFTSPacket *in_pPacket) = 0;
    virtual CFTSPacket *recv(bool in_bBlock = true, bool in_bUseQueue = true) = 0;
    virtual CFTSPacket *recv(master_request_t in_cID, bool in_bBlock = true) = 0;
    virtual int mreq(CFTSPacket *in_pPacket) = 0;
    virtual void waitAntiFlood(void) = 0;
    virtual int setBlocking(bool in_bBlocking) = 0;

protected:
    CFTSConnection() {};
};

/// A TCP/IP implementation of the connection class.
/**
 * This class is an implementation of the CFTSConnection class and implements
 * a network connection over TCP/IP using sockets.
 **/
class CFTSTraditionalConnection : public CFTSConnection {
public:
    friend class CFTSPacket;

    CFTSTraditionalConnection(SOCKET in_sock, SOCKADDR_IN in_sa);
    CFTSTraditionalConnection(String in_sName, int in_iPort, time_t in_nTimeout);
    virtual ~CFTSTraditionalConnection();

    eConnectionType getType() const {return D_CONNECTION_TRADITIONAL;};

    inline CFTSPacket *buildPacket() { return new CFTSPacket(); };
    inline CFTSPacket *buildPacket(char in_cType) { return new CFTSPacket(in_cType); };

    bool isConnected(void) const;
    void disconnect(void);

    String getCounterpartIP(void) const;

    int send(CFTSPacket *in_pPacket);
    CFTSPacket *recv(bool in_bBlock = true, bool in_bUseQueue = true);
    CFTSPacket *recv(master_request_t in_cID, bool in_bBlock = true);
    int mreq(CFTSPacket *in_pPacket);
    void waitAntiFlood(void);
    int setBlocking(bool in_bBlocking);

    static int setSocketBlocking(SOCKET out_socket, bool in_bBlocking);

protected:
    bool m_bConnected;           ///< Wether the connection is up or not.
    SOCKET m_sock;               ///< The connection socket.
    SOCKADDR_IN m_saCounterpart; ///< This is the address of our counterpart.
    unsigned long m_ulLastcall;  ///< The last time a networking function has been called.
    std::list<CFTSPacket *>m_lpPacketQueue; ///< A queue of packets that have been received but not consumed. Most recent are at the back.
    CFTSMutex m_mutex;           ///< The mutex to protect myself.

    int connectByName( String in_sName, int in_iPort, time_t in_nTimeout );
};


#endif

#endif /* FTS_CONNECTION_H */

 /* EOF */
