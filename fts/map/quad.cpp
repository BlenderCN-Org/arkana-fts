/**
 * \file quad.cpp
 * \author Pompei2
 * \date 07 January 2007
 * \brief This file contains the implementation of the quad class, used in the terrain.
 **/

#include "map/quad.h"
#include "map/tile.h"

#include "3d/3d.h"
#include "3d/Math.h"
#include "graphic/graphic.h"
#include "logging/logger.h"
#include "utilities/utilities.h"
#include "dLib/dFile/dFile.h"
#include "dLib/dConf/configuration.h"

#include <cmath>

using namespace FTS;

/// This is a simple constructor.
Quad::Quad(void)
{
    m_fTexCoordLowerTile[0] = 0.0f;
    m_fTexCoordLowerTile[1] = 0.0f;
    m_fTexCoordLowerTile[2] = 1.0f;
    m_fTexCoordLowerTile[3] = 1.0f;
    m_fTexCoordUpperTile[0] = 0.0f;
    m_fTexCoordUpperTile[1] = 0.0f;
    m_fTexCoordUpperTile[2] = 1.0f;
    m_fTexCoordUpperTile[3] = 1.0f;
    m_fTexCoordDetail[0] = 0.0f;
    m_fTexCoordDetail[1] = 0.0f;
    m_fTexCoordDetail[2] = 0.0f;

    for(int i = 0 ; i < 10 ; i++) {
        m_fCplxTexCoordLowerTile[i] = 0.0f;
        m_fCplxTexCoordUpperTile[i] = 0.0f;
    }
};

/// This is a simple destructor.
Quad::~Quad(void)
{
    this->unload();
}

/// Function to load a quad.
/** This functions loads ONE quad from a FILE that is
 *  already opened and where the file pointer is placed
 *  just in front of the quad structure.
 *
 * \param out_pFile The file pointer to read from.
 * \param in_fMultiplier How much to scale the tile.
 * \param in_bMultiTex If the option multitexturing is activated or not.
 *                     This param is only used not to retrieve that option
 *                     every time.
 * \param in_bComplex  If the option complex quads is activated or not.
 *                     This param is only used not to retrieve that option
 *                     every time.
 *
 * \return If successful: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note A quad structure doesn't have a header or so ... so if
 *       the FILE pointer is wrong, the behavior is undefined.
 *
 * \author Pompei2
 */
int Quad::load(File *out_pFile, float in_fMultiplier,
                bool in_bMultiTex, bool in_bComplex)
{
    if(nullptr == out_pFile) {
        FTS18N("InvParam", MsgType::Horror, "CQuad::load");
        return -1;
    }
    unload();

    // First, look if this is a complex or a simple quad.
    int8_t cFlags = 0;
    out_pFile->read(cFlags);
    m_bComplex = cFlags & 0x01;

    // Get the blendmask I want to use.
    out_pFile->read(m_cBlendmask);

    // And now get the heights of the edges.
    uint8_t nVerts = m_bComplex ? 25 : 4;
    int16_t *psEdges = new short[nVerts];
    if(nVerts != out_pFile->read(psEdges, sizeof(int16_t), nVerts)) {
        FTS18N("File_UnexpEOF", MsgType::Error, out_pFile->getName(), "read quad verts, cplx = "+String::b(m_bComplex));
        SAFE_DELETE_ARR(psEdges);
        return -2;
    }
    m_pfEdges = new float[nVerts];

    // Apply the multiplier.
    for(uint8_t i = 0; i < nVerts ; i++) {
        m_pfEdges[i] = (float)psEdges[i] * in_fMultiplier;
    }
    SAFE_DELETE_ARR(psEdges);

    // Now if complex quads are not supported, convert it back to simple.
    if(!in_bComplex && m_bComplex) {
        float pfEdgesSave[4] =
            { m_pfEdges[0], m_pfEdges[4], m_pfEdges[20], m_pfEdges[24] };
        SAFE_DELETE_ARR(m_pfEdges);
        m_pfEdges = new float[4];

        m_pfEdges[0] = pfEdgesSave[0];
        m_pfEdges[1] = pfEdgesSave[1];
        m_pfEdges[2] = pfEdgesSave[2];
        m_pfEdges[3] = pfEdgesSave[3];
        m_bComplex = false;
    }

    // Now allocate the array for my normals.
    m_pfNormals = new Vector[nVerts];
    for(uint8_t i = 0; i < nVerts ; i++) {
        m_pfNormals[i].x(0.0f).y(0.0f).z(1.0f);
    }

    // And choose the appropriate rendering function.
    if(m_bComplex) {
        if(in_bMultiTex) {
            m_pfnRender = &Quad::renderComplex_Multitex;
        } else {
            m_pfnRender = &Quad::renderComplex;
            m_pfnRenderUppertile = &Quad::renderComplexUppertile;
        }
    } else {
        if(in_bMultiTex) {
            m_pfnRender = &Quad::renderSimple_Multitex;
        } else {
            m_pfnRender = &Quad::renderSimple;
            m_pfnRenderUppertile = &Quad::renderSimpleUppertile;
        }
    }

    return ERR_OK;
}

/// Unloads the quad.
/** This unloads everything that was loaded using the load function.
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int Quad::unload(void)
{
    SAFE_DELETE_ARR(m_pfEdges);
    SAFE_DELETE_ARR(m_pfNormals);

    return ERR_OK;
}

/// Pre-calculates the texture coordinates of the quad's lower tile.
/** This pre-calculates the texture coordinates of this quad's lower tile.
 *
 * \param in_pTileset     The tileset to use to get the quads texture coordinates.
 * \param in_cTopLeft     The name of the top left lower tile.
 * \param in_cTopRight    The name of the top right lower tile.
 * \param in_cBottomLeft  The name of the bottom left lower tile.
 * \param in_cBottomRight The name of the bottom right lower tile.
 * \param in_cUpperTile   The name of the uppertile to use.
 *
 * \author Pompei2
 */
void Quad::initTexCoords(const Tileset * in_pTileset,
                         char in_cTopLeft,
                         char in_cTopRight,
                         char in_cBottomLeft,
                         char in_cBottomRight,
                         uint16_t in_cUpperTile)
{
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    if(!m_bComplex) {
        in_pTileset->lower()
                   ->getTileTexCoords(m_fTexCoordLowerTile, in_cTopLeft, in_cTopRight,
                                      in_cBottomLeft, in_cBottomRight, m_cBlendmask);
        // Whether to enable or disable multi texturing.
        if(conf.getBool("MultiTexturing")) {
            m_fTexCoordDetail[0] = m_usX / FTS_DETAILMAP_QUADS;
            m_fTexCoordDetail[1] = m_usY / FTS_DETAILMAP_QUADS;
            m_fTexCoordDetail[2] = 1.0f / FTS_DETAILMAP_QUADS;
        }

        in_pTileset->upper()->getTileTexCoords(m_fTexCoordUpperTile, in_cUpperTile);
    } else {
        float fTexCoordLowerTile[4];
        float fTexCoordUpperTile[4];
        in_pTileset->lower()
                   ->getTileTexCoords(fTexCoordLowerTile, in_cTopLeft, in_cTopRight,
                                      in_cBottomLeft, in_cBottomRight, m_cBlendmask);
        float fTexCoordLowerW = std::abs(fTexCoordLowerTile[0] - fTexCoordLowerTile[2]);
        float fTexCoordLowerH = std::abs(fTexCoordLowerTile[1] - fTexCoordLowerTile[3]);
        m_fCplxTexCoordLowerTile[0] = fTexCoordLowerTile[0];
        m_fCplxTexCoordLowerTile[1] = fTexCoordLowerTile[0] + (fTexCoordLowerW / 4.0f);
        m_fCplxTexCoordLowerTile[2] = fTexCoordLowerTile[0] + (fTexCoordLowerW / 4.0f) * 2.0f;
        m_fCplxTexCoordLowerTile[3] = fTexCoordLowerTile[0] + (fTexCoordLowerW / 4.0f) * 3.0f;
        m_fCplxTexCoordLowerTile[4] = fTexCoordLowerTile[2];
        m_fCplxTexCoordLowerTile[5] = fTexCoordLowerTile[1];
        m_fCplxTexCoordLowerTile[6] = fTexCoordLowerTile[1] + (fTexCoordLowerH / 4.0f);
        m_fCplxTexCoordLowerTile[7] = fTexCoordLowerTile[1] + (fTexCoordLowerH / 4.0f) * 2.0f;
        m_fCplxTexCoordLowerTile[8] = fTexCoordLowerTile[1] + (fTexCoordLowerH / 4.0f) * 3.0f;
        m_fCplxTexCoordLowerTile[9] = fTexCoordLowerTile[3];

        // Whether to enable or disable multi texturing.
        if(conf.getBool("MultiTexturing")) {
            m_fTexCoordDetail[0] = m_usX / FTS_DETAILMAP_QUADS;
            m_fTexCoordDetail[1] = m_usY / FTS_DETAILMAP_QUADS;
            m_fTexCoordDetail[2] = 1.0f / (FTS_DETAILMAP_QUADS * 4.0f);
        }

        in_pTileset->upper()->getTileTexCoords(fTexCoordUpperTile, in_cUpperTile);
        float fTexCoordUpperW = std::abs(fTexCoordUpperTile[0] - fTexCoordUpperTile[2]);
        float fTexCoordUpperH = std::abs(fTexCoordUpperTile[1] - fTexCoordUpperTile[3]);
        m_fCplxTexCoordUpperTile[0] = fTexCoordUpperTile[0];
        m_fCplxTexCoordUpperTile[1] = fTexCoordUpperTile[0] + (fTexCoordUpperW / 4.0f);
        m_fCplxTexCoordUpperTile[2] = fTexCoordUpperTile[0] + (fTexCoordUpperW / 4.0f) * 2.0f;
        m_fCplxTexCoordUpperTile[3] = fTexCoordUpperTile[0] + (fTexCoordUpperW / 4.0f) * 3.0f;
        m_fCplxTexCoordUpperTile[4] = fTexCoordUpperTile[2];
        m_fCplxTexCoordUpperTile[5] = fTexCoordUpperTile[1];
        m_fCplxTexCoordUpperTile[6] = fTexCoordUpperTile[1] + (fTexCoordUpperH / 4.0f);
        m_fCplxTexCoordUpperTile[7] = fTexCoordUpperTile[1] + (fTexCoordUpperH / 4.0f) * 2.0f;
        m_fCplxTexCoordUpperTile[8] = fTexCoordUpperTile[1] + (fTexCoordUpperH / 4.0f) * 3.0f;
        m_fCplxTexCoordUpperTile[9] = fTexCoordUpperTile[3];
    }
}

/// Stores the normals of the four outer vertices's of this quad and re-calcs the inner normals.
/** This stores the normals it gets for the four outer-most vertices's of this
 *  quad and then, if this is a complex quad, recalculates all inner-normals.
 *
 * \param in_vLUNormal The normal of the left -upper -most vertex of this quad.
 * \param in_vRUNormal The normal of the right-upper -most vertex of this quad.
 * \param in_vLBNormal The normal of the left -bottom-most vertex of this quad.
 * \param in_vRBNormal The normal of the right-bottom-most vertex of this quad.
 *
 * \author Pompei2
 */
void Quad::setupNormals(const Vector &in_vLUNormal, const Vector &in_vRUNormal,
                        const Vector &in_vLBNormal, const Vector &in_vRBNormal)
{
    if(!m_bComplex) {
        m_pfNormals[0] = in_vLUNormal;
        m_pfNormals[1] = in_vRUNormal;
        m_pfNormals[2] = in_vLBNormal;
        m_pfNormals[3] = in_vRBNormal;
    } else {
        // Init em all to 0.0f;
        for(int i = 0; i < 5*5 ; i++) {
            m_pfNormals[i] = Vector(0.0f, 0.0f, 0.0f);
        }
        m_pfNormals[0] = in_vLUNormal;
        m_pfNormals[4] = in_vRUNormal;
        m_pfNormals[20] = in_vLBNormal;
        m_pfNormals[24] = in_vRBNormal;

#define D_COMPLEXQUAD_BORDERCONST 1

#if D_COMPLEXQUAD_BORDERCONST
        // Pre-insert some normals into the border, interpolated from the edges.
        // Top
        m_pfNormals[1] = (m_pfNormals[0]*2 + m_pfNormals[4]  ).normalize();
        m_pfNormals[2] = (m_pfNormals[0]   + m_pfNormals[4]  ).normalize();
        m_pfNormals[3] = (m_pfNormals[0]   + m_pfNormals[4]*2).normalize();
        // Right
        m_pfNormals[9]  = (m_pfNormals[4]*2 + m_pfNormals[24]  ).normalize();
        m_pfNormals[14] = (m_pfNormals[4]   + m_pfNormals[24]  ).normalize();
        m_pfNormals[19] = (m_pfNormals[4]   + m_pfNormals[24]*2).normalize();
        // Bottom
        m_pfNormals[21] = (m_pfNormals[20]*2 + m_pfNormals[24]  ).normalize();
        m_pfNormals[22] = (m_pfNormals[20]   + m_pfNormals[24]  ).normalize();
        m_pfNormals[23] = (m_pfNormals[20]   + m_pfNormals[24]*2).normalize();
        // Left
        m_pfNormals[5]  = (m_pfNormals[0]*2 + m_pfNormals[20]  ).normalize();
        m_pfNormals[10] = (m_pfNormals[0]   + m_pfNormals[20]  ).normalize();
        m_pfNormals[15] = (m_pfNormals[0]   + m_pfNormals[20]*2).normalize();

        // Now, calculate the normal of every four edges of every sub-quad, and add it
        // To the probably already calculated normal of that vertex, or to 0.
        Vector v1, v2;
        for(int y = 0, i = 0; y < 4; y++) {
            for(int x = 0; x < 4; x++, i++) {
                // Calculate the left upper normal if needed:
                //       v2
                //    X----->X
                //    |
                // v1 |
                //    v
                //    X      X
                // v1 cross v2
                if(x != 0 && y != 0) {
                    v1 = Vector(0.0f, -1.0f, m_pfEdges[(y+1)*5+x] - m_pfEdges[y*5+x]);
                    v2 = Vector(1.0f,  0.0f, m_pfEdges[y*5+x+1] - m_pfEdges[y*5+x]);
                    m_pfNormals[y*5+x] += v1.cross(v2).normalize();
                }

                // Calculate the right upper normal if needed:
                //       v1
                //    X<-----X
                //           |
                //           | v2
                //           v
                //    X      X
                // v1 cross v2
                if(x != 3 && y != 0) {
                    v1 = Vector(-1.0f, 0.0f, m_pfEdges[y*5+x] - m_pfEdges[y*5+x+1]);
                    v2 = Vector( 0.0f,-1.0f, m_pfEdges[(y+1)*5+x+1] - m_pfEdges[y*5+x+1]);
                    m_pfNormals[y*5+x+1] += v1.cross(v2).normalize();
                }

                // Calculate the lower left normal if needed:
                //
                //    X      X
                //    ^
                // v2 |
                //    |
                //    X----->X
                //       v1
                // v1 cross v2
                if(x != 0 && y != 3) {
                    v1 = Vector(1.0f, 0.0f, m_pfEdges[(y+1)*5+x+1] - m_pfEdges[(y+1)*5+x]);
                    v2 = Vector(0.0f, 1.0f, m_pfEdges[(y)*5+x] - m_pfEdges[(y+1)*5+x]);
                    m_pfNormals[(y+1)*5+x] += v1.cross(v2).normalize();
                }

                // Calculate the lower right normal if needed:
                //
                //    X      X
                //           ^
                //           | v1
                //           |
                //    X<-----X
                //       v2
                // v1 cross v2
                if(x != 3 && y != 3) {
                    v1 = Vector( 0.0f, 1.0f, m_pfEdges[y*5+x+1] - m_pfEdges[(y+1)*5+x+1]);
                    v2 = Vector(-1.0f, 0.0f, m_pfEdges[(y+1)*5+x] - m_pfEdges[(y+1)*5+x+1]);
                    m_pfNormals[(y+1)*5+x+1] += v1.cross(v2).normalize();
                }
            }
        }
#else
        // Pre-insert some normals into the border, interpolated from the edges.
        // Top
        m_pfNormals[1] = (m_pfNormals[0]*2 + m_pfNormals[4]  ).normalize();
        m_pfNormals[2] = (m_pfNormals[0]   + m_pfNormals[4]  ).normalize();
        m_pfNormals[3] = (m_pfNormals[0]   + m_pfNormals[4]*2).normalize();
        // Right
        m_pfNormals[9]  = (m_pfNormals[4]*2 + m_pfNormals[24]  ).normalize();
        m_pfNormals[14] = (m_pfNormals[4]   + m_pfNormals[24]  ).normalize();
        m_pfNormals[19] = (m_pfNormals[4]   + m_pfNormals[24]*2).normalize();
        // Bottom
        m_pfNormals[21] = (m_pfNormals[20]*2 + m_pfNormals[24]  ).normalize();
        m_pfNormals[22] = (m_pfNormals[20]   + m_pfNormals[24]  ).normalize();
        m_pfNormals[23] = (m_pfNormals[20]   + m_pfNormals[24]*2).normalize();
        // Left
        m_pfNormals[5]  = (m_pfNormals[0]*2 + m_pfNormals[20]  ).normalize();
        m_pfNormals[10] = (m_pfNormals[0]   + m_pfNormals[20]  ).normalize();
        m_pfNormals[15] = (m_pfNormals[0]   + m_pfNormals[20]*2).normalize();

        // Now, calculate the normal of every four edges of every sub-quad, and add it
        // To the probably already calculated normal of that vertex, or to 0.
        Vector v1, v2;
        for(int y = 0, i = 0; y < 4; y++) {
            for(int x = 0; x < 4; x++, i++) {
                // Calculate the left upper normal if needed:
                //       v2
                //    X----->X
                //    |
                // v1 |
                //    v
                //    X      X
                // v1 cross v2
                if(i != 0) {
                    v1 = Vector(0.0f, -1.0f, m_pfEdges[(y+1)*5+x] - m_pfEdges[y*5+x]);
                    v2 = Vector(1.0f,  0.0f, m_pfEdges[y*5+x+1] - m_pfEdges[y*5+x]);
                    m_pfNormals[y*5+x] += v1.cross(v2).normalize();
                }

                // Calculate the right upper normal if needed:
                //       v1
                //    X<-----X
                //           |
                //           | v2
                //           v
                //    X      X
                // v1 cross v2
                if(!(x == 3 && y == 0)) {
                    v1 = Vector(-1.0f, 0.0f, m_pfEdges[y*5+x] - m_pfEdges[y*5+x+1]);
                    v2 = Vector( 0.0f,-1.0f, m_pfEdges[(y+1)*5+x+1] - m_pfEdges[y*5+x+1]);
                    m_pfNormals[y*5+1] += v1.cross(v2).normalize();
                }

                // Calculate the lower left normal if needed:
                //
                //    X      X
                //    ^
                // v2 |
                //    |
                //    X----->X
                //       v1
                // v1 cross v2
                if(!(x == 0 && y == 3)) {
                    v1 = Vector(1.0f, 0.0f, m_pfEdges[(y+1)*5+x+1] - m_pfEdges[(y+1)*5+x]);
                    v2 = Vector(0.0f, 1.0f, m_pfEdges[(y)*5+x] - m_pfEdges[(y+1)*5+x]);
                    m_pfNormals[(y+1)*5+x] += v1.cross(v2).normalize();
                }

                // Calculate the lower right normal if needed:
                //
                //    X      X
                //           ^
                //           | v1
                //           |
                //    X<-----X
                //       v2
                // v1 cross v2
                if(!(x == 3 && y == 3)) {
                    v1 = Vector( 0.0f, 1.0f, m_pfEdges[y*5+x+1] - m_pfEdges[(y+1)*5+x+1]);
                    v2 = Vector(-1.0f, 0.0f, m_pfEdges[(y+1)*5+x] - m_pfEdges[(y+1)*5+x+1]);
                    m_pfNormals[(y+1)*5+x+1] += v1.cross(v2).normalize();
                }
            }
        }
#endif

        // Normalize all of them.
        for(int y = 0, i = 0; y <= 4; y++) {
            for(int x = 0; x <= 4; x++, i++) {
                m_pfNormals[i] = m_pfNormals[i].normalize();
            }
        }
    }
}

/// Returns the blendmask name.
/** This function returns the name of the blendmask that
 *  was or will be used to create the tile.
 *
 * \return The name of the blendmask.
 *
 * \author Pompei2
 */
char Quad::getBlendmask(void) const
{
    return m_cBlendmask;
}

/// Sets the x position of the quad.
/** This function sets the X position (in quads, zero-based) of this quad
 *  in the quad-grid that represents the terrain. (actually only the variable).
 *
 * \param in_iX The X position.
 *
 * \author Pompei2
 */
void Quad::setX(unsigned short in_usX)
{
    m_usX = in_usX;
}

/// Sets the y position of the quad.
/** This function sets the Y position (in quads, zero-based) of this quad
 *  in the quad-grid that represents the terrain. (actually only the variable).
 *
 * \param in_iY The Y position.
 *
 * \author Pompei2
 */
void Quad::setY(unsigned short in_usY)
{
    m_usY = in_usY;
}

/// Returns the x position of the quad.
/** This function returns the X position (in quads, zero-based) of this quad
 *  in the quad-grid that represents the terrain.
 *
 * \return The X position.
 *
 * \author Pompei2
 */
unsigned short Quad::getX(void) const
{
    return m_usX;
}

/// Returns the y position of the quad.
/** This function returns the Y position (in quads, zero-based) of this quad
 *  in the quad-grid that represents the terrain.
 *
 * \return The Y position.
 *
 * \author Pompei2
 */
unsigned short Quad::getY(void) const
{
    return m_usY;
}

/// Returns the Z position (means: the height) of one edge of the quad.
/** This function returns the Z position (in units) of one edge of the
 *  quad.
 *
 *  \param in_iEdge The edge to get the height from.\n
 *                  0 for the left  upper,\n
 *                  1 for the right upper,\n
 *                  2 for the right lower,\n
 *                  3 for the left  lower,\n
 *
 * \return The Z position of an edge.
 *
 * \author Pompei2
 */
float Quad::getZ(int in_iEdge) const
{
    if(m_bComplex) {
        if(in_iEdge == 0)
            return m_pfEdges[0];
        if(in_iEdge == 1)
            return m_pfEdges[4];
        if(in_iEdge == 2)
            return m_pfEdges[20];
        if(in_iEdge == 3)
            return m_pfEdges[24];
    } else {
        if(0 <= in_iEdge && in_iEdge <= 3)
            return m_pfEdges[in_iEdge];
    }

    return 0.0f;
}

/// Returns the Z position (means: the height) of one vertex of the complex quad.
/** This function returns the Z position (in units) of one vertex of the quad.
 *
 *  \param in_iEdge For a simple quad, this parameter is the same as for
 *                  the \a getZ method.\n
 *                  For a complex quad, this parameter ranges from 0->24,
 *                  0 being left up, 4 right up, 20 left bottom, 24 right
 *                  bottom.\n
 *                  0.0f is returned if in_iEdge has a wrong value.
 *
 * \return The Z position of a vertex of this quad.
 *
 * \author Pompei2
 */
float Quad::getCplxZ(int in_iEdge) const
{
    return ((0 <= in_iEdge) && (in_iEdge <= (m_bComplex ? 24 : 4))) ? m_pfEdges[in_iEdge] : 0.0f;
}

/// Returns the normal of one edge of the quad.
/** This function returns the normal of one edge of the quad.
 *
 *  \param in_iEdge The edge to get the height from.\n
 *                  0 for the left  upper,\n
 *                  1 for the right upper,\n
 *                  2 for the left  lower,\n
 *                  3 for the right lower,\n
 *
 * \return The normal of an edge.
 *
 * \author Pompei2
 */
Vector Quad::getN(int in_iEdge)const
{
    if(m_bComplex) {
        if(in_iEdge == 0)
            return m_pfNormals[0];
        if(in_iEdge == 1)
            return m_pfNormals[4];
        if(in_iEdge == 2)
            return m_pfNormals[20];
        if(in_iEdge == 3)
            return m_pfNormals[24];
    } else {
        if(0 <= in_iEdge && in_iEdge <= 3)
            return m_pfNormals[in_iEdge];
    }

    return Vector();
}

/// Draws the quad.
/** Draws the quad by calling the appropriate member function.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Y position of the quad (height).
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::draw(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{
    if(m_pfnRender) {
        return CALL_MEMBER_FN(*this, m_pfnRender) (in_fX, in_fY, in_fZ, in_fMultip);
    } else {
        return -1;
    }
}

/// Draws the quad with uppertile texture.
/** Draws the quad with uppertile texture by calling the appropriate member function.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Y position of the quad (height).
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::drawUppertile(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{
    if(m_pfnRenderUppertile) {
        return CALL_MEMBER_FN(*this, m_pfnRenderUppertile) (in_fX, in_fY, in_fZ, in_fMultip);
    } else {
        return -1;
    }
}

/// Draws the quad.
/** This is the function to draw a complex quad with support of a detailmap.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Y position of the quad.
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::renderComplex_Multitex(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{
    // LINE NO 1 //
    ///////////////

    // Upper left edge.
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[0].array3f());
    glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
    // Upper right edge.
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    // Lower right edge.
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    // Lower left edge.
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[5].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[5]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[4].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY, in_fZ + m_pfEdges[4]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[9].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);

    // LINE NO 2 //
    ///////////////

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[5].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[10].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[6]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[9].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[14].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);

    // LINE NO 3 //
    ///////////////

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[10].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[15].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[7]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 2.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[14].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[19].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);

    // LINE NO 4 //
    ///////////////

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[15].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[21].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[20].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[20]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[22].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[21].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[23].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 2.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[22].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);

    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[8]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 3.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[19].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 4.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[24].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[24]);
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[9]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2] * 3.0f, m_fTexCoordDetail[1] + m_fTexCoordDetail[2] * 4.0f);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[23].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);

    return ERR_OK;
}

/// Draws the quad.
/** This is the function to draw a complex quad without support of a detailmap.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Y position of the quad.
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::renderComplex(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{
    // LINE NO 1 //
    ///////////////

    // Upper left edge.
    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[0].array3f());
    glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
    // Upper right edge.
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    // Lower right edge.
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    // Lower left edge.
    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[5].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[5]);
    glNormal3fv(m_pfNormals[4].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY, in_fZ + m_pfEdges[4]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[9].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);

    // LINE NO 2 //
    ///////////////

    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[5].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[10].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[6]);
    glNormal3fv(m_pfNormals[9].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[14].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);

    // LINE NO 3 //
    ///////////////

    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[10].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[15].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[7]);
    glNormal3fv(m_pfNormals[14].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[19].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);

    // LINE NO 4 //
    ///////////////

    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[15].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[21].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[0], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[20].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[20]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[22].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[1], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[21].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[23].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[2], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[22].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);

    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[8]);
    glNormal3fv(m_pfNormals[19].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[4], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[24].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[24]);
    glTexCoord2f(m_fCplxTexCoordLowerTile[3], m_fCplxTexCoordLowerTile[9]);
    glNormal3fv(m_pfNormals[23].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);

    return ERR_OK;
}

/// Draws the quad using uppertile texture.
/** This is the function to draw a complex quad without support of a detailmap
 *  but using it's uppertile texture.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Y position of the quad.
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::renderComplexUppertile(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{
    // LINE NO 1 //
    ///////////////

    // Upper left edge.
    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[0].array3f());
    glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
    // Upper right edge.
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    // Lower right edge.
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    // Lower left edge.
    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[5].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[5]);
    glNormal3fv(m_pfNormals[4].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY, in_fZ + m_pfEdges[4]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[9].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);

    // LINE NO 2 //
    ///////////////

    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[5].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[10].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[6].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[7].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[8].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[6]);
    glNormal3fv(m_pfNormals[9].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[14].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);

    // LINE NO 3 //
    ///////////////

    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[10].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[15].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[11].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[12].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[13].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[7]);
    glNormal3fv(m_pfNormals[14].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[19].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);

    // LINE NO 4 //
    ///////////////

    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[15].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[21].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[0], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[20].array3f());
    glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[20]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[16].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[22].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[1], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[21].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[17].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[23].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[2], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[22].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);

    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[18].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[8]);
    glNormal3fv(m_pfNormals[19].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[4], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[24].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[24]);
    glTexCoord2f(m_fCplxTexCoordUpperTile[3], m_fCplxTexCoordUpperTile[9]);
    glNormal3fv(m_pfNormals[23].array3f());
    glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);

    return ERR_OK;
}

/// Draws the quad.
/** This is the function to draw a simple quad with support of a detailmap.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Z position of the quad.
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::renderSimple_Multitex(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{
    // Upper left edge.
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fTexCoordLowerTile[0], m_fTexCoordLowerTile[1]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fTexCoordUpperTile[0], m_fTexCoordUpperTile[1]);
    glNormal3fv(m_pfNormals[0].array3f());
    glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
    // Upper right edge.
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fTexCoordLowerTile[2], m_fTexCoordLowerTile[1]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fTexCoordUpperTile[2], m_fTexCoordUpperTile[1]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    // Lower right edge.
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fTexCoordLowerTile[2], m_fTexCoordLowerTile[3]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0] + m_fTexCoordDetail[2], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fTexCoordUpperTile[2], m_fTexCoordUpperTile[3]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[3]);
    // Lower left edge
    glMultiTexCoord2f(GL_TEXTURE0+0, m_fTexCoordLowerTile[0], m_fTexCoordLowerTile[3]);
    glMultiTexCoord2f(GL_TEXTURE0+1, m_fTexCoordDetail[0], m_fTexCoordDetail[1] + m_fTexCoordDetail[2]);
    glMultiTexCoord2f(GL_TEXTURE0+2, m_fTexCoordUpperTile[0], m_fTexCoordUpperTile[3]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[2]);

    return ERR_OK;
}

/// Draws the quad.
/** This is the function to draw a simple quad without support of a detailmap.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Y position of the quad.
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::renderSimple(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{
    // Upper left edge.
    glTexCoord2f(m_fTexCoordLowerTile[0], m_fTexCoordLowerTile[1]);
    glNormal3fv(m_pfNormals[0].array3f());
    glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
    // Upper right edge.
    glTexCoord2f(m_fTexCoordLowerTile[2], m_fTexCoordLowerTile[1]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    // Lower right edge.
    glTexCoord2f(m_fTexCoordLowerTile[2], m_fTexCoordLowerTile[3]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[3]);
    // Lower left edge.
    glTexCoord2f(m_fTexCoordLowerTile[0], m_fTexCoordLowerTile[3]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[2]);

    return ERR_OK;
}

/// Draws the quad with its uppertile.
/** This is the function to draw a simple quad without support of a detailmap but
 *  using it's uppertile texture.
 *
 * \param in_fX       The X position of the quad.
 * \param in_fY       The Y position of the quad.
 * \param in_fZ       The Z position of the quad.
 * \param in_fMultip  The size multiplier to multiply the quads height (Y) with.
 *
 * \return If successful: The return code of the called drawing function.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Quad::renderSimpleUppertile(float in_fX, float in_fY, float in_fZ, float in_fMultip)
{

    // Upper left edge.
    glTexCoord2f(m_fTexCoordUpperTile[0], m_fTexCoordUpperTile[1]);
    glNormal3fv(m_pfNormals[0].array3f());
    glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
    // Upper right edge.
    glTexCoord2f(m_fTexCoordUpperTile[2], m_fTexCoordUpperTile[1]);
    glNormal3fv(m_pfNormals[1].array3f());
    glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
    // Lower right edge.
    glTexCoord2f(m_fTexCoordUpperTile[2], m_fTexCoordUpperTile[3]);
    glNormal3fv(m_pfNormals[3].array3f());
    glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[3]);
    // Lower left edge.
    glTexCoord2f(m_fTexCoordUpperTile[0], m_fTexCoordUpperTile[3]);
    glNormal3fv(m_pfNormals[2].array3f());
    glVertex3f(in_fX, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[2]);

    return ERR_OK;
}

int Quad::drawNormals(float in_fX, float in_fY, float in_fZ)
{
    if(m_bComplex) {
        // LINE NO 1 //
        ///////////////

        // Upper left edge.
        glVertex3f(in_fX + m_pfNormals[0].x(), in_fY + m_pfNormals[0].y(), in_fZ + m_pfEdges[0] + m_pfNormals[0].z());
        glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
        // Upper right edge.
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[1].x(), in_fY + m_pfNormals[1].y(), in_fZ + m_pfEdges[1] + m_pfNormals[1].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
        // Lower right edge.
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[6].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[6].y(), in_fZ + m_pfEdges[6] + m_pfNormals[6].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
        // Lower left edge.
        glVertex3f(in_fX + m_pfNormals[5].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[5].y(), in_fZ + m_pfEdges[5] + m_pfNormals[5].z());
        glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[1].x(), in_fY + m_pfNormals[1].y(), in_fZ + m_pfEdges[1] + m_pfNormals[1].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[2].x(), in_fY + m_pfNormals[2].y(), in_fZ + m_pfEdges[2] + m_pfNormals[2].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[7].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[7].y(), in_fZ + m_pfEdges[7] + m_pfNormals[7].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[6].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[6].y(), in_fZ + m_pfEdges[6] + m_pfNormals[6].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[2].x(), in_fY + m_pfNormals[2].y(), in_fZ + m_pfEdges[2] + m_pfNormals[2].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY, in_fZ + m_pfEdges[2]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[3].x(), in_fY + m_pfNormals[3].y(), in_fZ + m_pfEdges[3] + m_pfNormals[3].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[8].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[8].y(), in_fZ + m_pfEdges[8] + m_pfNormals[8].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[7].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[7].y(), in_fZ + m_pfEdges[7] + m_pfNormals[7].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[3].x(), in_fY + m_pfNormals[3].y(), in_fZ + m_pfEdges[3] + m_pfNormals[3].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY, in_fZ + m_pfEdges[3]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[4].x(), in_fY + m_pfNormals[4].y(), in_fZ + m_pfEdges[4] + m_pfNormals[4].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY, in_fZ + m_pfEdges[4]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[9].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[9].y(), in_fZ + m_pfEdges[9] + m_pfNormals[9].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[8].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[8].y(), in_fZ + m_pfEdges[8] + m_pfNormals[8].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);

        // LINE NO 2 //
        ///////////////

        glVertex3f(in_fX + m_pfNormals[5].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[5].y(), in_fZ + m_pfEdges[5] + m_pfNormals[5].z());
        glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[5]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[6].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[6].y(), in_fZ + m_pfEdges[6] + m_pfNormals[6].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[11].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[11].y(), in_fZ + m_pfEdges[11] + m_pfNormals[11].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
        glVertex3f(in_fX + m_pfNormals[10].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[10].y(), in_fZ + m_pfEdges[10] + m_pfNormals[10].z());
        glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[6].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[6].y(), in_fZ + m_pfEdges[6] + m_pfNormals[6].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[6]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[7].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[7].y(), in_fZ + m_pfEdges[7] + m_pfNormals[7].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].y(), in_fZ + m_pfEdges[12] + m_pfNormals[12].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[11].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[11].y(), in_fZ + m_pfEdges[11] + m_pfNormals[11].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[7].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[7].y(), in_fZ + m_pfEdges[7] + m_pfNormals[7].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[7]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[8].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[8].y(), in_fZ + m_pfEdges[8] + m_pfNormals[8].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[13].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[13].y(), in_fZ + m_pfEdges[13] + m_pfNormals[13].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].y(), in_fZ + m_pfEdges[12] + m_pfNormals[12].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[8].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[8].y(), in_fZ + m_pfEdges[8] + m_pfNormals[8].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[8]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[9].x(), in_fY - FTS_CQUAD_SIZE + m_pfNormals[9].y(), in_fZ + m_pfEdges[9] + m_pfNormals[9].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE, in_fZ + m_pfEdges[9]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[14].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[14].y(), in_fZ + m_pfEdges[14] + m_pfNormals[14].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[13].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[13].y(), in_fZ + m_pfEdges[13] + m_pfNormals[13].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);

        // LINE NO 3 //
        ///////////////

        glVertex3f(in_fX + m_pfNormals[10].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[10].y(), in_fZ + m_pfEdges[10] + m_pfNormals[10].z());
        glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[10]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[11].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[11].y(), in_fZ + m_pfEdges[11] + m_pfNormals[11].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[16].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[16].y(), in_fZ + m_pfEdges[16] + m_pfNormals[16].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
        glVertex3f(in_fX + m_pfNormals[15].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[15].y(), in_fZ + m_pfEdges[15] + m_pfNormals[15].z());
        glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[11].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[11].y(), in_fZ + m_pfEdges[11] + m_pfNormals[11].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[11]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].y(), in_fZ + m_pfEdges[12] + m_pfNormals[12].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[17].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[17].y(), in_fZ + m_pfEdges[17] + m_pfNormals[17].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[16].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[16].y(), in_fZ + m_pfEdges[16] + m_pfNormals[16].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[12].y(), in_fZ + m_pfEdges[12] + m_pfNormals[12].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[12]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[13].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[13].y(), in_fZ + m_pfEdges[13] + m_pfNormals[13].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].y(), in_fZ + m_pfEdges[18] + m_pfNormals[18].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[17].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[17].y(), in_fZ + m_pfEdges[17] + m_pfNormals[17].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[13].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[13].y(), in_fZ + m_pfEdges[13] + m_pfNormals[13].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[13]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[14].x(), in_fY - FTS_CQUAD_SIZE * 2.0f + m_pfNormals[14].y(), in_fZ + m_pfEdges[14] + m_pfNormals[14].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 2.0f, in_fZ + m_pfEdges[14]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[19].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[19].y(), in_fZ + m_pfEdges[19] + m_pfNormals[19].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].y(), in_fZ + m_pfEdges[18] + m_pfNormals[18].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);

        // LINE NO 4 //
        ///////////////

        glVertex3f(in_fX + m_pfNormals[15].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[15].y(), in_fZ + m_pfEdges[15] + m_pfNormals[15].z());
        glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[15]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[16].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[16].y(), in_fZ + m_pfEdges[16] + m_pfNormals[16].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[21].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[21].y(), in_fZ + m_pfEdges[21] + m_pfNormals[21].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);
        glVertex3f(in_fX + m_pfNormals[20].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[20].y(), in_fZ + m_pfEdges[20] + m_pfNormals[20].z());
        glVertex3f(in_fX, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[20]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[16].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[16].y(), in_fZ + m_pfEdges[16] + m_pfNormals[16].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[16]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[17].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[17].y(), in_fZ + m_pfEdges[17] + m_pfNormals[17].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[22].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[22].y(), in_fZ + m_pfEdges[22] + m_pfNormals[22].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE + m_pfNormals[21].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[21].y(), in_fZ + m_pfEdges[21] + m_pfNormals[21].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[21]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[17].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[17].y(), in_fZ + m_pfEdges[17] + m_pfNormals[17].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[17]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].y(), in_fZ + m_pfEdges[18] + m_pfNormals[18].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[23].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[23].y(), in_fZ + m_pfEdges[23] + m_pfNormals[23].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f + m_pfNormals[22].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[22].y(), in_fZ + m_pfEdges[22] + m_pfNormals[22].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 2.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[22]);

        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[18].y(), in_fZ + m_pfEdges[18] + m_pfNormals[18].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[18]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[19].x(), in_fY - FTS_CQUAD_SIZE * 3.0f + m_pfNormals[19].y(), in_fZ + m_pfEdges[19] + m_pfNormals[19].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 3.0f, in_fZ + m_pfEdges[19]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f + m_pfNormals[24].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[24].y(), in_fZ + m_pfEdges[24] + m_pfNormals[24].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 4.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[24]);
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f + m_pfNormals[23].x(), in_fY - FTS_CQUAD_SIZE * 4.0f + m_pfNormals[23].y(), in_fZ + m_pfEdges[23] + m_pfNormals[23].z());
        glVertex3f(in_fX + FTS_CQUAD_SIZE * 3.0f, in_fY - FTS_CQUAD_SIZE * 4.0f, in_fZ + m_pfEdges[23]);
    } else {
        // Upper left edge.
        glVertex3f(in_fX + m_pfNormals[0].x(), in_fY + m_pfNormals[0].y(), in_fZ + m_pfEdges[0] + m_pfNormals[0].z());
        glVertex3f(in_fX, in_fY, in_fZ + m_pfEdges[0]);
        // Upper right edge.
        glVertex3f(in_fX + FTS_QUAD_SIZE + m_pfNormals[1].x(), in_fY + m_pfNormals[1].y(), in_fZ + m_pfEdges[1] + m_pfNormals[1].z());
        glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY, in_fZ + m_pfEdges[1]);
        // Lower right edge.
        glVertex3f(in_fX + FTS_QUAD_SIZE + m_pfNormals[3].x(), in_fY - FTS_QUAD_SIZE + m_pfNormals[3].y(), in_fZ + m_pfEdges[3] + m_pfNormals[3].z());
        glVertex3f(in_fX + FTS_QUAD_SIZE, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[3]);
        // Lower left edge.
        glVertex3f(in_fX + m_pfNormals[2].x(), in_fY - FTS_QUAD_SIZE + m_pfNormals[2].y(), in_fZ + m_pfEdges[2] + m_pfNormals[2].z());
        glVertex3f(in_fX, in_fY - FTS_QUAD_SIZE, in_fZ + m_pfEdges[2]);
    }

    return ERR_OK;
}

 /* EOF */
