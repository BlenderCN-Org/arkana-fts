#include "dearchiver.h"

#include <string>

#include "logging/logger.h"
#include "dLib/dArchive/dArchive.h"

using namespace FTSArc;
using namespace FTS;

const String Dearchiver::m_sDefaultOutDir = "./";

Dearchiver::Dearchiver(const String &in_sOutDir)
    : m_sOutDir(in_sOutDir)
    , m_bYesToAll(false)
{
}

Dearchiver::~Dearchiver()
{
}

int Dearchiver::execute()
{
    FTSMSG("Dearchiving following archives: ");
    for(FileList::iterator i = m_lFilesToHandle.begin() ; i != m_lFilesToHandle.end() ; ++i) {
        FTSMSG(*i + " ");
    }
    FTSMSG("\ninto " + m_sOutDir + "\n");

    for(FileList::iterator iFile = m_lFilesToHandle.begin() ; iFile != m_lFilesToHandle.end() ; ++iFile) {
        try {
            Archive::Ptr a(Archive::loadArchive(*iFile));
            FTSMSG("\nArchive " + *iFile + ":\n");
            for(Archive::ChunkMap::const_iterator iChunk = a->begin() ; iChunk != a->end() ; ++iChunk) {
                Path sOutName = m_sOutDir + "/" + iChunk->first;

                // If the output directory is "-", we print to stdout.
                if(m_sOutDir == "-") {
                    sOutName = "-";
                }

                FTSMSG("  " + iChunk->first + " -> " + sOutName + " ... ");
                // If it is a file chunk, check if it already exists and then ask
                // if it should be overwritten
                if(!m_bYesToAll && FileUtils::fileExists(sOutName)) {
                    FTSMSG("Overwrite ? ");
                    std::string s;
                    std::cin >> s;
                    if(s[0] != 'y') {
                        FTSMSG("Ok, skipping.\n");
                        continue;
                    }
                }
                if(ERR_OK == iChunk->second->execute(sOutName))
                    FTSMSG("done\n");
                else
                    FTSMSG("failed (bug?)\n");
            }

            continue;
        } catch(...) { }
        // Ignore any exceptions, and check if it is a single compressed file.
        try {
            FTSMSG("\nFile " + *iFile);
            File::Ptr f = File::open(*iFile, File::Read);
            Path sOutName = m_sOutDir + "/" + *iFile;

            // If the output directory is "-", we print to stdout.
            if(m_sOutDir == "-") {
                sOutName = "-";
            }

            FTSMSG(" -> " + sOutName + " ... ");

            if(!m_bYesToAll && FileUtils::fileExists(sOutName)) {
                FTSMSG("Overwrite ? ");
                std::string s;
                std::cin >> s;
                if(s[0] != 'y') {
                    FTSMSG("Ok, skipping.\n");
                    continue;
                }
            }

            f->saveAs(sOutName, File::OverwriteFile, NoCompressor());
            FTSMSG("Done.\n");
        } catch(const ArkanaException& e) {
            e.show();
        }
    }

    return ERR_OK;
}
