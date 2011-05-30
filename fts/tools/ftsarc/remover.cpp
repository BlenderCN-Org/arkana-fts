#include "remover.h"
#include "logging/logger.h"
#include "dLib/dArchive/dArchive.h"
#include "dLib/dCompressor/dCompressor.h"

using namespace FTSArc;
using namespace FTS;

Remover::Remover(const String &in_sArcName)
    : m_sArcName(in_sArcName)
{
}

Remover::~Remover()
{
}

int Remover::execute()
{
    FTSMSG("Removing the following objects from archive {1}: ", FTS::MsgType::Raw, m_sArcName);
    for(FileList::iterator i = m_lFilesToHandle.begin() ; i != m_lFilesToHandle.end() ; ++i) {
        FTSMSG(*i + " ");
    }
    FTSMSG("\n");

    // Open the archive first.
    Archive *a = Archive::loadArchive(m_sArcName);
    if(a == NULL)
        return -1;

    // Now remove every chunk that is wanted.
    for(FileList::iterator iFile = m_lFilesToHandle.begin() ; iFile != m_lFilesToHandle.end() ; ++iFile) {
        FTSMSG("  " + *iFile + " ... ");
        Chunk *pChk = a->take(*iFile);
        if(pChk != NULL) {
            SAFE_DELETE(pChk);
            FTSMSG("removed\n");
        } else {
            FTSMSG("not present in archive (enter the full name, names are case sensitive)\n");
        }
    }

    // Save the whole archive again.
    FTSMSG("Saving ");
    FTSMSG("(and compressing with {1}) ", FTS::MsgType::Raw, a->getOriginalCompressor().getName());
    FTSMSG("the whole archive ... ");
    a->save();
    FTSMSG("Done\n");

    SAFE_DELETE(a);
    return ERR_OK;
}
