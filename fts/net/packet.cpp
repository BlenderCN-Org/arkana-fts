/**
 * \file packet.cpp
 * \author Pompei2
 * \date 11 May 2007
 * \brief This file implements the class that represents a packet
 *        that gets sent over a connection.
 **/

#include "net/packet.h"

#ifndef D_COMPILES_SERVER
#  include "utilities/utilities.h"
#endif

using namespace FTS;

/// Default constructor.
/** Creates the packet, fills it's identifier field and sets the type.
 *  Also places the cursor behind the header.
 *
 * \param in_cType The type of the message.
 *
 * \author Pompei2
 */
FTS::Packet::Packet(master_request_t in_cType)
{
    m_pData = (int8_t *)calloc(1,D_PACKET_HDR_LEN);
    memset(m_pData, 0, D_PACKET_HDR_LEN);
    fillPacketHeader((fts_packet_hdr_t *)m_pData, in_cType);

    this->setType(in_cType);
    this->rewind();
}

/// Default destructor
/** Destroys the packet object.
 *
 * \author Pompei2
 */
FTS::Packet::~Packet()
{
    SAFE_FREE(m_pData);
}

/// Checks wether the data is valid.
/** This checks wether the data is a valid FTS packet by looking at it's header.
 *
 * \return true if the packet is valid, false else.
 *
 * \author Pompei2
 */
bool FTS::Packet::isValid(void) const
{
    return isPacketHeaderValid((fts_packet_hdr_t *)m_pData);
}

/// Sets the type of the packet.
/** This sets the type of the packet. This HAS to be done once,
 *  if not, the default packet type is assumed: DSRV_MSG_NULL
 *
 * \param in_cType The type of the packet, one of the DSRV_MSG_xxx defines.
 *
 * \return a pointer to itself (this)
 *
 * \note The DSRV_MSG_xxx defines can be found in tools/server2/server.h
 *
 * \author Pompei2
 */
Packet *FTS::Packet::setType(master_request_t in_cType)
{
    ((fts_packet_hdr_t *) m_pData)->req_id = in_cType;

    return this;
}

/// Gets the type of the packet.
/** This returns the type of the packet.
 *
 * \return The type of the packet.
 *
 * \note The type is a DSRV_MSG_xxx define and can be found
 *       in tools/server2/server.h
 *
 * \author Pompei2
 */
master_request_t FTS::Packet::getType(void) const
{
#ifdef DEBUG
    fts_packet_hdr_t *hdr = (fts_packet_hdr_t *)m_pData;
    return hdr->req_id;
#else
    return ((fts_packet_hdr_t *) m_pData)->req_id;
#endif
}

/// Rewinds the cursor to the beginning.
/** This sets the cursor back to the beginning of the data.
 *
 * \return a pointer to itself (this)
 *
 * \author Pompei2
 */
Packet *FTS::Packet::rewind(void)
{
    this->m_uiCursor = D_PACKET_HDR_LEN;
    return this;
}

/// Appends a string to the message.
/** This appends a string at the current cursor position in the message.
 *  After adding the data, the cursor is moved to point right behind it.
 *
 * \param in The data to append.
 *
 * \return a pointer to itself (this)
 *
 * \author Pompei2
 */
Packet *FTS::Packet::append(const String & in)
{
    size_t iLen =  in.len( ) + m_uiCursor + 1;
    m_pData = (int8_t *)realloc((void *)m_pData, iLen);
    if(in.c_str() == NULL) {
        // special case : on NULL ptr a \0 string should be generated.
        m_pData[m_uiCursor] = 0;
    } else {
        strcpy((char *)&m_pData[m_uiCursor], in.c_str());
    }
    m_uiCursor += in.len( ) + 1;
    ((fts_packet_hdr_t*)m_pData)->data_len = m_uiCursor - D_PACKET_HDR_LEN ;
    return this;
}

/// Appends arbitrary data to the packet.
/** This appends any data you want to the packet. Use this only if you
 *  know what you do and how to get the correct amount of data back when
 *  reading the packet !
 *
 * \param in_pData Pointer where the data is stored.
 * \param in_iSize The length of the data to store.
 *
 * \return a pointer to itself (this)
 *
 * \note The data gets memcpy'ed that means that you may free the original
 *       data later on.
 *
 * \author Pompei2
 */
Packet *FTS::Packet::append(const void *in_pData, uint32_t in_iSize)
{
    uint32_t iLen = in_iSize + m_uiCursor + 1;
    m_pData = (int8_t *)realloc((void *)m_pData, iLen);
    if(in_pData == NULL) {
        // special case : on NULL ptr a \0 should be generated.
        m_pData[m_uiCursor] = 0;
        m_uiCursor++;
    } else {
        memcpy(&m_pData[m_uiCursor], in_pData, in_iSize);
        m_uiCursor += in_iSize;
    }
    ((fts_packet_hdr_t*)m_pData)->data_len = m_uiCursor - D_PACKET_HDR_LEN;
    return this;
}

/** This returns the string that is at the current cursor's position
 *  in the message. After reading the data, the cursor is moved to
 *  point right behind it.
 *
 * \return the string that has been read.
 *
 * \note
 *
 * \author Pompei2
 */
String FTS::Packet::get_string()
{
    size_t len = this->getTotalLen();
    if(m_uiCursor >= len)
        return String::EMPTY;

    String ret = String((char *)&m_pData[m_uiCursor]);

    m_uiCursor += ret.byteCount() + 1;
    return ret;
}

/** This returns the string that is at the current cursor's position
 *  in the message and then removes it from the message. After this
 *  operation, the cursor should be at the same absolute place as before,
 *  but the data behind the cursor should be different.
 *
 * \return the string that has been removed.
 *
 * \note
 *
 * \author Pompei2
 */
String FTS::Packet::extractString()
{
    size_t len = this->getTotalLen();
    if(m_uiCursor >= len)
        return String::EMPTY;

    String ret = String((char *)&m_pData[m_uiCursor]);

    // Now we got the string, we still need to remove it from the data.

    int8_t *pNewData = (int8_t *)calloc(1,len - (ret.byteCount() + 1));
    memcpy(pNewData, m_pData, m_uiCursor);
    memcpy(&pNewData[m_uiCursor], &m_pData[m_uiCursor+ret.byteCount()+1], len - ret.byteCount() - 1 - m_uiCursor);

    // The data size is less now.
    ((fts_packet_hdr_t*)pNewData)->data_len -= ret.byteCount() + 1;

    SAFE_FREE(m_pData);
    m_pData = pNewData;

    // We do not move the cursor!
    return ret;
}

/** This returns binary data that is at the current cursor's position
 *  in the message. If you want it to read more then there is, it will
 *  stop reading at the end of the packet. If you want it to read "0" bytes,
 *  it will read everything till the end of the packet.\n
 *  After reading the data, the cursor is moved to point right behind it.
 *
 * \param in_iSize The lenght of the data to retrieve. If the lenght
 *                 is too long, it gets cut, if it is 0 the whole data is read.
 * \param out_pData A pointer to allocated memory where to store the read data.
 *
 * \return ERR_OK or an error code below 0.
 *
 * \author Pompei2
 */
int FTS::Packet::get(void *out_pData, uint32_t in_iSize)
{
    uint32_t len = this->getTotalLen() - m_uiCursor;

    // Calculate the size of the data to get.
    len = in_iSize == 0 ? len : std::min(len,in_iSize);

    memcpy(out_pData, &m_pData[m_uiCursor], len);

    m_uiCursor += len;
    return ERR_OK;
}

/*! Return the total length of data.
 * \author kabey
 * \return size_t
 * \note
 *
 */
uint32_t FTS::Packet::getTotalLen( void ) const
{
#ifdef DEBUG
    fts_packet_hdr_t *hdr = (fts_packet_hdr_t *)m_pData;
    uint32_t s = sizeof(fts_packet_hdr_t);
    return hdr->data_len + s;
#else
    return ((fts_packet_hdr_t*)m_pData)->data_len + sizeof(fts_packet_hdr_t);
#endif
}

/*! Return the length of payload data.
* \author kabey
* \return size_t
* \note
*
*/
uint32_t FTS::Packet::getPayloadLen( void ) const
{
    // For debug purposes
#ifdef DEBUG
    fts_packet_hdr_t *hdr = (fts_packet_hdr_t *)m_pData;
    return hdr->data_len;
#else
    return ((fts_packet_hdr_t*)m_pData)->data_len;
#endif
}

/// Write this packet into another packet.
/** This writes the current packet (this) into another packet (\a in_pPack ) at
 *  the current cursor position.\n
 *  The things that are stored for later retrieval are only the master_request_t
 *  (packet type), the payload length and the data.
 *
 * \param in_pPack The packet where to write this into.
 *
 * \return ERR_OK or an error code below 0.
 *
 * \author Pompei2
 */
int FTS::Packet::writeToPacket(Packet *in_pPack)
{
    // We do not need to append the four ident chars.

    // Append the request type and the data length.
    in_pPack->append(this->getType());
    in_pPack->append(this->getPayloadLen());

    // Append the data.
    in_pPack->append(this->getDataPtr(), this->getPayloadLen());
    return ERR_OK;
}

/// Extracts a packet out of a packet.
/** This extracts a packet (stored earlyer using the \a writeToPacket method)
 *  from a packet (\a in_pPack ). The extracted packet will be stored in this.\n
 *  Every data currently stored in this will be removed first.
 *
 * \param in_pPack The packet to extract a packet from.
 *
 * \return ERR_OK or an error code below 0.
 *
 * \author Pompei2
 */
int FTS::Packet::readFromPacket(Packet *in_pPack)
{
    SAFE_FREE(m_pData);

    master_request_t type = DSRV_MSG_NULL;
    uint32_t uiPayloadSize = 0;

    in_pPack->get(type);
    in_pPack->get(uiPayloadSize);

    m_pData = (int8_t *)calloc(1,D_PACKET_HDR_LEN + uiPayloadSize);
    memset(m_pData, 0, D_PACKET_HDR_LEN + uiPayloadSize);
    m_pData[0] = 'F';
    m_pData[1] = 'T';
    m_pData[2] = 'S';
    m_pData[3] = 'S';

    this->setType(type);
    in_pPack->get(this->getDataPtr(), uiPayloadSize);
    ((fts_packet_hdr_t *)m_pData)->data_len = uiPayloadSize;
    this->rewind();

    return ERR_OK;
}

/// Print this packet into a file.
/** This prints the whole packet into a file. What it prints is the header,
 *  as it is, and then the whole data. Thus the current cursor position does
 *  not get changed neither does it get written to the file!
 *
 * \param in_pFile The file to print the packet onto.
 *
 * \return ERR_OK or an error code below 0.
 *
 * \author Pompei2
 */
int FTS::Packet::printToFile(FILE *in_pFile) const
{
    if(fwrite((fts_packet_hdr_t *)m_pData, sizeof(fts_packet_hdr_t), 1, in_pFile) != 1)
        return -1;

    if(fwrite(this->getConstDataPtr(), sizeof(uint8_t), this->getPayloadLen(), in_pFile) != this->getPayloadLen())
        return -2;

    return ERR_OK;
}
