#ifndef D_FTSARC_ARCHIVER_H
#define D_FTSARC_ARCHIVER_H

#include "ftsarc.h"
#include "dLib/dCompressor/dCompressor.h"
#include "logging/logger.h"

namespace FTSArc {

class ArchiverBase : public ExecutionMode {
protected:
    /// The name of the file to create
    FTS::Path m_sOutName;
    /// The compressor to use for the whole archive file.
    const FTS::Compressor::Ptr m_pComp;
    /// Recurse into subdirectories or not.
    bool m_bRecurse;
    /// Do not ask any question, always suppose yes as answer.
    bool m_bYesToAll;

    bool addDirectoryRecursive(const FTS::Path &in_sDir);

public:
    /// The default output file name if none was specified.
    virtual FTS::Path getDefaultOutName() const = 0;

    ArchiverBase(const FTS::Path& in_sOutName, FTS::Compressor::Ptr in_pComp = FTS::Compressor::Ptr(new FTS::NoCompressor), bool in_bRecurse = false);
    virtual ~ArchiverBase();

    virtual int execute() = 0;

    inline void yesToAll() {m_bYesToAll = true;};

    virtual void addFileToHandle(const FTS::Path& in_sFileName) {
        if(in_sFileName == "-") {
            m_bYesToAll = true;
            FTSMSG("Warning: enabled 'always yes' mode because you want to read"
                   " a file from the standard input. I won't ask any questions!",
                   FTS::MsgType::Warning);
        }

        return ExecutionMode::addFileToHandle(in_sFileName);
    }
};

class Archiver : public ArchiverBase {
public:
    /// The default output file name if none was specified.
    static FTS::Path defaultOutName() {return "out.ftsarc";};
    virtual FTS::Path getDefaultOutName() const {return this->defaultOutName();};

    Archiver(const FTS::Path &in_sOutName, FTS::Compressor::Ptr in_pComp = FTS::Compressor::Ptr(new FTS::NoCompressor), bool in_bRecurse = false);
    virtual ~Archiver();

    virtual int execute();
};

}

#endif // D_FTSARC_ARCHIVER_H
