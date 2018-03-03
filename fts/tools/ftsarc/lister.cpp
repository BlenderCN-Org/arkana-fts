#include "lister.h"
#include "logging/logger.h"
#include "dLib/dArchive/dArchive.h"

using namespace FTSArc;
using namespace FTS;

Lister::Lister()
{
}

Lister::~Lister()
{
}

int Lister::execute()
{
    FTSMSG("Listing following archives: ");
    for(FileList::iterator i = m_lFilesToHandle.begin() ; i != m_lFilesToHandle.end() ; ++i) {
        FTSMSG(*i + " ");
    }
    FTSMSG("\n");

    for(FileList::iterator iFile = m_lFilesToHandle.begin() ; iFile != m_lFilesToHandle.end() ; ++iFile) {
        try {
            FTSMSG("\nArchive " + *iFile + ":\n");
            Archive::Ptr a(Archive::loadArchive(*iFile));
            for(Archive::ChunkMap::const_iterator iChunk = a->begin() ; iChunk != a->end() ; ++iChunk) {
                this->printChunk(iChunk->second);
            }
        } catch(const ArkanaException& e) {
            e.show();
        }
    }

    return ERR_OK;
}

void Lister::printChunk(const Chunk *in_pChunk)
{
    String sType = "["+String::chr(in_pChunk->getTypeName()[0])+"] ";
    double fSize = (double)in_pChunk->getPayloadLength();
    String sSize;
    if(fSize / 1024.f < 1.f) {
        sSize = "(" + String::nr(fSize, 2) + " B)";
    } else if(fSize / (1024.f*1024.f) < 1.f) {
        sSize = "(" + String::nr(fSize/1024.f, 2) + " KB)";
    } else if(fSize / (1024.f*1024.f*1024.f) < 1.f) {
        sSize = "(" + String::nr(fSize/(1024.f*1024.f), 2) + " MB)";
    } else {
        sSize = "(" + String::nr(fSize/(1024.f*1024.f*1024.f), 2) + " GB)";
    }
    for(size_t i = sSize.len() ; i < 13 ; i++) {
        sSize += " ";
    }

    FTSMSG(sType + sSize + in_pChunk->getName() + "\n");
}
