#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "net/packet.h"
#include "net/connection.h"

struct timeval g_tvStart;

FILE *g_fErrLog = NULL;

/// Get a random number between two numbers.
/** Returns a random number between \a in_iMin and \a in_iMax.
 *
 * \param in_iMin The minimum numeric value of the number.
 * \param in_iMax The maximum numeric value of the number.
 *
 * \return If successfull: An valid number.
 * \return If failed:      0
 *
 * \Note in_iMin must be bigger than 0 and less than in_iMax. \n
 *       in_iMax must be bigger than 0 and bigger than in_iMin. \n
 *       So the result is: 0 <= min <= random( min, max ) <= max
 *
 * \author Pompei2
 */
int random(const int in_iMin, const int in_iMax)
{
    if(in_iMin == in_iMax)
        return in_iMin;

    /* Prevent zero divisions and other wrong parameters */
    if(in_iMax <= in_iMin || in_iMax <= 0 || in_iMin < 0) {
        return 0;
    }

    return (int)(((float)rand() / (float)RAND_MAX) *
                 (float)(in_iMax - in_iMin + 1) + (float)(in_iMin));
}

struct SRawPacket {
    fts_packet_hdr_t header;
    void *data;
};

SRawPacket getPacket() throw(master_request_t)
{
    // Get the size of data that was sent to us.
    const char *pszDataLen = getenv("CONTENT_LENGTH");
    if(pszDataLen == NULL) {
        fprintf(g_fErrLog, "Error getting env variable CONTENT_LENGTH: %s (%d)\n", strerror(errno), errno);
        throw DSRV_MSG_NONE;
    }
    uint32_t uiDataLen = atoi(pszDataLen);

    // If we didn't even get the header, we quit instantly.
    if(uiDataLen < D_PACKET_HDR_LEN) {
        fprintf(g_fErrLog, "Data len (%d) < D_PACKET_HDR_LEN (%d)\n", uiDataLen, D_PACKET_HDR_LEN);
        throw DSRV_MSG_NONE;
    }

    // Get the header.
    SRawPacket packet;
    if(fread(&packet.header, sizeof(fts_packet_hdr_t), 1, stdin) != 1) {
        fprintf(g_fErrLog, "error freading stdin: %s (%d)\n", strerror(errno), errno);
        throw DSRV_MSG_NONE;
    }

    // Check if the header is valid.
    if(!isPacketHeaderValid(&packet.header)) {
        fprintf(g_fErrLog, "Packet header is invalid (%c%c%c%c%d)\n", packet.header.ident[0],
                packet.header.ident[1], packet.header.ident[2], packet.header.ident[3], packet.header.req_id);
        throw DSRV_MSG_NONE;
    }

    // Check if the data length described in the packet corresponds to the
    // length of the data we have gotten.
    if(packet.header.data_len + D_PACKET_HDR_LEN != uiDataLen) {
        fprintf(g_fErrLog, "packet.header.data_len (%d) + D_PACKET_HDR_LEN (%d) != uiDataLen (%d)\n",
                packet.header.data_len, D_PACKET_HDR_LEN, uiDataLen);
        throw packet.header.req_id;
    }

    // Get the rest of the packet.
    packet.data = Alloc(packet.header.data_len);
    if(packet.data == NULL) {
        fprintf(g_fErrLog, "Alloc failed: %s (%d)\n", strerror(errno), errno);
        throw packet.header.req_id;
    }

    if(fread(packet.data, packet.header.data_len, 1, stdin) != 1) {
        fprintf(g_fErrLog, "error freading stdin: %s (%d)\n", strerror(errno), errno);
        throw packet.header.req_id;
    }

    return packet;
}

CFTSPacket *createPacketWithIP(SRawPacket &in_pack) throw(master_request_t)
{
    CFTSPacket *pack = new CFTSPacket(in_pack.header.req_id);

    try {
        const char *pszIP = getenv("REMOTE_ADDR");
        if(pszIP == NULL) {
            fprintf(g_fErrLog, "error getting env variable REMOTE_ADDR: %s (%d)\n", strerror(errno), errno);
            throw in_pack.header.req_id;
        }

        pack->append(String(pszIP))
            ->append(in_pack.data, in_pack.header.data_len);
    } catch(...) {
        SAFE_DELETE(pack);
        throw(in_pack.header.req_id);
    }

    return pack;
}

CFTSConnection *verySimpleConnect(const char *addr, uint16_t port)
{
    struct hostent *server_info = gethostbyname(addr);
    if(server_info == NULL) {
        fprintf(g_fErrLog, "error getting host by name: %s (%d)\n", strerror(errno), errno);
        return NULL;
    }

    int my_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(my_socket == -1) {
        fprintf(g_fErrLog, "error creating socket: %s (%d)\n", strerror(errno), errno);
        return NULL;
    }

    struct sockaddr_in server_socket;
    server_socket.sin_family = server_info->h_addrtype;
    memcpy((char *)&server_socket.sin_addr.s_addr, server_info->h_addr_list[0], server_info->h_length);
    server_socket.sin_port = htons(port);

    if(connect(my_socket, (struct sockaddr*) &server_socket, sizeof(server_socket)) < 0) {
        fprintf(g_fErrLog, "error connecting: %s (%d)\n", strerror(errno), errno);
        close(my_socket);
        return NULL;
    }

    return new CFTSTraditionalConnection(my_socket, server_socket);
}

int main()
{
    gettimeofday(&g_tvStart, NULL);

    g_fErrLog = fopen("errlog_fts_cgi", "w+");

    // TEST MODE:
    CFTSConnection *pConn = verySimpleConnect("127.0.0.1", 44917);
    if(pConn)
        fprintf(g_fErrLog, "GOOD\n", strerror(errno), errno);

    fprintf(stdout, "Content-type: text/html\r\n\r\nBla");
    return 0;
/*    const char *pszDataLen = getenv("CONTENT_LENGTH");
    if(pszDataLen == NULL) {
        fprintf(g_fErrLog, "Error getting env variable CONTENT_LENGTH: %s (%d)\n", strerror(errno), errno);
        fprintf(stdout, "Content-type: text/html\r\n\r\nBla");
        return 0;
    }
    uint32_t uiDataLen = atoi(pszDataLen);
    fprintf(g_fErrLog, "CONTENT_LENGTH: \"%s\" = %d\n", pszDataLen, uiDataLen);

    char *pBuf = (char *)calloc(uiDataLen+1, 1);
    fread(pBuf, sizeof(char), uiDataLen, stdin);
    pBuf[uiDataLen] = '\0';
    fwrite(pBuf, sizeof(char), uiDataLen, g_fErrLog);
    fflush(g_fErrLog);
    fclose(g_fErrLog);

    fprintf(stdout, "Content-type: text/html\r\n\r\nBla");
    return 0;*/
    // END TEST MODE.

    CFTSPacket *pAnswer = NULL;
    try {
        SRawPacket packet = getPacket();
        pAnswer = createPacketWithIP(packet);

        // Now we can delete the original data.
        SAFE_FREE(packet.data);

        // Build up a connection to the master server, that runs locally.
        // Choose a random port in the correct range to connect to.
//        uint16_t usPort = random(D_SERVER_PORT_FIRST, D_SERVER_PORT_LAST);
//         CFTSConnection *pConn = new CFTSTraditionalConnection("localhost", 44917, 10 * 1000 * 1000); // Wait 100 ms
        CFTSConnection *pConn = verySimpleConnect("127.0.0.1", 44917);
        if(!pConn)
            throw(packet.header.req_id);

        // Send the packet to the server and get an answer.
        if(ERR_OK != pConn->mreq(pAnswer))
            throw(pAnswer->getType());
        delete pConn;
    } catch(master_request_t req) {
        // On error, we send back a packet containing an error.
        SAFE_DELETE(pAnswer);
        pAnswer = new CFTSPacket(req);
        pAnswer->append((int8_t)-127);
    }

    // Print the preliminary needed HTTP header ...
    fprintf(stdout, "Content-type: text/html\r\n\r\n");

    // And then append the packet to it.
    pAnswer->printToFile(stdout);

    fflush(g_fErrLog);
    fclose(g_fErrLog);
    return 0;
}




// Misc function aliases needed.

// To be able to compile connection.cpp and packet.cpp
void FTS18N(const char *fmt, int in, ...)
{
    va_list ap;
    va_start(ap, in);

    if(in == FTS_DEBUG) {
        int iTmp = va_arg(ap, int);
        iTmp = 0;
    }

    if(in < 0) {
//         fprintf(g_fErrLog, "%s", String::svformat(fmt, ap).c_str());
    } else {
        const char *fmt2 = fmt;
        if(!strcmp(fmt, "Net_TCPIP_mksock")) {
            fmt2 = "Net: could not create a socket,\n\tbecause '%s' (%d)";
        } else if(!strcmp(fmt, "Net_TCPIP_hostname")) {
            fmt2 = "Net: could not resolve the hostname '%s',\n\tbecause '%s' (%d)";
        } else if(!strcmp(fmt, "Net_TCPIP_connect")) {
            fmt2 = "Net: could not connect to '%s' at port %d,\n\tbecause '%s' (%d)";
        } else if(!strcmp(fmt, "Net_TCPIP_connect_to")) {
            fmt2 = "Net: connection to '%s' at port %d timed out,\n\tbecause '%s' (%d)";
        } else if(!strcmp(fmt, "Net_TCPIP_send")) {
            fmt2 = "Net: could not send data:\n\t'%s' (%d)";
        } else if(!strcmp(fmt, "Net_TCPIP_recv")) {
            fmt2 = "Net: could not recieve data:\n\tconnection lost";
        } else if(!strcmp(fmt, "Net_TCPIP_select")) {
            fmt2 = "Net: error during select:\n\t%s (%d)";
        } else if(!strcmp(fmt, "Net_TCPIP_fcntl_get")) {
            fmt2 = "Net: Error getting fcntl:\n\t%s (%d)";
        } else if(!strcmp(fmt, "Net_TCPIP_fcntl_set")) {
            fmt2 = "Net: Error setting fcntl:\n\t%s (%d)";
        } else if(!strcmp(fmt, "Net_packet_len")) {
            fmt2 = "Net: the length of the packet is incorrect: %d";
        } else if(!strcmp(fmt, "Net_packet")) {
            fmt2 = "Net: an invalid packet has been received:\n\t%s";
        }
        fprintf(g_fErrLog, "%s", String::svformat(fmt2, ap).c_str());
    }
    va_end(ap);
}

uint32_t SDL_GetTicks(void)
{
    uint32_t ticks;
    struct timeval now;

    gettimeofday(&now, NULL);
    ticks = (now.tv_sec-g_tvStart.tv_sec)*1000 + (now.tv_usec-g_tvStart.tv_usec)/1000;

    return ticks;
}
