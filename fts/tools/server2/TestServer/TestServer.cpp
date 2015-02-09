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
    std::vector<int> nMreqSendErr ;
    std::vector<int> nMsgGetErr ;
    std::vector<double> times;

    while( !g_exit )
    {
        auto connection = new FTS::TraditionalConnection( "localhost", (int)port, 1000 );
        if ( !connection->isConnected() )
        {
            FTSMSGDBG( "[{1}] Connection to localhost failed\nQuit!\n", 1, String::nr( ( int ) port ) );
        }

        // Encrypt the password.
        char buffMD5[32];

        //md5Encode( in_sPassword.c_str(), in_sPassword.len(), buffMD5 );

        // Cut all the data to the right size and store the MD5 hash.
        //String sNick( in_sNickname );
        //m_sMD5 = String( buffMD5, 0, 32 );

        nMsgCreated++;
        Packet *p = new Packet( DSRV_MSG_LOGIN );
        p->append( "Test" + String::nr((int)port) );
        p->append( "Test" + String::nr( ( int ) port ) );

        // And login.
        Chronometer timer;
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

        p = new Packet( DSRV_MSG_LOGOUT );
        p->append( "Test" + String::nr( ( int ) port ) );

        // And logout.
        rc = connection->mreq( p );
        if( rc != ERR_OK ) {
            nMreqSendErr.push_back( rc );
        } else {
            rc = p->get();
            if( rc != ERR_OK ) {
                nMsgGetErr.push_back( rc );
            }
        }

        times.push_back(timer.measure());

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

    FTSMSGDBG( "[{1}] Create {2}/{3} SendErr {4} {5} GetErr {6} {7} {8}", 1 
               ,String::nr( ( int ) port )
               ,String::nr( nMsgCreated )
               ,String::nr( nConClosed )
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
    
    std::this_thread::sleep_for( std::chrono::milliseconds( 5000 ) );
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

