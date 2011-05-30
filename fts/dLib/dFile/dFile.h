/**
 * \file dFile.h
 * \author Pompei2
 * \date 6 Jan 2009
 * \brief This file contains file handling code.
 **/

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

    StreamedDataContainer *m_pSDC; ///< The content of the file.

    /// The compressor that was used to open the file. Is never NULL.
    Compressor::Ptr m_pOriginalCompressor;

#ifndef D_FILE_NO_ARCHMAP
    /// A list of archives to look in for files to open.
    static std::map<String, Archive*> m_lArchivesToLook;

    static std::pair<String, Archive*>isInArchive(const Path &in_sFile);

    /// The ID of the archive I got extracted from, if that is the case.
    String m_sMyArchiveID;
#endif

    std::size_t readRaw(void *out_ptr, std::size_t in_size);
    int insert(const void *in_ptr, std::size_t in_size);
    int writeRaw(const uint8_t * const in_ptr, std::size_t in_size);

    File(const Path& in_sFileName, const WriteMode& in_mode, const SaveMode& in_saveMode, const RawDataContainer& in_cont);
    File(const File& o);

public:
    virtual ~File();

    static bool available(const Path& in_sFileName, const WriteMode& in_mode);

    // Let's just have named constructors so everyone knows what he does.
    static File::Ptr open(const Path& in_sFileName, const WriteMode& in_mode);
    static File::Ptr fromRawData(const ConstRawDataContainer& in_data, const Path& in_sFileName, const WriteMode& in_mode, const SaveMode& in_saveMode);
    static File::Ptr create(const Path& in_sFileName, const WriteMode& in_mode);
    static File::Ptr overwrite(const Path& in_sFileName, const WriteMode& in_mode);
    static File::Ptr createDelayed(const Path& in_sFileName, const WriteMode& in_mode);
    static File::Ptr overwriteDelayed(const Path& in_sFileName, const WriteMode& in_mode);

    // Saving operations.
    const File& save() const;
    const File& save(const Compressor& in_pComp) const;
    const File& saveAs(const Path& in_sOtherFileName, const SaveMode& in_mode, const Compressor& in_pComp = NoCompressor()) const;
//     RawDataContainer *saveToBuf(Compressor *in_pComp) const;
    const File& saveToBuf(StreamedDataContainer& out_buf, const Compressor& in_pComp = NoCompressor()) const;

#ifndef D_FILE_NO_ARCHMAP
    static void addArchiveToLook(Archive *in_pArch);
    static void remArchiveToLook(Archive *in_pArch);
    static void autoRemArchiveToLook(Archive *in_pArch);
#endif

    // Direct access methods.

    /// \return An object that can be used to read data from the file.
    inline ReadableStream *getReader() {return dynamic_cast<ReadableStream *>(m_pSDC);};
    /// \return An object that can be used to write data to the file or NULL if
    ///         the file is opened in read only mode.
    inline WriteableStream *getWriter() { return this->isReadOnly() ? NULL : dynamic_cast<WriteableStream *>(m_pSDC);};
    /// \return The readable and writeable stream that this file holds.
    inline StreamedDataContainer *getStream() { return m_pSDC;};
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
    static uint64_t getSize(const Path &in_sFileName);
    static uint64_t getSize(std::FILE *out_pFile);

    // Data extraction functions, wrappers around the SDC, for convenience.

    /** This places the cursor at an arbitrary position in the file data. If you
     *  try to place it behind the end of file, it will be placed at the end and
     *  a subsequent call to \a eof will return true.
     *
     * \param in_uiPos The position where to place the cursor.
     * \note This is only a wrapper around the streamed data container.
     */
    inline void setCursorPos(uint64_t in_uiPos) {m_pSDC->setCursorPos(in_uiPos);};
    /// \return The current writing cursor position in the file, like ftell().
    /// \note This is only a wrapper around the streamed data container.
    inline uint64_t getCursorPos() const {return m_pSDC->getCursorPos();};
    inline uint32_t fletcher32FromCursor() const {return m_pSDC->fletcher32FromCursor();};

    /// \see ReadableStream::read
    inline std::size_t read(void *out_ptr, std::size_t in_size, std::size_t in_nmemb) {return m_pSDC->read(out_ptr, in_size, in_nmemb);};
    /// \see ReadableStream::readNoEndian
    inline std::size_t readNoEndian(void *out_ptr, std::size_t in_size) {return m_pSDC->readNoEndian(out_ptr, in_size);};
    /// \see ReadableStream::readNoEndian
    inline std::size_t readNoEndian(RawDataContainer &out_data) {return m_pSDC->readNoEndian(out_data);};

    std::size_t write(const void *in_ptr, std::size_t in_size, std::size_t in_nmemb);
    std::size_t writeNoEndian(const void *in_ptr, std::size_t in_size);
    std::size_t writeNoEndian(const DataContainer &in_data);

    // The following are written WITHOUT the use of templates with intention!
    // That is to force you use these types to be compatible 64bit and so one...

    /// \see ReadableStream::read
    inline ReadableStream *read(int8_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi8
    inline int8_t readi8() {return m_pSDC->readi8();};
    /// \see ReadableStream::read
    inline ReadableStream *read(uint8_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui8
    inline uint8_t readui8() {return m_pSDC->readui8();};
    /// \see ReadableStream::read
    inline ReadableStream *read(int16_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi16
    inline int16_t readi16() {return m_pSDC->readi16();};
    /// \see ReadableStream::read
    inline ReadableStream *read(uint16_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui16
    inline uint16_t readui16() {return m_pSDC->readui16();};
    /// \see ReadableStream::read
    inline ReadableStream *read(int32_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi32
    inline int32_t readi32() {return m_pSDC->readi32();};
    /// \see ReadableStream::read
    inline ReadableStream *read(uint32_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui32
    inline uint32_t readui32() {return m_pSDC->readui32();};
    /// \see ReadableStream::read
    inline ReadableStream *read(int64_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readi64
    inline int64_t readi64() {return m_pSDC->readi64();};
    /// \see ReadableStream::read
    inline ReadableStream *read(uint64_t &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readui64
    inline uint64_t readui64() {return m_pSDC->readui64();};
    /// \see ReadableStream::read
    inline ReadableStream *read(float &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readf
    inline float readf() {return m_pSDC->readf();};
    /// \see ReadableStream::read
    inline ReadableStream *read(double &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readd
    inline double readd() {return m_pSDC->readd();};
    /// \see ReadableStream::read
    inline ReadableStream *read(long double &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readld
    inline long double readld() {return m_pSDC->readld();};
    /// \see ReadableStream::read
    inline ReadableStream *read(String &out) {return m_pSDC->read(out);};
    /// \see ReadableStream::readstr
    inline String readstr() {return m_pSDC->readstr();};

    int write(int8_t in);
    int write(uint8_t in);
    int write(int16_t in);
    int write(uint16_t in);
    int write(int32_t in);
    int write(uint32_t in);
    int write(int64_t in);
    int write(uint64_t in);
    int write(float in);
    int write(double in);
    int write(long double in);
    int write(const String &in);

    template<typename T>
    File& operator <<(const T& in) {
        this->write(in);
        return *this;
    };

    template<typename T>
    File& operator >>(T& out) {
        this->read(out);
        return *this;
    };
};

namespace FileUtils {
bool fileExists(const Path & in_sFileName, const File::WriteMode& in_mode = File::Read);
bool dirExists(const Path & in_sDirName);
int mkdirIfNeeded(const Path & in_dPath, const bool bWithFile);
int fileCopy(const Path & in_sFrom, const Path & in_sTo, bool in_bOverwrite = true);
};

};

#endif // FTS_DFILE_H

 /* EOF */
