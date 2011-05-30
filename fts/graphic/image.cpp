#include "image.h"
#include "logging/logger.h"
#include "dLib/dFile/dFile.h"
#include "dLib/dCompressor/dCompressor.h" // For default compressor.

#include "3rdparty/lodepng.h"
#include <limits>

using namespace FTS;

/// Default constructor.
ImageFormat::ImageFormat()
    : m_usWidth(0)
    , m_usHeight(0)
    , m_pData(0)
{
}

/// Default destructor.
ImageFormat::~ImageFormat()
{
    this->unload();
}

/// Loads the image from a (possibly compressed) file.
/** This method uses the file it gets to read an image out of it and store it
 *  in memory.
 *
 * \param in_File An opened file that will be read from.
 *
 * \exception CorruptDataException in case there is some error in the file.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
File& ImageFormat::restore(File& in_File)
{
    FTSMSGDBG("Loading image from file '"+in_File.getName()+"'", 3);

    this->unload();

    // First, read all the data out of the file.
    std::vector<unsigned char> data(in_File.getReader()->getSizeTillEnd(), 0);
    in_File.readNoEndian(&data[0], in_File.getReader()->getSizeTillEnd());

    // Then, decode it
    std::vector<unsigned char> image;
    unsigned w, h;
    unsigned error = LodePNG::decode(image, w, h, data);

    if(error != 0 || w >= std::numeric_limits<uint16_t>::max() || h >= std::numeric_limits<uint16_t>::max())
        throw CorruptDataException(in_File.getName(), "This file is bad PNG: " + String::nr(error));

    m_usWidth = static_cast<uint16_t>(w);
    m_usHeight = static_cast<uint16_t>(h);

    // Now we need to copy the resultant data... TODO: avoid this copy.
    m_pData = new uint32_t[m_usWidth * m_usHeight];
    memcpy(m_pData, &image[0], m_usWidth * m_usHeight * 4);

    return in_File;
}

/// Loads the image from a (possibly compressed) file.
/** This opens the file given using the FTS File class that may uncompress that
 *  file on-the-fly. Then it loads the image into memory.
 *
 * \param in_sFileName The name of the file to be loaded.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int ImageFormat::load(const String &in_sFileName)
{
    try {
        this->restore(*File::open(in_sFileName, File::Read));
        return ERR_OK;
    } catch(const ArkanaException& e) {
        e.show();
        return -1;
    }
}

/// Loads the image from a (possibly compressed) data container.
/** This loads an image from the data container given, decompressing it using
 *  the file class and then it loads the image into memory.
 *
 * \param in_pData The data container that contains all the data.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int ImageFormat::load(const ConstRawDataContainer &in_data)
{
    try {
        this->restore(*File::fromRawData(in_data, "(Image from memory)", File::Read, File::CreateOnly));
        return ERR_OK;
    } catch(const ArkanaException& e) {
        e.show();
        return -1;
    }
}

/// Writes the image into a file.
/** This will write the image data in the newest possible version into the file.
 *  It will not save the file, rather it saves the image to the file!
 *
 * \param out_File The file to write the image to.
 *
 * \return the file that it has been written to.
 *
 * \note Don't forget that you may compress the file!
 *
 * \author Pompei2
 */
File& ImageFormat::store(File& out_File) const
{
    FTSMSGDBG("Writing image to file '"+out_File.getName()+"'", 3);

    // First, create the png file in memory.
    std::vector<unsigned char> out;
    LodePNG::encode(out, reinterpret_cast<const unsigned char*>(m_pData), m_usWidth, m_usHeight);

    // Then, write it to the file.
    out_File.writeNoEndian(&out[0], out.size() * sizeof(unsigned char));

    return out_File;
}

/// Writes the image into a (possibly compressed) file.
/** This will create a new file (or overwrite an existing one) and then write
 *  the image into it. The default compressor of the file class is used to
 *  compress the data, this may be none at all.
 *
 * \param in_sFileName The name of the file to create.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int ImageFormat::save(const String &in_sFileName) const
{
    return this->save(in_sFileName, NoCompressor());
}

/// Writes the image into a (possibly compressed) file.
/** This will create a new file (or overwrite an existing one) and then write
 *  the image into it. It is possible to have the file being compressed.
 *
 * \param in_sFileName The name of the file to create.
 * \param in_Comp The compressor to use to compress the image.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int ImageFormat::save(const String &in_sFileName, const Compressor& in_Comp) const
{
    // Create the new file (overwrite existing ones)
    try {
        File::Ptr f = File::overwrite(in_sFileName, File::Overwrite);
        this->store(*f);
        f->save(in_Comp);
        return ERR_OK;
    } catch(const ArkanaException& e) {
        e.show();
        return -1;
    }
}

/// Creates the image from some data in memory.
/** This will create the image using only data from memory.
 *  The data you give it will all be copied, thus you may free it after
 *  the call to this method if you want.
 *
 * \param in_w The width in pixels of the image.
 * \param in_h The height in pixels of the image.
 * \param in_pData The data to use. This have to be \a in_w times \a in_h
 *                 unsigned 32-bit integers that each hold one pixel with the
 *                 values RGBA.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int ImageFormat::createFromData(uint16_t in_w, uint16_t in_h, const uint32_t * const in_pData)
{
    this->unload();

    m_usWidth = in_w;
    m_usHeight = in_h;

    // Copy over the data.
    m_pData = new uint32_t[(uint32_t)m_usWidth*(uint32_t)m_usHeight];
    if(m_pData == NULL) {
        return -1;
    }

    memcpy(m_pData, in_pData, (uint32_t)m_usWidth*(uint32_t)m_usHeight*sizeof(uint32_t));
    return ERR_OK;
}

/// \return ERR_OK
int ImageFormat::unload()
{
    m_usWidth = m_usHeight = 0;
    SAFE_DELETE_ARR(m_pData);
    return ERR_OK;
}
