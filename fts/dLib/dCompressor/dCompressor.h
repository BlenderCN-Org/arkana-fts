/**
 * \file dCompressor.h
 * \author Pompei2
 * \date 19 Feb 2009
 * \brief This file contains an interface that any compressor must fullfill.
 **/

#ifndef D_COMPRESSOR_H
#define D_COMPRESSOR_H

#include <list>
#include <memory>

#include "main.h"
#include "main/Exception.h"
#include "dLib/dString/dString.h"
#include "utilities/StreamedDataContainer.h"
#include "utilities/Singleton.h"

namespace FTS {
    class Compressor;

/// This exception shall be inherited by any other exception somehow related to
/// the compressors. I know this is bad style, but it's to handle the formatting
/// of the error string.
class CompressorException : public virtual LoggableException {
    const Compressor* m_pComp;
protected:
    CompressorException(const Compressor* in_pCompr, BaseLoggerCmd* in_pLogger) throw();
public:
    CompressorException(const Compressor* in_pCompr, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Error) throw();
    inline const Compressor *getCompressor() const {return m_pComp;};
};

/// This is the exception for any failure during the compression. The reason can
/// be extracted from the compressor itself.
class CompressionException : public CompressorException {
public:
    CompressionException(const Compressor* in_pCompr, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Error) throw();
};

/// This is the generic exception for any failure during the decompression.
/// The reason can be extracted from the compressor itself.
class DecompressionException : public CompressorException {
public:
    DecompressionException(const Compressor* in_pCompr, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Error) throw();
};

/// I hear you scream: HE IS CRAZY!! (I stop capslock here, but you still scream)
/// He used the evil evil diamond shape!! w00t w00t!!\n
/// I just say: in some \e rare cases, the diamond shape is in fact the only
/// correct way of doing something. This is one such rare case, because
/// our \a CorruptDecompressionDataException really \e is both a
/// \a DecompressionException and a \a CorruptDataException, both of them are
/// unrelated \e but both of them are \a LoggableException s (somehow related).
/// But note that this is only one \a LoggableException thanks to the use of
/// virtual inheritance used for its two bases.
/// And I \e want to have a different construction of the base \a LoggableException.
class CorruptDecompressionDataException : public DecompressionException, public CorruptDataException {
public:
    CorruptDecompressionDataException(const Compressor* in_pCompr, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Error) throw();
};

/// This is the base-class (interface) for any compressor. Any new compressor
/// should implement this interface to achieve high integration.\n
/// Unlike the name might suggest, a compressor is also used to decompress.
class Compressor {
public:
    /// Auto-pointer to a compressor.
    typedef std::unique_ptr<Compressor> Ptr;

private:
    /// Protect against copying.
    Compressor(const Compressor &) {};
protected:
    /// Protect against construction.
    Compressor() {};
public:
    /// Default destructor.
    virtual ~Compressor() {};

    /// \return a short name for your compressor.
    virtual String getName() const = 0;
    /// \return a longer description of your compressor.
    virtual String getDescription() const = 0;
    /// \return a string describing the last occured problem. Empty string if
    ///         there has never ever been a problem.
    virtual String getLastProblem() const = 0;

    /// This method should check if a DataContainer contains data that may be
    /// compressed using this compressor.
    /// \param in_pData A pointer to the DataContainer that has to be checked.
    /// \return true if the data may be compressed using this compressor, false else.
    virtual bool isMyType(const DataContainer * const in_pData) const = 0;

    /// \return A copy of the object that can be used to compress and has the
    ///         same parameters set.
    virtual Compressor::Ptr copy() const = 0;

    /// This method should automagically decompress all the data it gets.
    /// \param out_where A streamed data container where you want to put the
    ///                 decompressed data into.
    /// \param in_pData The data that has to be decompressed.
    /// \exception CompressorException or any of its subtypes.
    /// \note If an error occurs, \a out_where should be left intact.
    virtual void decompress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const = 0;
    /// This method should automagically compress all the data it gets.
    /// \note If your compressor needs some more options (like compression level, ...)
    ///       they have to be passed in another way (for example in the constructor).
    ///       This method should compress the data with the "default" level.
    /// \note This method may (and should) store some additional data in the
    ///       compressed result to identify its compressed data format, for example.
    /// \note If an error occurs, \a out_where should be left intact.
    /// \param out_where A streamed data container where the decompressed data
    ///                  will be written behind the cursor. The cursor will be
    ///                  placed right behind the compressed data.
    /// \param in_pData The data that has to be compressed.
    /// \return true if the compressed data is less in size then the original data,
    ///         false if the data was uncompressible (thus is bigger in the
    ///         compressed form then in the uncompressed form).
    virtual bool compress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const = 0;
};

/// This is a dummy compressor implementation that does no compression at all,
/// just copies the data. It is used for testing and for better coding.
/// Always recognizes data as being his type.
class NoCompressor : public Compressor {
public:
    NoCompressor() {};
    virtual ~NoCompressor() {};

    virtual String getName() const {return "NoCompressor";};
    virtual String getDescription() const {return "Does no compression at all";};
    virtual String getLastProblem() const {return String();};

    virtual bool isMyType(const DataContainer * const in_pData) const {return true;};
    virtual Compressor::Ptr copy() const {return Compressor::Ptr(new NoCompressor);};

    virtual void decompress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const;
    virtual bool compress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const;
};

class CompressorFactory : public LazySingleton<CompressorFactory> {
public:
    CompressorFactory();
    virtual ~CompressorFactory();

    inline Compressor::Ptr getDefault() const {return m_pDefault->copy();};
    Compressor::Ptr create(const String &in_sTypeName) const;
    Compressor::Ptr determine(const DataContainer * const in_pData) const;
    std::list< std::pair<String, String> > list() const;

protected:
    std::list<Compressor *>m_lpCompressors;
    Compressor *m_pDefault;
};
};

#endif // D_COMPRESSOR_H
