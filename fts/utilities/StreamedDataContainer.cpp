#include "StreamedDataContainer.h"
#include "DataContainer.h"

#include "logging/logger.h"

using namespace FTS;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                   READABLE STREAM IMPLEMENTATIONS                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** \internal This method reads a certain amount of bytes from the data
 *            without doing anything else with the bytes.
 *
 * \param out_ptr A pointer to a location in memory where to store the read
 *                bytes into. Needs to be at least \a in_size bytes big.
 * \param in_size The number of bytes to read.
 *
 * \return The amout of bytes that have been read. This may not be \a in_size in
 *         case of an early end of data.
 *
 * \author Pompei2
 */
size_t FTS::ReadableStream::readRaw(void *out_ptr, size_t in_size)
{
    // Nothing to read.
//     if(this->invalid() || this->eod())
//         return 0;

    // Read only as much data as there is available.
    size_t uiToRead = std::min((size_t)this->getSizeTillEnd(), in_size);
    memcpy(out_ptr, this->getDataAtCursorPos(), uiToRead);
    this->setCursorPos(this->getCursorPos() + uiToRead);

    return uiToRead;
}

/** This method reads \a in_nmemb data chunks, where one chunk is \a in_size
 *  bytes long. Every data-chunk is assumed to be stored as little-endian in the
 *  data and is then converted into the system's endianness.
 *
 * \param out_ptr A pointer to a location in memory where to store the read
 *                bytes into. Needs to be at least \a in_size times \a in_nmemb
 *                bytes big.
 * \param in_size The number of bytes that each chunk has.
 * \param in_nmemb The number of chunks to read.
 *
 * \return The amout of chunks that have been read (not bytes!). This may not be
 *         \a in_nmemb in case of an early end of file.
 *
 * \note This method is similar to the stdio fread function, but this one here
 *       converts the endianness.
 *
 * \author Pompei2
 */
size_t FTS::ReadableStream::read(void *out_ptr, size_t in_size, size_t in_nmemb)
{
    // Nothing to read.
    if(this->invalid() || this->eod())
        return 0;

    // If the system has the "good" endianness, we don't need to swap bytes, so
    // we read everything at once, that speeds things up a lot
    if(systemHasGoodEndian()) {
        // Get the amount of chunks that we can read.
        size_t nAvailMembs = static_cast<size_t>(this->getSizeTillEnd()) / in_size;
        size_t nMembsToRead = std::min(in_nmemb, nAvailMembs);
        memcpy(out_ptr, this->getDataAtCursorPos(), nMembsToRead*in_size);
        this->setCursorPos(this->getCursorPos() + nMembsToRead*in_size);
        return nMembsToRead;
    } else {
        /// \TODO: The reading of stuff when there is need to swap endianness is
        ///        VERY slow, optimize!! (Maybe first copy all, then swap bytes
        ///        of all)
        size_t i = 0;
        uint8_t *pOutPosition = reinterpret_cast<uint8_t *>(out_ptr);
        for(i = 0 ; i < in_nmemb ; i++) {
            // Check if there is enough data available.
            if(this->getSizeTillEnd() < in_size)
                return i;

            // This already advances the read cursor.
            if(this->readRaw(pOutPosition, in_size) < in_size)
                return i;

            toSystemEndian(pOutPosition, in_size);
            pOutPosition += in_size;
        }
        return i;
    }
}

/** Reads a number of bytes from the data, exactly as they are in the data,
 *  without any endianness check.
 *
 * \param out_ptr An allocated pointer big enough to hold \a in_size bytes of data.
 * \param in_size The size of the data that will be read.
 *
 * \return The number of bytes really read. To check for an error, check if the
 *         number of bytes read is less then \a in_size.
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 *
 * \author Pompei2
 */
size_t FTS::ReadableStream::readNoEndian(void *out_ptr, size_t in_size)
{
    return this->readRaw(out_ptr, in_size);
}

/** Reads a number of bytes from the data, exactly as they are in the data,
 *  without any endianness check.
 *
 * \param out_data A raw data container that will hold the data. The amount of
 *                 data that will be read is exactly the size of the container.
 *
 * \return The number of bytes really read. To check for an error, check if the
 *         number of bytes read is less then \a out_data.getSize().
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 *
 * \author Pompei2
 */
size_t FTS::ReadableStream::readNoEndian(RawDataContainer &out_data)
{
    return this->readRaw(out_data.getData(), static_cast<size_t>(out_data.getSize()));
}

/** Reads one zero-terminated string out of the data.
 *
 * \param out This string will contain the read string.
 *
 * \return this
 *
 * \note If we are at the end of the file, this returns an empty string.
 *
 * \author Pompei2
 */
ReadableStream *FTS::ReadableStream::read(String &out)
{
    // Nothing to read.
    if(this->invalid() || this->eod()) {
        out = String::EMPTY;
        return this;
    }

    out = String((const char *)this->getDataAtCursorPos(), 0, this->getSizeTillEnd());
    this->setCursorPos(this->getCursorPos() + out.byteCount() + 1);

    return this;
}

/** Reads one zero-terminated string out of the data.
 *
 * \return The string that has been read (same as \a out).
 *
 * \note If we are at the end of the file, this returns an empty string.
 *
 * \author Pompei2
 */
String FTS::ReadableStream::readstr()
{
    String s;
    this->read(s);
    return s;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                   WRITEABLE STREAM IMPLEMENTATIONS                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** \internal This is our low-level insertion method that inserts some data
 *            into the data at the cursor's position. It first resizes the data
 *            buffer, then inserts some data into it. It does change the
 *            cursor's position.
 *
 * \param in_ptr The data to insert.
 * \param in_size The size of the data to be inserted.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \author Pompei2
 */
int FTS::WriteableStream::insert(const void *in_ptr, size_t in_size)
{
    // Moves the data behind the cursor further away so there is place for the new one.
    if(ERR_OK != this->moveAndResizeData(this->getCursorPos(), this->getSizeTillEnd(), static_cast<uint64_t>(in_size)))
        return -1;

    // Put the new data in place.
    this->memcpyToCursorPos(ConstRawDataContainer(in_ptr, in_size));

    // Move the cursor.
    this->setCursorPos(this->getCursorPos() + in_size);
    return ERR_OK;
}

/** \internal This method will overwrite the current data that is behind the
 *  cursor. The cursor will still move. This is a bit like the default-mode of
 *  most hex-editors.\n
 *  If it comes at the end of the data, it will insert the rest at the end, thus
 *  resizing the data.\n
 *  It does change the cursor's position. It does not care about endianness,
 *  it just copies the raw data.
 *
 * \param in_ptr The data to write.
 * \param in_size The total size of the data that will be written.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note If the call fails, the data is not guaranteed to be like before.
 *
 * \author Pompei2
 */
int FTS::WriteableStream::overwrite(const uint8_t *in_ptr, size_t in_size)
{
    // Not existent, create.
    if(this->invalid()) {
        this->moveAndResizeData(0,in_size,0);
    }

    // 3 Cases:
    if(in_size <= this->getSizeTillEnd()) {
        // Case 1: Can overwrite all in_size bytes. Do so.
        this->memcpyToCursorPos(ConstRawDataContainer(in_ptr, in_size));
        // Increase the cursor.
        this->setCursorPos(this->getCursorPos() + in_size);
    } else if(this->eod()) {
        // Case 2: I am at the end, append in_size bytes.
        return this->insert(in_ptr, in_size);
    } else {
        // Case 3: Can overwrite one or more, but less then in_size
        //         bytes. Do so and append the rest.

        // Part 1: Overwrite as much bytes as we can.
        uint64_t nBytesToOverwrite = this->getSizeTillEnd();
        this->memcpyToCursorPos(ConstRawDataContainer(in_ptr, static_cast<size_t>(nBytesToOverwrite)));

        // Place the cursor at the end.
        this->setCursorPos(this->getCursorPos() + nBytesToOverwrite);

        // Part 2: Insert the rest at the end.
        return this->insert(in_ptr + nBytesToOverwrite, static_cast<size_t>(in_size-nBytesToOverwrite));
    }

    return ERR_OK;
}

/** This inserts some data into the data, all data is inserted at the current
 *  cursor position, making the total size grow. The cursor will still move.\n
 * \n
 *  If you run a big-endian machine, every data-chunk gets its bytes swapped
 *  into little-endian order before being written. If you are on a little-endian
 *  machine, nothing happens to your bytes, they get written as-is.
 *
 * \param in_ptr The data to write.
 * \param in_size The size of one data-chunk that will be written.
 * \param in_nmemb This is how much data chunks will be written.
 *
 * \return The number of data-chunks actually written. If this value is less
 *         than \a in_nmemb there was an error during the write.
 *
 * \author Pompei2
 */
size_t FTS::WriteableStream::insert(const void *in_ptr, size_t in_size, size_t in_nmemb)
{
    uint8_t *pSwapBuff = new uint8_t[in_size];
    const uint8_t *pInPosition = reinterpret_cast<const uint8_t *>(in_ptr);
    const uint8_t *pWriteBuff = pInPosition;
    size_t i = 0;

    for(i = 0 ; i < in_nmemb ; i++) {
        // If needed, we first copy it into a buffer where we can swap the bytes
        // to handle endianness correctly. We always write little endian into
        // the files.
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        // Nothing to do.
        pWriteBuff = pInPosition;
#else
        // Swap the bytes around, in a temporary buffer.
        memcpy(pSwapBuff, pInPosition, in_size);
        toSystemEndian(pSwapBuff, in_size);
        pWriteBuff = pSwapBuff;
#endif
        if(this->insert(pWriteBuff, in_size) != ERR_OK) {
            break;
        }

        pInPosition += in_size;
    }

    SAFE_DELETE_ARR( pSwapBuff );
    return i;
}

/** Inserts a number of bytes to the data, exactly as they are in the memory,
 *  without any endianness check, byte swap or so.
 *
 * \param in_ptr  A pointer to the data that will be written.
 * \param in_size The size of the data that will be written.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 *
 * \author Pompei2
 */
size_t FTS::WriteableStream::insertNoEndian(const void *in_ptr, size_t in_size)
{
    return this->insert(in_ptr, in_size);
}

/** Inserts a number of bytes to the file, exactly as they are in the memory,
 *  without any endianness check, byte swap or so.
 *
 * \param in_data The data that will be written.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 *
 * \author Pompei2
 */
size_t FTS::WriteableStream::insertNoEndian(const DataContainer &in_data)
{
    return this->insert(in_data.getData(), static_cast<size_t>(in_data.getSize()));
}

/** Inserts one zero-terminated string into the data.
 *
 * \param in The string to write to the data.
 *
 * \return ERR_OK if all went right, an error code < 0 on failure.
 *
 * \author Pompei2
 */
int FTS::WriteableStream::insert(const String &in)
{
    if(this->insertNoEndian(in.c_str(), in.byteCount()) != ERR_OK)
        return -1;

    return this->insert(static_cast<int8_t>('\0'));
}

/** This overwrites some data in the data, the data will overwrite the current
 *  data that is behind the cursor. The cursor will still move. This is a
 *  bit like the default-mode of most hex-editors.\n
 *  If the cursor comes at the end of the data, the data is resized to have
 *  enough place to hold everything.
 * \n
 *  If you run a big-endian machine, every data-chunk gets its bytes swapped
 *  into little-endian order before being written. If you are on a little-endian
 *  machine, nothing happens to your bytes, they get written as-is.
 *
 * \param in_ptr The data to write.
 * \param in_size The size of one data-chunk that will be written.
 * \param in_nmemb This is how much data chunks will be written.
 *
 * \return The number of data-chunks actually written. If this value is less
 *         than \a in_nmemb there was an error during the write.
 *
 * \author Pompei2
 */
size_t FTS::WriteableStream::overwrite(const void *in_ptr, size_t in_size, size_t in_nmemb)
{
    uint8_t *pSwapBuff = new uint8_t[in_size];
    const uint8_t *pInPosition = reinterpret_cast<const uint8_t *>(in_ptr);
    const uint8_t *pWriteBuff = pInPosition;
    size_t i = 0;

    for(i = 0 ; i < in_nmemb ; i++) {
        // If needed, we first copy it into a buffer where we can swap the bytes
        // to handle endianness correctly. We always write little endian into
        // the files.
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        // Nothing to do.
        pWriteBuff = pInPosition;
#else
        // Swap the bytes around, in a temporary buffer.
        memcpy(pSwapBuff, pInPosition, in_size);
        toSystemEndian(pSwapBuff, in_size);
        pWriteBuff = pSwapBuff;
#endif
        if(this->overwrite(pWriteBuff, in_size) != ERR_OK) {
            break;
        }

        pInPosition += in_size;
    }

    delete [] pSwapBuff;
    return i;
}

/** Overwrites a number of bytes to the data, exactly as they are in the memory,
 *  without any endianness check, byte swap or so.
 *
 * \param in_ptr  A pointer to the data that will be written.
 * \param in_size The size of the data that will be written.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 *
 * \author Pompei2
 */
size_t FTS::WriteableStream::overwriteNoEndian(const void *in_ptr, size_t in_size)
{
    return this->insert(in_ptr, in_size);
}

/** Overwrites a number of bytes to the file, exactly as they are in the memory,
 *  without any endianness check, byte swap or so.
 *
 * \param in_data The data that will be written.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 *
 * \author Pompei2
 */
size_t FTS::WriteableStream::overwriteNoEndian(const DataContainer &in_data)
{
    return this->insert(in_data.getData(), static_cast<size_t>(in_data.getSize()));
}

/** Overwrites one zero-terminated string in the data.
 *
 * \param in The string to write to the data.
 *
 * \return ERR_OK if all went right, an error code < 0 on failure.
 *
 * \author Pompei2
 */
int FTS::WriteableStream::overwrite(const String &in)
{
    if(this->overwriteNoEndian(in.c_str(), in.byteCount()) != ERR_OK)
        return -1;

    return this->overwrite(static_cast<int8_t>('\0'));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//               STREAMED DATA CONTAINER IMPLEMENTATIONS                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// Default constructor, creates an empty streamed data container.
FTS::StreamedDataContainer::StreamedDataContainer()
    : m_pDC(new RawDataContainer)
    , m_uiDataCursor(0)
{
}

/// Creates an empty streamed data container with a certain pre-allocated size.
/// This can be used if the size the data will have is already known in advance,
/// to speed up things a bit.
/// \param in_uiSize The size to allocate in advance.
FTS::StreamedDataContainer::StreamedDataContainer(uint64_t in_uiSize)
    : m_pDC(new RawDataContainer(in_uiSize))
    , m_uiDataCursor(0)
{
}

/// Creates a streamed data container from a RawDataContainer. The streamed data
/// container takes full control over the data container and will also delete it.
/// \param out_pDC The data container to take control over. Don't pass NULL!
FTS::StreamedDataContainer::StreamedDataContainer(RawDataContainer *out_pDC)
    : m_pDC(out_pDC)
    , m_uiDataCursor(0)
{
    if(m_pDC == NULL) {
        FTS18N("InvParam", MsgType::Horror, "StreamedDataContainer::StreamedDataContainer(NULL)");
    }
}

FTS::StreamedDataContainer::StreamedDataContainer(const StreamedDataContainer& o)
    : m_pDC(o.getBoundDC()->copy())
    , m_uiDataCursor(o.m_uiDataCursor)
{
}

/// Deletes the bound data container too!
FTS::StreamedDataContainer::~StreamedDataContainer()
{
    SAFE_DELETE(m_pDC);
}

/** Binds a new data container to the streamed data container. This will loose
 *  control over the old data container (and delete it) and gain control over
 *  the data container passed as an argument. Also this will rewind the cursor.
 *
 * \param out_pDC The new data container to take control over.
 */
void FTS::StreamedDataContainer::bindDC(RawDataContainer *out_pDC)
{
//     if(out_pDC == NULL) {
//         FTS18N("InvParam", FTS_HORROR, "StreamedDataContainer::bindDC(NULL)");
//         return ;
//     }

    SAFE_DELETE(m_pDC);
    m_pDC = out_pDC;

    this->setCursorPos(0);
}

/** Un-Binds the data container that is currently bound, without deleting it.
 *  After a call to this method, no data container is bound to this stream.
 *
 * \return The data container that was previously bound to the stream. You need
 *         to delete it.
 */
RawDataContainer *FTS::StreamedDataContainer::unbindDC()
{
    RawDataContainer *pDC = m_pDC;
    m_pDC = NULL;
    this->setCursorPos(0);
    return pDC;
}

/// \return True if the data is invalid (for example non existent).
bool FTS::StreamedDataContainer::invalid() const
{
    return m_pDC == NULL || m_pDC->getData() == NULL;
}

/// \return True if the end of data has been reached, false else.
bool FTS::StreamedDataContainer::eod() const
{
    return this->invalid() || this->getCursorPos() >= m_pDC->getSize();
}

/// Moves the cursor into a new, arbitrary position.
/// \param in_uiPos The new position of the cursor (0 is at the beginning).
/// \return The old position of the cursor.
uint64_t FTS::StreamedDataContainer::setCursorPos(uint64_t in_uiPos)
{
    uint64_t uiOldPos = this->getCursorPos();
    m_uiDataCursor = std::min(m_pDC == NULL ? 0 : m_pDC->getSize(), in_uiPos);
    return uiOldPos;
}

/// \return A pointer that points into the data at exactly the place the cursor
///         currently resides. The pointer is const, meaning read-only.
const void *FTS::StreamedDataContainer::getDataAtCursorPos() const
{
    if(this->eod())
        return NULL;

    return &m_pDC->getData()[this->getCursorPos()];
}

/// \return How much bytes there are still left from the cursor's position until
///         the end of the data.
/// \note getCursorPos + getSizeTillEnd = getDataSize
uint64_t FTS::StreamedDataContainer::getSizeTillEnd() const
{
    if(this->eod())
        return 0;

    return m_pDC->getSize() - this->getCursorPos();
}

/** This method moves some data around in the buffer while resizing the buffer
 *  if needed. If there is no buffer, one should be created, with the correct size.
 *
 * \param in_uiFrom The position from where to move the data (the position of
 *                  the first byte in the block that has to be moved)
 * \param in_uiSize The size of the datablock that has to be moved.
 * \param in_iOffset The amount of bytes to move the data around, based on
 *                   in_uiFrom. May be negative, if negative, the data will be
 *                   moved backwards.
 *
 * \return The old position of the cursor.
 *
 * \note if \a in_uiSize is zero, the data should just be resized to
 *       in_uiFrom + in_iOffset bytes.
 * \note The place in the data where the block resided just before will be
 *       filled with zeros.
 * \note Even if data is moved backwards, there will never be the need to shrink
 *       the data.
 * \note If there is no data container yet, a new one, sized \a in_uiSize is created.
 */
int FTS::StreamedDataContainer::moveAndResizeData(uint64_t in_uiFrom, uint64_t in_uiSize, int64_t in_iOffset)
{
    // Preliminary safety checks.
    if(in_iOffset < 0 && in_uiFrom < static_cast<uint64_t>(-in_iOffset)) {
        FTS18N("InvParam", MsgType::Horror, "StreamedDataContainer::moveAndResizeData(in_uiFrom ("+String::nr(in_uiFrom)+") < -in_iOffset (-"+String::nr(in_uiFrom)+"))");
        return -1;
    }

    // No data yet ? create it with the right size. No need to move anything.
    if(this->invalid()) {
        if(in_uiSize == 0) {
            m_pDC = new RawDataContainer(in_uiFrom + in_iOffset);
        } else {
            m_pDC = new RawDataContainer(in_uiSize);
        }
        return ERR_OK;
    }

    // Another safety check.
    if(in_uiFrom + in_uiSize > m_pDC->getSize()) {
        FTS18N("InvParam", MsgType::Horror, "StreamedDataContainer::moveAndResizeData(in_uiFrom ("+String::nr(in_uiFrom)+") + in_uiSize ("+String::nr(in_uiSize)+") > m_pDC->getSize() ("+String::nr(m_pDC->getSize())+"))");
        return -1;
    }

    // Determine if we need to resize that thing.
    uint64_t uiLastByteToWrite = in_uiFrom + in_iOffset + in_uiSize;
    if(uiLastByteToWrite > m_pDC->getSize()) {
        m_pDC->resize(uiLastByteToWrite);
    }

    // Move the data, beginning with the last byte.
    for(uint64_t i = in_uiSize ; i > 0 ; i--) {
        m_pDC->getData()[in_uiFrom+in_iOffset+i-1] = m_pDC->getData()[in_uiFrom+i-1];
        m_pDC->getData()[in_uiFrom+i-1] = 0;
    }

    return ERR_OK;
}

/** This should just blindly copy \a in_uiSize bytes of data from \a in_pData
 *  into the data buffer right at the cursor's position. No need ro move, resize
 *  anything whatsoever. This is put here for abstraction means.
 *
 * \param in_dc The datablock to copy.
 */
void FTS::StreamedDataContainer::memcpyToCursorPos(const ConstRawDataContainer &in_dc)
{
    if(this->invalid())
        return ;

    memcpy(&m_pDC->getData()[this->getCursorPos()], in_dc.getData(), static_cast<size_t>(in_dc.getSize()));
}

void FTS::StreamedDataContainer::grow(uint64_t in_uiAdditional)
{
    if(!m_pDC) {
        m_pDC = new RawDataContainer(in_uiAdditional);
        return;
    }

    m_pDC->grow(in_uiAdditional);
}

void FTS::StreamedDataContainer::shrink(uint64_t in_uiLess)
{
    if(!m_pDC || in_uiLess > m_pDC->getSize()) {
        return;
    }

    m_pDC->resize(m_pDC->getSize() - in_uiLess);

    // put the cursor back if needed.
    if(m_uiDataCursor >= m_pDC->getSize())
        m_uiDataCursor = m_pDC->getSize()-1;
}

/// \return Pointer to the memory buffer in front of the cursor.
///         Do what you want with it, but take care!
/// \warning Use this only if you REALLY know what you're doing.
uint8_t *FTS::StreamedDataContainer::getBufferInFrontOfCursor()
{
    if(this->invalid())
        return NULL;

    return m_pDC->getData() + this->getCursorPos();
}

/// \return The fletcher32 checksum of the data beginning at the cursor.
uint32_t FTS::StreamedDataContainer::fletcher32FromCursor() const
{
    // Taken from wikipedia. Input only works on an array of uint16_t, thus
    // if our data has one byte too much, cut it off.

    // 0 if the length is pair (exactly enough bytes), 1 if we are missing 1 byte.
    uint64_t uiToCompare = this->getSizeTillEnd() % 2;

    uint64_t uiDataLen16Bit = this->getSizeTillEnd()/2;
    const uint16_t *pData16Bit = reinterpret_cast<const uint16_t*>(this->getDataAtCursorPos());

    uint32_t sum1 = 0xffff, sum2 = 0xffff;

    while(uiDataLen16Bit > uiToCompare) {
            uint64_t tlen = uiDataLen16Bit > 360 ? 360 : uiDataLen16Bit;
            uiDataLen16Bit -= tlen;
            do {
                    sum1 += *pData16Bit++;
                    sum2 += sum1;
            } while (--tlen);
            sum1 = (sum1 & 0xffff) + (sum1 >> 16);
            sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    // If we are missing one byte, add a zero.
    if(uiToCompare > 0) {
        sum1 += *(reinterpret_cast<const uint8_t*>(pData16Bit));
        sum2 += sum1;
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    // Second reduction step to reduce sums to 16 bits.
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return sum2 << 16 | sum1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                   SAME STORY WITH CONST                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// Creates a streamed data container that may only be read from, but won't ever
/// modify its data.
/// \param in_pDC The const data container to take the data from. The data won't
///               be copied, but the full control is taken over the data
///               container (but not the data in the data container, as it's a
///               const data container!).
FTS::StreamedConstDataContainer::StreamedConstDataContainer(const DataContainer *in_pDC)
    : m_pDC(in_pDC)
    , m_uiDataCursor(0)
{
    if(m_pDC == NULL) {
        FTS18N("InvParam", MsgType::Horror, "StreamedConstDataContainer::StreamedConstDataContainer(NULL)");
    }
}

/// Deletes the bound data container too! But note that the data that is within
/// the bound data container won't be deleted, as it is a const data container !
FTS::StreamedConstDataContainer::~StreamedConstDataContainer()
{
}

/** Binds a new data container to the streamed data container. This will loose
 *  control over the old data container (and delete it, but not its data!) and
 *  gain control over the data container passed as an argument.
 *  Also this will rewind the cursor.
 *
 * \param out_pDC The new data container to take control over.
 */
void FTS::StreamedConstDataContainer::bindDC(const DataContainer *in_pDC)
{
//     if(in_pDC == NULL) {
//         FTS18N("InvParam", FTS_HORROR, "StreamedConstDataContainer::bindDC(NULL)");
//         return ;
//     }

    m_pDC = in_pDC;

    this->setCursorPos(0);
}

/// \return True if the data is invalid (for example non existent).
bool FTS::StreamedConstDataContainer::invalid() const
{
    return m_pDC == NULL || m_pDC->getData() == NULL;
}

/// \return True if the end of data has been reached, false else.
bool FTS::StreamedConstDataContainer::eod() const
{
    return this->invalid() || this->getCursorPos() >= m_pDC->getSize();
}

/** This places the cursor at an arbitrary position in the file data. If you
 *  try to place it behind the end of file, it will be placed at the end and
 *  a subsequent call to \a eof will return true.
 *
 * \param in_uiPos The position where to place the cursor.
 *
 * \return The old position of the cursor.
 */
uint64_t FTS::StreamedConstDataContainer::setCursorPos(uint64_t in_uiPos)
{
    uint64_t uiOldPos = this->getCursorPos();
    m_uiDataCursor = std::min(m_pDC == NULL ? 0 : m_pDC->getSize(), in_uiPos);
    return uiOldPos;
}

/// \return A pointer that points into the data at exactly the place the cursor
///         currently resides. The pointer is const, meaning read-only.
const void *FTS::StreamedConstDataContainer::getDataAtCursorPos() const
{
    if(this->invalid() || this->eod())
        return NULL;

    return &m_pDC->getData()[this->getCursorPos()];
}

/// \return How much bytes there are still left from the cursor's position until
///         the end of the data.
/// \note getCursorPos + getSizeTillEnd = getDataSize
uint64_t FTS::StreamedConstDataContainer::getSizeTillEnd() const
{
    if(this->invalid() || this->eod())
        return 0;

    return m_pDC->getSize() - this->getCursorPos();
}
