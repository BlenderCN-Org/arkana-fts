#include "dArchive.h"

#include "dLib/dCompressor/dCompressor.h"
#include "logging/logger.h"

using namespace FTS;

ChunkFactory Archive::m_factory;

/** From a given chunktype ID, it creates a chunk object that is of the type
 *  indicated by the ID. For unknown IDs, it creates an UnknownChunk object.
 *
 * \param in_uiChunkID The ID of the chunk type to create.
 *
 * \return The new chunk object.
 */
Chunk *ChunkFactory::createChunk(uint8_t in_uiChunkID)
{
    switch(in_uiChunkID) {
    case 1:
        return new FileChunk;
    default:
        return new UnknownChunk;
    }
}

/** This determines the chunktype ID of a given chunk object. If the object type
 *  is unknown, this returns 0.
 *
 * \param in_pChunk The chunk object whose chunktype ID to determine.
 *
 * \return The chunktype ID of the given chunk object.
 */
uint8_t ChunkFactory::getChunkID(const Chunk *in_pChunk)
{
    if(dynamic_cast<const FileChunk *>(in_pChunk) != NULL)
        return 1;
    else
        return 0;
}

/// Reads in the chunk's header.
/// \param out_f The file to read the header from. The file pointer will be moved.
/// \return ERR_OK on success, an error code < 0 on failure (for example end of file reached).
int Chunk::read(File &out_f)
{
    out_f.read(m_uiPayloadLength);
    out_f.read(m_sName);

    FTSMSGDBG("Reading "+this->getTypeName()+" '"+m_sName+"' with size "
              +String::nr(m_uiPayloadLength), 3);

    return out_f.eof() ? -1 : ERR_OK;
}

/// Reads in the chunk's header.
/// \param out_f The file to read the header from. The file pointer will be moved.
/// \param in_sChunkPrefix A prefix to give the chunk name.
/// \return ERR_OK on success, an error code < 0 on failure (for example end of file reached).
String Chunk::prefix(const String &in_sChunkPrefix)
{
    if(!in_sChunkPrefix)
        return this->getName();

    String sDbgMsg = "Prefixing "+this->getName()+" with '"+in_sChunkPrefix+"'";
    FTSMSGDBG(sDbgMsg, 3);

    m_sName = in_sChunkPrefix + this->getName();
    return this->getName();
}

/// Writes the chunk's header.
/// \param out_f The file to write the header to. The file pointer will be moved.
/// \return ERR_OK on success, an error code < 0 on failure (unlikely).
int Chunk::write(File &out_f)
{
    out_f.write(this->getPayloadLength());
    out_f.write(this->getName());
    return ERR_OK;
}

/// \return The name that identifies this chunk.
String Chunk::getName() const
{
    return m_sName;
}

/// \return The name that identifies this chunk.
uint64_t Chunk::getPayloadLength() const
{
    return m_uiPayloadLength;
}

/// Reads in the chunk's header and seeks the file behind its data, in front of
/// the next chunk.
/// \param out_f The file to read the header from. The file pointer will be moved.
/// \return ERR_OK on success, an error code < 0 on failure (for example end of file reached).
int UnknownChunk::read(File &out_f)
{
    // Read the chunk header.
    if(Chunk::read(out_f) != ERR_OK)
        return -1;

    // Skip over the unknown chunk's data.
    out_f.setCursorPos(out_f.getCursorPos() + this->getPayloadLength());
    return out_f.eof() ? -1 : ERR_OK;
}

/// Write the chunk's header and fills the data with zeroes.
/// \param out_f The file to write the header to. The file pointer will be moved.
/// \return ERR_OK on success, an error code < 0 on failure (Unlikely).
int UnknownChunk::write(File &out_f)
{
    // Write the chunk header
    if(Chunk::write(out_f) != ERR_OK)
        return -1;

    // Fill with zeroes.
    for(uint64_t i = this->getPayloadLength() ; i > 0 ; --i) {
        out_f.write((uint8_t)0);
    }

    return ERR_OK;
}

/// Default constructor.
FileChunk::FileChunk()
    : m_pRawContent(NULL)
    , m_pFileCache() // Null pointer
{
}

/// Identical to calling new FileChunk()->give(in_pFile).
FileChunk::FileChunk(File::Ptr in_pFile, const String& in_sChunkName)
    : m_pRawContent(NULL)
    , m_pFileCache() // Null pointer
{
    this->give(std::move(in_pFile), in_sChunkName);
}

/// Default destructor.
FileChunk::~FileChunk()
{
    SAFE_DELETE(m_pRawContent);
}

/** Reads a file chunk out of a file object. The bytes right in front of the
 *  file cursor MUST be the beginning of a file chunk, no check can be done.
 *  (The chunktype ID is supposed to be already read).
 *
 * \param out_f The file to read the chunk from.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 */
int FileChunk::read(File &out_f)
{
    // Read the chunk header.
    if(Chunk::read(out_f) != ERR_OK)
        return -1;

    // Ignore leading "./" or ".\" in filenames
    while(m_sName.left(2) == "./" || m_sName.left(2) == ".\\") {
        m_sName = String(m_sName, 2);
    }

    // Read the file's data into a new data container.
    m_pRawContent = new RawDataContainer(this->getPayloadLength());
    if(out_f.readNoEndian(*m_pRawContent) < this->getPayloadLength()) {
        SAFE_DELETE(m_pRawContent);
        return -2;
    }

    // The file object will be created when it is first needed.
    // This avoids unnecessary creations of unneeded objects.
    m_pFileCache.reset();

    return ERR_OK;
}

/** This gives a file object to this chunk, That means that the old file object
 *  hold by this chunk object gets deleted and control is taken over the given
 *  one.\n
 *  After a call to this method, you no more need to care about the file object.
 *
 * \param out_pFile The file object auto-ptr to take control over.
 * \param in_sChunkName If a different chunk-name than file-name is desired, put it here.
 *
 * \note You should no more use \a out_pFile after a call to this method, as it
 *       is a NULL auto-ptr after that. Yep, you gave it away!
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 */
int FileChunk::give(File::Ptr in_pFile, const String& in_sChunkName)
{
    SAFE_DELETE(m_pRawContent);

    m_pFileCache.reset();
    m_pFileCache = std::move(in_pFile);
    if(m_pFileCache.get() != NULL) {
        if(!in_sChunkName.empty())
            m_sName = in_sChunkName;
        else
            m_sName = m_pFileCache->getName();

        // Compress it like it was originally.
        StreamedDataContainer sdc;
        m_pFileCache->saveToBuf(sdc, m_pFileCache->getOriginalCompressor());
        m_pRawContent = sdc.unbindDC();
        m_uiPayloadLength = m_pRawContent->getSize();
    } else {
        m_sName = String::EMPTY;
        m_uiPayloadLength = 0;
    }

    return ERR_OK;
}

/** This method saves the file on disk, overwriting existing files.\n
 *  When the file is saved on disk, if the file was compressed in the archive,
 *  it gets saved on disk in the exact same compressed form.
 *
 * \param in_sNewChunkname The file gets extracted to the path/filename specified
 *                         by this parameter instead of the one specified by
 *                         its chunkname in the archive.\n
 *                         This does not modify the chunkname, it is only used
 *                         for this execution.\n
 *                         The default empty string means use the chunk's name.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 */
int FileChunk::execute(const String &in_sNewChunkname)
{
    Path sFileName = in_sNewChunkname.empty() ? this->getName() : in_sNewChunkname;
    try {
        File::Ptr f = File::overwrite(sFileName, File::Insert);
        f->writeNoEndian(*m_pRawContent); // RawContent may already be comressed.
        f->save(NoCompressor()); // That's why no compression is explicitly wanted here.
    } catch(const ArkanaException& e) {
        e.show();
        return -1;
    }

    return ERR_OK;
}

/** Writes the chunk into a file, using the compression it origianlly had when
 *  it has been loaded from the disk.
 *
 * In case the underlying file object has been modified, the modifications get
 * saved, too!
 *
 * \param out_f The file to write it to.
 *
 * \return ERR_OK if all went right, or an error code < 0 on failure.
 */
int FileChunk::write(File &out_f)
{
    // Look for modifications of the underlying file.
    //if(m_pFileCache.get() && m_pFileCache->modified()) {
        /// \TODO.
    //}

    // Write the chunk header
    if(Chunk::write(out_f) != ERR_OK)
        return -1;

    // Write the payload.
    out_f.writeNoEndian(*m_pRawContent);

    return ERR_OK;
}

/// \return The File I represent, use this to read from the file or modify it.
///         If the archive this chunk belongs to is saved later, it is saved
///         with all the modifications you do to it.\n
///         The file is created with "create" semantics for saving and "insert"
///         semantics for writing something to it.
///
/// \Note if the file is compressed in the archive, it gets decompressed
/// when this method gets called the first time, thus this method may take a
/// little time (actually, with miniLZO it's very fast) the first time it's
/// called. The subsequent calls do make virutally no overhead.
FileChunk::operator File&()
{
    // Maybe load the cached file!
    this->getContents();
    return *m_pFileCache;
}

/// \return The File I represent, use this to read from the file or modify it.
///         If the archive this chunk belongs to is saved later, it is saved
///         with all the modifications you do to it.\n
///         The file is created with "create" semantics for saving and "insert"
///         semantics for writing something to it.
/// \n
/// Note that if the file is compressed in the archive, it gets decompressed
/// when this method gets called the first time, thus this method may take a
/// little time (actually, with miniLZO it's very fast) the first time it's
/// called. The subsequent calls do make virutally no overhead.
FTS::File &FileChunk::getFile()
{
    return this->operator File&();
}

/// \return A const raw data container that contains the data (not a copy!) of
///         the file contents.
/// \n
/// Note that if the file is compressed in the archive, it gets decompressed
/// when this method gets called the first time, thus this method may take a
/// little time (actually, with miniLZO it's very fast) the first time it's
/// called. The subsequent calls do make virutally no overhead.
ConstRawDataContainer FileChunk::getContents() const
{
    // First time used, load the file object.
    if(!m_pFileCache.get()) {
        if(m_pRawContent != NULL) {
            m_pFileCache = File::fromRawData(*m_pRawContent, this->getName(), File::Insert, File::OverwriteFile);
        } else {
            // Huh? Just create an empty file.
            FTS18N("InvParam", MsgType::Horror, "FileChunk::getFile, but no file loaded");
            m_pFileCache = File::create(this->getName(), File::Insert);
        }
    }

    return m_pFileCache->getDataContainer();
}

/// Default constructor, does nothing.
Archive::Archive(const Path& in_sFileName, Compressor *in_pComp)
    : m_pOrigComp(in_pComp == NULL ? Compressor::Ptr(new NoCompressor) : in_pComp->copy())
    , m_sFileName(in_sFileName)
{
}

/** Constructs the archive object, reading every chunk out of the file.\n
 *  After a call to this constructor, a call to \a in_pFile.eof() will return true.
 *
 * \param out_file The file to read the archive from. The cursor will be at the
 *                 end of the file after this call.
 * \param in_sChunkPrefix A prefix to give all chunk's names.
 */
Archive::Archive(File& out_file, const String &in_sChunkPrefix)
{
    // Get the original compressor and filename.
    m_pOrigComp = out_file.getOriginalCompressor().copy();
    m_sFileName = out_file.getName();

    // Read out the header.
    String sID = out_file.readstr();
    int8_t version = out_file.readi8();
    uint32_t uiFletcher32 = out_file.readui32();

    // Check for validity.
    if(out_file.eof() || sID != "FTSARC") {
        throw(CorruptDataException(out_file.getName(), "Invalid ID: \"" + String(sID, 0, 6) + "\" should be \"FTSARC\"", MsgType::Error));
    }

    // Log something.
    FTSMSGDBG("Loading archive (format version "+String::nr(version)+") from file "+out_file.getName(), 2);

    // We calculate the checksum and see if it is the one that was expected.
    ConstRawDataContainer cdc(out_file.getDataContainer().getData() + out_file.getCursorPos(), out_file.getSize() - out_file.getCursorPos());
    uint32_t uiFletcher32Calc = cdc.fletcher32();
    if(uiFletcher32 != uiFletcher32Calc) {
        throw(CorruptDataException(out_file.getName(), "Invalid Fletcher32 checksum: calculated " + String::nr(uiFletcher32Calc) + "but expected" + String::nr(uiFletcher32), MsgType::Error));
    }
    FTSMSGDBG("Checksum "+String::nr(uiFletcher32)+" is valid", 3);

    // Read out all chunks until the end of the file.
    uint8_t uiChunkType = 0;
    Chunk *pChunk = NULL;
    do {
        // Create the corresponding chunk.
        out_file.read(uiChunkType);
        pChunk = m_factory.createChunk(uiChunkType);

        // Read it and add it to our chunk map.
        pChunk->read(out_file);
        pChunk->prefix(in_sChunkPrefix);
        m_mChunks[pChunk->getName()] = pChunk;
    } while(!out_file.eof());

    // Another log.
    FTSMSGDBG("Done loading the archive with "+String::nr(this->getChunkCount())+" chunks.", 2);
}

/** Constructs the archive object, reading every file out of the directory.\n
 *  After a call to this constructor, \a out_dbi will be deleted.
 *
 * \param PDBrowseInfo The directory to read the archive from. It will be deleted.
 * \param in_sChunkPrefix A prefix to give all chunk's names.
 */
Archive::Archive(PDBrowseInfo out_dbi, const String &in_sChunkPrefix)
{
    // Get the original compressor and filename.
    m_pOrigComp.reset(new NoCompressor());
    m_sFileName = out_dbi->currDir;

    // Log something.
    FTSMSGDBG("Loading archive from directory "+out_dbi->currDir, 2);

    // We recursively enter each subdirectory and we read out every file as
    // a chunk if we encounter one.
    this->makeChunksFromDir(out_dbi, "", in_sChunkPrefix);

    // Another log.
    FTSMSGDBG("Done loading the archive with "+String::nr(this->getChunkCount())+" chunks.", 2);
}

void FTS::Archive::makeChunksFromDir(PDBrowseInfo out_dbi, const Path& in_sSubdir, const String &in_sChunkPrefix)
{
    for(Path sEntry = dBrowse_GetNext(out_dbi) ; !sEntry.empty() ; sEntry = dBrowse_GetNext(out_dbi)) {
        // Skip the stupid ones..
        if(sEntry == "." || sEntry == "..")
            continue ;

        // If it is a directory, recurse into it.
        if(FileUtils::dirExists(Path(out_dbi->currDir) + sEntry)) {
            PDBrowseInfo pdbi = dBrowse_Open(Path(out_dbi->currDir) + sEntry);
            this->makeChunksFromDir(pdbi, in_sSubdir + sEntry, in_sChunkPrefix);
        } else {
            // It is most probably a file, create a chunk for it.
            Chunk* pChunk = new FileChunk(File::open(Path(out_dbi->currDir) + sEntry, File::Insert), in_sSubdir + sEntry);
            pChunk->prefix(in_sChunkPrefix);
            m_mChunks[pChunk->getName()] = pChunk;
        }
    }

    // And after we're done with this directory, delete it.
    dBrowse_Close(out_dbi);
}

/** Constructs an archive object, reading every chunk out of the given file.\n
 *
 * \param in_sFileName The name of the file to be opened and read from. This name
 *                     has the same format as the name you give to \a File,
 *                     allowing you to even pass an URL.
 * \param in_sChunkPrefix A prefix to give all chunk's names.
 *
 * \exception CorruptDataException In case the archive is somehow corrupted, for
 *                                 example wrong checksum.
 * \exception ArkanaException Any other exception that may be thrown by any of
 *                            the \a File methods.
 *
 * \return An archive object that reflects the content of the file.
 */
Archive *Archive::loadArchive(const Path &in_sFileName, const String &in_sChunkPrefix)
{
    // We may open a directory as an archive.
    if(FTS::FileUtils::dirExists(in_sFileName)) {
        PDBrowseInfo dbi = dBrowse_Open(in_sFileName);
        if(dbi)
            return new Archive(dbi, in_sChunkPrefix);
        else
            throw CorruptDataException(in_sFileName, "Bad directory to open archive");
    } else {
        File::Ptr f = File::open(in_sFileName, File::Read);
        return new Archive(*f, in_sChunkPrefix);
    }
}

/** Constructs an archive object, reading every chunk out of the given file.\n
 *  After a call to this method, a call to \a out_file.eof() will return true.
 *
 * \param out_file The file to read the archive from. The cursor will be at the
 *                 end of the file after this call.
 * \param in_sChunkPrefix A prefix to give all chunk's names.
 *
 * \exception CorruptDataException In case the archive is somehow corrupted, for
 *                                 example wrong checksum.
 * \exception ArkanaException Any other exception that may be thrown by any of
 *                            the \a File methods.
 *
 * \return An archive object that reflects the content of the file.
 */
Archive *Archive::loadArchive(File& out_file, const String &in_sChunkPrefix)
{
    return new Archive(out_file, in_sChunkPrefix);
}

/// \return A new, empty archive object.
Archive *Archive::createEmptyArchive(const Path& in_sFileName, Compressor *in_pComp)
{
    return new Archive(in_sFileName, in_pComp);
}

/** Checks if the file contains a valid fts archive.
 *
 * \param in_sFileName The file to check.
 *
 * \return True if there is a valid archive at the cursor position, false if not.
 */
bool Archive::isValidArchive(const String &in_sFileName)
{
    try {
        return Archive::isValidArchive(*File::open(in_sFileName, File::Read));
    } catch(...) {
        return false;
    }
}

/** Checks if the file contains a valid fts archive at the current cursor
 *  position. The cursor is not moved (it is, but it gets put back where it was
 *  at the end of the method).
 *
 * \param out_file The file to check.
 *
 * \return True if there is a valid archive at the cursor position, false if not.
 */
bool Archive::isValidArchive(File& out_file)
{
    // Keep the original cursor position in mind.
    uint64_t uiOrigCursorPos = out_file.getCursorPos();

    // Read out the header.
    String sID = out_file.readstr();
    // skip version
    out_file.readi8();
    uint32_t uiFletcher32 = out_file.readui32();

    // Check for validity.
    if(out_file.eof() || sID != "FTSARC") {
        return false;
    }

    // We calculate the checksum and see if it is the one that was expected.
    ConstRawDataContainer cdc(out_file.getDataContainer().getData() + out_file.getCursorPos(), out_file.getSize() - out_file.getCursorPos());
    uint32_t uiFletcher32Calc = cdc.fletcher32();
    if(uiFletcher32 != uiFletcher32Calc) {
        return false;
    }

    // Put the cursor back where it was.
    out_file.setCursorPos(uiOrigCursorPos);
    return true;
}

/// Unloads all the content of the archive, resulting in the archive being empty.
void Archive::unload()
{
#ifndef D_FILE_NO_ARCHMAP
    // Remove me from the file class's list of archives to look if I'm in it.
    File::autoRemArchiveToLook(this);
#endif

    // Remove any chunks.
    for(ChunkMap::iterator i = m_mChunks.begin() ; i != m_mChunks.end() ; ++i) {
        SAFE_DELETE(i->second);
    }
    m_mChunks.clear();

    m_pOrigComp.reset(new NoCompressor);
}

/// Destroys every chunk loaded with the archive.
Archive::~Archive()
{
    this->unload();
}

/** This saves the whole archive into a file object. No whole-archive compression
 *  gets done, but if for example one file-chunk was compressed on-disk, it gets
 *  compressed again in the archive, this happens in this method, thus the
 *  method may be slow if there are some big compressed chunks in it.
 *
 * \param out_file The file to write the archive to.
 *
 * \return a reference to this, allowing chaining.
 */
const Archive& Archive::save(File &out_file) const
{
    // We write the header with a dummy fletcher sum and keep the position of
    // that dummy fletcher sum in our mind.
    out_file.write("FTSARC");
    out_file.write(static_cast<uint8_t>(1));
    uint64_t uiCursorPosFletcher = out_file.getCursorPos();
    out_file.write(static_cast<uint32_t>(12345)); // Dummy fletcher sum.

    // Now we keep in mind where the payload (over which the sum is done) began.
    uint64_t uiCursorPosBefore = out_file.getCursorPos();

    // And now write the payload (all the chunks).
    for(ChunkMap::const_iterator i = this->begin() ; i != this->end() ; ++i) {
        out_file.write(m_factory.getChunkID(i->second));
        i->second->write(out_file);
    }

    // Now go back at the front of the payload and calculate the fletcher sum.
    // But keep the end of our payload in mind.
    uint64_t uiCursorPosEnd = out_file.getCursorPos();
    out_file.setCursorPos(uiCursorPosBefore);
    uint32_t uiFletcher32 = out_file.fletcher32FromCursor();

    // Go overwrite the dummy fletcher with the correct one.
    out_file.setCursorPos(uiCursorPosFletcher);
    out_file.getWriter()->overwrite(uiFletcher32);

    // Go back at the end .. that's all, folks!
    out_file.setCursorPos(uiCursorPosEnd);

    return *this;
}

/** This saves the whole archive on the disk. The file is being compressed using
 *  the original compressor that has been determined during the creation or load
 *  of the archive.\n
 *  The name of the file it gets saved as is the one returned by the \a getName
 *  method and this file will be overwritten if it already exists.\n
 *  This method is useful in case you open an archive, modify it and want to
 *  save these changes to disk.
 *
 * \exception ArkanaException Any other exception that may be thrown by any of
 *                            the \a File methods.
 *
 * \return a reference to this, allowing chaining.
 */
const Archive& Archive::save() const
{
    File::Ptr pFile = File::overwrite(this->getName(), File::Insert);
    this->save(*pFile);
    pFile->save(this->getOriginalCompressor());
    return *this;
}

/** This executes every chunk in the archive. Executing a chunk means let him do
 *  What he has to do to be "extracted" from the archive. For example a file
 *  chunk writes its file onto the disc when executed.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 */
int Archive::execute()
{
    int iRet = ERR_OK;
    for(ChunkMap::iterator i = m_mChunks.begin() ; i != m_mChunks.end() ; ++i) {
        iRet += i->second->execute();
    }

    return iRet;
}

/// \return the total amount of chunks stored in the archive.
uint64_t Archive::getChunkCount() const
{
    return m_mChunks.size();
}

/** This returns a chunk of the archive.
 *
 * \param in_sName The name of the chunk you want to get from the archive. Names
 *                 are, of course, case sensitive.
 *
 * \return The chunk that has the given name. If there is no chunk with that
 *         name, NULL is returned.
 */
const Chunk *Archive::getChunk(const String &in_sName) const
{
    ChunkMap::const_iterator i = m_mChunks.find(in_sName);
    return (i == m_mChunks.end()) ? NULL : i->second;
}

/** This returns a chunk of the archive.
 *
 * \param in_sName The name of the chunk you want to get from the archive. Names
 *                 are, of course, case sensitive.
 *
 * \return The chunk that has the given name. If there is no chunk with that
 *         name, NULL is returned.
 */
Chunk *Archive::getChunk(const String &in_sName)
{
    ChunkMap::iterator i = m_mChunks.find(in_sName);
    return (i == m_mChunks.end()) ? NULL : i->second;
}

bool Archive::hasChunk(const String& in_sName) const
{
    return m_mChunks.find(in_sName) != m_mChunks.end();
}

/// \return the total number of file chunks stored in the archive.
uint64_t Archive::getFileCount() const
{
    uint64_t uiFileCount = 0;

    for(ChunkMap::const_iterator i = m_mChunks.begin() ; i != m_mChunks.end() ; ++i) {
        if(dynamic_cast<const FileChunk *>(i->second) != NULL)
            uiFileCount++;
    }

    return uiFileCount;
}

/// \return A list of names of the files that are stored in this archive.
std::list<String> Archive::getFileList() const
{
    std::list<String> lFileList;

    for(ChunkMap::const_iterator i = m_mChunks.begin() ; i != m_mChunks.end() ; ++i) {
        if(dynamic_cast<const FileChunk *>(i->second) != NULL)
            lFileList.push_back(i->first);
    }

    return lFileList;
}

/// \return An iterator that may be used to iterate trough the chunks of the
///         archive and that points at the first chunk in the archive.
Archive::ChunkMap::const_iterator Archive::begin() const
{
    return m_mChunks.begin();
}

/// \return An iterator that points after the last chunk in the archive.
Archive::ChunkMap::const_iterator Archive::end() const
{
    return m_mChunks.end();
}

/** This takes a chunk out of the archive. This action will NOT invalidate any
 *  existing iterator except iterators pointing on the said chunk.
 *
 * \param in_sChunkName The chunk that has to be taken out of the archive.
 *
 * \return The chunk that has been taken out. If there is no such chunk, NULL.
 *
 * \note A chunk returned by this method MUST be deleted by the caller, as the
 *       archive looses any control over the object returned by this method.
 */
Chunk *Archive::take(const String &in_sChunkName)
{
    ChunkMap::iterator i = m_mChunks.find(in_sChunkName);

    // None found, we do nothing.
    if(i == m_mChunks.end())
        return NULL;

    // Take it out of the map and then return it.
    Chunk *pChunk = i->second;
    m_mChunks.erase(i);
    return pChunk;
}

/** This adds a chunk into the archive. This action will NOT invalidate any
 *  existing iterator.
 *
 * \param in_pChunk The chunk that has to be insterted into the archive.
 * \param in_bReplace Whether to replace a chunk that may be already existing
 *                    with the same name or not.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \note you are not allowed to insert two chunks that have the same name,
 *       thus if you try to insert a chunk with a name that is already used by
 *       another chunk in this archive, that won't work. That's the reason of \a in_bReplace.
 * \note This method takes the full control over \a in_pChunk and may delete it
 *       whenever it wants.
 */
int Archive::give(Chunk *in_pChunk, bool in_bReplace)
{
    if(in_pChunk == NULL) {
        FTS18N("InvParam", MsgType::Horror, "Archive::giveChunk(NULL)");
        return -1;
    }

    // If there is already such a chunk in the archive, we reject that one.
    if(this->getChunk(in_pChunk->getName()) != NULL) {
        // If he wants to replace it, delete the original and go ahead.
        if(in_bReplace) {
            delete this->take(in_pChunk->getName());
        // But if the user doesn't want to replace it, we're gone.
        } else {
            // Delete it because we take control over the given chunk.
            SAFE_DELETE(in_pChunk);
            return -2;
        }
    }

    m_mChunks[in_pChunk->getName()] = in_pChunk;
    return ERR_OK;
}

/// Convenience method.
/// \param in_sName The name of the file to get.
/// \note The pointer you get is only valid as long as the archive object exists.
/// \throw NotExistException in case the file does not exist within the archive.
File& Archive::getFile(const String &in_sName)
{
    FileChunk *pChk = dynamic_cast<FileChunk *>(this->getChunk(in_sName));
    if(pChk == NULL)
        throw NotExistException(in_sName, this->getName(), MsgType::Error);

    return *pChk;
}

/// Convenience method.
/// \param in_sName The name of the file whose content to get.
/// \note The pointer you get is only valid as long as the archive object exists.
/// \throw NotExistException in case the file does not exist within the archive.
ConstRawDataContainer Archive::getFileContent(const String &in_sName) const
{
    const FileChunk *pChk = dynamic_cast<const FileChunk *>(this->getChunk(in_sName));
    if(pChk == NULL)
        throw NotExistException(in_sName, this->getName(), MsgType::Error);

    return pChk->getContents();
}
