#ifndef D_FTSARC_REMOVER_H
#define D_FTSARC_REMOVER_H

#include "ftsarc.h"

namespace FTSArc {

class Remover : public ExecutionMode {
    /// The name of the archive to remove files from.
    FTS::String m_sArcName;

public:
    Remover(const FTS::String &in_sArcName);
    virtual ~Remover();

    int execute();
};

}

#endif // D_FTSARC_REMOVER_H
