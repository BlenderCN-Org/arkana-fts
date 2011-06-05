#define TOOLNAME "ftsarc"
#define TOOLVERSIONSTR "0.1"

#include "../toolcompat.h"

#include "lister.h"
#include "remover.h"
#include "archiver.h"
#include "dearchiver.h"
#include "sfcompressor.h"
#include "internaltester.h"
#include "compressorlister.h"

#include "dLib/dCompressor/dCompressor.h"
#include "dLib/dArchive/dArchive.h"

using namespace FTSArc;
using namespace FTS;

void usage()
{
    FTSMSG("-----\n");
    FTSMSG("Usage for archiving: {1} [-s] [-o OUTFILE] [-c COMPR] [-R] [-y] FILE_1 [FILE_2 ... FILE_N]\n", FTS::MsgType::Raw, TOOLNAME);
    FTSMSG("  -o OUTFILE    name the created archive file OUTPUT. For example -o bla.ftsarc\n");
    FTSMSG("                will name the created archive bla.ftsarc\n");
    FTSMSG("                If this is not specified, the name will default to {1}\n", FTS::MsgType::Raw, Archiver::defaultOutName());
    FTSMSG("  -c COMPR      The name of the compressor to be used to do a\n");
    FTSMSG("                whole archive compression. Please note that this\n");
    FTSMSG("                may not always be the best thing to do.\n");
    FTSMSG("                Omitting this option will not compress the archive.\n");
    FTSMSG("                To see all available compressors, use the -lc option.\n");
    FTSMSG("                Example: -c MiniLZO.\n");
    FTSMSG("  -R            Recurse into subdirectories\n");
    FTSMSG("  -y            Answer yes to all questions automatically (e.g. overwrite files).\n");
    FTSMSG("  -s            If this option is set, it means that the archiver won't create\n");
    FTSMSG("                an archive, but it will compress every single file.\n");
    FTSMSG("                The OUTFILE should then not be a simple filename but rather\n");
    FTSMSG("                it is a pattern on how to name the output files.\n");
    FTSMSG("                The following strings will be replaced in the pattern:\n");
    FTSMSG("                   * {1} will be replaced by the path to the file, with trailing slash\n");
    FTSMSG("                   * {2} will be replaced by the file name, w/o extension\n");
    FTSMSG("                   * {3} will be replaced by the file's extension, w/o dot\n");
    FTSMSG("                Thus for the input file /foo/bar/file.name.baba, the following\n");
    FTSMSG("                replacements would be made:\n");
    FTSMSG("                   * {1} would be replaced by /foo/bar/\n");
    FTSMSG("                   * {2} would be replaced by file.name\n");
    FTSMSG("                   * {3} would be replaced by baba\n");
    FTSMSG("                If not specified, the pattern will default to {1}\n", FTS::MsgType::Raw, SingleFileCompressor::defaultOutName());
    FTSMSG("  Note: If the specified archive already exists, files are added to it.\n");
    FTSMSG("        Be aware that the original whole-archive compression is\n");
    FTSMSG("        NOT restored if you omit the -c option.\n");
    FTSMSG("  Note: You can write a - as OUTFILE, the output will be redirected to stdout.\n");
    FTSMSG("        Writing a - as FILE_1 will read the input from stdin.\n");
    FTSMSG("-----\n");
    FTSMSG("Usage for dearchiving: {1} [-d DIRECTORY] [-y] ARCHIVE_1 [ARCHIVE_2 .. ARCHIVE_N]\n", FTS::MsgType::Raw, TOOLNAME);
    FTSMSG("  -d DIRECTORY  will use the given DIRECTORY as base directory for the\n");
    FTSMSG("                dearchivation, meaning files will go into that directory.\n");
    FTSMSG("                By default, the current directory ({1}) is used.\n", FTS::MsgType::Raw, Dearchiver::m_sDefaultOutDir);
    FTSMSG("                Specifying - as the directory will print all to stdout.\n");
    FTSMSG("  -y            Answer yes to all questions automatically (e.g. overwrite files).\n");
    FTSMSG("  Note that ARCHIVE_x can also be a single compressed file.\n");
    FTSMSG("  It will then be decompressed in-place (or into DIRECTORY).\n");
    FTSMSG("-----\n");
    FTSMSG("Usage for listing contents of archive: {1} -l ARCHIVE_1 [ARCHIVE_2 .. ARCHIVE_N]\n", FTS::MsgType::Raw, TOOLNAME);
    FTSMSG("  Lists the contents of the given archive.\n");
    FTSMSG("-----\n");
    FTSMSG("Usage for removing objects from an archive: {1} -r ARCHIVE OBJECT_1 [OBJECT_2 ... OBJECT_N]\n", FTS::MsgType::Raw, TOOLNAME);
    FTSMSG("  -r ARCHIVE    name of the archive to remove the objects from.\n");
    FTSMSG("                For example -r bla.ftsarc removes the objects from the\n");
    FTSMSG("                archive named bla.ftsarc\n");
    FTSMSG("-----\n");
    FTSMSG("Usage for listing compressors: {1} -lc\n", FTS::MsgType::Raw, TOOLNAME);
    FTSMSG("  Litsts all the compressors you may currently use.\n");
    FTSMSG("-----\n");
    FTSMSG("Usage for an internal test: {1} -it\n", FTS::MsgType::Raw, TOOLNAME);
    FTSMSG("  Runs an internal test to check if the main functionality is OK.\n");
    FTSMSG("-----\n");
    FTSMSG("You may also give {1} just a (list of) file(s) as an argument,\n", FTS::MsgType::Raw, TOOLNAME);
    FTSMSG("it will then, if all arguments are either compressed or an archive,\n");
    FTSMSG("decompress/extract all of them into the default directory ({1}).\n", FTS::MsgType::Raw, Dearchiver::m_sDefaultOutDir);
    FTSMSG("If at least one of the files is neither compressed nor an archive,\n");
    FTSMSG("ALL given files are put together into an archive called {1}\n", FTS::MsgType::Raw, Archiver::defaultOutName());
    FTSMSG("-----\n");
}

bool allArgsAreArchivesOrCompressed(int argc, char *argv[])
{
    NoCompressor noComp;

    for(int i = 0 ; i < argc ; i++) {
        if(Archive::isValidArchive(argv[i]))
            continue;

        // This one is neither an archive nor is it compressed
        try {
            if(!File::open(argv[i], File::Read)->wasCompressed())
                return false;
        } catch(...) {
            // an invalid file is neither an archive, nor is it compressed!
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    FTSTools::init();

    // Init the logging system.
    new FTSTools::MinimalLogger(2);
    FTSMSG("Arkana-FTS {1} Version {2}\n", FTS::MsgType::Raw, TOOLNAME, TOOLVERSIONSTR);
    for(int i = 0 ; i < argc ; ++i) {
        FTSMSG("\"{1}\" ", FTS::MsgType::Raw, argv[i]);
    }
    FTSMSG("\n", FTS::MsgType::Raw);
#if 0
    String s = "/bla.conf.bla/conf.bla.bli";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "/bla.conf.bla/conf.bla.";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "/bla.conf.bla/.conf.bla.";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "/bla.conf.bla/.conf.bla.bli";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "/bla.conf.bla/.conf";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "/bla.conf.bla/conf";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = ".";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = ".a";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "a.";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "a.b";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = "..";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
    s = ".a.";
    FTSMSG("{1}: base = {4}, ext = {2}, noExt = {3}\n", FTS_NOMSG, s, s.ext(), s.noExt(), s.basename());
#endif
    new FTS::CompressorFactory();

    if(argc <= 1) {
        usage();
        return 0;
    }

    ExecutionMode *exe = NULL;
    int nArgsHandled = 1;

    // If some argument is given, detect that.
    if(argv[1][0] == '-'){
        // We want to create an archive or add files to an archive with the
        // following name.
        if((argv[1][1] == 'o' || argv[1][1] == 'c' || argv[1][1] == 's') && argv[1][2] == '\0') {
            int argpos = 1;
            String sOutName;
            Compressor::Ptr pComp(new NoCompressor);
            bool bRec = false;
            bool bYesToAll = false;
            bool bCompressSingleFile = false;

            // Parse the -o and -c
            while(argpos < argc && argv[argpos][0] == '-') {
                if(argv[argpos][1] == 'o') {
                    if(argc < argpos + 2) {
                        FTSMSG("You said you want to create/add to an archive (-o),"
                               "but you didn't tell me the name of the archive.\n"
                               "Do so by adding the name after the -o option,\n"
                               "separated by a space. For example: -o bla.ftsarc");
                        return 1;
                    }
                    sOutName = argv[argpos+1];
                    nArgsHandled += 2;
                    argpos += 2;
                } else if(argv[argpos][1] == 'c') {
                    if(argc < argpos + 2) {
                        FTSMSG("You didn't tell me what compressor you want to use.\n"
                               "Do so by adding the compressor's name after the -c option,\n"
                               "separated by a space. For example: -c MiniLZO", FTS::MsgType::Error);
                        return 1;
                    }
                    pComp.reset();
                    pComp = CompressorFactory::getSingletonPtr()->create(argv[argpos+1]);
                    if(pComp->getName() != argv[argpos+1]) {
                        FTSMSG("I don't know the compressor you gave me. Type\n"
                               "{1} -lc to see a list of all compressors.", FTS::MsgType::Error, argv[0]);
                        return 1;
                    }
                    nArgsHandled += 2;
                    argpos += 2;
                } else if(argv[argpos][1] == 'R') {
                    bRec = true;
                    nArgsHandled++;
                    argpos++;
                } else if(argv[argpos][1] == 'y') {
                    bYesToAll = true;
                    nArgsHandled++;
                    argpos++;
                } else if(argv[argpos][1] == 's') {
                    bCompressSingleFile = true;
                    nArgsHandled++;
                    argpos++;
                } else {
                    break;
                }
            }

            if(argc < argpos+1) {
                FTSMSG("You gave me nothing to put into the archive O.o Wanna test me?", FTS::MsgType::Error);
                return 1;
            }

            ArchiverBase *arc = NULL;
            if(bCompressSingleFile)
                arc = new SingleFileCompressor(sOutName, std::move(pComp), bRec);
            else
                arc = new Archiver(sOutName, std::move(pComp), bRec);

            if(bYesToAll) {
                arc->yesToAll();
            }
            exe = arc;
        // We want to dearchive an archive into the following directory.
        } else if((argv[1][1] == 'd' || argv[1][1] == 'y') && argv[1][2] == '\0') {
            int argpos = 1;
            String sOutDir = Dearchiver::m_sDefaultOutDir;
            bool bYesToAll = false;

            // Parse the -d and -y
            while(argv[argpos][0] == '-') {
                if(argv[argpos][1] == 'd') {
                    if(argc < argpos + 2) {
                        FTSMSG("You said you want to extract the archive into a specific"
                               "location (-d) but did not tell me that location.\n"
                               "Do so by adding the location after the -d option,"
                               "separated by a space. For example: -d foo/bar", FTS::MsgType::Error);
                        return 1;
                    }
                    sOutDir = argv[argpos+1];
                    nArgsHandled += 2;
                    argpos += 2;
                } else if(argv[argpos][1] == 'y') {
                    bYesToAll = true;
                    nArgsHandled++;
                    argpos++;
                }
            }

            if(argc < argpos+1) {
                FTSMSG("You want to extract an archive into a directory, but you"
                       " did not specify such an archive", FTS::MsgType::Error);
                return 1;
            }

            Dearchiver *d = new Dearchiver(sOutDir);
            if(bYesToAll)
                d->yesToAll();
            exe = d;
        } else if(argv[1][1] == 'l' && argv[1][2] == '\0') {
            if(argc <= 2) {
                FTSMSG("You didn't tell me what archive you want to list.\n"
                       "Do so by adding the archive file name after the -l option,"
                       "separated by a space. For example: -l bla.ftsarc", FTS::MsgType::Error);
                return 1;
            }
            exe = new Lister();
            nArgsHandled += 1;
        } else if(argv[1][1] == 'r' && argv[1][2] == '\0') {
            if(argc <= 2) {
                FTSMSG("You didn't tell me what archive you want to remove objects from.\n"
                       "Do so by adding the archive file name after the -r option,"
                       "separated by a space. For example: -r bla.ftsarc", FTS::MsgType::Error);
                return 1;
            }
            exe = new Remover(argv[2]);
            nArgsHandled += 2;
        } else if(argv[1][1] == 'l' && argv[1][2] == 'c' && argv[1][3] == '\0') {
            exe = new CompressorLister();
            nArgsHandled += 1;
        } else if(argv[1][1] == 'i' && argv[1][2] == 't' && argv[1][3] == '\0' ) {
            exe = new InternalTester();
            nArgsHandled += 1;
        } else {
            usage();
            return 0;
        }
    } else {
        // If there is no option, just a list of files, we check if ALL files in
        // the list are archives, then we dearchive them all. If at least one
        // file is not an archive, we put them all into the archive with the
        // default name and no compression.
        // The latter is useful so windobe users can just drag&drop files on the
        // exe file in the explorer, those newbs who don't know commandlines :)
        if(allArgsAreArchivesOrCompressed(argc-1, argv+1)) {
            exe = new Dearchiver;
        } else {
            exe = new Archiver(String()); // Default out file name.
        }
    }

    for(int i = nArgsHandled ; i < argc ; i++) {
        exe->addFileToHandle(argv[i]);
    }

    int iRet = exe->execute();
    delete exe;
    return iRet;
}
