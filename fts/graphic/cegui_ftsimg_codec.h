#ifndef D_CEGUI_FTSIMG_CODEC_H
#define D_CEGUI_FTSIMG_CODEC_H

#include <CEGUIImageCodec.h>

namespace CEGUI
{
class ImageCodec;

/*!
  \brief
  Default image codec

  This image codec is able to load TGA file only.
  it is always available.
*/
class FTSImageCodec : public ImageCodec {
    static FTSImageCodec *m_pSingleton;
public:
    FTSImageCodec();
    virtual ~FTSImageCodec();
    Texture* load(const RawDataContainer& data, Texture* result);

    static void init();
    static void deinit();
    static FTSImageCodec *getSingletonPtr();
};

/*!
  \brief
  exported function that creates the ImageCodec based object and
  returns a pointer to that object.
*/
extern "C" CEGUI::ImageCodec* createImageCodec();

/*!
  \brief
  exported function that deletes an ImageCodec based object previously
  created by this module.
*/
extern "C" void destroyImageCodec(CEGUI::ImageCodec* imageCodec);

} // End of CEGUI namespace section

#endif /* D_CEGUI_FTSIMG_CODEC_H */
