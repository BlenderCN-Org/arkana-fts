#ifndef D_IMAGE_H
#define D_IMAGE_H

#include "main.h"
#include "dLib/dString/dString.h"

namespace FTS {
    class File;
    class Compressor;
    class ConstRawDataContainer;

/// This class is a wrapper around our own image format. This will NOT load an
/// image into the graphics card neither will it display one! It is only used to
/// load the image out of a file, decompress it, handle different file formats
/// and finally have it stored in the memory in some useful format.\n
/// It may then be used later-on by another (graphics) class to upload the data
/// to the graphics card and display it. It may also be used to convert images
/// or to save an image in our format.
class ImageFormat {
    /// The width of the image, in pixels.
    uint16_t m_usWidth;
    /// The height of the image, in pixels.
    uint16_t m_usHeight;
    /// Pointer to the raw data (w*h pixels, one pixel is RGBA as uint32_t).
    uint32_t *m_pData;

    /// Protect from copying.
    ImageFormat(const ImageFormat &) {};

public:
    ImageFormat();
    virtual ~ImageFormat();

    File& restore(File& in_File);
    File& store(File& out_File) const;
    int unload();
    int createFromData(uint16_t in_w, uint16_t in_h, const uint32_t * const in_pData);

    // For convenience:
    int load(const ConstRawDataContainer &in_data);
    int load(const String &in_sFileName);
    int save(const String &in_sFileName) const;
    int save(const String &in_sFileName, const Compressor& in_pComp) const;

    /// \return The width of the image, in pixels.
    inline uint16_t w() const {return m_usWidth;};
    /// \return The height of the image, in pixels.
    inline uint16_t h() const {return m_usHeight;};
    /// \return A pointer to the raw data, containing w*h pixes, one pixel is RGBA.
    inline uint32_t *data() const {return m_pData;};
};

inline File& operator<<(File& o, const ImageFormat& fmt) {
    return fmt.store(o);
}

inline File& operator>>(File& i, ImageFormat& fmt) {
    return fmt.restore(i);
}

} // namespace FTS

#endif /* D_IMAGE_H */

 /* EOF */
