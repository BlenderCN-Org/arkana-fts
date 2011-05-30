#include "dLib/aTest/TestHarness.h"

#include "dLib/dFile/dFile.h"
#include "dLib/dArchive/dArchive.h"

using namespace FTS;

SUITE(dFileArchive);

class ArchiveSetup : public TestSetup {
public:
    void setup()
    {
        File::Ptr pFile1 = File::overwrite("dummy.file", File::Insert);
        pFile1->write("Hello, Moto!");
        pFile1->write(10.0);
        File::Ptr pFile2 = File::overwrite("dummy.file2", File::Insert);
        pFile2->write("Bye, Moto!");
        pFile2->write(25L);
        m_pArch = Archive::createEmptyArchive("dummy.ftsarc");
        m_pArch->give(new FileChunk(std::move(pFile1)));
        m_pArch->give(new FileChunk(std::move(pFile2)));
    }
    void teardown()
    {
        SAFE_DELETE(m_pArch);
    }
protected:
	Archive* m_pArch;
};

TEST_INSUITE_WITHSETUP(dFileArchive, Archive, LoadSaveFromArchiveToLook)
{
    File::addArchiveToLook(m_pArch);

    // First, check if we can read from the archive's file correctly.
	File::Ptr pFile = File::open("dummy.file", File::Insert);
	CHECK_EQUAL("Hello, Moto!", pFile->readstr());
    CHECK_DOUBLES_EQUAL(10.0, pFile->readd());

    // This should write the file back into the archive.
    pFile->write("Bye, Moto!");
    pFile->save();

    // Re-read that file
    pFile.reset();
    pFile = File::open("dummy.file", File::Read);
	CHECK_EQUAL("Hello, Moto!", pFile->readstr());
    CHECK_DOUBLES_EQUAL(10.0, pFile->readd());
    
    // And especially check if that change is still in!
	CHECK_EQUAL("Bye, Moto!", pFile->readstr());
}
