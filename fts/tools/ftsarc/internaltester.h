#ifndef D_FTSARC_INTERNALTESTER_H
#define D_FTSARC_INTERNALTESTER_H

#include "ftsarc.h"

class Chunk;

namespace FTSArc {

class InternalTester : public ExecutionMode {
public:
    InternalTester();
    virtual ~InternalTester();

    int execute();
};

}

#endif // D_FTSARC_INTERNALTESTER_H
