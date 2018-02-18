#include "packet.h"

#include "3d/3d.h"

#include "dLib/dFile/dFile.h"
#include "dLib/dConf/configuration.h"
#include "graphic/errtex.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "3d/3d.h"
#include "3d/Mathfwd.h"
#include "logging/logger.h"
#include "logging/Chronometer.h"
#include "utilities/Math.h"

#ifndef D_NOCEGUI
#  include <CEGUI.h>
#  include <CEGUIDefaultResourceProvider.h>
#endif

#include <cmath> // floor and ceil

using namespace FTS;

Graphic::TextureFilter Graphic::toTexureFilter(int i)
{
    if(i == (int)Nearest_Nearest)
        return Nearest_Nearest;

    if(i == (int)Linear_Linear)
        return Linear_Linear;

    if(i == (int)Linear_LinearMipMap)
        return Linear_LinearMipMap;

    return DefaultFilter;
}

Graphic::Anisotropy Graphic::toAnisotropy(int i)
{
    if(i == (int)FullAnisotropy)
        return FullAnisotropy;

    if(i == (int)NoAnisotropy)
        return NoAnisotropy;

    return DefaultAnisotropy;
}

/// Default constructor
Graphic::Graphic()
    : m_uiID(0)
    , m_uiRealW(0)
    , m_uiRealH(0)
    , m_uiW(0)
    , m_uiH(0)
    , m_usedAnisotropy(DefaultAnisotropy)
    , m_usedFilter(DefaultFilter)
    , m_pGrabbedPixels(0)
{
    m_pfTexcoord[0] = m_pfTexcoord[1] = m_pfTexcoord[2] = m_pfTexcoord[3] = 0.0f;
}

/// Default destructor
Graphic::~Graphic()
{
    this->destroy();
    SAFE_DELETE_ARR(m_pGrabbedPixels);
}

/// Creates a new texture in the graphic card's memory.
/** This method stores the texture in the graphic card's memory and sets
 *  everything up so that it is ready to be drawn.
 *
 * \param in_pData Pointer to the pixel-data.
 * \param in_uiW The width in pixels of the image.
 * \param in_uiH The height in pixels of the image.
 * \param in_forceAnisotropy Any anisotropy filtering wants to be forced?
 * \param in_forceFilter Do you want to force a texture filter to be used?
 *
 * \return ERR_OK if successful, an error code < 0 on failure.
 *
 * \note The data pointed to by \a in_pData must be 32-bit RRGGBBAA format.
 * \note You can free the data pointed to by \a in_pData after having called
 *       this method if you want as it is COPIED over.
 *
 * \exception InvalidCallException when data is NULL.
 *
 * \author Pompei2
 */
void Graphic::create(const uint8_t * const in_pData, uint16_t in_uiW, uint16_t in_uiH, TextureFilter in_forceFilter, Anisotropy in_forceAnisotropy)
{
    verifGL("Graphic::create(" + String::nr(in_uiW) + "x" + String::nr(in_uiH) + ") start");
    // Preliminary sanity checks.
    const uint64_t uiMaxTexSize = GraphicManager::getSingleton().getMaxTextureSize();
    if(in_pData == NULL)
        throw InvalidCallException("Graphic::create(NULL)");

    if(in_uiW > uiMaxTexSize)
        throw HardwareLimitException("Texture width", in_uiW, uiMaxTexSize);
    if(in_uiH > uiMaxTexSize)
        throw HardwareLimitException("Texture height", in_uiH, uiMaxTexSize);

    // Destroy this if it is already loaded.
    if(this->isLoaded()) {
        this->destroy();
    }

    // Use the surface width and height expanded to powers of 2 because OPENGL
    // only accepts textures with power of 2.
    // TODO: Make use of non-power-of-2 textures extensions?
    m_uiW = in_uiW;
    m_uiH = in_uiH;

    if(FTS::glHasExtension("GL_ARB_texture_non_power_of_two")) {
        m_uiRealW = in_uiW;
        m_uiRealH = in_uiH;
    } else {
        m_uiRealW = power_of_two(in_uiW);
        m_uiRealH = power_of_two(in_uiH);
    }

    // Calculate the texcoords that got to be applied to the quad.
    m_pfTexcoord[0] = 0.0f;
    m_pfTexcoord[1] = 0.0f;
    m_pfTexcoord[2] = (float)m_uiW / (float)m_uiRealW;
    m_pfTexcoord[3] = (float)m_uiH / (float)m_uiRealH;

    // Alloc a surface of powered by two sizes.
    uint8_t *pPOTData = new uint8_t[(uint32_t)m_uiRealW*(uint32_t)m_uiRealH*4];

    // Set it all to zero.
    memset(pPOTData, sizeof(uint8_t), (uint32_t)m_uiRealW*(uint32_t)m_uiRealH*4);

    // Copy the data into that one.
    for(uint16_t y=0 ; y < m_uiH ; ++y) {
        memcpy(pPOTData + (uint32_t)y*m_uiRealW*4, in_pData + (uint32_t)y*m_uiW*4, (uint32_t)m_uiW*4);
    }

    // Ask for the memory for the texture.
    glGenTextures(1, &m_uiID);
    verifGL("Graphic::create(" + String::nr(in_uiW) + "x" + String::nr(in_uiH) + ") glGenTextures");
    FTSMSGDBG("Created texture with ID "+String::nr(m_uiID), 3);

    // Set the texture's params
    glBindTexture(GL_TEXTURE_2D, m_uiID);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Choose the filter, depending on what the settings are.
    // But we can force a filter in code by setting the
    // in_iForceFilter parameter to a value different then 0.
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    if(in_forceFilter == DefaultFilter)
        m_usedFilter = toTexureFilter(conf.getInt("TextureFilter"));
    else
        m_usedFilter = in_forceFilter;

    switch(m_usedFilter) {
    case Nearest_Nearest:
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    case Linear_LinearMipMap:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        break;
    case Linear_Linear:
    default:
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    }

    // Choose the filter, depending on what the settings are.
    // But we can force a filter in code by setting the
    // in_iForceFilter parameter to a value different then 0.
    if(in_forceAnisotropy == DefaultAnisotropy)
        m_usedAnisotropy = toAnisotropy(conf.getBool("Anisotropic"));
    else
        m_usedAnisotropy = in_forceAnisotropy;

    // Set the desired anisotropy filtering.
    switch(m_usedAnisotropy) {
    case FullAnisotropy:
/*        if(GLEE_EXT_texture_filter_anisotropic) {
            GLfloat fMaxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fMaxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fMaxAnisotropy);
        }*/
        break;
    case NoAnisotropy:
    default:
        break;
    }
    verifGL("Graphic::create(" + String::nr(in_uiW) + "x" + String::nr(in_uiH) + ") glTexParameterf");

    // And finally create the OpenGL texture(s).
    /// \todo: mipmaps in our own file format.
/*    if(m_usedFilter == Linear_LinearMipMap)
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, m_uiRealW, m_uiRealH, GL_RGBA, GL_UNSIGNED_BYTE, pPOTData);
    else*/
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_uiRealW, m_uiRealH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPOTData);

    verifGL("Graphic::create(" + String::nr(in_uiW) + "x" + String::nr(in_uiH) + ") end");
    SAFE_DELETE_ARR(pPOTData);
}

/// Destroys this texture object.
/** This method destroys the textore object on the graphic card, freeing all its
 *  memory and making it impossible to use it anymore.
 *
 * \author Pompei2
 */
void Graphic::destroy()
{
    verifGL("Graphic::destroy(id="+String::nr(m_uiID)+") start");
    if(this->isLoaded()) {
        glDeleteTextures(1, &m_uiID);
    }
    verifGL("Graphic::destroy(id="+String::nr(m_uiID)+") end");

    m_uiID = 0;
    m_pfTexcoord[0] = m_pfTexcoord[1] = m_pfTexcoord[2] = m_pfTexcoord[3] = 0.0f;
    m_uiRealW = 0;
    m_uiRealH = 0;
    m_uiW = 0;
    m_uiH = 0;
    m_usedAnisotropy = DefaultAnisotropy;
    m_usedFilter = DefaultFilter;
}

bool Graphic::isLoaded() const
{
    return (m_uiID != 0) && glIsTexture(m_uiID);
}

/// Copies the pixel-data from the graphics card into a buffer.
/** This function gets all pixel-data that belong to this texture from the
 *  graphics card (this might be slow!) and copies it over into a buffer. A
 *  mip-mapped texture will only copy the "original" size mipmap.
 *
 * \param in_bRealTextureSize Set this to true if you want the copied pixels to
 *        be of the **real** texture size, that is the size of the texture in
 *        graphics card memory, that can be bigger then your actual image.\n
 *        If you set this to false, you will just get the image that has been
 *        loaded from disk, memory or whatever.
 *
 * \return A pointer to a buffer containing the pixel data as described above.
 *         This buffer must be freed by the caller using the SAFE_DELETE_ARR macro.
 *
 * \author Pompei2
 */
uint8_t *Graphic::copyPixels(bool in_bRealTextureSize) const
{
    if(!this->isLoaded()) {
        FTS18N("InvParam", MsgType::Horror, "Graphic::copyPixels, m_uiID = "+String::nr(m_uiID));
        return NULL;
    }

    verifGL("Graphic::copyPixels(id="+String::nr(m_uiID)+") start");

    // First at all, select the texture and store the previously selected one.
    // NOT using this->select as we want to hide those changes, not screw up the
    //     manager's state.
    GLint iLastTex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iLastTex);
    glBindTexture(GL_TEXTURE_2D, m_uiID);

    // Now, Verify it has the right format.
    int iW = 0, iH = 0, iFmt = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iW);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iH);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &iFmt);

    if(iFmt != GL_RGBA || iW != m_uiRealW || iH != m_uiRealH) {
        glBindTexture(GL_TEXTURE_2D, iLastTex);
        FTS18N("InvParam", MsgType::Horror, "Graphic::copyPixels(inconsistent data), m_uiID = "+String::nr(m_uiID));
        verifGL("Graphic::copyPixels(id="+String::nr(m_uiID)+") badend");
        return NULL;
    }

    uint8_t *pMem = new uint8_t[(uint32_t)m_uiRealW*(uint32_t)m_uiRealH*4];
    if(pMem == NULL) {
        glBindTexture(GL_TEXTURE_2D, iLastTex);
        verifGL("Graphic::copyPixels(id="+String::nr(m_uiID)+") badend2");
        return NULL;
    }

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pMem);

    // Restore the old texture.
    glBindTexture(GL_TEXTURE_2D, iLastTex);

    verifGL("Graphic::copyPixels(id="+String::nr(m_uiID)+") end");

    // If for a good hazard (or the rectangle texture extension is used) the
    // size of the OpenGL texture matches the original image size, this is an
    // easy task, just use these pixels.
    if((iW == m_uiW && iH == m_uiH) || in_bRealTextureSize) {
        return pMem;
    } else {
    // Else, we extract only the "useful" pictures, that is the pictures of the
    // original image.
        if(!pMem) return NULL;
        uint8_t *pGoodMem = new uint8_t[(uint32_t)m_uiW*(uint32_t)m_uiH*4];
        for(int y = 0 ; y < m_uiH ; ++y) {
            memcpy(&pGoodMem[y*m_uiW*4], &pMem[y*m_uiRealW*4], m_uiW*4);
        }
        delete [] pMem;
        return pGoodMem;
    }
}

/// Creates a CEGUI Imageset containing this image.
/** This function creates a new CEGUI Imageset containing only this image.
 *  The image is called "image" and the Imageset is called whatever you want.
 *
 * \param in_sImagesetName The name of the imageset that should be created.
 * \param in_bReplace Wether to replace the imageset if it already exists or not.
 *
 * \return A pointer to the imageset that has been created or NULL in case of error.
 *
 * \note Setting \a in_bReplace to true may cause problems if the old imageset is still
 *       used somewhere !
 * \note You may create pseudo-anonymous imagesets by setting \a in_sImagesetName
 *       to an empty string: the method will choose a random, available name
 *       for the imageset automagically.
 *
 * \author Pompei2
 */
CEGUI::Imageset *Graphic::createCEGUI(const String &in_sImagesetName, bool in_bReplace) const
{
    CEGUI::String sImgSetName;
    CEGUI::ImagesetManager *pImgMgr = CEGUI::ImagesetManager::getSingletonPtr();
    if(!pImgMgr || !this->isLoaded())
        return NULL;

    // Find a disponible random name for the imageset ...
    if(in_sImagesetName.empty()) {
        do {
            String sName = String::random("##########", random<int>);
            if(!pImgMgr->isImagesetPresent(sName))
                sImgSetName = sName;
        } while(sImgSetName.empty());
    // ... Or check if the imageset is already present and destroy it if wanted.
    } else {
        try {
            if(pImgMgr->isImagesetPresent(in_sImagesetName)) {
                if(!in_bReplace)
                    return NULL;

                pImgMgr->destroyImageset(in_sImagesetName);
            }
            sImgSetName = in_sImagesetName;
        } catch(CEGUI::Exception & e) {
            FTS18N("CEGUI", MsgType::Error, e.getMessage());
            return NULL;
        }
    }

    // Get the data from opengl.
    uint8_t *pData = this->copyPixels(true);
    CEGUI::Imageset *pImgSet = NULL;

    // And then load it into CEGUI.
    try {
        CEGUI::Texture *pTex = CEGUI::System::getSingleton().getRenderer()->createTexture();
        pTex->loadFromMemory(pData, m_uiRealW, m_uiRealH, CEGUI::Texture::PF_RGBA);
        pImgSet = pImgMgr->createImageset(sImgSetName, pTex);
        pImgSet->setAutoScalingEnabled(false);
        pImgSet->defineImage("image", CEGUI::Rect(0.0f,0.0f,(float)this->getW(),(float)this->getH()), CEGUI::Point(0.0f,0.0f));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        pImgSet = NULL;
    }

    SAFE_DELETE_ARR(pData);
    return pImgSet;
}

int Graphic::draw(int16_t in_iX, int16_t in_iY) const
{
    const int32_t iX_W = (int32_t)in_iX + (int32_t)m_uiW;
    const int32_t iY_H = (int32_t)in_iY + (int32_t)m_uiH;

    verifGL("Graphic::draw(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") start");
    this->select();

    // Draw a rectangle with the texture on it.
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
        // Top left
      glTexCoord2f(m_pfTexcoord[0], m_pfTexcoord[1]);
      glVertex2i(in_iX, in_iY);
        // Top right
      glTexCoord2f(m_pfTexcoord[2], m_pfTexcoord[1]);
      glVertex2i(iX_W, in_iY);
        // Bottom left.
      glTexCoord2f(m_pfTexcoord[0], m_pfTexcoord[3]);
      glVertex2i(in_iX, iY_H);
        // Bottom Right
      glTexCoord2f(m_pfTexcoord[2], m_pfTexcoord[3]);
      glVertex2i(iX_W, iY_H);
    glEnd();
    verifGL("Graphic::draw(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") end");

    return ERR_OK;
}

int Graphic::drawCentered(int16_t in_iX, int16_t in_iY) const
{
    return this->draw(in_iX - (int16_t)(((float)(m_uiW)) / 2.0f),
                      in_iY - (int16_t)(((float)(m_uiH)) / 2.0f));
}

int Graphic::drawSub(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
                     uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH) const
{
    const int32_t iX = (int32_t)in_iX + (int32_t)in_iSubX;
    const int32_t iY = (int32_t)in_iY + (int32_t)in_iSubY;
    const uint32_t iW = iX + (uint32_t)in_iSubW;
    const uint32_t iH = iX + (uint32_t)in_iSubH;
    const int32_t iX_W = iX + iW;
    const int32_t iY_H = iY + iH;
    const float fLeftTex = m_pfTexcoord[0] + (float)in_iSubX/(float)m_uiW;
    const float fRightTex = m_pfTexcoord[2] - ((float)m_uiW-(float)in_iSubW-(float)in_iSubX)/(float)m_uiW;
    const float fTopTex = m_pfTexcoord[1] + (float)in_iSubY/(float)m_uiH;
    const float fBottomTex = m_pfTexcoord[3] - ((float)m_uiH-(float)in_iSubH-(float)in_iSubY)/(float)m_uiH;

    verifGL("Graphic::drawSub(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") start");

    this->select();

    // Draw a rectangle with the texture on it.
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
        // Top left
      glTexCoord2f(fLeftTex, fTopTex);
      glVertex2i(iX, iY);
        // Top right
      glTexCoord2f(fRightTex, fTopTex);
      glVertex2i(iX_W, iY);
        // Bottom left.
      glTexCoord2f(fLeftTex, fBottomTex);
      glVertex2i(iX, iY_H);
        // Bottom Right
      glTexCoord2f(fRightTex, fBottomTex);
      glVertex2i(iX_W, iY_H);
    glEnd();
    verifGL("Graphic::drawSub(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") end");

    return ERR_OK;
}

int Graphic::drawSubCentered(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
                             uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH) const
{
    return this->drawSub(in_iX - (int16_t)(((float)(m_uiW)) / 2.0f),
                         in_iY - (int16_t)(((float)(m_uiH)) / 2.0f),
                         in_iSubX, in_iSubY, in_iSubW, in_iSubH);
}

int Graphic::drawRot(int16_t in_iX, int16_t in_iY, float in_fDegrees) const
{
    glPushMatrix();
    glRotatef(in_fDegrees, 0.0f, 0.0f, 1.0f);

    const int32_t iX_W = (int32_t)in_iX + (int32_t)m_uiW;
    const int32_t iY_H = (int32_t)in_iY + (int32_t)m_uiH;

    verifGL("Graphic::drawRot(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") start");
    this->select();

    // Draw a rectangle with the texture on it.
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
        // Top left
      glTexCoord2f(m_pfTexcoord[0], m_pfTexcoord[1]);
      glVertex2i(in_iX, in_iY);
        // Top right
      glTexCoord2f( m_pfTexcoord[2], m_pfTexcoord[1]);
      glVertex2i(iX_W, in_iY);
        // Bottom left.
      glTexCoord2f( m_pfTexcoord[0], m_pfTexcoord[3]);
      glVertex2i(in_iX, iY_H);
        // Bottom Right
      glTexCoord2f(m_pfTexcoord[2], m_pfTexcoord[3]);
      glVertex2i(iX_W, iY_H);
    glEnd();
    glPopMatrix();

    verifGL("Graphic::drawRot(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") end");
    return ERR_OK;
}

int Graphic::drawRotCentered(int16_t in_iX, int16_t in_iY, float in_fDegrees) const
{
    return this->drawRot(in_iX - (int16_t)(((float)(m_uiW)) / 2.0f),
                         in_iY - (int16_t)(((float)(m_uiH)) / 2.0f),
                         in_fDegrees);
}

int Graphic::drawZoom(int16_t in_iX, int16_t in_iY, float in_fZoomX, float in_fZoomY) const
{
    const float fX_WZX = (float)in_iX + (float)m_uiW*in_fZoomX;
    const float fY_HZY = (float)in_iY + (float)m_uiH*in_fZoomY;

    verifGL("Graphic::drawZoom(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") start");
    this->select();

    // And finally, draw a rectangle with the texture on it.
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
        // Top left
      glTexCoord2f(m_pfTexcoord[0], m_pfTexcoord[1]);
      glVertex2f((float)in_iX, (float)in_iY);
        // Top right
      glTexCoord2f(m_pfTexcoord[2], m_pfTexcoord[1]);
      glVertex2f(fX_WZX, (float)in_iY);
        // Bottom left.
      glTexCoord2f(m_pfTexcoord[0], m_pfTexcoord[3]);
      glVertex2f((float)in_iX, fY_HZY);
        // Bottom Right
      glTexCoord2f(m_pfTexcoord[2], m_pfTexcoord[3]);
      glVertex2f(fX_WZX, fY_HZY);
    glEnd();
    verifGL("Graphic::drawZoom(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") end");

    return ERR_OK;
}

int Graphic::drawZoomCentered(int16_t in_iX, int16_t in_iY, float in_fZoomX, float in_fZoomY) const
{
    return this->drawZoom(in_iX - (uint16_t)(((float)m_uiW*in_fZoomX) / 2.0f),
                          in_iY - (uint16_t)(((float)m_uiH*in_fZoomY) / 2.0f),
                          in_fZoomX, in_fZoomY);
}

int Graphic::drawColoured(int16_t in_iX, int16_t in_iY,
                          float in_fR, float in_fG, float in_fB, float in_fA) const
{
    const int32_t iX_W = (int32_t)in_iX + (int32_t)m_uiW;
    const int32_t iY_H = (int32_t)in_iY + (int32_t)m_uiH;

    verifGL("Graphic::drawColoured(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") start");
    this->select();

    // Draw a rectangle with the texture on it.
    glColor4f(in_fR, in_fG, in_fB, in_fA);
    glBegin(GL_TRIANGLE_STRIP);
        // Top left
      glTexCoord2f(m_pfTexcoord[0], m_pfTexcoord[1]);
      glVertex2i(in_iX, in_iY);
        // Top right
      glTexCoord2f( m_pfTexcoord[2], m_pfTexcoord[1]);
      glVertex2i(iX_W, in_iY);
        // Bottom left.
      glTexCoord2f( m_pfTexcoord[0], m_pfTexcoord[3]);
      glVertex2i(in_iX, iY_H);
        // Bottom Right
      glTexCoord2f(m_pfTexcoord[2], m_pfTexcoord[3]);
      glVertex2i(iX_W, iY_H);
    glEnd();
    verifGL("Graphic::drawColoured(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") end");

    return ERR_OK;
}

int Graphic::drawColouredCentered(int16_t in_iX, int16_t in_iY,
                                  float in_fR, float in_fG, float in_fB, float in_fA) const
{
    return this->drawColoured(in_iX - (int16_t)(((float)(m_uiW)) / 2.0f),
                              in_iY - (int16_t)(((float)(m_uiH)) / 2.0f),
                              in_fR, in_fG, in_fB, in_fA);
}

int Graphic::drawEx(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
                    uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH,
                    float in_fRotate, float in_fZoomX, float in_fZoomY,
                    float in_fR, float in_fG, float in_fB, float in_fA) const
{
    verifGL("Graphic::drawEx(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") start");
    this->select();

    // And finally, draw a rectangle with the texture on it.
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_TRIANGLE_STRIP );
        // Top left
      glTexCoord2f( m_pfTexcoord[0], m_pfTexcoord[1] );
      glVertex2f( (float)in_iX, (float)in_iY );
        // Top right
      glTexCoord2f( m_pfTexcoord[2], m_pfTexcoord[1] );
      glVertex2f( (float)in_iX + (float)m_uiW*in_fZoomX, (float)in_iY );
        // Bottom left.
      glTexCoord2f( m_pfTexcoord[0], m_pfTexcoord[3] );
      glVertex2f( (float)in_iX, (float)in_iY + (float)m_uiH*in_fZoomY );
        // Bottom Right
      glTexCoord2f( m_pfTexcoord[2], m_pfTexcoord[3] );
      glVertex2f( (float)in_iX + (float)m_uiW*in_fZoomX, (float)in_iY + (float)m_uiH*in_fZoomY );
    glEnd( );
    verifGL("Graphic::drawEx(id="+String::nr(m_uiID)+", "+String::nr(in_iX)+","+String::nr(in_iY)+") end");

    return ERR_OK;
}

int Graphic::drawExCentered(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
                            uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH,
                            float in_fRotate, float in_fZoomX, float in_fZoomY,
                            float in_fR, float in_fG, float in_fB, float in_fA) const
{
    return this->drawEx(in_iX - (int16_t)(((float)(m_uiW)*in_fZoomX) / 2.0f),
                        in_iY - (int16_t)(((float)(m_uiH)*in_fZoomY) / 2.0f),
                        in_iSubX, in_iSubY, in_iSubW, in_iSubH,
                        in_fRotate,            // Rotate it a bit ?
                        in_fZoomX, in_fZoomY,
                        in_fR, in_fG, in_fB, in_fA);
}

/// Selects a texture to be used on the next drawn GL primitives.
/** This method Selects a texture, that means all future drawing that doesn't
 *  select another texture will be textured with this texture.
 *
 * \author Pompei2
 */
void Graphic::select(uint8_t in_iTexUnit) const
{
    verifGL("Graphic::select(id="+String::nr(m_uiID)+", texUnit="+String::nr(in_iTexUnit)+") start");
    // We don't call this->isLoaded() as this one calling glIsTexture may be slow.
    if(m_uiID != 0) {
        // Check for silly input arguments and ignore them.
        if(in_iTexUnit > GraphicManager::getSingleton().getMaxTextureUnits())
            return;

        // Do not re-select the same texture again.
        if(GraphicManager::getSingleton().isGraphicSelected(in_iTexUnit, this->getID()))
            return;

        // Bind it where it has to be bound.
        glActiveTexture(GL_TEXTURE0 + in_iTexUnit);
        glEnable(GL_TEXTURE_2D); /// \TODO: We only need GL_TEXTURE_2D as long as we use old-style OpenGL! (Currently only in quad.cpp)
        glBindTexture(GL_TEXTURE_2D, m_uiID);
        GraphicManager::getSingleton().setSelectedGraphic(in_iTexUnit, this->getID());
        verifGL("Graphic::select(id="+String::nr(m_uiID)+", texUnit="+String::nr(in_iTexUnit)+") end");
    } else {
        // Select the error texture:
        GraphicManager::getSingleton().getErrorTexture()->select(in_iTexUnit);
        GraphicManager::getSingleton().setSelectedGraphic(in_iTexUnit, GraphicManager::getSingleton().getErrorTexture()->getID());
    }
}

/// This copies the graphic's pixels from the graphic card back into RAM in a
/// way that it can later be reloaded back into the graphic card's memory.
/// \return true if the grab was successful, false if not.
/// \note This also completely removes the graphic from the graphic card
///       (it deletes the opengl texture)
bool Graphic::grab()
{
    if(!this->isLoaded())
        return true;

    verifGL("Graphic::grab(id="+String::nr(m_uiID)+") start");
    m_pGrabbedPixels = this->copyPixels(false);

    // Calling destroy() is _not_ the good thing. We need to keep the w, h, ...
    glDeleteTextures(1, &m_uiID);
    verifGL("Graphic::grab(id="+String::nr(m_uiID)+") start");

    // But some of the vars need to be reset.
    m_uiID = 0;
    m_pfTexcoord[0] = m_pfTexcoord[1] = m_pfTexcoord[2] = m_pfTexcoord[3] = 0.0f;
    m_uiRealW = 0;
    m_uiRealH = 0;

    return m_pGrabbedPixels != 0;
}

/// Restores the graphic as it was grabbed before and discards the grab.
/// \return true if the restore was successful, false if not or if it has never
///         been grabbed before.
bool Graphic::restore()
{
    if(!m_pGrabbedPixels)
        return false;

    // Re-create myself... grabbed data gets lost here.
    this->create(m_pGrabbedPixels, this->getW(), this->getH(), m_usedFilter, m_usedAnisotropy);
    SAFE_DELETE_ARR(m_pGrabbedPixels);
    return true;
}

/// Write this image into another packet.
/** This writes the current image (this) into a packet (\a in_pPacket ) at
 *  the current cursor position in the packet.\n
 *  To see more details about how this is done, refer to the Dokuwiki
 *  http://wiki.arkana-fts.org/doku.php?id=design_documents:networking:misc:images
 *  The category design_documents/networking/misc/images.
 *
 * \param in_pPack The packet where to write this image into.
 *
 * \return ERR_OK or an error code below 0.
 *
 * \author Pompei2
 */
int Graphic::writeToPacket(Packet *in_pPacket) const
{
    /// TODO: If the graphic is not loaded, what to do? WRITE SOMETHING!
    if(!this->isLoaded())
        return -1;

    // Write informations to the packet.
    in_pPacket->append((uint16_t)m_uiW);
    in_pPacket->append((uint16_t)m_uiH);
    in_pPacket->append((int8_t)m_usedAnisotropy);
    in_pPacket->append((int8_t)m_usedFilter);

    uint8_t *pData = this->copyPixels(false);

    // Write data to the packet.
    in_pPacket->append(pData, (uint32_t)m_uiW*(uint32_t)m_uiH*4);

    SAFE_DELETE_ARR(pData);
    return ERR_OK;
}

ImageFormat *Graphic::toFTSImageFormat() const
{
    /// TODO: If the graphic is not loaded, what to do? WRITE SOMETHING!
    if(!this->isLoaded())
        return NULL;

    uint8_t *pData = this->copyPixels(false);
    if(pData == NULL)
        return NULL;

    ImageFormat *fmt = new ImageFormat;
    fmt->createFromData(m_uiW, m_uiH, reinterpret_cast<uint32_t*>(pData));

    SAFE_DELETE_ARR(pData);
    return fmt;
}

#define MITCHELL

#ifdef BOXFILTER
double filterSupport = 0.5;
double filter(double pos)
{
    if((pos > -0.5) && (pos <= 0.5)) return(1.0);
    return(0.0);
}
#elif defined(LANCZOS3)
double filterSupport = 3.0;

double sinc(double x)
{
    x *= M_PI;
    if(x != 0) return(sin(x) / x);
    return(1.0);
}

double filter(double pos)
{
    if(pos < 0) pos = -pos;
    if(pos < 3.0) return(sinc(pos) * sinc(pos/3.0));
    return(0.0);
}
#else /* MITCHELL */
double filterSupport = 2.0;

#define B   (1.0 / 3.0)
#define C   (1.0 / 3.0)

double filter(double pos)
{
    double pos2 = pos*pos;
    if(pos < 0) pos = -pos;
    if(pos < 1.0) {
        pos = (((12.0 - 9.0 * B - 6.0 * C) * (pos * pos2))
             + ((-18.0 + 12.0 * B + 6.0 * C) * pos2)
             + (6.0 - 2 * B));
        return(pos / 6.0);
    } else if(pos < 2.0) {
        pos = (((-1.0 * B - 6.0 * C) * (pos * pos2))
             + ((6.0 * B + 30.0 * C) * pos2)
             + ((-12.0 * B - 48.0 * C) * pos)
             + (8.0 * B + 24 * C));
        return(pos / 6.0);
    }
    return(0.0);
}
#endif

typedef std::pair<uint16_t, double> Contrib;
typedef std::list<Contrib> ContribList;
Graphic *Graphic::copyResized(uint16_t in_uiNewW, uint16_t in_uiNewH) const
{
    uint8_t *pOrigData = this->copyPixels(false);
    uint8_t *pScaledWData = new uint8_t[4*in_uiNewW*this->getW()];
    uint8_t *pScaledData = new uint8_t[4*in_uiNewW*in_uiNewH];

    // Inspired by filter.c from Graphic Gems III
    double xScale = (double)in_uiNewW / (double)this->getW();
    double yScale = (double)in_uiNewH / (double)this->getH();

    // First, scale every row. For this, calculate the contributing pixels
    // in the source image for every pixel in the destination image.
    ContribList *pContribs = new ContribList[in_uiNewW];

    // Mignification
    if(xScale < 1.0) {
        double width = filterSupport / xScale;

        for(uint16_t x = 0 ; x < in_uiNewW ; ++x) {
            double center = (double)x / xScale;
            int left = static_cast<int>(ceil(center - width));
            int right = static_cast<int>(floor(center + width));
            for(int filterX = left ; filterX <= right ; ++filterX) {
                double weight = filter((center - (double)filterX) * xScale) * xScale;
                // Don't go over the borders of the images, then give another guy my weight.
                if(filterX < 0)
                    pContribs[x].push_back(Contrib(-filterX, weight));
                else if(filterX >= this->getW())
                    pContribs[x].push_back(Contrib(2*this->getW()-filterX-1, weight));
                else
                    pContribs[x].push_back(Contrib(filterX, weight));
            }
        }
    // Magnification, a bit easier.
    } else {
        for(uint16_t x = 0 ; x < in_uiNewW ; ++x) {
            double center = (double)x / xScale;
            int left = static_cast<int>(ceil(center - filterSupport));
            int right = static_cast<int>(floor(center + filterSupport));
            for(int filterX = left ; filterX <= right ; ++filterX) {
                double weight = filter(center - (double)filterX);
                // Don't go over the borders of the images, then give another guy my weight.
                if(filterX < 0)
                    pContribs[x].push_back(Contrib(-filterX, weight));
                else if(filterX >= this->getW())
                    pContribs[x].push_back(Contrib(2*this->getW()-filterX-1, weight));
                else
                    pContribs[x].push_back(Contrib(filterX, weight));
            }
        }
    }

#define XYR(x, y, w) (y)*(w)*4+(x)*4+0
#define XYG(x, y, w) (y)*(w)*4+(x)*4+1
#define XYB(x, y, w) (y)*(w)*4+(x)*4+2
#define XYA(x, y, w) (y)*(w)*4+(x)*4+3

    // Now, apply the filter to magnify horizontally first.
    for(uint16_t y = 0 ; y < this->getH() ; ++y) {
        for(uint16_t x = 0 ; x < in_uiNewW ; ++x) {
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
            // Sum the colors of the pixels around scaled by their weight.
            for(ContribList::iterator i = pContribs[x].begin() ; i != pContribs[x].end() ; ++i) {
                r += static_cast<float>(pOrigData[XYR(i->first,y,this->getW())]*i->second);
                g += static_cast<float>(pOrigData[XYG(i->first,y,this->getW())]*i->second);
                b += static_cast<float>(pOrigData[XYB(i->first,y,this->getW())]*i->second);
                a += static_cast<float>(pOrigData[XYA(i->first,y,this->getW())]*i->second);
            }

            pScaledWData[XYR(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(r, 0.0f, 255.0f));
            pScaledWData[XYG(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(g, 0.0f, 255.0f));
            pScaledWData[XYB(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(b, 0.0f, 255.0f));
            pScaledWData[XYA(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(a, 0.0f, 255.0f));
        }
    }

    // Now, go on to scale every column, just like the rows.
    SAFE_DELETE_ARR(pContribs);
    pContribs = new ContribList[in_uiNewH];

    // Mignification
    if(yScale < 1.0) {
        double height = filterSupport / yScale;

        for(uint16_t y = 0 ; y < in_uiNewH ; ++y) {
            double center = (double)y / yScale;
            int top = static_cast<int>(ceil(center - height));
            int bottom = static_cast<int>(floor(center + height));
            for(int filterY = top ; filterY <= bottom ; ++filterY) {
                double weight = filter((center - (double)filterY) * yScale) * yScale;
                // Don't go over the borders of the images, then give another guy my weight.
                if(filterY < 0)
                    pContribs[y].push_back(Contrib(-filterY, weight));
                else if(filterY >= this->getH())
                    pContribs[y].push_back(Contrib(2*this->getH()-filterY-1, weight));
                else
                    pContribs[y].push_back(Contrib(filterY, weight));
            }
        }
    // Magnification, a bit easier.
    } else {
        for(uint16_t y = 0 ; y < in_uiNewH ; ++y) {
            double center = (double)y / yScale;
            int top = static_cast<int>(ceil(center - filterSupport));
            int bottom = static_cast<int>(floor(center + filterSupport));
            for(int filterY = top ; filterY <= bottom ; ++filterY) {
                double weight = filter(center - (double)filterY);
                // Don't go over the borders of the images, then give another guy my weight.
                if(filterY < 0)
                    pContribs[y].push_back(Contrib(-filterY, weight));
                else if(filterY >= this->getH())
                    pContribs[y].push_back(Contrib(2*this->getH()-filterY-1, weight));
                else
                    pContribs[y].push_back(Contrib(filterY, weight));
            }
        }
    }

    // Now, apply the filter to magnify vertically.
    for(uint16_t y = 0 ; y < in_uiNewH ; ++y) {
        for(uint16_t x = 0 ; x < in_uiNewW ; ++x) {
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
            // Sum the colors of the pixels around scaled by their weight.
            for(ContribList::iterator i = pContribs[y].begin() ; i != pContribs[y].end() ; ++i) {
                r += static_cast<float>(pScaledWData[XYR(x,i->first,in_uiNewW)]*i->second);
                g += static_cast<float>(pScaledWData[XYG(x,i->first,in_uiNewW)]*i->second);
                b += static_cast<float>(pScaledWData[XYB(x,i->first,in_uiNewW)]*i->second);
                a += static_cast<float>(pScaledWData[XYA(x,i->first,in_uiNewW)]*i->second);
            }

            pScaledData[XYR(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(r, 0.0f, 255.0f));
            pScaledData[XYG(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(g, 0.0f, 255.0f));
            pScaledData[XYB(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(b, 0.0f, 255.0f));
            pScaledData[XYA(x,y,in_uiNewW)] = static_cast<uint8_t>(clamp(a, 0.0f, 255.0f));
        }
    }

    // Done with scaling, create the image.
    Graphic *pScaledGraph = new Graphic();
    pScaledGraph->create(pScaledData, in_uiNewW, in_uiNewH, m_usedFilter, m_usedAnisotropy);

    SAFE_DELETE_ARR(pContribs);
    SAFE_DELETE_ARR(pOrigData);
    SAFE_DELETE_ARR(pScaledWData);
    SAFE_DELETE_ARR(pScaledData);
    return pScaledGraph;
}

////////////////////////////////////////////////////////////////////////////////
// GRAPHIC MANAGER
////////////////////////////////////////////////////////////////////////////////

#include "errtex.h"

const String FTS::GraphicManager::ErrorTextureName = "Error";

GraphicManager::GraphicManager()
{
    FTSMSGDBG("Initialised graphics manager, max texture size: "+String::nr(this->getMaxTextureSize()), 2);

    // Create the texture that will be used for errorneous textures.
    Graphic *pErrGraph = new Graphic();
    pErrGraph->create(reinterpret_cast<const uint8_t * const>(g_errTex.pixel_data), g_errTex.width, g_errTex.height);
    m_mGraphicsFromFiles[GraphicManager::ErrorTextureName] = pErrGraph;

    m_vSelectedTextures.assign(this->getMaxTextureUnits(), 0);
}

GraphicManager::~GraphicManager()
{
    FTSMSGDBG("Destroyed graphics manager", 2);

    // Everything should already be deleted.

    for(std::map<String, Graphic *>::iterator i = m_mGraphicsFromFiles.begin() ; i != m_mGraphicsFromFiles.end() ; ++i) {
        // If there was an error loading that graphic, we close one eye.
        if(i->second == this->getErrorTexture())
            continue;

        FTS18N("Graph_NotUnloaded_File", MsgType::Horror, i->first);
    }

    // First, search that graphic in the list of graphics made from memory.
    for(std::list<Graphic *>::iterator i = m_lGraphicsFromMem.begin() ; i != m_lGraphicsFromMem.end() ; ++i) {
        // If there was an error loading that graphic, we close one eye.
        if(*i == this->getErrorTexture())
            continue;

        FTS18N("Graph_NotUnloaded_Mem", MsgType::Horror, String::nr((*i)->getW()), String::nr((*i)->getH()));
    }

    // Destroy the error texture.
    SAFE_DELETE(m_mGraphicsFromFiles[GraphicManager::ErrorTextureName]);

    m_mGraphicsFromFiles.clear();
    m_lGraphicsFromMem.clear();
}

Graphic *GraphicManager::getOrLoadGraphic(File& out_file, String in_sGraphicName,
                                          Graphic::TextureFilter in_forceFilter,
                                          Graphic::Anisotropy in_forceAnisotropy)
{
    // No name specified? Use the file's name.
    if(in_sGraphicName.empty()) {
        in_sGraphicName = out_file.getName();
    }

    // Check if the graphic has already been loaded.
    std::map<String, Graphic*>::iterator i = m_mGraphicsFromFiles.find(in_sGraphicName);
    if(i != m_mGraphicsFromFiles.end())
        return i->second;

    try {
        // If we come here, it means the graphic has not yet been loaded. Do so!
        LoggingChronometer loadChron("Loading of graphic " + in_sGraphicName + " from file " + out_file.getName(), 2);

        // Read the content of the file and decode it.
        ImageFormat fmt;
        out_file >> fmt;

        // Create a new texture with the decoded data.
        Graphic* pGraph = new Graphic();
        pGraph->create(reinterpret_cast<uint8_t *>(fmt.data()), fmt.w(), fmt.h(),
                       in_forceFilter, in_forceAnisotropy);

        // If it hasn't been loaded successfully, we still add it to the map as being
        // the error texture.

        // We can safely add it into the map at that position as the map does not
        // contain anything on this position ; tested before.
        m_mGraphicsFromFiles[in_sGraphicName] = pGraph;

        loadChron.measure();
        return pGraph;
    } catch(const ArkanaException& e) {
        e.show();
        return this->getErrorTextureNonConst();
    }
}

Graphic *GraphicManager::getOrLoadGraphic(const String &in_sFileName,
                                          Graphic::TextureFilter in_forceFilter,
                                          Graphic::Anisotropy in_forceAnisotropy)
{
    // Check if the graphic has already been loaded.
    std::map<String, Graphic*>::iterator i = m_mGraphicsFromFiles.find(in_sFileName);
    if(i != m_mGraphicsFromFiles.end())
        return i->second;

    try {
        LoggingChronometer loadChron("Loading of graphic file " + in_sFileName, 2);

        // Read the content of the file and decode it.
        ImageFormat fmt;
        *File::open(in_sFileName, File::Read) >> fmt;

        // Create a new texture with the decoded data.
        Graphic* pGraph = new Graphic();
        pGraph->create(reinterpret_cast<uint8_t *>(fmt.data()), fmt.w(), fmt.h(),
                       in_forceFilter, in_forceAnisotropy);

        // We can safely add it into the map at that position as the map does not
        // contain anything on this position ; tested before.
        m_mGraphicsFromFiles[in_sFileName] = pGraph;

        loadChron.measure();
        return pGraph;
    } catch(const ArkanaException& e) {
        e.show();

        // If it hasn't been loaded successfully, we still add it to the map as being
        // the error texture. To avoid hundreds of reload trials.
        m_mGraphicsFromFiles[in_sFileName] = this->getErrorTextureNonConst();

        return this->getErrorTextureNonConst();
    }
}

/// Extracts an image out of a packet.
/** This extracts an image (stored earlyer using the \a writeToPacket method)
 *  from a packet (\a in_pPacket ). The extracted image will be stored in this.\n
 *  This will be unloaded first.
 *
 * \param in_pPack The packet to extract an image from.
 *
 * \return ERR_OK or an error code below 0.
 *
 * \author Pompei2
 */
Graphic *GraphicManager::readGraphicFromPacket(Packet *out_pPacket)
{
    LoggingChronometer loadChron("Loading of graphic from packet", 3);

    // Read the informations from the packet.
    uint16_t usW = 0, usH = 0;
    int8_t iUsedAnisotropy = 0, iUsedFilter = 0;
    out_pPacket->get(usW);
    out_pPacket->get(usH);
    out_pPacket->get(iUsedAnisotropy);
    out_pPacket->get(iUsedFilter);

    // If some invalid data came, stop here.
    if(usW > 4096 || usH > 4096)
        return this->getErrorTextureNonConst();

    // Read the data from the packet.
    uint8_t *pData = new uint8_t[(uint32_t)usW*(uint32_t)usH*4];
    out_pPacket->get(pData, (uint32_t)usH*(uint32_t)usW*4);

    // Create the graphic from the data packet.
    Graphic *g = this->createGraphicFromData(pData, usW, usH,
                                             Graphic::toTexureFilter(iUsedFilter),
                                             Graphic::toAnisotropy(iUsedAnisotropy));
    SAFE_DELETE_ARR(pData);
    loadChron.measure();
    return g;
}

Graphic *GraphicManager::createGraphicFromData(const uint8_t * const in_pData,
                                               uint16_t in_uiW, uint16_t in_uiH,
                                               Graphic::TextureFilter in_forceFilter,
                                               Graphic::Anisotropy in_forceAnisotropy)
{
    LoggingChronometer loadChron("Creating "+String::nr(in_uiW)+"x"+String::nr(in_uiH)+" graphic from data", 3);
    Graphic *g = new Graphic;
    g->create(in_pData, in_uiW, in_uiH, in_forceFilter, in_forceAnisotropy);
    m_lGraphicsFromMem.push_back(g);
    loadChron.measure();
    return g;
}

Graphic *GraphicManager::getOrCreateResizedGraphic(const String &in_sOrig, uint16_t in_uiW, uint16_t in_uiH)
{
    String sNameOrig = in_sOrig;

    if(!this->isGraphicPresent(in_sOrig))
        sNameOrig = GraphicManager::ErrorTextureName;

    String sName(sNameOrig + String::nr(in_uiW) + "x" + String::nr(in_uiH));

    // If an error texture of this size has already been created, use that one.
    if(this->isGraphicPresent(sName))
        return this->getOrLoadGraphic(sName);

    // At this point we are sure that a graphic named 'sNameOrig' is present in the map.
    Graphic *pOrig = this->getOrLoadGraphic(sNameOrig);

    // Now create a resized version of it.
    LoggingChronometer loadChron("Scaling graphic " + sNameOrig + " to " + String::nr(in_uiW) + "x" + String::nr(in_uiH), 2);

    // Create a new texture with the resized data.
    Graphic *pGraph = pOrig->copyResized(in_uiW, in_uiH);

    // We can safely add it into the map at that position as the map does not
    // contain anything on this position ; tested before.
    m_mGraphicsFromFiles[sName] = pGraph;

#ifdef DEBUG
    pGraph->toFTSImageFormat()->save(Path::userdir("Logfiles") + Path(sName + ".png"));
#endif

    loadChron.measure();
    return pGraph;
}

std::list<Graphic *>::iterator GraphicManager::findUnnamedGraphicFromMem(const Graphic *in_pGraphic)
{
    std::list<Graphic*>::iterator i = m_lGraphicsFromMem.begin();

    for( ; i != m_lGraphicsFromMem.end() ; ++i) {
        if(*i == in_pGraphic) {
            return i;
        }
    }

    // Not found? return the end!
    return i;
}

std::map<String, Graphic *>::iterator GraphicManager::findUnnamedGraphicFromFile(const Graphic *in_pGraphic)
{
    std::map<String, Graphic*>::iterator i = m_mGraphicsFromFiles.begin();
    for( ; i != m_mGraphicsFromFiles.end() ; ++i) {
        // We won't give you the error texture! you'd delete it! (happens rly!)
        if(i->second == in_pGraphic && i->first != GraphicManager::ErrorTextureName) {
            return i;
        }
    }

    // Not found? return the end!
    return i;
}

void GraphicManager::destroyGraphic(const String &in_sFileName)
{
    // Don't destroy the error texture.
    if(in_sFileName == GraphicManager::ErrorTextureName)
        return;

    FTSMSGDBG("Destroying graphic " + in_sFileName, 4);

    std::map<String, Graphic*>::iterator i = m_mGraphicsFromFiles.find(in_sFileName);

    // If we got no graphic named like this, we just quit.
    if(i == m_mGraphicsFromFiles.end()) {
        FTSMSGDBG(" -> Inexistent in system, skipped", 4);
        return ;
    }

    // Delete the graphic only if it is not the error texture.
    if(i->second != this->getErrorTexture()) {
        SAFE_DELETE(i->second);
    } else {
        FTSMSGDBG(" -> Was error texture .. skipped", 4);
    }

    // Take it out of the list.
    m_mGraphicsFromFiles.erase(i);
}

void GraphicManager::destroyGraphic(Graphic *& in_graphic)
{
    if(in_graphic == NULL || in_graphic == this->getErrorTexture()) {
        return ;
    }

    // First, search that graphic in the list of graphics made from memory.
    std::list<Graphic *>::iterator iMem = this->findUnnamedGraphicFromMem(in_graphic);
    if(iMem != m_lGraphicsFromMem.end()) {
        FTSMSGDBG("Destroying unnamed graphic from mem ("+String::nr((*iMem)->getW())+"x"+String::nr((*iMem)->getH())+")", 4);

        // Only delete the graphic if it is not the error graphic!
        if(*iMem != this->getErrorTexture()) {
            SAFE_DELETE(*iMem);
        } else {
            FTSMSGDBG(" -> It was error texture .. skipped", 4);
        }

        m_lGraphicsFromMem.erase(iMem);
        in_graphic = NULL; // Invalidate the pointer.
        return ;
    }

    // Then, search it in the list of graphics made from a file:
    std::map<String, Graphic *>::iterator iFile = this->findUnnamedGraphicFromFile(in_graphic);
    if(iFile != m_mGraphicsFromFiles.end()) {
        String sName = iFile->first;
        FTSMSGDBG("Destroying graphic " + sName, 4);

        // Again, don't delete the graphic if it is the error graphic but
        // used in another name.
        if(iFile->second != this->getErrorTexture()) {
            SAFE_DELETE(iFile->second);
        } else {
            FTSMSGDBG(" -> It was error texture .. skipped", 4);
        }

        m_mGraphicsFromFiles.erase(iFile);
        in_graphic = NULL; // Invalidate the pointer.
        return ;
    }

    // Not found ? better not delete it, but display a warning!
    FTS18N("InvParam", MsgType::Warning, "GraphicManager::destroyGraphic unknown ptr");
}

bool GraphicManager::isGraphicPresent(const String &in_sFileName)
{
    return m_mGraphicsFromFiles.find(in_sFileName) != m_mGraphicsFromFiles.end();
}

const Graphic *GraphicManager::getErrorTexture() const
{
    std::map<String, Graphic *>::const_iterator i = m_mGraphicsFromFiles.find(GraphicManager::ErrorTextureName);
    if(i == m_mGraphicsFromFiles.end())
        return NULL;
    return i->second;
}

uint64_t GraphicManager::getMaxTextureSize() const
{
    verifGL("Graphic::getMaxTextureSize start");
    GLint iMaxTex = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iMaxTex);
    verifGL("Graphic::getMaxTextureSize end");
    return (uint64_t)iMaxTex;
}

uint8_t GraphicManager::getMaxTextureUnits() const
{
    verifGL("Graphic::getMaxTextureUnits start");
    GLint iMaxTex = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxTex);
    verifGL("Graphic::getMaxTextureUnits end");
    return (uint8_t)iMaxTex;
}

/// Changes the name of a graphic in the manager.
/** This changes the name with which a graphic is registered in the manager.
 *  The new name must not be "Error" and must not match to any currently
 *  registered graphic.
 *
 * \param in_sOldName The current name the graphic you want to rename has.
 * \param in_sNewName The new name with which the graphic should be registered.
 *
 * \return ERR_OK or an error code below 0.
 *
 * \author Pompei2
 */
int GraphicManager::renameGraphic(const String &in_sOldName, const String &in_sNewName)
{
    if(!this->isGraphicPresent(in_sOldName)
       || in_sOldName == GraphicManager::ErrorTextureName
       || this->isGraphicPresent(in_sNewName)
      )
        return -1;

    // Here, we're sure this exists.
    std::map<String, Graphic *>::iterator i = m_mGraphicsFromFiles.find(in_sOldName);
    Graphic *g = i->second;
    m_mGraphicsFromFiles.erase(i);
    m_mGraphicsFromFiles[in_sNewName] = g;
    return ERR_OK;
}

/// Resizes a graphic to a certain size.
/** Except for the error texture, this changes the size of a graphic registered
 *  under a certain name. The original graphic will be lost! and replaced by
 *  the resized one. It will keep the same name.
 *
 * \param in_sOrig Name of the graphic to resize.
 * \param in_uiNewW The new width of the graphic (after the resize).
 * \param in_uiNewH The new height of the graphic (after the resize).
 *
 * \return The resized graphic, or the Error Texture in case of an error.
 *
 * \author Pompei2
 */
Graphic *GraphicManager::resizeGraphic(const String &in_sOrig, uint16_t in_uiNewW, uint16_t in_uiNewH)
{
    // Don't resize the error graphic.
    if(in_sOrig == GraphicManager::ErrorTextureName || !this->isGraphicPresent(in_sOrig))
        return this->getErrorTextureNonConst();

    Graphic *pGraph = this->getOrCreateResizedGraphic(in_sOrig, in_uiNewW, in_uiNewH);
    this->destroyGraphic(in_sOrig);
    this->renameGraphic(in_sOrig + String::nr(in_uiNewW) + "x" + String::nr(in_uiNewH), in_sOrig);
    return pGraph;
}

/// Resizes a graphic by a certain ratio.
/** Except for the error texture, this changes the size of a graphic registered
 *  under a certain name. The original graphic will be lost! and replaced by
 *  the resized one. It will keep the same name.\n
 *
 *  Both the width and the height are changed by the same factor, that is
 *  \a in_fRatio. If the ratio is < 1 then the graphic gets shrinked, if it is
 *  > 1 the graphic gets magnified.
 *
 * \param in_sOrig Name of the graphic to resize.
 * \param in_fRatio The resizing ratio.
 *
 * \return The resized graphic, or the Error Texture in case of an error.
 *
 * \author Pompei2
 */
Graphic *GraphicManager::resizeGraphic(const String &in_sOrig, float in_fRatio)
{
    // Don't resize the error graphic.
    if(in_sOrig == GraphicManager::ErrorTextureName || !this->isGraphicPresent(in_sOrig))
        return this->getErrorTextureNonConst();

    Graphic *pGraph = this->getOrLoadGraphic(in_sOrig);

    return this->resizeGraphic(in_sOrig, static_cast<uint16_t>((float)pGraph->getW() * in_fRatio),
                                         static_cast<uint16_t>((float)pGraph->getH() * in_fRatio));
}

/// Resizes a graphic to a certain size.
/** Except for the error texture, this changes the size of a graphic. If the
 *  graphic is registered under a certain name, the name will be kept.\n
 *  The original graphic will be lost! and replaced by the resized one.
 *
 * \param in_pOrig The graphic to resize. If the operation is successfull, the
 *                 pointer will be redirected to the resized graphic.
 * \param in_uiNewW The new width of the graphic (after the resize).
 * \param in_uiNewH The new height of the graphic (after the resize).
 *
 * \return The resized graphic, or the Error Texture in case of an error.
 *
 * \author Pompei2
 */
Graphic *GraphicManager::resizeGraphic(Graphic *& in_pOrig, uint16_t in_uiNewW, uint16_t in_uiNewH)
{
    if(in_pOrig == NULL) {
        return this->getErrorTextureNonConst();
    }

    // First, search that graphic in the list of graphics made from memory.
    std::list<Graphic *>::iterator iMem = this->findUnnamedGraphicFromMem(in_pOrig);
    // Only resize the graphic if it is not the error graphic!
    if(iMem != m_lGraphicsFromMem.end() && *iMem != this->getErrorTexture()) {
        // Resize it.
        Graphic *pResizedGraph = in_pOrig->copyResized(in_uiNewW, in_uiNewH);
        // Replace the orig by the resized.
        this->destroyGraphic(in_pOrig);
        m_lGraphicsFromMem.push_back(pResizedGraph);
        in_pOrig = pResizedGraph;
        return pResizedGraph;
    }

    // Then, search it in the list of graphics made from a file:
    std::map<String, Graphic *>::iterator iFile = this->findUnnamedGraphicFromFile(in_pOrig);
    // Again, don't resize the graphic if it is the error graphic but
    // used in another name.
    if(iFile != m_mGraphicsFromFiles.end() && iFile->second != this->getErrorTexture()) {
        // Delegate the work.
        in_pOrig = this->resizeGraphic(iFile->first, in_uiNewW, in_uiNewH);
        return in_pOrig;
    }

    // Not found ? better not delete it, but display a warning!
    FTS18N("InvParam", MsgType::Warning, "GraphicManager::resizeGraphic unknown ptr");
    return this->getErrorTextureNonConst();
}

/// Resizes a graphic by a certain ratio.
/** Except for the error texture, this changes the size of a graphic.  If the
 *  graphic is registered under a certain name, the name will be kept.\n
 *  The original graphic will be lost! and replaced by the resized one.
 *
 *  Both the width and the height are changed by the same factor, that is
 *  \a in_fRatio. If the ratio is < 1 then the graphic gets shrinked, if it is
 *  > 1 the graphic gets magnified.
 *
 * \param in_pOrig The graphic to resize. If the operation is successfull, the
 *                 pointer will be redirected to the resized graphic.
 * \param in_uiNewW The new width of the graphic (after the resize).
 * \param in_fRatio The resizing ratio.
 *
 * \return The resized graphic, or the Error Texture in case of an error.
 *
 * \author Pompei2
 */
Graphic *GraphicManager::resizeGraphic(Graphic *&in_pOrig, float in_fRatio)
{
    if(in_pOrig == NULL) {
        return this->getErrorTextureNonConst();
    }

    // First, search that graphic in the list of graphics made from memory.
    std::list<Graphic *>::iterator iMem = this->findUnnamedGraphicFromMem(in_pOrig);
    // Only resize the graphic if it is not the error graphic!
    if(iMem != m_lGraphicsFromMem.end() && *iMem != this->getErrorTexture()) {
        // Resize it.
        Graphic *pResizedGraph = in_pOrig->copyResized(static_cast<uint16_t>(in_pOrig->getW()*in_fRatio),
                                                       static_cast<uint16_t>(in_pOrig->getH()*in_fRatio));
        // Replace the orig by the resized.
        this->destroyGraphic(in_pOrig);
        m_lGraphicsFromMem.push_back(pResizedGraph);
        in_pOrig = pResizedGraph;
        return pResizedGraph;
    }

    // Then, search it in the list of graphics made from a file:
    std::map<String, Graphic *>::iterator iFile = this->findUnnamedGraphicFromFile(in_pOrig);
    // Again, don't resize the graphic if it is the error graphic but
    // used in another name.
    if(iFile != m_mGraphicsFromFiles.end() && iFile->second != this->getErrorTexture()) {
        // Delegate the work.
        in_pOrig = this->resizeGraphic(iFile->first, in_fRatio);
        return in_pOrig;
    }

    // Not found ? better not delete it, but display a warning!
    FTS18N("InvParam", MsgType::Warning, "GraphicManager::resizeGraphic unknown ptr");
    return this->getErrorTextureNonConst();
}

/** This gets called whenever the graphics manager should grab all his graphics
 *  in order to restore them later, as they might get lost in the way.
 *
 * \throws Arkana::Exception Anything that might come from Graphic::grab.
 */
void GraphicManager::grabAllGraphics()
{
    // First, grab all graphics that were loaded from file.
    for(std::map<String, Graphic*>::iterator i = m_mGraphicsFromFiles.begin() ; i != m_mGraphicsFromFiles.end() ; ++i) {
        i->second->grab();
    }

    // Then, grab all graphics created from memory.
    for(std::list<Graphic*>::iterator i = m_lGraphicsFromMem.begin() ; i != m_lGraphicsFromMem.end() ; ++i) {
        (*i)->grab();
    }
}

/** This gets called whenever the graphics manager should restore all the
 *  graphics he grabbed some time before.
 *
 * \throws Arkana::Exception Anything that might come from Graphic::restore.
 */
void GraphicManager::restoreAllGraphics()
{
    // First, restore all graphics that were loaded from file.
    for(std::map<String, Graphic*>::iterator i = m_mGraphicsFromFiles.begin() ; i != m_mGraphicsFromFiles.end() ; ++i) {
        i->second->restore();
    }

    // Then, restore all graphics created from memory.
    for(std::list<Graphic*>::iterator i = m_lGraphicsFromMem.begin() ; i != m_lGraphicsFromMem.end() ; ++i) {
        (*i)->restore();
    }
}

void GraphicManager::setSelectedGraphic(uint8_t in_uiTexUnit, uint32_t in_uiTexID)
{
    m_vSelectedTextures[in_uiTexUnit] = in_uiTexID;
}

bool GraphicManager::isGraphicSelected(uint8_t in_uiTexUnit, uint32_t in_uiTexID) const
{
    return m_vSelectedTextures[in_uiTexUnit] == in_uiTexID;
}

void GraphicManager::reinitFrame()
{
    verifGL("Graphic::reinitFrame start");
    // Deselect any still selected textures.
    /// \TODO: Ideally, there is no more need to do this _if_ we draw
    ///        _anything_ using our own shaders, just as OpenGL 3.x wants it.
    ///        currently, the 2D rendering still doesn't do that.
    for(auto iTexUnit = m_vSelectedTextures.size() - 1 ; (iTexUnit > 0) && (m_vSelectedTextures.size() > 0) ; --iTexUnit) {
        if(m_vSelectedTextures[iTexUnit] != 0) {
            m_vSelectedTextures[iTexUnit] = 0;
            glActiveTexture(GL_TEXTURE0 + (GLenum)iTexUnit);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    m_vSelectedTextures[0] = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    verifGL("Graphic::reinitFrame end");
}

 /* EOF */
