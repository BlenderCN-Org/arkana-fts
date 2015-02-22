// TestServer.cpp : Defines the entry point for the console application.
//

#include <thread>
#include <string>
#include <vector>

#include "stdafx.h"
#include "net/packet.h"
#include "net/connection.h"
#include "utilities/threading.h"
#include "utilities/utilities.h"
#include "toolcompat.h"
#include "logging/Chronometer.h"

#if WINDOOF
#pragma comment(lib, "Ws2_32.lib")
#endif

using namespace FTS;
bool g_exit = false;

void workerThread( void* port )
{
    int nMsgCreated = 0;
    int nConClosed = 0;
    int nChatMsgs = 0;
    int nQueueLeft = 0;
    std::vector<int> nMreqSendErr;
    std::vector<int> nMsgGetErr ;
    std::vector<double> times;

    while( !g_exit )
    {
        auto connection = new FTS::TraditionalConnection( "192.168.1.10", (int)port, 1000 );
        if ( !connection->isConnected() )
        {
            FTSMSGDBG( "[{1}] Connection to localhost failed\nQuit!\n", 1, String::nr( ( int ) port ) );
        }

        // Encrypt the password.
        //char buffMD5[32];

        //md5Encode( in_sPassword.c_str(), in_sPassword.len(), buffMD5 );

        // Cut all the data to the right size and store the MD5 hash.
        //String sNick( in_sNickname );
        //m_sMD5 = String( buffMD5, 0, 32 );
        String user = "Test" + String::nr( ( int ) port );
        String pwd = "Test" + String::nr( ( int ) port );
        nMsgCreated++;
        Packet *p = new Packet( DSRV_MSG_LOGIN );
        p->append( user );
        p->append( pwd );

        // And login.
        auto rc = connection->mreq( p );
        if ( rc != ERR_OK )
        {
            nMreqSendErr.push_back(rc);
        } else {
            rc = p->get();
            if( rc != ERR_OK ) {
                nMsgGetErr.push_back(rc);
            }
        }
        delete p;

        // Join a chat room. 
        p = new Packet( DSRV_MSG_CHAT_JOIN );
        p->append( pwd );
        p->append( "UselessChan" );

        rc = connection->mreq( p );
        if( rc != ERR_OK ) {
            nMreqSendErr.push_back( rc );
        } else {
            rc = p->get();
            if( rc != ERR_OK ) {
                nMsgGetErr.push_back( rc );
            }
        }
        delete p;

        while( !g_exit ) {
            Chronometer timer;
            nChatMsgs++;

            // Get list of chat #players and the user names. 
            p = new Packet( DSRV_MSG_CHAT_LIST );
            p->append( pwd );

            rc = connection->mreq( p );
            if( rc != ERR_OK ) {
                nMreqSendErr.push_back( rc );
            } else {
                rc = p->get();
                if( (rc != ERR_OK) && (rc != -3) /*no list available*/ ) {
                    nMsgGetErr.push_back( rc );
                }
            }
            delete p;
            times.push_back( timer.measure() );

            // Chat a message 
            p = new Packet( DSRV_MSG_CHAT_SENDMSG );
            p->append( pwd );
            p->append( 1 /*DSRV_CHAT_TYPE_NORMAL*/ );
            p->append( 0 /* flags */ );
            p->append( user + " now here!" );

            rc = connection->mreq( p );
            if( rc != ERR_OK ) {
                nMreqSendErr.push_back( rc );
            } else {
                rc = p->get();
                if( (rc != ERR_OK) ) {
                    nMsgGetErr.push_back( rc );
                }
            }
            delete p;
            times.push_back( timer.measure() );
        }
        
        // And logout.
        p = new Packet( DSRV_MSG_LOGOUT );
        p->append( user );

        rc = connection->mreq( p );
        if( rc != ERR_OK ) {
            nMreqSendErr.push_back( rc );
        } else {
            rc = p->get();
            if( rc != ERR_OK ) {
                nMsgGetErr.push_back( rc );
            }
        }

        while( true ) {
            p = connection->getPacketIfPresent();
            if( p != nullptr ) {
                nQueueLeft++;
                //FTSMSGDBG( "{1}:Packet 0x{2}",1, user, String::nr( p->getType(),2,'0', std::ios_base::hex ) );
                delete p;
            } else {
                break;
            }
        }


        delete p;
        nConClosed++;
        connection->disconnect();
    }

    String sGetErr;
    sGetErr = "{";
    for( auto no : nMsgGetErr ) {
        sGetErr += String::nr( no ) + ",";
    }
    sGetErr += "}";

    String sSendErr;
    sSendErr = "{";
    for( auto no : nMreqSendErr ) {
        sSendErr += String::nr( no ) + ",";
    }
    sSendErr += "}";

    double avg = 0;
    if( times.size() > 0 ) {
        for( auto t : times ) {
            avg += t;
        }
        avg = avg / times.size() * 1000.;
    }

    FTSMSGDBG( "[{1}] Queue {2} Chat {3} SendErr {4} {5} GetErr {6} {7} {8} ms", 1 
               ,String::nr( ( int ) port )
               ,String::nr( nQueueLeft )
               ,String::nr( nChatMsgs )
               ,String::nr( nMreqSendErr.size() )
               ,sSendErr
               ,String::nr(nMsgGetErr.size())
               ,sGetErr
               , String::nr(avg)
               );

}

int _tmain(int argc, _TCHAR* argv[])
{
#if WINDOOF
    //----------------------
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( iResult != NO_ERROR )
    {
        std::cout << "WSAStartup failed with error:" << iResult << "\n";
        return 1;
    }
#endif
    // Logging
    // =======
    new FTSTools::MinimalLogger();
#define NTHREADS    10
    std::thread worker[NTHREADS];
    Chronometer timer;
    for( int i = 0; i < NTHREADS; ++i )
    {
        worker[i] = std::thread( workerThread, ( void * ) (44917+i) );
    }
    
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
    auto timeConsumed = timer.measure();
    
    g_exit = true;
    for ( int i = 0; i < NTHREADS; ++i )
    {
        worker[i].join();
    }

    std::cout << "Time used " << timeConsumed << std::endl;
    std::string s;
    std::getline( std::cin, s );

    return 0;
}

