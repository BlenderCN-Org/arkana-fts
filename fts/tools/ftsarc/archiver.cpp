#include "archiver.h"

#include <string>

#include "logging/logger.h"
#include "dLib/dArchive/dArchive.h"
#include "dLib/dCompressor/dCompressor.h"
#include "dLib/dBrowse/dBrowse.h"

using namespace FTSArc;
using namespace FTS;

ArchiverBase::ArchiverBase(const FTS::Path& in_sOutName, Compressor::Ptr in_pComp, bool in_bRecurse)
    : m_sOutName(in_sOutName)
    , m_pComp(std::move(in_pComp))
    , m_bRecurse(in_bRecurse)
    , m_bYesToAll(false)
{
}

ArchiverBase::~ArchiverBase()
{
}

bool ArchiverBase::addDirectoryRecursive(const FTS::Path& in_sDir)
{
    // Check if the current "file" is a directory.
    PDBrowseInfo pdbi = dBrowse_Open(in_sDir, false);
    if(pdbi == NULL || !m_bRecurse)
        return false;

    // It is a directory and we want to recurse into it.
    for(FTS::Path sElem = dBrowse_GetNext(pdbi) ; !sElem.empty() ; sElem = dBrowse_GetNext(pdbi)) {
#if !WINDOOF
        // If it is hidden, do not add it.
        if(sElem.getCharAt(0) == '.')
            continue;
#endif
        switch(dBrowse_GetType(pdbi)) {
        // If it is a file, add it to the file list
        case DB_FILE:
            m_lFilesToHandle.push_back(in_sDir.appendWithSeparator(sElem));
            break;
        // If it is a directory, recurse into it again.
        case DB_DIR:
            addDirectoryRecursive(in_sDir.appendWithSeparator(sElem));
            break;
        default: continue;
        }
    }

    dBrowse_Close(pdbi);
    return true;
}

Archiver::Archiver(const FTS::Path& in_sOutName, Compressor::Ptr in_pComp, bool in_bRecurse)
    : ArchiverBase(in_sOutName.empty() ? Archiver::defaultOutName() : in_sOutName,
                   std::move(in_pComp),
                   in_bRecurse)
{
}

Archiver::~Archiver()
{
}

int Archiver::execute()
{
    Archive *pArchive = NULL;

    if(FileUtils::fileExists(m_sOutName)) {
        // If it is a fts archive file, we will add files to it, thus we first
        // need to open the file.
        if(Archive::isValidArchive(m_sOutName)) {
            if(!m_bYesToAll) {
                FTSMSG(m_sOutName + " already exists and is a valid FTS archive. Insert the files into it (i), overwrite it (o) or cancel (c) ?");
                std::string sAnswer;
                std::cin >> sAnswer;
                if(sAnswer[0] == 'i') {
                    pArchive = Archive::loadArchive(m_sOutName);
                } else if(sAnswer[0] == 'o') {
                    pArchive = Archive::createEmptyArchive(m_sOutName, m_pComp.get());
                } else {
                    FTSMSG("Ok, ciao.");
                    return ERR_OK;
                }
            } else {
                pArchive = Archive::loadArchive(m_sOutName);
            }
        } else {
            // If it ain't a fts archive, we ask the user if we should overwrite it.
            if(!m_bYesToAll) {
                FTSMSG("Overwrite " + m_sOutName + " ? ");
                std::string sAnswer;
                std::cin >> sAnswer;
                if(sAnswer[0] != 'y') {
                    FTSMSG("Ok, ciao.");
                    return ERR_OK;
                }
            }
            pArchive = Archive::createEmptyArchive(m_sOutName, m_pComp.get());
        }
    } else {
        // Create a new archive.
        pArchive = Archive::createEmptyArchive(m_sOutName, m_pComp.get());
    }

    FTSMSG("Archiving the following files: \n");
    for(FileList::iterator i = m_lFilesToHandle.begin() ; i != m_lFilesToHandle.end() ; ++i) {
        // If the current entry is a directory and files were added because of
        // recursing into the directory, we need to go to the next iterator.
        if(this->addDirectoryRecursive(*i)) {
            continue;
        }

        FTSMSG(*i + "... ");

        try {
            // It is a file, try to load it into memory.
            File::Ptr pFile = File::open(*i, File::Read);

            // If such a file is already present in the archive, ask if we should
            // overwrite it or ignore it.
            if(pArchive->getChunk(pFile->getName()) != NULL) {
                if(!m_bYesToAll) {
                    FTSMSG(pFile->getName() + " is already present in the archive, overwrite it ? (y/n) ");
                    std::string sAnswer;
                    std::cin >> sAnswer;
                    if(sAnswer[0] == 'y') {
                        // Remove the file from the archive.
                        Chunk *pChk = pArchive->take(pFile->getName());
                        SAFE_DELETE(pChk);
                    } else {
                        FTSMSG("Skipping\n");
                        continue;
                    }
                } else {
                    // Remove the file from the archive.
                    Chunk *pChk = pArchive->take(pFile->getName());
                    SAFE_DELETE(pChk);
                }
            }

            // Not present in the archive yet, add it to the archive.
            pArchive->give(new FileChunk(std::move(pFile)));
            FTSMSG("Done\n");
        } catch(const ArkanaException& e) {
            e.show();
        }
    }

    FTSMSG("Saving ");
    if(m_pComp.get() != NULL) {
        FTSMSG("(and compressing with {1}) ", FTS::MsgType::Raw, m_pComp->getName());
    }
    FTSMSG("the whole archive ... ");
    pArchive->save();
    FTSMSG("Done\n");

    SAFE_DELETE(pArchive);
    return ERR_OK;
}
