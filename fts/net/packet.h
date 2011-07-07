/**
 * \file packet.h
 * \author Pompei2
 * \date 11 May 2007
 * \brief This file describes the class that represents a packet
 *        that gets sent over a connection.
 **/

#ifndef FTS_PACKET_H
#define FTS_PACKET_H

#include "main.h"

#include "packet_header.h"
#include "dLib/dString/dString.h"

namespace FTS {

/// The FTS packet class
/** This class represents a packet that can be sent over a connection.
 *  This connection is described by another class.
 **/
class Packet {
    friend class Connection;
    friend class TraditionalConnection;
    friend class OnDemandHTTPConnection;

private:
    int8_t *m_pData;     ///< The data this packet contains.
    uint32_t m_uiCursor; ///< The current cursor position in the data.

    Packet(const Packet &in_copy) {}; ///< Block the copy-constructor.
    Packet(Packet &in_copy) {}; ///< Block the copy-constructor.
    Packet() {}; ///< Block the default-constructor.

public:
    Packet(master_request_t in_cType);
    virtual ~Packet();

    bool isValid() const;

    Packet *setType(master_request_t in_cType);
    master_request_t getType() const;

    uint32_t getTotalLen() const;
    uint32_t getPayloadLen()const;
    Packet *rewind();

    Packet *append(const FTS::String & in);
    Packet *append(const void *in_pData, uint32_t in_iSize);
    inline Packet *append(int8_t in) {return this->append_intern(in);};
    inline Packet *append(uint8_t in) {return this->append_intern(in);};
    inline Packet *append(int16_t in) {return this->append_intern(in);};
    inline Packet *append(uint16_t in) {return this->append_intern(in);};
    inline Packet *append(int32_t in) {return this->append_intern(in);};
    inline Packet *append(uint32_t in) {return this->append_intern(in);};
    inline Packet *append(int64_t in) {return this->append_intern(in);};
    inline Packet *append(uint64_t in) {return this->append_intern(in);};

    inline int8_t get() {int8_t out = 0; this->get_intern(out); return out;};
    inline int8_t get(int8_t &out) {this->get_intern(out); return out;};
    inline uint8_t get(uint8_t &out) {this->get_intern(out); return out;};
    inline int16_t get(int16_t &out) {this->get_intern(out); return out;};
    inline uint16_t get(uint16_t &out) {this->get_intern(out); return out;};
    inline int32_t get(int32_t &out) {this->get_intern(out); return out;};
    inline uint32_t get(uint32_t &out) {this->get_intern(out); return out;};
    inline int64_t get(int64_t &out) {this->get_intern(out); return out;};
    inline uint64_t get(uint64_t &out) {this->get_intern(out); return out;};
    inline FTS::String get(FTS::String &out) {out = this->get_string(); return out;};
    FTS::String get_string();
    FTS::String extractString();
    int get(void *out_pData, uint32_t in_iSize);

    int writeToPacket(Packet *in_pPack);
    int readFromPacket(Packet *in_pPack);

    int printToFile(FILE *in_pFile) const;

private:
    /// Appends something to the message.
    /** This appends something at the current cursor position in the message.
     *  After adding the data, the cursor is moved to point right behind it.
     *
     * \param in The data to append.
     *
     * \return a pointer to itself (this)
     *
     * \note the data can be anything, like an int, float, .. even a struct.
     *       Of course, pointers gets only their address stored.
     *       If there is not enough space to hold it, nothing is stored.
     *
     * \author Pompei2
     */
     template<typename T> Packet * append_intern(T in) {
        int8_t* pBuf ; 
        pBuf = (int8_t *)realloc((void *)m_pData, sizeof(T) + m_uiCursor);
        if( pBuf != NULL ) {
            m_pData = pBuf;
            *((T *) & m_pData[m_uiCursor]) = in;
            m_uiCursor += sizeof(T);
            ((fts_packet_hdr_t*)m_pData)->data_len = m_uiCursor - D_PACKET_HDR_LEN ;
        }
        return this;
    }

    /// Retrieves something from the message.
    /** This retrieves something from the current cursor position in the message.
     *  After retrieving the data, the cursor is moved to point right behind it.
     *
     * \param in Reference to the data to retrieve. if there was an error, this is set to 0.
     *
     * \return nothing.
     *
     * \note the data can be anything, like an int, float, .. even a struct.
     *       THIS FUNCTION IS DANGEROUS as it does not do any bound-checking,
     *       what means that you can get more then there is !
     *
     *       Added a try to bound-check. TODO: Test it.
     *
     * \author Pompei2
     */
    template<typename T> void get_intern(T & in) {
        size_t len = this->getTotalLen();
        if(m_uiCursor >= len || m_uiCursor + sizeof(T) > len) {
            in = (T)0;
            return;
        }

        in = *((T *) (&m_pData[m_uiCursor]));
        m_uiCursor += sizeof(T);
        return;
    }

    /// Returns a pointer to the beginning of the data.
    inline int8_t *getDataPtr() {return &m_pData[D_PACKET_HDR_LEN];};
    inline const int8_t *getConstDataPtr() const {return &m_pData[D_PACKET_HDR_LEN];};
};

}

#endif /* FTS_PACKET_H */

 /* EOF */