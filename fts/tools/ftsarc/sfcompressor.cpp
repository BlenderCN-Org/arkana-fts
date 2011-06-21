#include "sfcompressor.h"

#include <string>

#include "logging/logger.h"
#include "dLib/dFile/dFile.h"

using namespace FTSArc;
using namespace FTS;

SingleFileCompressor::SingleFileCompressor(const Path& in_sOutPat, Compressor::Ptr in_pComp, bool in_bRecurse)
    : ArchiverBase(in_sOutPat.empty() ? SingleFileCompressor::defaultOutName() : in_sOutPat,
                   std::move(in_pComp),
                   in_bRecurse)
{
}

SingleFileCompressor::~SingleFileCompressor()
{
}

int SingleFileCompressor::execute()
{
    FTSMSG("Compressing the following files: \n");
    for(FileList::iterator i = m_lFilesToHandle.begin() ; i != m_lFilesToHandle.end() ; ++i) {
        // If the current entry is a directory and files were added because of
        // recursing into the directory, we need to go to the next iterator.
        if(this->addDirectoryRecursive(*i)) {
            continue;
        }

        FTSMSG(*i + " -> ");

        // Determine the output file name.
        Path sOutFile = String(m_sOutName).fmtRemoveEmpty(i->directory() + FTS_DIR_SEPARATOR,
                                                          i->basename().withoutExt(),
                                                          i->ext());

        FTSMSG(sOutFile + " ... ");

        try {
            // It is a file, try to load it into memory.
            File::Ptr pFile = File::open(*i, File::Read);

            // If such a file is already present on the disk, ask if we should
            // overwrite it or ignore it.
            if(FileUtils::fileExists(sOutFile)) {
                if(!m_bYesToAll) {
                    FTSMSG("A file named " + sOutFile + " already exists, overwrite it ? (y/n) ");
                    std::string sAnswer;
                    std::cin >> sAnswer;
                    if(sAnswer[0] != 'y') {
                        FTSMSG("Skipping\n");
                        continue;
                    }
                }
            }

            // Create (and maybe overwrite) the file
            pFile->saveAs(sOutFile, File::OverwriteFile, *m_pComp);
            FTSMSG("Done\n");
        } catch(const ArkanaException& e) {
            e.show();
        }
    }

#if 0
    FTSMSG("Saving ");
    if(m_pComp != NULL) {
        FTSMSG("(and compressing with {1}) ", FTS::MsgType::Raw, m_pComp->getName());
    }
    FTSMSG("the whole archive ... ");
    pArchive->save(m_sOutName, File::Overwrite, m_pComp);
    FTSMSG("Done\n");
#endif

    return ERR_OK;
#if 0
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
                    pArchive = Archive::createEmptyArchive();
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
            pArchive = Archive::createEmptyArchive();
        }
    } else {
        // Create a new archive.
        pArchive = Archive::createEmptyArchive();
    }

    FTSMSG("Archiving following files: \n");
    for(FileList::iterator i = m_lFilesToHandle.begin() ; i != m_lFilesToHandle.end() ; ++i) {
        String sCleanPath = i->clearPath();

        // If the current entry is a directory and files were added because of
        // recursing into the directory, we need to go to the next iterator.
        if(this->addDirectoryRecursive(sCleanPath)) {
            continue;
        }

        FTSMSG(sCleanPath + "... ");

        // It is a file, try to load it into memory.
        File *pFile = new File(sCleanPath);
        if(ERR_OK != pFile->load()) {
            SAFE_DELETE(pFile);
            continue;
        }

        // If such a file is already present in the archive, ask if we should
        // overwrite it or ignore it.
        if(pArchive->getChunk(pFile->getFileName()) != NULL) {
            if(!m_bYesToAll) {
                FTSMSG(pFile->getFileName() + " is already present in the archive, overwrite it ? (y/n) ");
                std::string sAnswer;
                std::cin >> sAnswer;
                if(sAnswer[0] == 'y') {
                    Chunk *pChk = pArchive->take(pFile->getFileName());
                    SAFE_DELETE(pChk);
                } else {
                    FTSMSG("Skipping\n");
                    continue;
                }
            } else {
                Chunk *pChk = pArchive->take(pFile->getFileName());
                SAFE_DELETE(pChk);
            }
        }

        // Not present in the archive yet, add it to the archive.
        pArchive->give(pFile);
        FTSMSG("Done\n");
    }

    FTSMSG("Saving ");
    if(m_pComp != NULL) {
        FTSMSG("(and compressing with {1}) ", FTS::MsgType::Raw, m_pComp->getName());
    }
    FTSMSG("the whole archive ... ");
    pArchive->save(m_sOutName, File::Overwrite, m_pComp);
    FTSMSG("Done\n");

    SAFE_DELETE(pArchive);
    return ERR_OK;
#endif
}
