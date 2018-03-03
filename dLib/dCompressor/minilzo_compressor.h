/**
 * \file minilzo_compressor.h
 * \author Pompei2
 * \date 19 Feb 2009
 * \brief This file contains the definition of the minilzo compressor.
 **/

#ifndef D_MINILZO_COMPRESSOR_H
#define D_MINILZO_COMPRESSOR_H

#include "dLib/dCompressor/dCompressor.h"
#include "dLib/dCompressor/minilzo/minilzo.h"

namespace FTS {

/// This is the implementation of a compressor using the LZO algorithm provided
/// by the miniLZO library (see minilzo.h for legal stuff).
class MiniLZOCompressor : public Compressor {
    /// This memory is needed by the miniLZO to work in during compression.
    /// It is allocated per-object instead of statically to provide thread-safety.
    mutable lzo_align_t __LZO_MMODEL *m_pWorkingMemory;

    /// This is used to initialise the miniLZO library when the first object is
    /// constructed.
    static bool m_bMiniLZOInited;

    /// A description of the last probem that occured.
    mutable String m_sLastProblem;

protected:
    virtual String getHeaderID() const;

public:
    MiniLZOCompressor();
    virtual ~MiniLZOCompressor();

    virtual String getName() const;
    virtual String getDescription() const;
    virtual String getLastProblem() const {return m_sLastProblem;};

    virtual bool isMyType(const DataContainer * const in_pData) const;
    virtual Compressor::Ptr copy() const {return Compressor::Ptr(new MiniLZOCompressor);};

    virtual void decompress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const;
    virtual bool compress(StreamedDataContainer& out_where, const DataContainer * const in_pData) const;
};
};

#endif // D_MINILZO_COMPRESSOR_H

