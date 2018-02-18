#ifndef D_GRAPHIC_H
#define D_GRAPHIC_H

#include "main.h"
#include "utilities/Singleton.h"
#include "dLib/dString/dString.h"

#include <map>
#include <list>
#include <vector>

namespace CEGUI {
    class Imageset;
}

namespace FTS {
    class ImageFormat;
    class File;
    class Packet;

class Graphic {
public:
    /// This is an enumeration of texture filters supported by the graphics.
    enum TextureFilter {
        DefaultFilter = 0,       ///< Use the default filtering (from conf file)
        Nearest_Nearest = 1,     ///< Nearest for close-view, Nearest for far-view
        Linear_Linear = 2,       ///< Linear for close-view, Linear for far-view
        Linear_LinearMipMap = 3, ///< Linear for close-view, Linear Mipmaps for far-view
    };
    static TextureFilter toTexureFilter(int i);

    /// This is an enumeration of the anisotropic filtering possibilities.
    enum Anisotropy {
        DefaultAnisotropy = 0,   ///< Use the default anisotropy (from conf file)
        FullAnisotropy = 1,      ///< Use the highest available anisotropy filter.
        NoAnisotropy = 2,        ///< Do not use any anisotropy filter.
    };
    static Anisotropy toAnisotropy(int i);

public:
    friend class GraphicManager;

    ////////////////////////////
    // Data accessing methods //
    inline uint16_t getW() const { return m_uiW; };
    inline uint16_t getH() const { return m_uiH; };
    inline uint32_t getID() const { return m_uiID; };
    /// \returns an array of size 4 containing the left, top, right, bottom
    ///          texture coordinates respectively.
    inline const float *getTexco() const { return m_pfTexcoord; };

    bool isLoaded() const;
    uint8_t *copyPixels(bool in_bRealTextureSize) const;
    CEGUI::Imageset *createCEGUI(const String &in_sImagesetName = String::EMPTY, bool in_bReplace = false) const;

    ////////////////////////////
    // Direct Drawing methods //
    // To use in 2D mode      //
    int draw(int16_t in_iX, int16_t in_iY) const;
    int drawCentered(int16_t in_iX, int16_t in_iY) const;
    int drawSub(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
                uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH) const;
    int drawSubCentered(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
                        uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH) const;
    int drawRot(int16_t in_iX, int16_t in_iY, float in_fDegrees) const;
    int drawRotCentered(int16_t in_iX, int16_t in_iY, float in_fDegrees) const;
    int drawZoom(int16_t in_iX, int16_t in_iY, float in_fZoomX, float in_fZoomY) const;
    int drawZoomCentered(int16_t in_iX, int16_t in_iY, float in_fZoomX, float in_fZoomY) const;
    int drawColoured(int16_t in_iX, int16_t in_iY,
                     float in_fR, float in_fG, float in_fB, float in_fA) const;
    int drawColouredCentered(int16_t in_iX, int16_t in_iY,
                             float in_fR, float in_fG, float in_fB, float in_fA) const;
    int drawEx(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
               uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH,
               float in_fRotate, float in_fZoomX, float in_fZoomY,
               float in_fR, float in_fG, float in_fB, float in_fA) const;
    int drawExCentered(int16_t in_iX, int16_t in_iY, uint16_t in_iSubX,
                       uint16_t in_iSubY, uint16_t in_iSubW, uint16_t in_iSubH,
                       float in_fRotate, float in_fZoomX, float in_fZoomY,
                       float in_fR, float in_fG, float in_fB, float in_fA) const;

    ///////////////////////
    // Select as texture //
    void select(uint8_t in_iTexUnit = 0) const;

    /////////////////
    // Misc. Stuff //
    bool grab();
    bool restore();

    int writeToPacket(Packet *in_pPacket) const;
    ImageFormat *toFTSImageFormat() const;

    Graphic *copyResized(uint16_t in_uiNewW, uint16_t in_uiNewH) const;

private:
    /// OPENGL: the texture's ID.
    uint32_t m_uiID;
    /// OPENGL: the texcoords to apply to the quad.
    float m_pfTexcoord[4];
    /// OPENGL: the width and height of the texture.
    uint16_t m_uiRealW, m_uiRealH;
    /// The picture's width and height.
    uint16_t m_uiW, m_uiH;
    /// This is the anisotropy that has been forced (or 0 if none has been forced).
    Anisotropy m_usedAnisotropy;
    /// This is the texture filter that has been forced (or 0 if none has been forced).
    TextureFilter m_usedFilter;
    /// When the graphic is grabbed, this holds its data.
    uint8_t* m_pGrabbedPixels;

    Graphic();
    Graphic(const Graphic &) = delete ;
    virtual ~Graphic();

    void create(const uint8_t * const in_pData, uint16_t in_uiW, uint16_t in_uiH, TextureFilter in_forceFilter = DefaultFilter, Anisotropy in_forceAnisotropy = DefaultAnisotropy);
    void destroy();
};

class GraphicManager : public Singleton<GraphicManager> {
    /// All graphics that have been loaded from a file, mapped to their filename.
    std::map<String, Graphic *>m_mGraphicsFromFiles;
    /// All graphics that have been created from memory (network packets, ...).
    std::list<Graphic *>m_lGraphicsFromMem;

    /// The non-const pendant may only be used internally.
    inline Graphic *getErrorTextureNonConst() {return m_mGraphicsFromFiles["Error"];};

    std::list<Graphic *>::iterator findUnnamedGraphicFromMem(const Graphic *in_pGraphic);
    std::map<String, Graphic *>::iterator findUnnamedGraphicFromFile(const Graphic *in_pGraphic);

    /// For optimisation: keeps track of which texture is selected in which slot.
    std::vector<uint32_t> m_vSelectedTextures;

    void setSelectedGraphic(uint8_t in_uiTexUnit, uint32_t in_uiTexID);
    bool isGraphicSelected(uint8_t in_uiTexUnit, uint32_t in_uiTexID) const;

    friend class Graphic;

public:
    GraphicManager();
    virtual ~GraphicManager();

    static const String ErrorTextureName;

    ////////////////////
    // Creation stuff //
    Graphic *getOrLoadGraphic(const String &in_sFileName,
                              Graphic::TextureFilter in_forceFilter = Graphic::DefaultFilter,
                              Graphic::Anisotropy in_forceAnisotropy = Graphic::DefaultAnisotropy);
    Graphic *getOrLoadGraphic(FTS::File& out_file, String in_sGraphicName = String::EMPTY,
                              Graphic::TextureFilter in_forceFilter = Graphic::DefaultFilter,
                              Graphic::Anisotropy in_forceAnisotropy = Graphic::DefaultAnisotropy);
    Graphic *readGraphicFromPacket(Packet *out_pPacket);
    Graphic *createGraphicFromData(const uint8_t * const in_pData, uint16_t in_uiW, uint16_t in_uiH,
                                   Graphic::TextureFilter in_forceFilter = Graphic::DefaultFilter,
                                   Graphic::Anisotropy in_forceAnisotropy = Graphic::DefaultAnisotropy);
    Graphic *getOrCreateResizedGraphic(const String &in_sOrig, uint16_t in_uiNewW, uint16_t in_uiNewH);

    ///////////////////////
    // Destruction stuff //
    void destroyGraphic(const String &in_sFileName);
    void destroyGraphic(Graphic*& in_graphic);

    /////////////////
    // Misc. stuff //
    const Graphic *getErrorTexture() const;
    bool isGraphicPresent(const String &in_sFileName);
    uint64_t getMaxTextureSize() const;
    uint8_t getMaxTextureUnits() const;

    int renameGraphic(const String &in_sOldName, const String &in_sNewName);
    Graphic *resizeGraphic(const String &in_sOrig, uint16_t in_uiNewW, uint16_t in_uiNewH);
    Graphic *resizeGraphic(const String &in_sOrig, float in_fRatio);
    Graphic *resizeGraphic(Graphic *&in_pOrig, uint16_t in_uiNewW, uint16_t in_uiNewH);
    Graphic *resizeGraphic(Graphic *&in_pOrig, float in_fRatio);

    void grabAllGraphics();
    void restoreAllGraphics();

    void reinitFrame();
};
};

#endif /* D_GRAPHIC_H */

 /* EOF */
