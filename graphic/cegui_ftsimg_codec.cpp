#include "cegui_ftsimg_codec.h"
#include "CEGUILogger.h"

#include "image.h"
#include "utilities/DataContainer.h"
#include "errtex.h"

namespace CEGUI
{

FTSImageCodec::FTSImageCodec()
    : ImageCodec("FTSImageCodec - Arkana-FTS Image codec for CEGUI")
{
    d_supportedFormat = (utf8*)"png";
}

FTSImageCodec::~FTSImageCodec()
{
}

Texture* FTSImageCodec::load(const RawDataContainer& data, Texture* result)
{
    Logger::getSingleton().logEvent("FTSImageCodec::load()", Informative);

    FTS::ImageFormat fmt;
    FTS::ConstRawDataContainer dc(data.getDataPtr(), data.getSize());
    if(fmt.load(dc) == ERR_OK) {
        result->loadFromMemory(fmt.data(), fmt.w(), fmt.h(), Texture::PF_RGBA);
    } else {
        result->loadFromMemory(g_errTex.pixel_data, g_errTex.width, g_errTex.height, Texture::PF_RGBA);
    }

    return result;
}

FTSImageCodec *FTSImageCodec::m_pSingleton = NULL;

void FTSImageCodec::init()
{
    m_pSingleton = new FTSImageCodec;
}

void FTSImageCodec::deinit()
{
    SAFE_DELETE(m_pSingleton);
}

FTSImageCodec *FTSImageCodec::getSingletonPtr()
{
    return m_pSingleton;
}

CEGUI::ImageCodec* createImageCodec()
{
    return new CEGUI::FTSImageCodec();
}

void destroyImageCodec(CEGUI::ImageCodec* imageCodec)
{
    delete imageCodec;
}

} // End of CEGUI namespace section
