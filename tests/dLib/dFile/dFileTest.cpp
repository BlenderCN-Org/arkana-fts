#include "dLib/aTest/TestHarness.h"

#include "dLib/dFile/dFile.h"
#include "logging/MinimalLogger.h"
#include <fstream>
#include <experimental/filesystem>

using namespace FTS;
using namespace std;
namespace fs = std::experimental::filesystem;

SUITE(dFile);

class FileSetup : public TestSetup {
public:
    void setup()
    {
        auto f = ofstream("dummy.file");
        f << "0123456789";
    }
    void teardown()
    {
        fs::remove("dummy.file");
    }
protected:
};

TEST_INSUITE_WITHSETUP(dFile, File, getSize)
{
    auto actual = File::getSize("dummy.file");
    CHECK_EQUAL(10, actual);
}

TEST_INSUITE_WITHSETUP(dFile, File, invalid_file_name)
{
    try {
        auto actual = File::getSize("invalid.file");
        FAIL("Expected a file system exception");
    } catch(FTS::SyscallException& se) {
        CHECK_EQUAL("SyscallFailure", FTS::String(se.what()).left(14));
    }
}

TEST_INSUITE_WITHSETUP(dFile, File, mk_dir_if_needed)
{
    // Create a new directory
    auto cwd = fs::current_path() / fs::path("tmp");
    auto rc = FileUtils::mkdirIfNeeded(Path(cwd.string()), false);
    CHECK_EQUAL(ERR_OK, rc);
    CHECK_EQUAL(true, fs::exists(cwd));
    fs::remove(cwd);

    // Create a directory w/ the file, but don't indicate it.
    auto dummy_file = cwd / "dummy.file";
    rc = FileUtils::mkdirIfNeeded(Path(dummy_file.string()), false);
    CHECK(ERR_OK == rc);
    CHECK_EQUAL(true, fs::exists(cwd));
    CHECK_EQUAL(true, fs::exists(dummy_file));
    fs::remove_all(cwd);

    // Create a directory but input contains a file part, too.
    rc = FileUtils::mkdirIfNeeded(Path(dummy_file.string()), true);
    CHECK_EQUAL(ERR_OK, rc);
    CHECK_EQUAL(true, fs::exists(cwd));
    CHECK_EQUAL(false, fs::exists(dummy_file));
    fs::remove(cwd);

    // Create a already existing directory.
    fs::create_directory(cwd);
    rc = FileUtils::mkdirIfNeeded(Path(cwd.string()), false);
    CHECK_EQUAL(ERR_OK, rc);
    CHECK_EQUAL(true, fs::exists(cwd));
    fs::remove_all(cwd);

    // Invalid path name.
    auto pDefLogger = new MinimalLogger(); // Needed by the error message generation.
    rc = FileUtils::mkdirIfNeeded(Path(), false);
    CHECK_EQUAL(-1, rc);
    rc = FileUtils::mkdirIfNeeded(Path("/,/:"), false); // Should fail on Linux, too.
    CHECK_EQUAL(-5, rc);
    delete pDefLogger;

    // The ignored . .. path names
    rc = FileUtils::mkdirIfNeeded(Path("."), false);
    CHECK_EQUAL(ERR_OK, rc);
    rc = FileUtils::mkdirIfNeeded(Path(".."), false);
    CHECK_EQUAL(ERR_OK, rc);
}

TEST_INSUITE_WITHSETUP(dFile, File, rm_dir)
{
    auto cwd = fs::current_path() / fs::path("tmp");
    CHECK(fs::create_directory(cwd));
    auto rc = FileUtils::rmdir(Path(cwd.string()));
    CHECK_EQUAL(ERR_OK, rc);
    CHECK_EQUAL(false, fs::exists(cwd));

    auto dummy_file = cwd / "dummy";
    CHECK(fs::create_directories(dummy_file));
    rc = FileUtils::rmdir(Path(cwd.string()));
    CHECK(ERR_OK == rc);
    CHECK_EQUAL(false, fs::exists(dummy_file));
    CHECK_EQUAL(false, fs::exists(cwd));

    // the remove doesn't complain if the path doesn't exist.
    rc = FileUtils::rmdir(Path(cwd.string()));
    CHECK(ERR_OK == rc);
}

TEST_INSUITE(dFile, dir_exist)
{
    auto cwd = fs::current_path() / fs::path("tmp");
    auto rc = FileUtils::dirExists(Path(cwd.string()));
    CHECK_EQUAL(false, rc);

    CHECK(fs::create_directory(cwd));
    rc = FileUtils::dirExists(Path(cwd.string()));
    CHECK_EQUAL(true, rc);

    rc = FileUtils::dirExists(Path("."));
    CHECK_EQUAL(true,rc);

    rc = FileUtils::dirExists(Path(".."));
    CHECK_EQUAL(true, rc);

    rc = FileUtils::dirExists(Path(".."));
    CHECK_EQUAL(true, rc);

    fs::remove(cwd);
}

TEST_INSUITE_WITHSETUP(dFile, File, copy_file)
{
    auto cwd = fs::current_path() / fs::path("tmp");
    auto from = fs::current_path() / "dummy.file";
    auto to = cwd / "dummy.file";
    fs::create_directory(cwd);

    FileUtils::fileCopy(from.string(), to.string());
    CHECK(fs::exists(to));

    // Overwrite the file
    auto f = ofstream("dummy.file");
    f << "abcdef";
    f.close();

    FileUtils::fileCopy(from.string(), to.string());
    CHECK(fs::exists(to));
    std::string in_data;
    auto in = ifstream("./tmp/dummy.file");
    in >> in_data;
    in.close();
    CHECK_EQUAL("abcdef", in_data);

    fs::remove_all(cwd);
}

TEST_INSUITE_WITHSETUP(dFile, File, copy_file_no_overwrite)
{
    auto cwd = fs::current_path() / fs::path("tmp");
    auto from = fs::current_path() / "dummy.file";
    auto to = cwd / "dummy.file";
    fs::create_directory(cwd);

    auto f = ofstream("./tmp/dummy.file");
    f << "abcdef";
    f.close();
    FileUtils::fileCopy(from.string(), to.string(), false);
    CHECK(fs::exists(to));

    std::string in_data;
    auto in = ifstream("./tmp/dummy.file");
    in >> in_data;
    in.close();
    CHECK_EQUAL("abcdef", in_data);

    fs::remove_all(cwd);
}