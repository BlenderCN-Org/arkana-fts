#ifndef D_FTSARC_COMPRESSOR_LISTER_H
#define D_FTSARC_COMPRESSOR_LISTER_H

#include "ftsarc.h"

namespace FTSArc {

class CompressorLister : public ExecutionMode {
public:
    CompressorLister();
    virtual ~CompressorLister();

    int execute();
};

}

#endif // D_FTSARC_COMPRESSOR_LISTER_H
