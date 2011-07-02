/**
 * \file dFile.cpp
 * \author Pompei2
 * \date 6 Jan 2009
 * \brief This file contains the implementation of the file handling class.
 **/

#include "dFile.h"

#include "logging/logger.h"

#ifndef D_NONET
#  include "net/connection.h"
#endif

#ifndef D_FILE_NO_ARCHMAP
# include "dLib/dArchive/dArchive.h"
std::map<FTS::String, FTS::Archive*> FTS::File::m_lArchivesToLook;
#endif

#include <sys/stat.h>

using namespace FTS;

UnknownProtocolException::UnknownProtocolException(const Path& in_sFile) throw()
    : LoggableException(new I18nLoggerCmd("UnknownProtocol", MsgType::Error, in_sFile.protocol(), in_sFile))
{
}

FileNotExistException::FileNotExistException(const Path& in_sFile) throw()
    : LoggableException(new I18nLoggerCmd("FileNotExist", MsgType::Error, in_sFile))
{
}

FileAlreadyExistException::FileAlreadyExistException(const Path& in_sFile) throw()
    : LoggableException(new I18nLoggerCmd("FileAlreadyExist", MsgType::Error, in_sFile))
{
}

#ifndef D_FILE_NO_ARCHMAP
/** Checks if the file with the given name is located in one of the archives to
 *  look in. The archive list works kind-of like a stack: we first look in the
 *  last added archive, then in the second-last and so forth.\n
 *
 *  The file class has a list of archives that it looks into for a file before
 *  loading it from disk.
 *
 * \param in_sFile The filename to look for in the archives (with the path), for
 *                 example bla.fts or Data/bla.fts
 * \return A pointer to the archive where the file was found or NULL if the file
 *         is in no archive that has to be checked.
 */
std::pair<String, Archive*> File::isInArchive(const Path& in_sFile)
{
    // Run trough all entries that have the directory as key.
    for(std::map<String, Archive *>::iterator i = m_lArchivesToLook.begin() ; i != m_lArchivesToLook.end() ; ++i) {
        if(dynamic_cast<FileChunk *>(i->second->getChunk(in_sFile)))
            return *i;
    }

    // Nothing found? that's life...
    return std::make_pair(Path(),reinterpret_cast<FTS::Archive*>(NULL));
}
#endif

/** This constructs a FTS::File object that just makes a copy of all that data
 *  for itself and decompresses it if necessary. Only for internal use by the
 *  named constructors.
 *
 * \param in_sFileName The name of the opened file.
 * \param in_mode The write mode that should be used when writing to the file.
 * \param in_saveMode How the default file saving will be handled (overwrite existing?)
 * \param in_cont The contents to use (make a copy of and maybe decompress)
 *
 * \exception CompressorException You may get any of the compressor's exceptions
 *                                forwarded if something went wrong during
 *                                decompression of the data.
 *
 * \author Pompei2
 */
File::File(const Path& in_sFileName, const WriteMode& in_mode, const SaveMode& in_saveMode, const RawDataContainer& in_cont)
    : m_sName(in_sFileName)
    , m_mode(in_mode)
    , m_saveMode(in_saveMode)
    , m_pSDC(new StreamedDataContainer())
{
    // Decompress the data if needed and keep in mind who decompressed.
    m_pOriginalCompressor = CompressorFactory::getSingleton().determine(&in_cont);
    m_pOriginalCompressor->decompress(*m_pSDC, &in_cont);
    m_pSDC->setCursorPos(0);
}

/// \internal Only for class-use.
File::File(const File& o)
    : m_sName(o.m_sName)
    , m_mode(o.m_mode)
    , m_saveMode(o.m_saveMode)
    , m_pSDC(new StreamedDataContainer(*(o.m_pSDC)))
    , m_pOriginalCompressor(o.m_pOriginalCompressor->copy())
#ifndef D_FILE_NO_ARCHMAP
    , m_sMyArchiveID(o.m_sMyArchiveID)
#endif
{
}

/// Destroys the file object. CARE: This will not save the file before doing so.
File::~File()
{
    delete m_pSDC;
}

/** This not only looks if the file really exists on the harddisk, but it also
 *  checks its availability in an opened archive-to-be-watched or over a
 *  protocol (for example over http, if the address exists).
 *
 * \param in_sFileName The name of the file to check (optionally with protocol)
 * \param in_mode What you want to check accessibility for: only read it,
 *                modify it or replace it?
 */
bool File::available(const Path& in_sFileName, const WriteMode& in_mode)
{
    // cin and cout are easy :)
    if(in_sFileName == "-")
        return true;

    /// \todo have a protocol-system to handle http, file and maybe more.
    if(in_sFileName.protocol() != "file") {
        return false;
    }

#ifndef D_FILE_NO_ARCHMAP
    // Search in the archives
    if(File::isInArchive(in_sFileName).second != NULL) {
        return true;
    }
#endif

    return FileUtils::fileExists(in_sFileName, in_mode);
}

/** This method opens a file and loads it into memory.
 *  If the file does not exist or can not be opened for reading, an exception
 *  will be thrown. The file is fully loaded into memory ; any operation done
 *  on this object is done in memory, it is only flushed to disk upon calling
 *  the \a save method or a similar one.
 *
 * \param in_sFileName The path to the file to open.
 * \param in_mode How to perform operations on the file by default.
 * \param in_throw Whether to throw or not upon failure. If it doesn't throw,
 *                 an empty file is returned and all upcoming read operations
 *                 will fail.
 *
 * \return A pointer to a new file object containing the whole file. This is never NULL.
 * \exception FileNotExistException In case the file can't be opened.
 * \exception UnknownProtocolException In case you use a protocol that is not supported.
 * \exception CompressorException Something went wrong during decompression.
 * \exception SyscallException Something else went wrong...
 *
 * \note the upcoming write operations will either perform an insertion or over
 *       write the data or both (or none), depending on the \a in_mode parameter.
 *       It is still possible to enforce a different write mode later.
 * \note This is the only named constructor capable to open the standard input.
 *       If you want to read from the standard input, just open a file named "-"
 *       (without the quotes) and watch the magic :)\n
 *       In case you open the standard input, your code may block in this function
 *       if the standard input is held open, and only continue when it is released
 *       (e.g. by pressing Ctrl+Z in the console or so..)
 */
File::Ptr File::open(const Path& in_sFileName, const WriteMode& in_mode)
{
    if(in_sFileName == "-") {
        RawDataContainer rawData(0);
        // We want to read the standard input, do so, by chunks.
#define D_FILE_STDIN_CHUNK 512
        char s[D_FILE_STDIN_CHUNK] = {0};
        while(std::cin.read(s,D_FILE_STDIN_CHUNK)) {
            std::cerr << "." << std::cin.gcount();
            std::cerr.flush();
            // Append that chunk to the contianer.
            rawData.append(ConstRawDataContainer(s, std::cin.gcount()));
        }

        std::cerr << "." << std::cin.gcount();
        std::cerr.flush();
        // Process the last chunk exactly the same way.
        rawData.append(ConstRawDataContainer(s, std::cin.gcount()));

        // Now we got the whole standard input in our buffer, fine!
        return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, rawData));
    /// \todo have a protocol-system to handle http, file and maybe more.
    } else if(in_sFileName.protocol() != "file") {
        // We want to open a transport protocol.
        throw UnknownProtocolException(in_sFileName);
#ifndef D_FILE_NO_ARCHMAP
    } else if(File::isInArchive(in_sFileName).second != NULL) {
        // The file we want to open is present in a registered archive.

        // Get some info about that archive where it is in.
        std::pair<String, Archive*>bla = File::isInArchive(in_sFileName);

        // We need to get the file out of the archive. We're sure the cast will
        // succeed as File::isInArchive above didn't return NULL for this one.
        FileChunk *pChk = dynamic_cast<FileChunk *>(bla.second->getChunk(in_sFileName));

        // The raw data container will be copied by the file object.
        File* pFile = new File(in_sFileName, in_mode, File::OverwriteFile, *(pChk->getRawContent()));

        // Now with that info we can later on store the file back into the archive.
        pFile->m_sMyArchiveID = bla.first;
        return File::Ptr(pFile);
#endif
    } else {
        // We want to open a regular file on the local disk.

        // Open the file.
        std::FILE *pFile = fopen(in_sFileName.c_str(), "rb");
        if(pFile == NULL) {
            throw FileNotExistException(in_sFileName);
        }

        // Try to get the size of the file.
        uint64_t uiDataLen = File::getSize(in_sFileName);

        // Create the data container that will hold the file's data.
        RawDataContainer data(uiDataLen);

        // Read the whole content of the file into the buffer.
        if(fread(data.getData(), static_cast<std::size_t>(uiDataLen), 1, pFile) != (uiDataLen == 0 ? 0 : 1)) {
            SAFE_FCLOSE(pFile);
            throw SyscallException("File::open::fread(" + in_sFileName + ")");
        }

        // The data container takes over the control over the data.
        SAFE_FCLOSE(pFile);
        return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, data));
    }
}

/** This method creates a file object from reading raw data.
 *  The file object fully resides in memory ; any operation done
 *  on this object is done in memory, it is only flushed to disk upon calling
 *  the \a save method or a similar one.
 *
 * \param in_data The data to create the file object with.
 * \param in_sFileName The path to the file to open.
 * \param in_mode How to perform operations on the file by default.
 * \param in_saveMode How a default save operation should act.
 *
 * \return An auto-pointer to a new file object containing the whole file.
 *         This is never NULL.
 * \exception CompressorException Something went wrong during decompression.
 *
 * \note the upcoming write operations will either perform an insertion or over
 *       write the data or both (or none), depending on the \a in_mode parameter.
 *       It is still possible to enforce a different write mode later.
 * \note the data-container is copied, thus it is safe for you to do whatever
 *       you want with it afterwards.
 */
File::Ptr File::fromRawData(const ConstRawDataContainer& in_data, const Path& in_sFileName, const WriteMode& in_mode, const SaveMode& in_saveMode)
{
    return File::Ptr(new File(in_sFileName, in_mode, in_saveMode, in_data));
}

/** This method creates a file on the disk only if it does not exist yet. If
 *  this succeeds, it opens that file. If it fails (or the file already exists),
 *  it throws an exception. The file is opened in memory and any operation done
 *  on this object is done in memory, it is only flushed to disk upon calling
 *  the \a save method or a similar one.
 *
 * \param in_sFileName The path to the file to create.
 * \param in_mode How to perform operations on the file by default.
 * \exception FileAlreadyExistException In case the file already exists.
 * \exception UnknownProtocolException In case you use a protocol that is not supported.
 * \exception NoRightsException In case you don't have the rights to create that file.
 * \exception InvalidCallException If you want to create something that is in no
 *                                 way createable (for example the Pipe).
 *
 * \note the upcoming write operations will either perform an insertion or over
 *       write the data or both (or none), depending on the \a in_mode parameter.
 *       It is still possible to enforce a different write mode later.
 * \note If you create a file with the name "-", upcoming save methods will write
 *       to stdout.
 */
File::Ptr File::create(const Path& in_sFileName, const WriteMode& in_mode)
{
    if(in_sFileName == "-") {
        return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, RawDataContainer(0)));
    } else if(in_sFileName.protocol() != "file") {
        // We want to open a transport protocol.
        throw UnknownProtocolException(in_sFileName);
    } else {
        // We want to create a regular file on the local disk.

        // It does already exist? baaad.
        if(FileUtils::fileExists(in_sFileName)) {
            throw FileAlreadyExistException(in_sFileName);
        }

        // Create all missing directories in the path to the file.
        FileUtils::mkdirIfNeeded(in_sFileName, true);

        // Create the file now.
        std::FILE *pFile = fopen(in_sFileName.c_str(), "w+b");
        if(pFile == NULL)
            throw NoRightsException(in_sFileName);

        // We can close it right away, just wanted to create it.
        SAFE_FCLOSE(pFile);

        // And create an empty file object now.
        return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, RawDataContainer(0)));
    }
}

/** This method creates a file on the disk, possibly overwriting an already
 *  existing one with the same name. If this succeeds, it then opens that file.
 *  If it fails (for example no permission), it throws an exception.
 *  The file is opened in memory and any operation done on this object is done
 *  in memory, it is only flushed to disk upon calling the \a save method or a
 *  similar one.
 *
 * \param in_sFileName The path to the file to create.
 * \param in_mode How to perform operations on the file by default.
 * \exception UnknownProtocolException In case you use a protocol that is not supported.
 * \exception NoRightsException In case you don't have the rights to create that file.
 * \exception InvalidCallException If you want to create something that is in no
 *                                 way createable (for example the Pipe).
 *
 * \note the upcoming write operations will either perform an insertion or over
 *       write the data or both (or none), depending on the \a in_mode parameter.
 *       It is still possible to enforce a different write mode later.
 * \note If you create a file with the name "-", upcoming save methods will write
 *       to stdout.
 */
File::Ptr File::overwrite(const Path& in_sFileName, const WriteMode& in_mode)
{
    if(in_sFileName == "-") {
        return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, RawDataContainer(0)));
    } else if(in_sFileName.protocol() != "file") {
        // We want to open a transport protocol.
        throw UnknownProtocolException(in_sFileName);
    } else {
        // We want to create a regular file on the local disk.

        // Create all missing directories in the path to the file.
        FileUtils::mkdirIfNeeded(in_sFileName, true);

        // Create the file now and overwrite anything maybe existing.
        std::FILE *pFile = fopen(in_sFileName.c_str(), "w+b");
        if(pFile == NULL)
            throw NoRightsException(in_sFileName);

        // We can close it right away, just wanted to create it.
        SAFE_FCLOSE(pFile);

        // And create an empty file object now.
        return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, RawDataContainer(0)));
    }
}

/** This method creates an empty file, but only in memory. Nothing whatsoever
 *  is done on the harddisk in this constructor. Rather, the file will be
 *  created on the disk during a call to \a save and the like. Thus this named
 *  constructor will not fail.\n
 *  Later on, if you call the \a save method, it will fail in case the file
 *  already exists.
 *
 * \param in_sFileName The path to the file to be saved later by default.
 * \param in_mode How to perform operations on the file by default.
 *
 * \note the upcoming write operations will either perform an insertion or over
 *       write the data or both (or none), depending on the \a in_mode parameter.
 *       It is still possible to enforce a different write mode later.
 */
File::Ptr File::createDelayed(const Path& in_sFileName, const WriteMode& in_mode)
{
    return File::Ptr(new File(in_sFileName, in_mode, File::CreateOnly, RawDataContainer(0)));
}

/** This method creates an empty file, but only in memory. Nothing whatsoever
 *  is done on the harddisk in this constructor. Rather, the file will be
 *  created on the disk during a call to \a save and the like. Thus this named
 *  constructor will not fail.\n
 *  Later on, if you call the \a save method, it will \e overwrite any already
 *  existing file with the same name!
 *
 * \param in_sFileName The path to the file to be saved later by default.
 * \param in_mode How to perform operations on the file by default.
 *
 * \note the upcoming write operations will either perform an insertion or over
 *       write the data or both (or none), depending on the \a in_mode parameter.
 *       It is still possible to enforce a different write mode later.
 */
File::Ptr File::overwriteDelayed(const Path& in_sFileName, const WriteMode& in_mode)
{
    return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, RawDataContainer(0)));
}

/** This method saves the content of the file onto the disk using the file's
 *  original compression.\n
 *  In fact, it flushes all changes made in-memory (by \a write, for example)
 *  onto the disk.\n
 *  If it is impossible to do so (maybe because of insufficient access rights),
 *  this method throws an exception.\n
 *  If the file originally comes from an archive, it will be saved back into
 *  the archive and the archive will be saved.\n
 *  If the file was compressed at the time it has been opened, it is compressed
 *  using exactly the same compressor again.
 *
 * \param in_comp A compressor object to use to compress data before saving it.
 *                Pass a NoCompressor if you don't want data compression.
 *
 * \return a reference to this, allowing chaining.
 *
 * \note If the file-name is the pipe ("-"), the file's content will be fushed
 *       to the standard output.
 */
const File& File::save() const
{
    return this->save(*m_pOriginalCompressor);
}

/** This method saves the content of the file onto the disk.
 *  In fact, it flushes all changes made in-memory (by \a write, for example)
 *  onto the disk.\n
 *  If it is impossible to do so (maybe because of insufficient access rights),
 *  this method throws an exception.\n
 *  If the file originally comes from an archive, it will be saved back into
 *  the archive and the archive will be saved.\n
 *  It is possible to compress the file before saving it using a \a Compressor.
 *
 * \param in_comp A compressor object to use to compress data before saving it.
 *                Pass a NoCompressor if you don't want data compression.
 *
 * \return a reference to this, allowing chaining.
 *
 * \note If the file-name is the pipe ("-"), the file's content will be fushed
 *       to the standard output.
 */
const File& File::save(const Compressor& in_comp) const
{
#ifndef D_FILE_NO_ARCHMAP
    // Maybe the file came from an archive? save it back.
    if(!m_sMyArchiveID.empty()) {
        if(m_lArchivesToLook.find(m_sMyArchiveID) != m_lArchivesToLook.end()) {
            // If the archive is still open, we can just use it.
            Archive* pArch = m_lArchivesToLook[m_sMyArchiveID];
            pArch->give(new FileChunk(File::Ptr(new File(*this))), true);
            pArch->save();
        } else {
            // No more open, we need to open it now.
            Archive::Ptr pArch;
            try {
                pArch.reset(Archive::loadArchive(m_sMyArchiveID));
            } catch(const ArkanaException&) {
                // Or create it, if unavailable.
                pArch.reset(Archive::createEmptyArchive(m_sMyArchiveID));
            }
            pArch->give(new FileChunk(File::Ptr(new File(*this))), true);
            pArch->save();
        }
        return *this;
    }
#endif

    return this->saveAs(this->getName(), m_saveMode, in_comp);
}

/** This method saves the content of the file onto the disk but with another name.
 *  In case the file already exists, the behaviour of this method is controlled
 *  by the parameter \a in_mode .\n
 *  If it is impossible to finish (maybe because of insufficient access rights),
 *  this method throws an exception.\n
 *  As for \a save, any compressor can be used to compress the file before
 *  saving it. NULL means no compression.\n
 *
 * \param in_sOtherFileName The name as which to save the file.
 * \param in_mode What to do in case the file already exists.
 * \param in_comp A compressor object to use to compress data before saving it.
 *                Pass a NoCompressor if you don't want data compression.
 * \exception UnknownProtocolException In case you want to save into an unknown
 *                                     protocol.
 * \exception FileAlreadyExistException In case the file you want to write
 *                                      already exists and you don't want to
 *                                      overwrite it.
 * \exception NoRightsException In case you don't have enough access rights to
 *                              create/overwrite the file.
 * \exception SyscallException In case any systemcall (fwrite) failed.
 *
 * \note This method does not change the internal file name to the given one,
 *       thus the next call to \a save will save the file with its original name.
 *
 * \return a reference to this, allowing chaining.
 */
const File& File::saveAs(const Path& in_sOtherFileName, const SaveMode& in_mode, const Compressor& in_comp) const
{
    // Catch possible errors before compressing that.
    if(in_sOtherFileName.protocol() != "file" && in_sOtherFileName != "-") {
        // We want to save to a transport protocol.
        throw UnknownProtocolException(in_sOtherFileName);
    }

    // This compresses it if needed.
    StreamedDataContainer data;
    this->saveToBuf(data, in_comp);

    if(in_sOtherFileName == "-") {
        // We want to write to the stdout
#define D_FILE_STDOUT_CHUNK 512
        uint64_t uiLeftToWrite = data.getBoundDC()->getSize();
        const char *p = reinterpret_cast<const char*>(data.getBoundDC()->getData());
        while(uiLeftToWrite >= D_FILE_STDOUT_CHUNK) {
            std::cout.write(p, D_FILE_STDOUT_CHUNK);
            p += D_FILE_STDOUT_CHUNK;
            uiLeftToWrite -= D_FILE_STDOUT_CHUNK;
        }

        // Write the last chunk exactly the same way and then flush it.
        std::cout.write(p, static_cast<std::streamsize>(uiLeftToWrite));
        std::cout.flush();
    } else if(in_sOtherFileName.protocol() != "file") {
        // We want to save to a transport protocol.
        throw UnknownProtocolException(in_sOtherFileName);
    } else {
        // We want to save to a regular file on the local disk.

        // In case we want to keep existing files, first check for that.
        if(in_mode == CreateOnly) {
            if(FileUtils::fileExists(in_sOtherFileName)) {
                throw FileAlreadyExistException(in_sOtherFileName);
            }
        }

        // Create all missing directories in the path to the file.
        FileUtils::mkdirIfNeeded(in_sOtherFileName, true);

        // If we come here, we can safely assume he wants to overwrite existing files.
        std::FILE *pFile = fopen(in_sOtherFileName.c_str(), "w+b");
        if(pFile == NULL)
            throw NoRightsException(in_sOtherFileName);

        std::size_t r = fwrite(data.getBoundDC()->getData(), static_cast<std::size_t>(data.getBoundDC()->getSize()), 1, pFile);
        SAFE_FCLOSE(pFile);

        if(r != 1)
            throw SyscallException("File::saveAs::fwrite()");
    }

    return *this;
}

/** This method saves the modifications that have been done to the file into
 *  a buffer in memory. The buffer may be compressed optionally.
 *
 * \param out_buf Where to write the data into. Gets inserted at the cursor pos.
 * \param in_comp A compressor object to use to compress data before saving it.
 *                Pass a NoCompressor if you don't want data compression.
 *
 * \return a reference to this, allowing chaining.
 *
 * \note If the compression somehow fails, the uncompressed data is stored.
 *
 * \author Pompei2
 */
const File& File::saveToBuf(StreamedDataContainer& out_buf, const Compressor& in_comp) const
{
    try {
        // Append the data compressed.
        in_comp.compress(out_buf, m_pSDC->getBoundDC());
        return *this;
    } catch(...) { }

    // If we failed during the compression or no compression is wanted,
    // fallback to uncompressed data.
    out_buf.insertNoEndian(*(m_pSDC->getBoundDC()));
    return *this;
}

#ifndef D_FILE_NO_ARCHMAP
/** This adds an archive to the list of archives to check for files on opening.
 *  For more informations on this theme, check our dokuwiki->design docs->map->
 *  flexibility.
 *
 * \param in_pArch The archive that has to be added to the list
 *
 * \note The archive is added in no special category, it is just added like this.
 *
 * \author Pompei2
 */
void File::addArchiveToLook(Archive *in_pArch)
{
    m_lArchivesToLook[in_pArch->getName()] = in_pArch;
}

/** This removes an archive from the list of archives to check for files on opening.
 *  For more informations on this theme, check our dokuwiki->design docs->map->
 *  flexibility.
 *
 * \param in_pArch The archive that has to be removed from the list
 *
 * \note A warning is displayed in case the archive is not in the list.
 *
 * \author Pompei2
 */
void File::remArchiveToLook(Archive *in_pArch)
{
    std::map<String, Archive*>::iterator i = m_lArchivesToLook.find(in_pArch->getName());
    if(i != m_lArchivesToLook.end()) {
        m_lArchivesToLook.erase(i);
    } else {
        // Print some warning but otherwise ignore it:
        FTS18N("InvParam", MsgType::Warning, in_pArch->getName());
    }
}

/** This removes an archive from the list of archives to check for files on opening.
 *  For more informations on this theme, check our dokuwiki->design docs->map->
 *  flexibility.
 *
 * \param in_pArch The archive that has to be removed from the list
 *
 * \note This method does not display a warning in case the archive is not registered.
 *
 * \author Pompei2
 */
void File::autoRemArchiveToLook(Archive *in_pArch)
{
    std::map<String, Archive*>::iterator i = m_lArchivesToLook.find(in_pArch->getName());
    if(i != m_lArchivesToLook.end()) {
        m_lArchivesToLook.erase(i);
    }
}
#endif

/** This method returns the size of a file.
 *
 * \param in_sFileName The name of the file to get the length from.
 *
 * \return The size of the file.
 * \exception FileNotExistException The file specified by \a in_sFileName doesn't exist.
 * \exception SyscallException Any of the system-calls used to work with the file failed.
 *
 * \note Although the return value is a 64-bit unsigned integer, the support for
 *       large files (>4GB) is not implemented yet.
 * \note This method may fail if the file doesn't exist or is not readable.
 *
 * \author Pompei2
 */
uint64_t File::getSize(const Path &in_sFileName)
{
    std::FILE *pFile = fopen(in_sFileName.c_str(), "rb");
    uint64_t uiLen = File::getSize(pFile);
    SAFE_FCLOSE(pFile);

    return uiLen;
}

/** This method returns the size of an already-opened file descriptor.
 *
 * \param in_pFile The file to get the length from.
 *
 * \return The size of the file.
 * \exception FileNotExistException \a in_pFile is NULL
 * \exception SyscallException Any of the system-calls used to work with the file failed.
 *
 * \note Although the return value is a 64-bit unsigned integer, the support for
 *       large files (>4GB) is not implemented/tested yet.
 * \note The file position indicator will be at the same position after a call
 *       to this method.
 *
 * \author Pompei2
 */
uint64_t File::getSize(std::FILE *in_pFile)
{
    if(NULL == in_pFile)
        throw FileNotExistException("NULL");

    long lOrigPos = ftell(in_pFile);
    if(lOrigPos == -1)
        throw SyscallException("File::getSize::ftell");

    if(0 != fseek(in_pFile, 0, SEEK_END))
        throw SyscallException("File::getSize::fseek(0, SEEK_END)");

    long lSize = ftell(in_pFile);

    if(0 != fseek(in_pFile, lOrigPos, SEEK_SET))
        throw SyscallException("File::getSize::fseek("+String::nr(lOrigPos)+", SEEK_SET)");

    return static_cast<uint64_t>(lSize);
}

/** This writes data into the file (actually into the memory-buffer of the file)
 *  but the way the data is written depends on the file opening mode.\n
 *  If the file was opened in \a Read mode, no data is written, a warning is
 *  displayed and an error code is returned.\n
 *  If the file was opened in \a Insert mode, all data is inserted at the
 *  current cursor position, making the total file size grow. The cursor will
 *  still move.\n
 *  If the file was opened in \a Overwrite mode, the data will overwrite the
 *  current data that is after the cursor. The cursor will still move. This is a
 *  bit like the default-mode of most hex-editors.\n
 * \n
 *  If you run a big-endian machine, every data-chunk gets its bytes swapped
 *  into little-endian order before being written. If you are on a little-endian
 *  machine, nothing happens to your bytes, they get written as-is.
 *
 * \param in_ptr The data to write.
 * \param in_size The size of one data-chunk that will be written.
 * \param in_nmemb This is how much data chunks will be written.
 *
 * \return The number of data-chunks actually written. If this value is less
 *         than \a in_nmemb there was an error during the write.
 * \note Everything happens in memory, to write back to the file on the disk,
 *       call the \a save method.
 *
 * \author Pompei2
 */
std::size_t File::write(const void *in_ptr, std::size_t in_size, std::size_t in_nmemb)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in_ptr, in_size, in_nmemb);
    case Overwrite: return m_pSDC->overwrite(in_ptr, in_size, in_nmemb);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/** Writes a number of bytes to the file, exactly as they are in the memory,
 *  without any endianness check, byte swap or so.
 *
 * \param in_ptr  A pointer to the data that will be written.
 * \param in_size The size of the data that will be written.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 * \note Everything happens in memory, to write back to the file on the disk,
 *       call the \a save method.
 *
 * \author Pompei2
 */
std::size_t File::writeNoEndian(const void *in_ptr, std::size_t in_size)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insertNoEndian(in_ptr, in_size);
    case Overwrite: return m_pSDC->overwriteNoEndian(in_ptr, in_size);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/** Writes a number of bytes to the file, exactly as they are in the memory,
 *  without any endianness check, byte swap or so.
 *
 * \param in_data The data that will be written.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note You should only use this method if you *REALLY* know what you're doing.
 * \note Everything happens in memory, to write back to the file on the disk,
 *       call the \a save method.
 *
 * \author Pompei2
 */
std::size_t File::writeNoEndian(const DataContainer &in_data)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insertNoEndian(in_data);
    case Overwrite: return m_pSDC->overwriteNoEndian(in_data);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 8 bit (1 byte) signed integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(int8_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 8 bit (1 byte) unsigned integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(uint8_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 16 bit (2 byte) signed integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(int16_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 16 bit (2 byte) unsigned integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(uint16_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 32 bit (4 byte) signed integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(int32_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 32 bit (4 byte) unsigned integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(uint32_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 64 bit (8 byte) signed integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(int64_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 64 bit (8 byte) unsigned integer into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note This converts the data into little-endian format if needed before
///       writing it to the file.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(uint64_t in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 32 bit (4 byte) floating point value into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(float in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 64 bit (8 byte) floating point value into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(double in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one 128 bit (16 byte) floating point value into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(long double in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Writes one zero-terminated string into the file.
/// \param in The data to write.
/// \return ERR_OK on success, an error code < 0 on failure.
/// \note All writes only occur in memory. You must call the save method to
///       store the changes on the disk.
int File::write(const String &in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

/// Creates a directory (full path) if it doesn't exist.
/** This function takes a path (optionally with a filename appended)
 *  And looks if the path exists, if not, it creates all the directories
 *  that are non-existing.
 *
 * \param in_sPath     The path to create.
 * \param in_bWithFile Wether there is a filename appended to the path or not.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FileUtils::mkdirIfNeeded(const Path& in_sPath, const bool in_bWithFile)
{
    struct stat buf;
    int ret = 0;

    if(in_sPath.empty()) {
        FTS18N("InvParam", MsgType::Horror, "mkdirIfNeeded(in_sPath = empty)");
        return -1;
    }

    Path sDirPath = in_sPath;

    if(in_bWithFile) {
        // If there is a filename appended, ignore it.
        sDirPath = in_sPath.directory();

        // This happens if it's just a file, w/o path.
        if(sDirPath.empty())
            return ERR_OK;
    }

    // Ignore the current and upper directory (. and ..).
    if(sDirPath == "." || sDirPath == ".." ||
       sDirPath == "./" || sDirPath == "../")
        return ERR_OK;

    // If the path already exists, all is ok.
    if((ret = stat(sDirPath.c_str(), &buf)) == 0)
        return ERR_OK;

    // Create the command to execute, depending on OS.
#if WINDOOF
    String sCmd = "mkdir \"" + sDirPath + "\"";

    int i = 0;

    // Put in only backslashes for fu*king windows.
    while(-1 != (i = sCmd.find("/")))
        sCmd[i] = '\\';

    // TODO: evaluate return value. what is the return value of the command O.o ?
    system(sCmd.c_str());

    return ERR_OK;
#else
    return system(("mkdir -p \"" + sDirPath + "\"").c_str()) == 0 ? ERR_OK : -5;
#endif
}

/// Get existance and right of a file.
/** Shows if the file \a in_pszFileName exists and can be accessed with mode \a in_iMode.
 *
 * \param in_sFileName The file to look for.
 * \param in_mode      The mode to test accessing to the file. If it is
 *                     File::Read, we only check if it is readable/openable. If
 *                     it is something else, also check if it is writeable.
 *
 * \return true If file seems to exist and you have the asked rights on it.
 * \return false If file doesn't exist or you don't have the asked rights on it
 *               or there was an error.
 *
 * \Note The test depends on the permission of the directories and/or symlinks
 *       in the path of \a in_pszFileName. \n
 *       If a directory is found as writeable, it means that we can create files
 *       in the directory, but we aren't 100 % sure to be able to write the
 *       directory with directory - handling functions or system( ... ) calls. \n
 *
 * \author Pompei2
 */
bool FileUtils::fileExists(const Path& in_sFileName, const File::WriteMode& in_mode)
{
    const char *pszOpenString = "rb";

    if(in_mode == File::Insert)
        pszOpenString = "wb";
    else if(in_mode == File::Overwrite)
        pszOpenString = "w+b";

    // Check if we have the rights.
    std::FILE *pFile = NULL;
    if(NULL != (pFile = fopen(in_sFileName.c_str(), pszOpenString))) {
        SAFE_FCLOSE(pFile);
        return true;
    }

    return false;
}

/// Get existance of a directory.
/** Shows if the direcotry \a in_sDirName exists.
 *
 * \param in_sDirName The directory to look for.
 *
 * \return True if the directory exist, false if not.
 *
 * \author Pompei2
 */
bool FileUtils::dirExists(const Path& in_sDirName)
{
    struct stat buf;

    if(!in_sDirName) {
        FTS18N("InvParam", MsgType::Horror, "DirExists");
        return false;
    }

    // Ignore the current and upper directory (. and ..).
    // we could rewrite this now that we have the Path class...
    if(in_sDirName == "." || in_sDirName == ".." ||
       in_sDirName == ".\\" || in_sDirName == "..\\" ||
       in_sDirName == "./" || in_sDirName == "../")
        return true;

    // Remove the trailing / or \ if there is one.
    String sFinalDirName = in_sDirName;
    if(in_sDirName.right(1) == "/" || in_sDirName.right(1) == "\\") {
        sFinalDirName = in_sDirName.left(in_sDirName.len()-1);
    }

    if(stat(sFinalDirName.c_str(), &buf) != 0)
        return false;

#if WINDOOF
    if((buf.st_mode & _S_IFDIR) != 0)
#else
    if(S_ISDIR(buf.st_mode))
#endif
        return true;

    return false;
}

/// copies a file.
/** This function copies a file to another one and, if the "other on" already
 *  exists, this function can overwrite it (if you want).
 *
 * \param in_sFrom The file to copy
 * \param in_sTo   The destination filename
 * \param in_sFrom Wether to overwrite the destination or not, if it already exists.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int FileUtils::fileCopy(const Path& in_sFrom, const Path& in_sTo, bool in_bOverwrite)
{
    std::FILE *pFile = NULL;
    char *pData = NULL;
    std::size_t lLength = 0;

    if(!in_sFrom || !in_sTo) {
        FTS18N("InvParam", MsgType::Horror, "fileCopy(!in_sFrom || !in_sTo)");
        return -1;
    }

    // The source file doesn't exist.
    if(!fileExists(in_sFrom, File::Read))
        return -1;

    // The dest file already exists and we don't want to overwrite it.
    if(fileExists(in_sTo, File::Read) && !in_bOverwrite)
        return ERR_OK;

    // Now copy it over.
    lLength = File::getSize(in_sFrom);
    if(lLength == (std::size_t)-1)
        return -2;

    pData = new char[lLength + 1];
    pFile = fopen(in_sFrom.c_str(), "rb");
    fread(pData, lLength, 1, pFile);
    pData[lLength] = '\0';

    pFile = freopen(in_sTo.c_str(), "wb+", pFile);
    if(pFile == NULL) {
        SAFE_DELETE_ARR(pData);
        return -3;
    }
    fwrite(pData, lLength, 1, pFile);
    fclose(pFile);
    SAFE_DELETE_ARR(pData);

    return ERR_OK;
}

 /* EOF */
