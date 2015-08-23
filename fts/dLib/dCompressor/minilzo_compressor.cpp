/**
 * \file minilzo_compressor.cpp
 * \author Pompei2
 * \date 19 Feb 2009
 * \brief This file contains the implementation of the minilzo compressor.
 **/

#include "minilzo_compressor.h"
#include "logging/logger.h"
#include "logging/Chronometer.h"
#include "utilities/StreamedDataContainer.h"
#include "utilities/DataContainer.h"

using namespace FTS;

bool MiniLZOCompressor::m_bMiniLZOInited = false;

/// Default constructor.
MiniLZOCompressor::MiniLZOCompressor()
    : m_pWorkingMemory(NULL)
{
    // Only initialise the miniLZO once.
    if(!m_bMiniLZOInited) {
        if(lzo_init() != LZO_E_OK) {
            throw SyscallException("lzo_init", MsgType::Horror);
        } else {
            m_bMiniLZOInited = true;
        }
    }

    // Allocate some memory that we will use when (de)compressing.
    // Every object uses its own working memory to stay threadsafe.
    m_pWorkingMemory = new lzo_align_t __LZO_MMODEL[((LZO1X_1_MEM_COMPRESS) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t)];
}

/// Default destructor.
MiniLZOCompressor::~MiniLZOCompressor()
{
    SAFE_DELETE_ARR(m_pWorkingMemory);
}

/// \return The string that should be at the beginning of every data that is
///         compressed using this compressor.
String MiniLZOCompressor::getHeaderID() const
{
    return "FTSLZO";
}

/// \return The name of this compressor.
String MiniLZOCompressor::getName() const
{
    return "MiniLZO";
}

/// \return A quick description and credits of this compressor.
String MiniLZOCompressor::getDescription() const
{
    return "MiniLZO is a VERY fast (de)compressor written by Markus Franz Xaver Johannes Oberhumer";
}

/// This method checks if a DataContainer contains data that may be compressed
/// using this compressor.
/// \param in_pData A pointer to the DataContainer that has to be checked.
/// \return true if the data may be compressed using this compressor, false else.
bool MiniLZOCompressor::isMyType(const DataContainer * const in_pData) const
{
    if(in_pData == NULL || in_pData->getData() == NULL) {
        return false;
    }

    StreamedConstDataContainer scdc(in_pData);
    String sDataBegin = scdc.readstr();
    return this->getHeaderID() == sDataBegin && !scdc.eod();
}

/// This method should automagically decompress all the data it gets.
/// \param out_where A streamed data container where you want to put the
///                 decompressed data into.
/// \param in_pData The data that has to be decompressed.
/// \exception CorruptDecompressionDataException for anything that went wrong.
/// \note If an error occurs, \a out_where should be left intact.
void MiniLZOCompressor::decompress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const
{
    // Check the header ID.
    StreamedConstDataContainer scdc(in_pData);
    if(scdc.readstr() != this->getHeaderID()) {
        m_sLastProblem = "Invalid header";
        throw CorruptDecompressionDataException(this, MsgType::Error);
    }

    // Check the version. Nothing to check.
    scdc.read<uint8_t>();

    // Get the size of the whole thing when uncompressed.
    uint64_t uiUncompSize = scdc.read<uint64_t>();
    lzo_uint uiUncompSizeLib = static_cast<lzo_uint>(uiUncompSize);

    // Check if all went right.
    if(scdc.eod()) {
        m_sLastProblem = "Unexpected end of data";
        throw CorruptDecompressionDataException(this, MsgType::Error);
    }

    // Debug output.
    const float fInSzMB = static_cast<float>(in_pData->getSize())/(1024.0f*1024.0f);
    const float fOutSzMB = static_cast<float>(uiUncompSize)/(1024.0f*1024.0f);
    String sLog = "decompression of "+String::nr(in_pData->getSize())+" Bytes "
                   "("+String::nr(fInSzMB, 2)+" MB) into "+String::nr(uiUncompSize)+" Bytes "
                   "("+String::nr(fOutSzMB, 2)+" MB)";
    LoggingChronometer compChron(sLog, 3);

    // Allocate enough data for the uncompressed thing.
    out_where.grow(uiUncompSize);

    // We compress the data into that buffer.
    const lzo_uint uiDataSize = static_cast<lzo_uint>(scdc.getSizeTillEnd());
    int r = lzo1x_decompress_safe(in_pData->getData()+scdc.getCursorPos(), uiDataSize,
                                  out_where.getBufferInFrontOfCursor(), &uiUncompSizeLib, NULL);
    if(r != LZO_E_OK || uiUncompSize != static_cast<uint64_t>(uiUncompSizeLib)) {
        m_sLastProblem = "Broken data";
        out_where.shrink(uiUncompSize); // Back into the original state!
        throw CorruptDecompressionDataException(this, MsgType::Error);
    }

    // Move the cursor behind the uncompressed data.
    out_where.moveCursor(uiUncompSize);

    // Debug output.
    compChron.measure();
}

/// This method should automagically compress all the data it gets.
/// \note This method stores some additional data (the header) in front of the
///       compressed result to identify its compressed data format, and store
///       the size the data has when not compressed.
/// \note If an error occurs, \a out_where should be left intact.
/// \param out_where A streamed data container where the decompressed data
///                  will be written behind the cursor. The cursor will be
///                  placed right behind the compressed data.
/// \param in_pData The data that has to be compressed.
/// \note No exception will be thrown. It just never fails :)
bool MiniLZOCompressor::compress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const
{
    // This is the maximum size the compressed data may have.
    uint64_t uiWorstCaseSize = in_pData->getSize() + in_pData->getSize() / 16 + 64 + 3;

    // We write the ID into the first bytes and the version.
    out_where.insert(this->getHeaderID());
    out_where.insert(static_cast<uint8_t>(1));

    // ... we write the real uncompressed data size into the next 8 bytes.
    out_where.insert(in_pData->getSize());

    lzo_uint uiCompressedSize = static_cast<lzo_uint>(uiWorstCaseSize);

    // Debug output.
    const float fInSzMB = static_cast<float>(in_pData->getSize())/(1024.0f*1024.0f);
    String sLog = "compression of "+String::nr(in_pData->getSize())+" Bytes "
                   "("+String::nr(fInSzMB, 2)+" MB)";
    LoggingChronometer compChron(sLog, 3);

    // Allocate enough memory to hold the worst-case compression:
    out_where.grow(uiWorstCaseSize);

    // We compress the data into that buffer. The function CAN'T fail.
    lzo1x_1_compress(in_pData->getData(), static_cast<lzo_uint>(in_pData->getSize()),
                     out_where.getBufferInFrontOfCursor(),
                     &uiCompressedSize, m_pWorkingMemory);

    // Debug output.
    const float fOutSzMB = static_cast<float>(uiCompressedSize)/(1024.0f*1024.0f);
    const uint64_t uiCompressedSize64 = static_cast<uint64_t>(uiCompressedSize);

    // check for an incompressible block
    String sAddInfo = "into "+String::nr(uiCompressedSize64)+" Bytes ("+String::nr(fOutSzMB, 2)+" MB)";
    if(uiCompressedSize64 >= in_pData->getSize()) {
        sAddInfo = "INCOMPRESSIBLE DATA!" + sAddInfo;
    }

    // Now we resize the data container so that it is only the size the data needs,
    // no more trailing bullshit in it.
    out_where.shrink(uiWorstCaseSize - uiCompressedSize64);

    // Move the cursor behind the uncompressed data.
    out_where.moveCursor(uiCompressedSize64);

    compChron.measure(sAddInfo);
    return uiCompressedSize64 < in_pData->getSize();
}
