///
///\file dFile.h
///\author Pompei2
///\date 6 Jan 2009
///\brief This file contains file handling code.
///*/

#ifndef FTS_DFILE_H
#define FTS_DFILE_H

#include "main.h"
#include "dLib/dString/dPath.h"
#include "dLib/dCompressor/dCompressor.h"
#include "utilities/DataContainer.h"
#include "utilities/StreamedDataContainer.h"
#include "main/Exception.h"

#include <memory>
#include <map>
#include <cstdio>  // std::FILE
#include <cstddef> // std::size_t

namespace FTS {
    class Archive;
    class FileChunk;

class UnknownProtocolException : public LoggableException {
public:
    UnknownProtocolException(const Path& in_sFile) throw();
};

class FileNotExistException : public NotExistException {
public:
    FileNotExistException(const Path& in_sFile) throw();
};

class FileAlreadyExistException : public AlreadyExistException {
public:
    FileAlreadyExistException(const Path& in_sFile) throw();
};

class File {
public:
    typedef enum {
        /// Opens the file in read-only mode. Any write trial will display a
        /// warning and return an error.
        Read,
        /// Opens the file in insertion mode. Everything you write to the file
        /// will make it grow, inserting the data at the cursor's position.
        Insert,
        /// Opens the file in overwrite mode. Unless the cursor reaches the end
        /// of the file, everything you write to the file won't make the file
        /// grow, it will just overwrite the current content.
        /// Like in the default mode of most hexeditors.\n
        /// If the cursor is at the end of the file, it will add what you write
        /// to the file and thus make the file grow.\n
        Overwrite,
        /// It is only possible to add data at the end of the file. Before any
        /// writing operation, the cursor is placed at the end of the file.
//         Append,
    } WriteMode;

    typedef enum {
        /// Save the file only if it does not yet exist.
        CreateOnly,
        /// If a file of the same name already exists, just overwrite it.
        OverwriteFile,
    } SaveMode;

    /// The auto-pointer to a file.
    typedef std::unique_ptr<File> Ptr;

protected:
    Path m_sName;     ///< This is the name of the file.
    WriteMode m_mode; ///< How we should write data into this file.
    SaveMode m_saveMode; ///< How to save the file by default later (overwrite?)

    StreamedDataContainer* m_pSDC; ///< The content of the file.

    /// The compressor that was used to open the file. Is never NULL.
    Compressor::Ptr m_pOriginalCompressor;

#ifndef D_FILE_NO_ARCHMAP
    /// A list of archives to look in for files to open.
    static std::map<String, Archive*> m_lArchivesToLook;

    /// Checks if the file with the given name is located in one of the archives to
    /// look in. The archive list works kind-of like a stack: we first look in the
    /// last added archive, then in the second-last and so forth.\n
    ///
    /// The file class has a list of archives that it looks into for a file before
    /// loading it from disk.
    ///
    ///\param in_sFile The filename to look for in the archives (with the path), for
    ///                example bla.fts or Data/bla.fts
    ///\return A pointer to the archive where the file was found or NULL if the file
    ///        is in no archive that has to be checked.
    static std::pair<String, Archive*>isInArchive(const Path &in_sFile);

    /// The ID of the archive I got extracted from, if that is the case.
    String m_sMyArchiveID;
#endif

    std::size_t readRaw(void* out_ptr, std::size_t in_size);
    int insert(const void* in_ptr, std::size_t in_size);
    int writeRaw(const uint8_t* const in_ptr, std::size_t in_size);

    /// This constructs a FTS::File object that just makes a copy of all that data
    /// for itself and decompresses it if necessary. Only for internal use by the
    /// named constructors.
    ///
    ///\param in_sFileName The name of the opened file.
    ///\param in_mode The write mode that should be used when writing to the file.
    ///\param in_saveMode How the default file saving will be handled (overwrite existing?)
    ///\param in_cont The contents to use (make a copy of and maybe decompress)
    ///
    ///\exception CompressorException You may get any of the compressor's exceptions
    ///                               forwarded if something went wrong during
    ///                               decompression of the data.
    File(const Path& in_sFileName, const WriteMode& in_mode, const SaveMode& in_saveMode, const RawDataContainer& in_cont);

    /// \internal Only for class-use.
    File(const File& o);

public:
    /// Destroys the file object. CARE: This will not save the file before doing so.
    virtual ~File();

    /// This not only looks if the file really exists on the harddisk, but it also
    /// checks its availability in an opened archive-to-be-watched or over a
    /// protocol (for example over http, if the address exists).
    ///
    ///\param in_sFileName The name of the file to check (optionally with protocol)
    ///\param in_mode What you want to check accessibility for: only read it,
    ///               modify it or replace it?
    static bool available(const Path& in_sFileName, const WriteMode& in_mode);

    // Let's just have named constructors so everyone knows what he does.

    /// This method opens a file and loads it into memory.
    /// If the file does not exist or can not be opened for reading, an exception
    /// will be thrown. The file is fully loaded into memory ; any operation done
    /// on this object is done in memory, it is only flushed to disk upon calling
    /// the \a save method or a similar one.
    ///
    ///\param in_sFileName The path to the file to open.
    ///\param in_mode How to perform operations on the file by default.
    ///\param in_throw Whether to throw or not upon failure. If it doesn't throw,
    ///                an empty file is returned and all upcoming read operations
    ///                will fail.
    ///
    ///\return A pointer to a new file object containing the whole file. This is never NULL.
    ///\exception FileNotExistException In case the file can't be opened.
    ///\exception UnknownProtocolException In case you use a protocol that is not supported.
    ///\exception CompressorException Something went wrong during decompression.
    ///\exception SyscallException Something else went wrong...
    ///
    ///\note the upcoming write operations will either perform an insertion or over
    ///      write the data or both (or none), depending on the \a in_mode parameter.
    ///      It is still possible to enforce a different write mode later.
    ///\note This is the only named constructor capable to open the standard input.
    ///      If you want to read from the standard input, just open a file named "-"
    ///      (without the quotes) and watch the magic :)\n
    ///      In case you open the standard input, your code may block in this function
    ///      if the standard input is held open, and only continue when it is released
    ///      (e.g. by pressing Ctrl+Z in the console or so..)
    static File::Ptr open(const Path& in_sFileName, const WriteMode& in_mode);

    /// This method creates a file object from reading raw data.
    /// The file object fully resides in memory ; any operation done
    /// on this object is done in memory, it is only flushed to disk upon calling
    /// the \a save method or a similar one.
    ///
    ///\param in_data The data to create the file object with.
    ///\param in_sFileName The path to the file to open.
    ///\param in_mode How to perform operations on the file by default.
    ///\param in_saveMode How a default save operation should act.
    ///
    ///\return An auto-pointer to a new file object containing the whole file.
    ///        This is never NULL.
    ///\exception CompressorException Something went wrong during decompression.
    ///
    ///\note the upcoming write operations will either perform an insertion or over
    ///      write the data or both (or none), depending on the \a in_mode parameter.
    ///      It is still possible to enforce a different write mode later.
    ///\note the data-container is copied, thus it is safe for you to do whatever
    ///      you want with it afterwards.
    static File::Ptr fromRawData(const ConstRawDataContainer& in_data, const Path& in_sFileName, const WriteMode& in_mode, const SaveMode& in_saveMode);

    /// This method creates a file on the disk only if it does not exist yet. If
    /// this succeeds, it opens that file. If it fails (or the file already exists),
    /// it throws an exception. The file is opened in memory and any operation done
    /// on this object is done in memory, it is only flushed to disk upon calling
    /// the \a save method or a similar one.
    ///
    ///\param in_sFileName The path to the file to create.
    ///\param in_mode How to perform operations on the file by default.
    ///\exception FileAlreadyExistException In case the file already exists.
    ///\exception UnknownProtocolException In case you use a protocol that is not supported.
    ///\exception NoRightsException In case you don't have the rights to create that file.
    ///\exception InvalidCallException If you want to create something that is in no
    ///                                way createable (for example the Pipe).
    ///
    ///\note the upcoming write operations will either perform an insertion or over
    ///      write the data or both (or none), depending on the \a in_mode parameter.
    ///      It is still possible to enforce a different write mode later.
    ///\note If you create a file with the name "-", upcoming save methods will write
    ///      to stdout.
    static File::Ptr create(const Path& in_sFileName, const WriteMode& in_mode);

    /// This method creates a file on the disk, possibly overwriting an already
    /// existing one with the same name. If this succeeds, it then opens that file.
    /// If it fails (for example no permission), it throws an exception.
    /// The file is opened in memory and any operation done on this object is done
    /// in memory, it is only flushed to disk upon calling the \a save method or a
    /// similar one.
    ///
    ///\param in_sFileName The path to the file to create.
    ///\param in_mode How to perform operations on the file by default.
    ///\exception UnknownProtocolException In case you use a protocol that is not supported.
    ///\exception NoRightsException In case you don't have the rights to create that file.
    ///\exception InvalidCallException If you want to create something that is in no
    ///                                way createable (for example the Pipe).
    ///
    ///\note the upcoming write operations will either perform an insertion or over
    ///      write the data or both (or none), depending on the \a in_mode parameter.
    ///      It is still possible to enforce a different write mode later.
    ///\note If you create a file with the name "-", upcoming save methods will write
    ///      to stdout.
    static File::Ptr overwrite(const Path& in_sFileName, const WriteMode& in_mode);

    /// This method creates an empty file, but only in memory. Nothing whatsoever
    /// is done on the harddisk in this constructor. Rather, the file will be
    /// created on the disk during a call to \a save and the like. Thus this named
    /// constructor will not fail.\n
    /// Later on, if you call the \a save method, it will fail in case the file
    /// already exists.
    ///
    ///\param in_sFileName The path to the file to be saved later by default.
    ///\param in_mode How to perform operations on the file by default.
    ///
    ///\note the upcoming write operations will either perform an insertion or over
    ///      write the data or both (or none), depending on the \a in_mode parameter.
    ///      It is still possible to enforce a different write mode later.
    static File::Ptr createDelayed(const Path& in_sFileName, const WriteMode& in_mode);

    /// This method creates an empty file, but only in memory. Nothing whatsoever
    /// is done on the harddisk in this constructor. Rather, the file will be
    /// created on the disk during a call to \a save and the like. Thus this named
    /// constructor will not fail.\n
    /// Later on, if you call the \a save method, it will \e overwrite any already
    /// existing file with the same name!
    ///
    ///\param in_sFileName The path to the file to be saved later by default.
    ///\param in_mode How to perform operations on the file by default.
    ///
    ///\note the upcoming write operations will either perform an insertion or over
    ///      write the data or both (or none), depending on the \a in_mode parameter.
    ///      It is still possible to enforce a different write mode later.
    static File::Ptr overwriteDelayed(const Path& in_sFileName, const WriteMode& in_mode);

    // Saving operations.

    /// This method saves the content of the file onto the disk using the file's
    /// original compression.\n
    /// In fact, it flushes all changes made in-memory (by \a write, for example)
    /// onto the disk.\n
    /// If it is impossible to do so (maybe because of insufficient access rights),
    /// this method throws an exception.\n
    /// If the file originally comes from an archive, it will be saved back into
    /// the archive and the archive will be saved.\n
    /// If the file was compressed at the time it has been opened, it is compressed
    /// using exactly the same compressor again.
    ///
    ///\param in_comp A compressor object to use to compress data before saving it.
    ///               Pass a NoCompressor if you don't want data compression.
    ///
    ///\return a reference to this, allowing chaining.
    ///
    ///\note If the file-name is the pipe ("-"), the file's content will be fushed
    ///      to the standard output.
    const File& save() const;

    /// This method saves the content of the file onto the disk.
    /// In fact, it flushes all changes made in-memory (by \a write, for example)
    /// onto the disk.\n
    /// If it is impossible to do so (maybe because of insufficient access rights),
    /// this method throws an exception.\n
    /// If the file originally comes from an archive, it will be saved back into
    /// the archive and the archive will be saved.\n
    /// It is possible to compress the file before saving it using a \a Compressor.
    ///
    ///\param in_comp A compressor object to use to compress data before saving it.
    ///               Pass a NoCompressor if you don't want data compression.
    ///
    ///\return a reference to this, allowing chaining.
    ///
    ///\note If the file-name is the pipe ("-"), the file's content will be fushed
    ///      to the standard output.
    const File& save(const Compressor& in_pComp) const;

    /// This method saves the content of the file onto the disk but with another name.
    /// In case the file already exists, the behaviour of this method is controlled
    /// by the parameter \a in_mode .\n
    /// If it is impossible to finish (maybe because of insufficient access rights),
    /// this method throws an exception.\n
    /// As for \a save, any compressor can be used to compress the file before
    /// saving it. NULL means no compression.\n
    ///
    ///\param in_sOtherFileName The name as which to save the file.
    ///\param in_mode What to do in case the file already exists.
    ///\param in_comp A compressor object to use to compress data before saving it.
    ///               Pass a NoCompressor if you don't want data compression.
    ///\exception UnknownProtocolException In case you want to save into an unknown
    ///                                    protocol.
    ///\exception FileAlreadyExistException In case the file you want to write
    ///                                     already exists and you don't want to
    ///                                     overwrite it.
    ///\exception NoRightsException In case you don't have enough access rights to
    ///                             create/overwrite the file.
    ///\exception SyscallException In case any systemcall (fwrite) failed.
    ///
    ///\note This method does not change the internal file name to the given one,
    ///      thus the next call to \a save will save the file with its original name.
    ///
    ///\return a reference to this, allowing chaining.
    const File& saveAs(const Path& in_sOtherFileName, const SaveMode& in_mode, const Compressor& in_pComp = NoCompressor()) const;

    /// This method saves the modifications that have been done to the file into
    /// a buffer in memory. The buffer may be compressed optionally.
    ///
    ///\param out_buf Where to write the data into. Gets inserted at the cursor pos.
    ///\param in_comp A compressor object to use to compress data before saving it.
    ///               Pass a NoCompressor if you don't want data compression.
    ///
    ///\return a reference to this, allowing chaining.
    ///
    ///\note If the compression somehow fails, the uncompressed data is stored.
    const File& saveToBuf(StreamedDataContainer& out_buf, const Compressor& in_pComp = NoCompressor()) const;

#ifndef D_FILE_NO_ARCHMAP
    /// This adds an archive to the list of archives to check for files on opening.
    /// For more informations on this theme, check our dokuwiki->design docs->map->
    /// flexibility.
    ///
    ///\param in_pArch The archive that has to be added to the list
    ///
    ///\note The archive is added in no special category, it is just added like this.
    static void addArchiveToLook(Archive* in_pArch);

    /// This removes an archive from the list of archives to check for files on opening.
    /// For more informations on this theme, check our dokuwiki->design docs->map->
    /// flexibility.
    ///
    ///\param in_pArch The archive that has to be removed from the list
    ///
    ///\note A warning is displayed in case the archive is not in the list.
    static void remArchiveToLook(Archive* in_pArch);

    /// This removes an archive from the list of archives to check for files on opening.
    /// For more informations on this theme, check our dokuwiki->design docs->map->
    /// flexibility.
    ///
    ///\param in_pArch The archive that has to be removed from the list
    ///
    ///\note This method does not display a warning in case the archive is not registered.
    static void autoRemArchiveToLook(Archive* in_pArch);
#endif

    // Direct access methods.

    /// \return An object that can be used to read data from the file.
    inline ReadableStream* getReader() {return dynamic_cast<ReadableStream*>(m_pSDC);};
    /// \return An object that can be used to write data to the file or NULL if
    ///         the file is opened in read only mode.
    inline WriteableStream* getWriter() { return this->isReadOnly() ? NULL : dynamic_cast<WriteableStream*>(m_pSDC);};
    /// \return The readable and writeable stream that this file holds.
    inline StreamedDataContainer* getStream() { return m_pSDC;};
    /// \return The data container that corresponds to this file. You can access
    ///         the raw data using this one, but in read-only!
    inline ConstRawDataContainer getDataContainer() const {return ConstRawDataContainer(*m_pSDC->getBoundDC());};

    // Information methods.

    /// \return True if the file is loaded, false if not.
    inline bool isLoaded() const {return !m_pSDC->invalid();};
    /// \return True if the end of file has been reached, false else.
    inline bool eof() const {return m_pSDC->eod();};
    /// \return True if the file was compressed during the loading, false if not.
    inline bool wasCompressed() const {return m_pOriginalCompressor->getName() != NoCompressor().getName();};
    /// \return The compressor that was used to load the file.
    ///         An instance of NoCompressor if none was used.
    inline Compressor& getOriginalCompressor() const {return *m_pOriginalCompressor;};

    /// \return Whether the file is opened in read-only mode.
    inline bool isReadOnly() const {return m_mode == Read;};
    /// \return The mode this file has been opened with.
    inline WriteMode getMode() const {return m_mode;};
    /// \return The name of the file that has been given in the constructor.
    inline Path getName() const {return m_sName;};

    /// \return the size of the file or 0 if no file is opened.
    inline uint64_t getSize() const {return m_pSDC->getBoundDC() == NULL ? 0 : m_pSDC->getBoundDC()->getSize();};

    /// This method returns the size of a file.
    ///
    ///\param in_sFileName The name of the file to get the length from.
    ///
    ///\return The size of the file.
    ///\exception FileNotExistException The file specified by \a in_sFileName doesn't exist.
    ///\exception SyscallException Any of the system-calls used to work with the file failed.
    ///
    ///\note Although the return value is a 64-bit unsigned integer, the support for
    ///      large files (>4GB) is not implemented yet.
    ///\note This method may fail if the file doesn't exist or is not readable.
    static uint64_t getSize(const Path &in_sFileName);

    /// This method returns the size of an already-opened file descriptor.
    ///
    ///\param in_pFile The file to get the length from.
    ///
    ///\return The size of the file.
    ///\exception FileNotExistException \a in_pFile is NULL
    ///\exception SyscallException Any of the system-calls used to work with the file failed.
    ///
    ///\note Although the return value is a 64-bit unsigned integer, the support for
    ///      large files (>4GB) is not implemented/tested yet.
    ///\note The file position indicator will be at the same position after a call
    ///      to this method.
    static uint64_t getSize(std::FILE* out_pFile);

    // Data extraction functions, wrappers around the SDC, for convenience.

    /// This places the cursor at an arbitrary position in the file data. If you
    /// try to place it behind the end of file, it will be placed at the end and
    /// a subsequent call to \a eof will return true.
    ///
    ///\param in_uiPos The position where to place the cursor.
    ///\note This is only a wrapper around the streamed data container.
    inline void setCursorPos(uint64_t in_uiPos) {m_pSDC->setCursorPos(in_uiPos);};
    /// \return The current writing cursor position in the file, like ftell().
    /// \note This is only a wrapper around the streamed data container.
    inline uint64_t getCursorPos() const {return m_pSDC->getCursorPos();};
    inline uint32_t fletcher32FromCursor() const {return m_pSDC->fletcher32FromCursor();};

    /// \see ReadableStream::read
    inline std::size_t read(void* out_ptr, std::size_t in_size, std::size_t in_nmemb) {return m_pSDC->read(out_ptr, in_size, in_nmemb);};
    /// \see ReadableStream::readNoEndian
    inline std::size_t readNoEndian(void* out_ptr, std::size_t in_size) {return m_pSDC->readNoEndian(out_ptr, in_size);};
    /// \see ReadableStream::readNoEndian
    inline std::size_t readNoEndian(RawDataContainer &out_data) {return m_pSDC->readNoEndian(out_data);};

    /// This writes data into the file (actually into the memory-buffer of the file)
    /// but the way the data is written depends on the file opening mode.\n
    /// If the file was opened in \a Read mode, no data is written, a warning is
    /// displayed and an error code is returned.\n
    /// If the file was opened in \a Insert mode, all data is inserted at the
    /// current cursor position, making the total file size grow. The cursor will
    /// still move.\n
    /// If the file was opened in \a Overwrite mode, the data will overwrite the
    /// current data that is after the cursor. The cursor will still move. This is a
    /// bit like the default-mode of most hex-editors.\n
    ///\n
    /// If you run a big-endian machine, every data-chunk gets its bytes swapped
    /// into little-endian order before being written. If you are on a little-endian
    /// machine, nothing happens to your bytes, they get written as-is.
    ///
    ///\param in_ptr The data to write.
    ///\param in_size The size of one data-chunk that will be written.
    ///\param in_nmemb This is how much data chunks will be written.
    ///
    ///\return The number of data-chunks actually written. If this value is less
    ///        than \a in_nmemb there was an error during the write.
    ///\note Everything happens in memory, to write back to the file on the disk,
    ///      call the \a save method.
    std::size_t write(const void* in_ptr, std::size_t in_size, std::size_t in_nmemb);

    /// Writes a number of bytes to the file, exactly as they are in the memory,
    /// without any endianness check, byte swap or so.
    ///
    ///\param in_ptr  A pointer to the data that will be written.
    ///\param in_size The size of the data that will be written.
    ///
    ///\return ERR_OK on success, an error code < 0 on failure.
    ///
    ///\note You should only use this method if you///REALLY* know what you're doing.
    ///\note Everything happens in memory, to write back to the file on the disk,
    ///      call the \a save method.
    std::size_t writeNoEndian(const void* in_ptr, std::size_t in_size);

    /// Writes a number of bytes to the file, exactly as they are in the memory,
    /// without any endianness check, byte swap or so.
    ///
    ///\param in_data The data that will be written.
    ///
    ///\return ERR_OK on success, an error code < 0 on failure.
    ///
    ///\note You should only use this method if you///REALLY* know what you're doing.
    ///\note Everything happens in memory, to write back to the file on the disk,
    ///      call the \a save method.
    ///
    ///\author Pompei2
    std::size_t writeNoEndian(const DataContainer& in_data);

    // The following are written WITHOUT the use of templates with intention!
    // That is to force you use these types to be compatible 64bit and so one...

    /// \see ReadableStream::read
    inline ReadableStream& read(int8_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi8
    inline int8_t readi8() {return m_pSDC->readi8();};
    /// \see ReadableStream::read
    inline ReadableStream& read(uint8_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui8
    inline uint8_t readui8() {return m_pSDC->readui8();};
    /// \see ReadableStream::read
    inline ReadableStream& read(int16_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi16
    inline int16_t readi16() {return m_pSDC->readi16();};
    /// \see ReadableStream::read
    inline ReadableStream& read(uint16_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui16
    inline uint16_t readui16() {return m_pSDC->readui16();};
    /// \see ReadableStream::read
    inline ReadableStream& read(int32_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi32
    inline int32_t readi32() {return m_pSDC->readi32();};
    /// \see ReadableStream::read
    inline ReadableStream& read(uint32_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui32
    inline uint32_t readui32() {return m_pSDC->readui32();};
    /// \see ReadableStream::read
    inline ReadableStream& read(int64_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi64
    inline int64_t readi64() {return m_pSDC->readi64();};
    /// \see ReadableStream::read
    inline ReadableStream& read(uint64_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui64
    inline uint64_t readui64() {return m_pSDC->readui64();};
    /// \see ReadableStream::read
    inline ReadableStream& read(float &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readf
    inline float readf() {return m_pSDC->readf();};
    /// \see ReadableStream::read
    inline ReadableStream& read(double &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readd
    inline double readd() {return m_pSDC->readd();};
    /// \see ReadableStream::read
    inline ReadableStream& read(long double &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readld
    inline long double readld() {return m_pSDC->readld();};
    /// \see ReadableStream::read
    inline ReadableStream& read(String &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readstr
    inline String readstr() {return m_pSDC->readstr();};

    /// Writes one 8 bit (1 byte) signed integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(int8_t in);

    /// Writes one 8 bit (1 byte) unsigned integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(uint8_t in);

    /// Writes one 16 bit (2 byte) signed integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(int16_t in);

    /// Writes one 16 bit (2 byte) unsigned integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(uint16_t in);

    /// Writes one 32 bit (4 byte) signed integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(int32_t in);

    /// Writes one 32 bit (4 byte) unsigned integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(uint32_t in);

    /// Writes one 64 bit (8 byte) signed integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(int64_t in);

    /// Writes one 64 bit (8 byte) unsigned integer into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note This converts the data into little-endian format if needed before
    ///       writing it to the file.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(uint64_t in);

    /// Writes one 32 bit (4 byte) floating point value into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(float in);

    /// Writes one 64 bit (8 byte) floating point value into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(double in);

    /// Writes one 128 bit (16 byte) floating point value into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(long double in);

    /// Writes one zero-terminated string into the file.
    /// \param in The data to write.
    /// \return ERR_OK on success, an error code < 0 on failure.
    /// \note All writes only occur in memory. You must call the save method to
    ///       store the changes on the disk.
    int write(const String &in);

    // Again, these can't be templated if we want other classes to define
    // the >> and << operator with a templated input.

    inline File& operator << (int8_t in) { this->write(in); return *this; };
    inline File& operator << (uint8_t in) { this->write(in); return *this; };
    inline File& operator << (int16_t in) { this->write(in); return *this; };
    inline File& operator << (uint16_t in) { this->write(in); return *this; };
    inline File& operator << (int32_t in) { this->write(in); return *this; };
    inline File& operator << (uint32_t in) { this->write(in); return *this; };
    inline File& operator << (int64_t in) { this->write(in); return *this; };
    inline File& operator << (uint64_t in) { this->write(in); return *this; };
    inline File& operator << (float in) { this->write(in); return *this; };
    inline File& operator << (double in) { this->write(in); return *this; };
    inline File& operator << (long double in) { this->write(in); return *this; };
    inline File& operator << (String in) { this->write(in); return *this; };

    inline File& operator >>(int8_t& out) { this->read(out); return *this; };
    inline File& operator >>(uint8_t& out) { this->read(out); return *this; };
    inline File& operator >>(int16_t& out) { this->read(out); return *this; };
    inline File& operator >>(uint16_t& out) { this->read(out); return *this; };
    inline File& operator >>(int32_t& out) { this->read(out); return *this; };
    inline File& operator >>(uint32_t& out) { this->read(out); return *this; };
    inline File& operator >>(int64_t& out) { this->read(out); return *this; };
    inline File& operator >>(uint64_t& out) { this->read(out); return *this; };
    inline File& operator >>(float& out) { this->read(out); return *this; };
    inline File& operator >>(double& out) { this->read(out); return *this; };
    inline File& operator >>(long double& out) { this->read(out); return *this; };
    inline File& operator >>(String& out) { this->read(out); return *this; };
};

namespace FileUtils {

    /// This function takes a path (optionally with a filename appended)
    /// And looks if the path exists, if not, it creates all the directories
    /// that are non-existing.
    ///
    /// \param in_sPath     The path to create.
    /// \param in_bWithFile Whether there is a filename appended to the path or not.
    ///
    /// \return If successfull: ERR_OK
    /// \return If failed:      Error code < 0
    int mkdirIfNeeded(const Path& in_dPath, const bool bWithFile);

    /// Checks if the file \a in_sFileName exists and can be accessed with mode \a in_mode.
    ///
    /// \param in_sFileName The file to look for.
    /// \param in_mode      The mode to test accessing to the file. If it is
    ///                     File::Read, we only check if it is readable/openable. If
    ///                     it is something else, also check if it is writeable.
    ///
    /// \return true If file seems to exist and you have the asked rights on it.
    /// \return false If file doesn't exist or you don't have the asked rights on it
    ///               or there was an error.
    ///
    /// \Note The test depends on the permission of the directories and/or symlinks
    ///       in the path of \a in_pszFileName. \n
    ///       If a directory is found as writeable, it means that we can create files
    ///       in the directory, but we aren't 100 % sure to be able to write the
    ///       directory with directory - handling functions or system( ... ) calls.\n
    bool fileExists(const Path& in_sFileName, const File::WriteMode& in_mode = File::Read);

    /// Checks if the direcotry \a in_sDirName exists.
    /// \param in_sDirName The directory to look for.
    /// \return True if the directory exist, false if not.
    bool dirExists(const Path& in_sDirName);

    /// Checks if the given path exists, whether it is a directory or a file doesn't matter.
    /// \param in_sPathname The path to check for existance.
    /// \return true if the path exists, false if not.
    bool exists(const Path& in_sPathname);

    /// This function copies a file to another one and, if the "other on" already
    /// exists, this function can overwrite it (if you want).
    ///
    /// \param in_sFrom The file to copy
    /// \param in_sTo   The destination filename
    /// \param in_bOverwrite Whether to overwrite the destination or not, if it already exists.
    ///
    /// \return If successfull: ERR_OK
    /// \return If failed:      Error code < 0
    int fileCopy(const Path& in_sFrom, const Path & in_sTo, bool in_bOverwrite = true);

}; // namespace FileUtils

}; // namespace FTS

#endif // FTS_DFILE_H

// EOF
