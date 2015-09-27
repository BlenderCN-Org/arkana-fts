/**
 * \file dFile.cpp
 * \author Pompei2
 * \date 6 Jan 2009
 * \brief This file contains the implementation of the file handling class.
 **/

#include "dFile.h"

#include "logging/logger.h"

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

File::~File()
{
    delete m_pSDC;
}

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

File::Ptr File::fromRawData(const ConstRawDataContainer& in_data, const Path& in_sFileName, const WriteMode& in_mode, const SaveMode& in_saveMode)
{
    return File::Ptr(new File(in_sFileName, in_mode, in_saveMode, in_data));
}

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

File::Ptr File::createDelayed(const Path& in_sFileName, const WriteMode& in_mode)
{
    return File::Ptr(new File(in_sFileName, in_mode, File::CreateOnly, RawDataContainer(0)));
}

File::Ptr File::overwriteDelayed(const Path& in_sFileName, const WriteMode& in_mode)
{
    return File::Ptr(new File(in_sFileName, in_mode, File::OverwriteFile, RawDataContainer(0)));
}

const File& File::save() const
{
    return this->save(*m_pOriginalCompressor);
}

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
void File::addArchiveToLook(Archive *in_pArch)
{
    m_lArchivesToLook[in_pArch->getName()] = in_pArch;
}

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

void File::autoRemArchiveToLook(Archive *in_pArch)
{
    std::map<String, Archive*>::iterator i = m_lArchivesToLook.find(in_pArch->getName());
    if(i != m_lArchivesToLook.end()) {
        m_lArchivesToLook.erase(i);
    }
}
#endif

uint64_t File::getSize(const Path &in_sFileName)
{
    std::FILE *pFile = fopen(in_sFileName.c_str(), "rb");
    uint64_t uiLen = File::getSize(pFile);
    SAFE_FCLOSE(pFile);

    return uiLen;
}

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

std::size_t File::write(const void *in_ptr, std::size_t in_size, std::size_t in_nmemb)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in_ptr, in_size, in_nmemb);
    case Overwrite: return m_pSDC->overwrite(in_ptr, in_size, in_nmemb);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

std::size_t File::writeNoEndian(const void *in_ptr, std::size_t in_size)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insertNoEndian(in_ptr, in_size);
    case Overwrite: return m_pSDC->overwriteNoEndian(in_ptr, in_size);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

std::size_t File::writeNoEndian(const DataContainer &in_data)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insertNoEndian(in_data);
    case Overwrite: return m_pSDC->overwriteNoEndian(in_data);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

int File::write(const String &in)
{
    switch(this->getMode()) {
    case Insert: return m_pSDC->insert(in);
    case Overwrite: return m_pSDC->overwrite(in);
    default: FTS18N("File_Write", MsgType::Warning, this->getName(), "File opened in read only mode"); return -1;
    }
}

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

bool FileUtils::exists(const Path& in_sPathname)
{
    return fileExists(in_sPathname) || dirExists(in_sPathname);
}

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
