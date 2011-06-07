#ifndef D_ARCHIVE_H
#define D_ARCHIVE_H

#include "main.h"

#include <map>
#include <list>

#include "dLib/dString/dString.h"
#include "dLib/dFile/dFile.h"
#include "dLib/dBrowse/dBrowse.h"

namespace FTS {
    class StreamedDataContainer;

/// This is the baseclass for any chunk. It is used to read a chunk's header
/// that is the same for all chunks.
class Chunk {
private:
    /// Protect against copying
    Chunk(const Chunk &) {};

protected:
    /// Protect against construction.
    Chunk() : m_uiPayloadLength(0) {};

    /// The name that identifies the chunk.
    String m_sName;

    /// The length of the chunk's data alone.
    uint64_t m_uiPayloadLength;

public:
    /// Default destructor.
    virtual ~Chunk() {};

    virtual String getTypeName() const = 0;

    virtual int read(File &out_f);
    virtual int execute(const String &in_sNewChunkname = String::EMPTY) = 0;
    virtual int write(File &out_f);

    String getName() const;
    String prefix(const String &in_sChunkPrefix);
    uint64_t getPayloadLength() const;
};

/// This class may be used to read any unknown chunk. It will just skip over the
/// chunk's payload length where you will find the next chunk.
class UnknownChunk : public Chunk {
public:
    /// Default constructor
    UnknownChunk() {};
    /// Default destructor.
    virtual ~UnknownChunk() {};

    virtual String getTypeName() const {return "UnknownChunk";};

    virtual int read(File &out_f);
    virtual int execute(const String & = String::EMPTY) {return ERR_OK;};
    virtual int write(File &out_f);
};

/// This class is used to read/write file chunks. A file chunk is a chunk that
/// contains a whole file (compressed or uncompressed). The whole path of the
/// file is stored in the chunk name.
class FileChunk : public Chunk {
    /// The raw chunk content.
    RawDataContainer *m_pRawContent;

    /// The content of the file.
    mutable File::Ptr m_pFileCache;

public:
    FileChunk();
    FileChunk(File::Ptr in_pFile, const String& in_sChunkName = String::EMPTY);
    virtual ~FileChunk();

    virtual String getTypeName() const {return "FileChunk";};

    virtual int read(File &out_f);
    virtual int give(File::Ptr in_pFile, const String& in_sChunkName = String::EMPTY);
    virtual int execute(const String &in_sNewChunkname = String::EMPTY);
    virtual int write(File &out_f);

    operator FTS::File&();
    FTS::File &getFile();
    ConstRawDataContainer getContents() const;

protected:
    virtual RawDataContainer *getRawContent() {return m_pRawContent;};
    friend class File;
};

/// \internal This is used internally only to create the chunk that is read from
/// the file or to create an unknown chunk.
class ChunkFactory {
private:
    /// Protect from construction.
    ChunkFactory() {};

    /// Protect from copying.
    ChunkFactory(ChunkFactory &) {};

    /// Default destructor.
    virtual ~ChunkFactory() {};

public:
    // Only the archive may use me.
    friend class Archive;

    /// \param in_uiChunkID The ID of the chunk to create.
    /// \return the chunk that is described by the ID. If the ID is unknown,
    /// create an unknown chunk.
    Chunk *createChunk(uint8_t in_uiChunkID);

    /// \param in_pChunk The chunk you want to get the ID from.
    /// \return The id that belongs to a chunk of a certain type.
    uint8_t getChunkID(const Chunk *in_pChunk);
};

class Archive {
public:
    typedef std::map<String, Chunk*> ChunkMap;
    typedef std::unique_ptr<Archive> Ptr;

private:
    /// All chunks are stored in this map.
    ChunkMap m_mChunks;

    /// Factory to create the chunks.
    static ChunkFactory m_factory;

    /// The compressor that was used to decompress the archive when loaded.
    Compressor::Ptr m_pOrigComp;

    /// The original file I was loaded from.
    Path m_sFileName;

    Archive(File& out_file, const String& in_sFileChunkPrefix = String::EMPTY);
    Archive(const Path& in_sFileName, Compressor* in_pComp = NULL);
    Archive(PDBrowseInfo out_dbi, const String& in_sChunkPrefix);

    void makeChunksFromDir(PDBrowseInfo out_dbi, const Path& in_sSubdir, const String& in_sChunkPrefix);

public:
    static Archive* createEmptyArchive(const Path& in_sFileName, Compressor* in_pComp = NULL);
    static Archive* loadArchive(const Path& in_sFileName, const String& in_sFileChunkPrefix = String::EMPTY);
    static Archive* loadArchive(File& out_file, const String& in_sFileChunkPrefix = String::EMPTY);
    static bool isValidArchive(const String& in_sFileName);
    static bool isValidArchive(File& out_file);

    virtual ~Archive();

    const Archive& save(File& out_file) const;
    const Archive& save() const;
    inline Compressor& getOriginalCompressor() const {return *m_pOrigComp;};
    inline Path getName() const {return m_sFileName;};
    void unload();
    int execute();

    /// \return true if there is no chunk in the archive, false if there is one.
    /// \note You may use this to check if there was an error while loading the archive.
    inline bool isEmpty() {return this->getChunkCount() == 0;};

    uint64_t getChunkCount() const;
    const Chunk* getChunk(const String& in_sName) const;
    Chunk* getChunk(const String& in_sName);
    bool hasChunk(const String& in_sName) const;

    uint64_t getFileCount() const;
    std::list<String> getFileList() const;

    ChunkMap::const_iterator begin() const;
    ChunkMap::const_iterator end() const;

    Chunk* take(const String& in_sChunkName);
    int give(Chunk *in_pChunk, bool in_bReplace = false);

    File& getFile(const String& in_sName);
    ConstRawDataContainer getFileContent(const String& in_sName) const;
};
};

#endif // D_ARCHIVE_H
