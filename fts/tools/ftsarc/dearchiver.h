#ifndef D_FTSARC_DEARCHIVER_H
#define D_FTSARC_DEARCHIVER_H

#include "ftsarc.h"

namespace FTSArc {

class Dearchiver : public ExecutionMode {
    /// The directory where to dearchive.
    FTS::String m_sOutDir;

    /// Do not ask any question, always suppose yes as answer.
    bool m_bYesToAll;

public:
    Dearchiver(const FTS::String &in_sOutDir = m_sDefaultOutDir);
    virtual ~Dearchiver();

    virtual int execute();

    inline void yesToAll() {m_bYesToAll = true;};

    /// The default output directory name.
    static const FTS::String m_sDefaultOutDir;
};

}

#endif // D_FTSARC_DEARCHIVER_H
