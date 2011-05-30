#include "dCompressor.h"

#ifndef D_NOMINILZO
#  include "dLib/dCompressor/minilzo_compressor.h"
#endif

#include "logging/logger.h"

using namespace FTS;

CompressorException::CompressorException(const Compressor* in_pCompr, BaseLoggerCmd* in_pLogger) throw()
    : LoggableException(in_pLogger)
    , m_pComp(in_pCompr)
{
}

CompressorException::CompressorException(const Compressor* in_pCompr, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("Compressor_"+in_pCompr->getName(), in_gravity, in_pCompr->getLastProblem()))
    , m_pComp(in_pCompr)
{
}

CompressionException::CompressionException(const Compressor* in_pCompr, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("Compressor_"+in_pCompr->getName()+"_Comp", in_gravity, in_pCompr->getLastProblem()))
    , CompressorException(in_pCompr, in_gravity)
{
}

DecompressionException::DecompressionException(const Compressor* in_pCompr, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("Compressor_"+in_pCompr->getName()+"_Decomp", in_gravity, in_pCompr->getLastProblem()))
    , CompressorException(in_pCompr, in_gravity)
{
}

CorruptDecompressionDataException::CorruptDecompressionDataException(const Compressor* in_pCompr, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("Compressor_"+in_pCompr->getName()+"_CorruptData", in_gravity, in_pCompr->getLastProblem())) // This is the base!
    , DecompressionException(in_pCompr, in_gravity)
    , CorruptDataException("??", in_pCompr->getLastProblem(), in_gravity)
{
}

void NoCompressor::decompress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const
{
    out_where.insertNoEndian(*in_pData);
}

bool NoCompressor::compress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const
{
    out_where.insertNoEndian(*in_pData);
    return true;
}

/// Default constructor. Creates an instance of every compressor.
CompressorFactory::CompressorFactory()
{
    m_pDefault = NULL;

#ifndef D_NOMINILZO
    m_pDefault = new MiniLZOCompressor;
    m_lpCompressors.push_back(m_pDefault);
#endif

    // This one has to be the last in the list because it recognizes every data
    // as his own type.
    if(m_pDefault == NULL) {
        m_pDefault = new NoCompressor;
        m_lpCompressors.push_back(m_pDefault);
    } else {
        m_lpCompressors.push_back(new NoCompressor);
    }
}

/// Default destructor. Deletes everything created by the constructor :)
CompressorFactory::~CompressorFactory()
{
    while(!m_lpCompressors.empty()) {
        delete m_lpCompressors.front();
        m_lpCompressors.pop_front();
    }
}

/** Creates a new instance of the compressor specified in the parameter.
 *
 * \param in_sTypeName The name of the compressor to create.
 *
 * \return A new instance of the compressor specified by its name. Returns an
 *         instance of NoCompressor if there is no compressor named like this in
 *         the system.
 */
Compressor::Ptr CompressorFactory::create(const String &in_sTypeName) const
{
    for(std::list<Compressor *>::const_iterator i = m_lpCompressors.begin() ; i != m_lpCompressors.end() ; ++i) {
        if(in_sTypeName == (*i)->getName()) {
            return (*i)->copy();
        }
    }

    return Compressor::Ptr(new NoCompressor);
}

/** Determines what compressor has to be used to decompress the given data.
 *  If no known compression is determined, this returns an instance of the
 *  NoCompressor. If some compression is recognized, it returns a new instance
 *  of the corresponding decompressor that the caller will need to delete.
 *
 * \param in_pData The data to analyze.
 *
 * \return A new instance of the compressor that is needed to decompress the
 *         data or an instance of the NoCompressor if nothing was recognized.
 */
Compressor::Ptr CompressorFactory::determine(const DataContainer * const in_pData) const
{
    for(std::list<Compressor *>::const_iterator i = m_lpCompressors.begin() ; i != m_lpCompressors.end() ; ++i) {
        if((*i)->isMyType(in_pData)) {
            return (*i)->copy();
        }
    }

    // Should never come here anyway!
    return Compressor::Ptr(new NoCompressor);
}

/// \return A list of every compressor's name along with his description.
std::list< std::pair<String, String> > CompressorFactory::list() const
{
    std::list< std::pair<String, String> >lRet;

    for(std::list<Compressor *>::const_iterator i = m_lpCompressors.begin() ; i != m_lpCompressors.end() ; ++i) {
        lRet.push_back(std::make_pair((*i)->getName(), (*i)->getDescription()));
    }

    return lRet;
}
