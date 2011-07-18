#ifndef D_STREAMED_DATA_CONTAINER_H
#define D_STREAMED_DATA_CONTAINER_H

#include "main.h"

namespace FTS {
    class DataContainer;
    class RawDataContainer;
    class ConstRawDataContainer;
    class String;

/// This is an abstract base class that must be implemented by anything that
/// wants to be treated as a readable stream. That means that it can be read
/// from and has a cursor that advances after every read. Your implementation
/// must only provide these pure virtual methods, all read code resides in this
/// class.
class ReadableStream {
protected:
    /// Protect against direct creation.
    ReadableStream() {};

    size_t readRaw(void *out_ptr, size_t in_size);

    // These are the methods that need to be implemented by a class that wants
    // to act as a readable stream.

    virtual const void *getDataAtCursorPos() const = 0;

public:
    virtual ~ReadableStream() {};

    size_t read(void *out_ptr, size_t in_size, size_t in_nmemb);
    size_t readNoEndian(void *out_ptr, size_t in_size);
    size_t readNoEndian(RawDataContainer &out_data);

    // The following are written WITHOUT the use of templates with intention!
    // That is to force you use these types to be compatible 64bit and so one...

    /// Reads one 8 bit (1 byte) signed integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(int8_t &out) {this->read(&out, sizeof(int8_t), 1); return *this;};
    /// Reads one 8 bit (1 byte) signed integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline int8_t readi8() {int8_t out; this->read(out); return out;};
    /// Reads one 8 bit (1 byte) unsigned integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(uint8_t &out) {this->read(&out, sizeof(uint8_t), 1); return *this;};
    /// Reads one 8 bit (1 byte) unsigned integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline uint8_t readui8() {uint8_t out; this->read(out); return out;};
    /// Reads one 16 bit (2 bytes) signed integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(int16_t &out) {this->read(&out, sizeof(int16_t), 1); return *this;};
    /// Reads one 16 bit (2 bytes) signed integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline int16_t readi16() {int16_t out; this->read(out); return out;};
    /// Reads one 16 bit (2 bytes) unsigned integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(uint16_t &out) {this->read(&out, sizeof(uint16_t), 1); return *this;};
    /// Reads one 16 bit (2 bytes) unsigned integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline uint16_t readui16() {uint16_t out; this->read(out); return out;};
    /// Reads one 32 bit (4 bytes) signed integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(int32_t &out) {this->read(&out, sizeof(int32_t), 1); return *this;};
    /// Reads one 32 bit (4 bytes) signed integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline int32_t readi32() {int32_t out; this->read(out); return out;};
    /// Reads one 32 bit (4 bytes) unsigned integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(uint32_t &out) {this->read(&out, sizeof(uint32_t), 1); return *this;};
    /// Reads one 32 bit (4 bytes) unsigned integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline uint32_t readui32() {uint32_t out; this->read(out); return out;};
    /// Reads one 64 bit (8 bytes) signed integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(int64_t &out) {this->read(&out, sizeof(int64_t), 1); return *this;};
    /// Reads one 64 bit (8 bytes) signed integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline int64_t readi64() {int64_t out; this->read(out); return out;};
    /// Reads one 64 bit (8 bytes) unsigned integer from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(uint64_t &out) {this->read(&out, sizeof(uint64_t), 1); return *this;};
    /// Reads one 64 bit (8 bytes) unsigned integer from the file.
    /// \return The read data
    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline uint64_t readui64() {uint64_t out; this->read(out); return out;};
    /// Reads one 32 bit (4 bytes) floating point value from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(float &out) {this->readNoEndian(&out, sizeof(float)); return *this;};
    /// Reads one 32 bit (4 bytes) floating point value from the file.
    /// \return The read data
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline float readf() {float out; this->read(out); return out;};
    /// Reads one 64 bit (8 bytes) floating point value from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(double &out) {this->readNoEndian(&out, sizeof(double)); return *this;};
    /// Reads one 64 bit (8 bytes) floating point value from the file.
    /// \return The read data
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline double readd() {double out; this->read(out); return out;};
    /// Reads one 128 bit (16 bytes) floating point value from the file.
    /// \param out The variable where to store the read data.
    /// \return this
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline ReadableStream& read(long double &out) {this->readNoEndian(&out, sizeof(long double)); return *this;};
    /// Reads one 128 bit (16 bytes) floating point value from the file.
    /// \return The read data
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    inline long double readld() {long double out; this->read(out); return out;};

    ReadableStream& read(String &out);
    String readstr();

    // These are the methods that need to be implemented by a class that wants
    // to act as a readable stream.

    virtual bool invalid() const = 0;
    virtual bool eod() const = 0;
    virtual uint64_t getCursorPos() const = 0;
    virtual uint64_t setCursorPos(uint64_t in_uiCursorPos) = 0;
    virtual uint64_t getSizeTillEnd() const = 0;
};

/// This is an abstract base class that must be implemented by anything that
/// wants to be treated as a writeable stream. That means that it can be written
/// to and has a cursor that advances after every write. Your implementation
/// must only provide these pure virtual methods, all write code resides in this
/// class.
/// \TODO Get this going, like in the file class.
class WriteableStream {
protected:
    /// Protect against direct creation.
    WriteableStream() {};

    int insert(const void *in_ptr, size_t in_size);
    int overwrite(const uint8_t *in_ptr, size_t in_size);

    // These are the methods that need to be implemented by a class that wants
    // to act as a readable stream.

    virtual const void *getDataAtCursorPos() const = 0;
    virtual int moveAndResizeData(uint64_t in_uiFrom, uint64_t in_uiSize, int64_t in_iOffset) = 0;
    virtual void memcpyToCursorPos(const ConstRawDataContainer &in_dc) = 0;

public:
    virtual ~WriteableStream() {};

    size_t insert(const void *in_ptr, size_t in_size, size_t in_nmemb);
    size_t insertNoEndian(const void *in_ptr, size_t in_size);
    size_t insertNoEndian(const DataContainer &in_data);
    size_t overwrite(const void *in_ptr, size_t in_size, size_t in_nmemb);
    size_t overwriteNoEndian(const void *in_ptr, size_t in_size);
    size_t overwriteNoEndian(const DataContainer &in_data);

    // The following are written WITHOUT the use of templates with intention!
    // That is to force you use these types to be compatible 64bit and so one...

    /// Inserts one 8 bit (1 byte) signed integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(int8_t in) {return this->insert(&in, sizeof(int8_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 8 bit (1 byte) unsigned integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(uint8_t in) {return this->insert(&in, sizeof(uint8_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 16 bit (2 byte) signed integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(int16_t in) {return this->insert(&in, sizeof(int16_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 16 bit (2 byte) unsigned integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(uint16_t in) {return this->insert(&in, sizeof(uint16_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 32 bit (4 byte) signed integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(int32_t in) {return this->insert(&in, sizeof(int32_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 32 bit (4 byte) unsigned integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(uint32_t in) {return this->insert(&in, sizeof(uint32_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 64 bit (8 byte) signed integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(int64_t in) {return this->insert(&in, sizeof(int64_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 64 bit (8 byte) unsigned integer into the data.
    /// \param in The integer to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int insert(uint64_t in) {return this->insert(&in, sizeof(uint64_t), 1) == 1 ? ERR_OK : -1;};
    /// Inserts one 32 bit (4 byte) floating point value into the data.
    /// \param in The value to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    inline int insert(float in) {return this->insertNoEndian(&in, sizeof(float));};
    /// Inserts one 64 bit (8 byte) floating point value into the data.
    /// \param in The value to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    inline int insert(double in) {return this->insertNoEndian(&in, sizeof(double));};
    /// Inserts one 128 bit (16 byte) floating point value into the data.
    /// \param in The value to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    inline int insert(long double in) {return this->insertNoEndian(&in, sizeof(long double));};

    int insert(const String &in);

    /// Overwrites one 8 bit (1 byte) signed integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(int8_t in) {return this->overwrite(&in, sizeof(int8_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 8 bit (1 byte) unsigned integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(uint8_t in) {return this->overwrite(&in, sizeof(uint8_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 16 bit (2 byte) signed integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(int16_t in) {return this->overwrite(&in, sizeof(int16_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 16 bit (2 byte) unsigned integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(uint16_t in) {return this->overwrite(&in, sizeof(uint16_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 32 bit (4 byte) signed integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(int32_t in) {return this->overwrite(&in, sizeof(int32_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 32 bit (4 byte) unsigned integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(uint32_t in) {return this->overwrite(&in, sizeof(uint32_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 64 bit (8 byte) signed integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(int64_t in) {return this->overwrite(&in, sizeof(int64_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 64 bit (8 byte) unsigned integer in the data.
    /// \param in The integer to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(uint64_t in) {return this->overwrite(&in, sizeof(uint64_t), 1) == 1 ? ERR_OK : -1;};
    /// Overwrites one 32 bit (4 byte) floating point value in the data.
    /// \param in The value to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(float in) {return this->overwriteNoEndian(&in, sizeof(float));};
    /// Overwrites one 64 bit (8 byte) floating point value in the data.
    /// \param in The value to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(double in) {return this->overwriteNoEndian(&in, sizeof(double));};
    /// Overwrites one 128 bit (16 byte) floating point value in the data.
    /// \param in The value to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    inline int overwrite(long double in) {return this->overwriteNoEndian(&in, sizeof(long double));};

    int overwrite(const String &in);

    // These are the methods that need to be implemented by a class that wants
    // to act as a readable stream.

    virtual bool invalid() const = 0;
    virtual bool eod() const = 0;
    virtual uint64_t getCursorPos() const = 0;
    virtual uint64_t setCursorPos(uint64_t in_uiCursorPos) = 0;
    virtual uint64_t getSizeTillEnd() const = 0;
};

/// This class represents a fully streamed data container, that means it acts as
/// a data container (thus deleting its data when being deleted) and it can be
/// read from, using the \a ReadableStream interface, and it can also be written
/// to it, using the \a WriteableStream interface.
class StreamedDataContainer : public ReadableStream, public WriteableStream {
    /// The const data container that is bound to this object.
    RawDataContainer *m_pDC;

    /// The actual position of the read/write cursor in the data stream.
    uint64_t m_uiDataCursor;

protected:
    virtual const void *getDataAtCursorPos() const;
    virtual int moveAndResizeData(uint64_t in_uiFrom, uint64_t in_uiSize, int64_t in_iOffset);
    virtual void memcpyToCursorPos(const ConstRawDataContainer &in_dc);

public:
    StreamedDataContainer();
    StreamedDataContainer(uint64_t in_uiSize);
    StreamedDataContainer(RawDataContainer *out_pDC);
    StreamedDataContainer(const StreamedDataContainer& o);
    virtual ~StreamedDataContainer();

    /// \return the data container that currently is under control.
    inline const RawDataContainer *getBoundDC() const {return m_pDC;};
    void bindDC(RawDataContainer *out_pDC);
    RawDataContainer* unbindDC();

    void grow(uint64_t in_uiAdditional);
    void shrink(uint64_t in_uiLess);
    uint8_t *getBufferInFrontOfCursor();

    virtual bool invalid() const;
    virtual bool eod() const;

    /// \return The current position of the cursor.
    virtual uint64_t getCursorPos() const {return m_uiDataCursor;};
    virtual uint64_t setCursorPos(uint64_t in_uiCursorPos);
    /// Moves the cursor's position relatively to the current position.
    /// \param in_uiOffset The offset to move the cursor around. Negative moves
    ///                the cursor backwards, positive moves the cursor forwards.
    /// \return The old position of the cursor.
    inline uint64_t moveCursor(int64_t in_uiOffset) {return this->setCursorPos(this->getCursorPos() + in_uiOffset);};
    virtual uint64_t getSizeTillEnd() const;

    uint32_t fletcher32FromCursor() const;
};

/// This class represents a readonly streamed const data container, that means
/// it acts as a const data container (thus NOT deleting its data when being
/// deleted, like CONST data container do) and it can be read from, using
/// the \a ReadableStream interface, but it cannot be written to.\n
/// That means the data it contains will never get changed.
class StreamedConstDataContainer : public ReadableStream {
    /// The const data container that is bound to this object.
    const DataContainer *m_pDC;

    /// The actual position of the read/write cursor in the data stream.
    uint64_t m_uiDataCursor;

protected:
    virtual const void *getDataAtCursorPos() const;

public:
    StreamedConstDataContainer(const DataContainer *in_pDC);
    virtual ~StreamedConstDataContainer();

    /// \return The data container that currently is under control.
    inline const DataContainer *getBoundDC() const {return m_pDC;};
    void bindDC(const DataContainer *in_pDC);

    virtual bool invalid() const;
    virtual bool eod() const;

    /// \return The current position of the cursor.
    virtual uint64_t getCursorPos() const {return m_uiDataCursor;};
    virtual uint64_t setCursorPos(uint64_t in_uiCursorPos);
    /// Moves the cursor's position relatively to the current position.
    /// \param in_uiOffset The offset to move the cursor around. Negative moves
    ///                the cursor backwards, positive moves the cursor forwards.
    /// \return The old position of the cursor.
    inline uint64_t moveCursor(int64_t in_uiOffset) {return this->setCursorPos(this->getCursorPos() + in_uiOffset);};
    virtual uint64_t getSizeTillEnd() const;
};

};

#endif // D_STREAMED_DATA_CONTAINER_H
