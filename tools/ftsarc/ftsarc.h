#ifndef D_FTSARC_H
#define D_FTSARC_H

#include <list>

#include "dLib/dString/dPath.h"

namespace FTSArc {

class ExecutionMode {
    /// Protect from copying.
    ExecutionMode(const ExecutionMode &) {};

protected:
    /// Protect from creation.
    ExecutionMode() {};

    typedef std::list<FTS::Path> FileList;

    /// A list of files to handle. Do what you want with that.
    FileList m_lFilesToHandle;

public:
    /// Default destructor.
    virtual ~ExecutionMode() {};

    virtual int execute() = 0;

    /// \param in_sFileName The name of a file to add to the list of files to handle.
    virtual void addFileToHandle(const FTS::Path& in_sFileName) {
        m_lFilesToHandle.push_back(in_sFileName);
    };
};

}

#endif // D_FTSARC_H
