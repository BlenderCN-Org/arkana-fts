#ifndef FTS_QUAD_H
#define FTS_QUAD_H

#include "main.h"

#include "3d/Mathfwd.h"

namespace FTS {
    class Quad;
    class Tileset;
    class File;

    typedef int (Quad::*pfnQuadDraw) (float x, float y, float z, float multip);

/** How much units (1unit = 1meter) one quad is. */
#  define FTS_QUAD_SIZE 5.0f
#  define FTS_CQUAD_SIZE 5.0f / 4.0f

/** Over how much quads the detailmap gets stretched.
 *  FIXME: maybe do this camera-zoom dependent ?
 */
#  define FTS_DETAILMAP_QUADS 5.0f

/** This class describes a quad, please refer to the dokuwiki to
 *  understand the differences and relations between quads and tiles.
 *  http://pompei2.cesar4.be/fts/dokuwiki/doku.php/dev:map:terrain.ftst
 */
class Quad {
private:
    bool m_bComplex;           ///< Wether this is a complex quad or not.
    float *m_pfEdges;          ///< The heights of the 4 or 25 edges.
    Vector *m_pfNormals;   ///< The normals of the 4 or 25 edges.

    uint16_t m_usX; ///< The X position of this quad on the map, unit is quads.
    uint16_t m_usY; ///< The Y position of this quad on the map, unit is quads.

    float m_fTexCoordDetail[3];         ///< The texture coordinats for the detailmap X,Y,W.
    float m_fTexCoordLowerTile[4];      ///< The texture coordinates for the lower tile image (Left,Top,Right,Bottom).
    float m_fTexCoordUpperTile[4];      ///< The texture coordinates for the upper tile image (Left,Top,Right,Bottom).
    float m_fCplxTexCoordLowerTile[10]; ///< The texture coordinates for the lower tile image, if the quad is complex
    float m_fCplxTexCoordUpperTile[10]; ///< The texture coordinates for the upper tile image, if the quad is complex

    uint8_t m_cBlendmask;    ///< The ID of the blendmask.

    int renderComplex_Multitex(float in_fX, float in_fY, float in_fZ, float in_fMultip);
    int renderComplex(float in_fX, float in_fY, float in_fZ, float in_fMultip);
    int renderComplexUppertile(float in_fX, float in_fY, float in_fZ, float in_fMultip);
    int renderSimple_Multitex(float in_fX, float in_fY, float in_fZ, float in_fMultip);
    int renderSimple(float in_fX, float in_fY, float in_fZ, float in_fMultip);
    int renderSimpleUppertile(float in_fX, float in_fY, float in_fZ, float in_fMultip);

    pfnQuadDraw m_pfnRender; ///< A pointer to the render function you need to use.
    pfnQuadDraw m_pfnRenderUppertile; ///< A pointer to the render function used for second pass rendering.

public:
    Quad(void);
    virtual ~Quad();

    int load(FTS::File * out_pFile, float in_fMultiplier,
             bool in_bMultiTex, bool in_bComplex); // Two params for buffering.
    int unload(void);
    void initTexCoords(const Tileset *in_pTileset,
                       char in_cTopLeft,
                       char in_cTopRight,
                       char in_cBottomLeft,
                       char in_cBottomRight,
                       uint16_t in_cUpperTile);
    void setupNormals(const Vector &in_vLUNormal, const Vector &in_vRUNormal,
                      const Vector &in_vLBNormal, const Vector &in_vRBNormal);

    char getBlendmask(void) const;

    void setX(unsigned short in_usX);
    void setY(unsigned short in_usY);
    unsigned short getX(void) const;
    unsigned short getY(void) const;

    float getZ(int in_iEdge) const;
    float getCplxZ(int in_iEdge) const;
    Vector getN(int in_iEdge) const;

    inline bool isComplex(void) const {return m_bComplex;};

    int draw(float in_fX, float in_fY, float in_fZ, float in_fMultip);
    int drawUppertile(float in_fX, float in_fY, float in_fZ, float in_fMultip);
    int drawNormals(float in_fX, float in_fY, float in_fZ);
};

} // namespace FTS

#endif                          /* FTS_QUAD_H */

 /* EOF */
