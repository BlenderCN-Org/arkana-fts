#ifndef D_FTSARC_SFCOMPRESSOR_H
#define D_FTSARC_SFCOMPRESSOR_H

#include "archiver.h"

namespace FTS {
    class Compressor;
}

namespace FTSArc {

class SingleFileCompressor : public ArchiverBase {
public:
    /// The default output file name if none was specified.
    static FTS::Path defaultOutName() {return "{1}{2}.ftscmp";};
    virtual FTS::Path getDefaultOutName() const {return this->defaultOutName();};

    SingleFileCompressor(const FTS::Path &in_sOutPat, FTS::Compressor::Ptr in_pComp = FTS::Compressor::Ptr(new FTS::NoCompressor), bool in_bRecurse = false);
    virtual ~SingleFileCompressor();

    virtual int execute();
};

}

#endif // D_FTSARC_SFCOMPRESSOR_H
