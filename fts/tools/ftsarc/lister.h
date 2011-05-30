#ifndef D_FTSARC_LISTER_H
#define D_FTSARC_LISTER_H

#include "ftsarc.h"

namespace FTS {
    class Chunk;
}

namespace FTSArc {

class Lister : public ExecutionMode {
    void printChunk(const FTS::Chunk *in_pChunk);

public:
    Lister();
    virtual ~Lister();

    int execute();
};

}

#endif // D_FTSARC_LISTER_H
