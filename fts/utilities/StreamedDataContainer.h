#ifndef D_STREAMED_DATA_CONTAINER_H
#define D_STREAMED_DATA_CONTAINER_H

#include <type_traits>

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

    /// \note This method assumes the file is stored little-endian on the disk.
    ///       That is the format Arkana-FTS and its tools save all files on disk.
    /// \note To see if there was an error during the read (end of file reached)
    ///       check the eof method after the call to read.
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr >
    ReadableStream& read( T& out )
    {
        this->read( &out, sizeof( T ), 1 );
        return *this;
    }

    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr >
    T read()
    {
        T out;
        this->read( &out, sizeof( T ), 1 );
        return out;
    }

    ReadableStream& read(String &out);
    String readstr();

    // These are the methods that need to be implemented by a class that wants
    // to act as a readable stream.

    virtual bool invalid() const = 0;
    virtual bool eod() const = 0;
    virtual std::uint64_t getCursorPos() const = 0;
    virtual std::uint64_t setCursorPos( std::uint64_t in_uiCursorPos) = 0;
    virtual std::uint64_t getSizeTillEnd() const = 0;
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
    int overwrite(const std::uint8_t *in_ptr, size_t in_size);

    // These are the methods that need to be implemented by a class that wants
    // to act as a readable stream.

    virtual const void *getDataAtCursorPos() const = 0;
    virtual int moveAndResizeData(std::uint64_t in_uiFrom, std::uint64_t in_uiSize, std::int64_t in_iOffset) = 0;
    virtual void memcpyToCursorPos(const ConstRawDataContainer &in_dc) = 0;

public:
    virtual ~WriteableStream() {};

    size_t insert(const void *in_ptr, size_t in_size, size_t in_nmemb);
    size_t insertNoEndian(const void *in_ptr, size_t in_size);
    size_t insertNoEndian(const DataContainer &in_data);
    size_t overwrite(const void *in_ptr, size_t in_size, size_t in_nmemb);
    size_t overwriteNoEndian(const void *in_ptr, size_t in_size);
    size_t overwriteNoEndian(const DataContainer &in_data);


    /// Inserts one value into the data.
    /// \param in The arithmetic data to insert.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    int insert( T in ) 
    { 
        return this->insert( &in, sizeof( T ), 1 ) == 1 ? ERR_OK : -1; 
    }


    int insert(const String &in);

    /// Overwrites one value in the data.
    /// \param in The arithmetic data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr >
    int overwrite( T in ) 
    { 
        return this->overwrite( &in, sizeof( T ), 1 ) == 1 ? ERR_OK : -1; 
    }

    int overwrite(const String &in);

    // These are the methods that need to be implemented by a class that wants
    // to act as a readable stream.

    virtual bool invalid() const = 0;
    virtual bool eod() const = 0;
    virtual std::uint64_t getCursorPos() const = 0;
    virtual std::uint64_t setCursorPos( std::uint64_t in_uiCursorPos) = 0;
    virtual std::uint64_t getSizeTillEnd() const = 0;
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
    virtual int moveAndResizeData( std::uint64_t in_uiFrom, std::uint64_t in_uiSize, std::int64_t in_iOffset);
    virtual void memcpyToCursorPos(const ConstRawDataContainer &in_dc);

public:
    StreamedDataContainer();
    StreamedDataContainer( std::uint64_t in_uiSize);
    StreamedDataContainer(RawDataContainer *out_pDC);
    StreamedDataContainer(const StreamedDataContainer& o);
    virtual ~StreamedDataContainer();

    /// \return the data container that currently is under control.
    inline const RawDataContainer *getBoundDC() const {return m_pDC;};
    void bindDC(RawDataContainer *out_pDC);
    RawDataContainer* unbindDC();

    void grow( std::uint64_t in_uiAdditional);
    void shrink( std::uint64_t in_uiLess);
    uint8_t *getBufferInFrontOfCursor();

    virtual bool invalid() const;
    virtual bool eod() const;

    /// \return The current position of the cursor.
    virtual uint64_t getCursorPos() const {return m_uiDataCursor;};
    virtual uint64_t setCursorPos( std::uint64_t in_uiCursorPos);
    /// Moves the cursor's position relatively to the current position.
    /// \param in_uiOffset The offset to move the cursor around. Negative moves
    ///                the cursor backwards, positive moves the cursor forwards.
    /// \return The old position of the cursor.
    inline uint64_t moveCursor( std::int64_t in_uiOffset) {return this->setCursorPos(this->getCursorPos() + in_uiOffset);};
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
    virtual std::uint64_t getCursorPos() const {return m_uiDataCursor;};
    virtual std::uint64_t setCursorPos( std::uint64_t in_uiCursorPos);
    /// Moves the cursor's position relatively to the current position.
    /// \param in_uiOffset The offset to move the cursor around. Negative moves
    ///                the cursor backwards, positive moves the cursor forwards.
    /// \return The old position of the cursor.
    inline std::uint64_t moveCursor( std::int64_t in_uiOffset) {return this->setCursorPos(this->getCursorPos() + in_uiOffset);};
    virtual std::uint64_t getSizeTillEnd() const;
};

};

#endif // D_STREAMED_DATA_CONTAINER_H
