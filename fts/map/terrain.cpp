#include "map/terrain.h"
#include "map/tile.h"
#include "map/quad.h"

#include "dLib/dFile/dFile.h"

#include "3d/3d.h"
#include "3d/math/Vector.h"
#include "logging/logger.h"
#include "graphic/graphic.h"
#include "utilities/utilities.h"
#include "ui/ui.h"
#include "dLib/dConf/configuration.h"

#include "3d/3d.h"

using namespace FTS;

/// Default constructor. \param in_sMapName The name of the map, used for the error messages.
Terrain::SLoadingInfo::SLoadingInfo(const String &in_sMapName)
    : pFile(nullptr),
      pBaseTileset(new BasicTileset),
      sMapName(in_sMapName),
      usOffsetQuads(0),
      usOffsetLowerTiles(0),
      usOffsetUpperTiles(0),
      pszLowerTilesBuffer(nullptr),
      pUpperTilesBuffer(nullptr)
{
}

/// Default destructor
Terrain::SLoadingInfo::~SLoadingInfo()
{
    SAFE_DELETE(pBaseTileset);
    SAFE_DELETE_ARR(pszLowerTilesBuffer);
    SAFE_DELETE_ARR(pUpperTilesBuffer);
}

/// Default constructor.
Terrain::Terrain(void)
    : m_bMultiTex(true),
      m_bComplex(true),
      m_usWidth(0),
      m_usHeight(0),
      m_fMultiplier(1.0f),
      m_pQuads(nullptr),
      m_pTileset(new Tileset())
{
}

/// Default destructor.
Terrain::~Terrain(void)
{
    this->unload();
}

/// \brief This loads various informations from the terrain file.
/** This method loads various informations like the size, the tileset name
 *  and so one from the terrain file. These informations will mainly be needed
 *  for further loading of the terrain.
 *
 * \param in_sTerrainFile The real filename of the terrain (with full location)
 *                        file to load. No path reinterpretation is done.
 * \param out_info An info structure that will hold temporary informations
 *                 needed in the further loading process.
 *
 * \return ERR_OK in case of success, an error code <0 on failure.
 *
 * \author Pompei2
 */
int Terrain::loadInfo(const String &in_sTerrainFile, SLoadingInfo &out_info)
{
    // First, check for multitexturing and complex quads options.
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    m_bMultiTex = conf.getBool("MultiTexturing");
    m_bComplex  = conf.getBool("ComplexQuads");

    if(in_sTerrainFile.isEmpty() || out_info.sMapName.isEmpty()) {
        FTS18N("InvParam", MsgType::Horror, "Terrain::load");
        return -1;
    }

    try {
        out_info.pFile = File::open(in_sTerrainFile, File::Read);
    } catch(const ArkanaException& e) {
        e.show();
        return -2;
    }

    // Read the ID and verify it is a valid terrain file.
    char pcID[4] = {0};
    out_info.pFile->readNoEndian(pcID, 4);
    if(strncmp(pcID, "FTST", 4) != 0 || out_info.pFile->eof()) {
        FTS18N("MAP_Terrain_InvFile", MsgType::Error, out_info.sMapName);
        return -3;
    }

    // Read the width and height.
    out_info.pFile->read(m_usWidth);
    out_info.pFile->read(m_usHeight);

    // And now get the tileset name.
    out_info.pFile->read(m_sShortTilesetName);

    // Check for errors before we proceed.
    if(out_info.pFile->eof()) {
        FTS18N("File_UnexpEOF", MsgType::Error, out_info.sMapName, "terrain w,h,tileset");
        return -4;
    }

    // Load that tileset.
    if(ERR_OK != out_info.pBaseTileset->load(m_sShortTilesetName))
        return -5;

    // Get the multiplier.
    out_info.pFile->read(m_fMultiplier);

    // And the offsets where are the quads and the tiles.
    out_info.pFile->read(out_info.usOffsetQuads);
    out_info.pFile->read(out_info.usOffsetLowerTiles);
    out_info.pFile->read(out_info.usOffsetUpperTiles);

    if(out_info.pFile->eof()) {
        FTS18N("File_UnexpEOF", MsgType::Error, out_info.sMapName, "terrain mult,offsets");
        return -6;
    }

    return ERR_OK;
}

/// \brief This loads all the quads from the terrain file.
/** Using the loading info it gets, this method loads all quads from the terrain
 *  file and stores them in an array of quads.
 *
 * \param out_info An info structure that holds temporary informations needed
 *                 for the loading process.
 *
 * \return ERR_OK in case of success, an error code <0 on failure.
 *
 * \author Pompei2
 */
int Terrain::loadQuads(SLoadingInfo &out_info)
{
    // Go read the quads.
    out_info.pFile->setCursorPos(out_info.usOffsetQuads);
    m_pQuads = new Quad[m_usWidth * m_usHeight];

    for(int i = 0; i < m_usWidth * m_usHeight; i++) {
        if(ERR_OK != m_pQuads[i].load(out_info.pFile.get(), m_fMultiplier, m_bMultiTex, m_bComplex))
            return -2;
        m_pQuads[i].setX(i % m_usWidth);
        m_pQuads[i].setY(i / m_usWidth);
    }

    return ERR_OK;
}

/// \brief This loads all the lowertiles from the terrain file.
/** Using the loading info it gets, this method loads all the names of the lower
 *  tiles described in the terrain file and stores them in an array of chars, as
 *  the name of a lowertile is a char.
 *
 * \param out_info An info structure that holds temporary informations needed
 *                 for the loading process.
 *
 * \return ERR_OK in case of success, an error code <0 on failure.
 *
 * \author Pompei2
 */
int Terrain::loadLowerTiles(SLoadingInfo &out_info)
{
    // Create the extended tileset.
    m_pTileset->setDetailMap(out_info.pBaseTileset->getDetailmap());

    // Create the buffer for all lowertiles of the terrain.
    out_info.pszLowerTilesBuffer = new char[(m_usWidth + 1) * (m_usHeight + 1)];

    // Go to them and read them out of the file.
    out_info.pFile->setCursorPos(out_info.usOffsetLowerTiles);
    // Same as read(sizeof(char), (w+1)*(h+1))
    uint64_t nTiles = (static_cast<uint64_t>(m_usWidth) + 1) * (static_cast<uint64_t>(m_usHeight) + 1);
    if(out_info.pFile->readNoEndian(out_info.pszLowerTilesBuffer, nTiles) < nTiles) {
        FTS18N("File_UnexpEOF", MsgType::Error, out_info.sMapName, "terrain lower tiles, n="+String::nr(nTiles));
        return -1;
    }

    return ERR_OK;
}

/// \brief This compiles all the lowertiles from the terrain file into a map.
/** Using the loading info it gets, this method compiles all possible lowertile
 *  combinations that come up in the terrain into one lowertilemap that will be
 *  used to draw the terrain's lower layer.
 *
 * \param out_info An info structure that holds temporary informations needed
 *                 for the loading process.
 *
 * \return ERR_OK in case of success, an error code <0 on failure.
 *
 * \author Pompei2
 */
int Terrain::compileLowerTiles(SLoadingInfo &out_info)
{
    /* What we do here is to build up the extended lowertileset, we go trough
     * every quad and look if its texture already is in the extended tileset.
     * If it isn't, we create it and add it to the extended tileset.
     */
    LowerTileset *pLowerTileset = new LowerTileset(*out_info.pBaseTileset);
    for(int y = 0, i = 0; y < m_usHeight; y++) {
        for(int x = 0; x < m_usWidth; x++, i++) {
            // The four edges and blendmask.
            char cTL = out_info.pszLowerTilesBuffer[(y+0)*(m_usWidth+1) +(x+0)];
            char cTR = out_info.pszLowerTilesBuffer[(y+0)*(m_usWidth+1) +(x+1)];
            char cBL = out_info.pszLowerTilesBuffer[(y+1)*(m_usWidth+1) +(x+0)];
            char cBR = out_info.pszLowerTilesBuffer[(y+1)*(m_usWidth+1) +(x+1)];
            char cBM = m_pQuads[i].getBlendmask();

            // If the current tile doesn't exists yet, create it
            // and add it to the extended lowertileset.
            if(!pLowerTileset->isUncompiledTilePresent(cTL,cTR,cBL,cBR,cBM)) {
                Tile *pT = new Tile(cTL, cTR, cBL, cBR, cBM);
                if(ERR_OK == pT->load(out_info.pBaseTileset))
                    pLowerTileset->addTile(pT);
            }
        }
    }

    // Now we can compile the lower tileset.
    if(ERR_OK != pLowerTileset->compile()) {
        SAFE_DELETE(pLowerTileset);
        return -1;
    }

    m_pTileset->setLower(pLowerTileset);
    return ERR_OK;
}

/// \brief This loads all the uppertiles from the terrain file.
/** Using the loading info it gets, this method loads all the uppertiles from
 *  the terrain and puts them into the real tileset.
 *
 * \param out_info An info structure that holds temporary informations needed
 *                 for the loading process.
 *
 * \return ERR_OK in case of success, an error code <0 on failure.
 *
 * \author Pompei2
 */
int Terrain::loadUpperTiles(SLoadingInfo &out_info)
{
    // Create the buffer to hold all uppertiles info.
    out_info.pUpperTilesBuffer = new uint16_t[m_usWidth * m_usHeight];
    if(NULL == out_info.pUpperTilesBuffer)
        return -1;

    // Read all upper tiles info into the buffer.
    uint64_t nTiles = static_cast<uint64_t>(m_usWidth) * static_cast<uint64_t>(m_usHeight);
    out_info.pFile->setCursorPos(out_info.usOffsetUpperTiles);
    if(out_info.pFile->read(out_info.pUpperTilesBuffer, sizeof(uint16_t), nTiles) < nTiles) {
        FTS18N("File_UnexpEOF", MsgType::Error, out_info.sMapName, "terrain upper tiles, n="+String::nr(nTiles));
        return -2;
    }

    // Now we must load the upper layer of the map.
    UpperTileset *pUpperTileset = new UpperTileset(out_info.pBaseTileset->getName());
    if(ERR_OK != pUpperTileset->load(*out_info.pBaseTileset)) {
        return -3;
    }

    m_pTileset->setUpper(pUpperTileset);
    return ERR_OK;
}

/// Unloads the terrain.
/** This unloads everything that was loaded using the load function.
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int Terrain::unload(void)
{
    SAFE_DELETE(m_pTileset);
    SAFE_DELETE_ARR(m_pQuads);

    return ERR_OK;
}

/// Pre-calculates the texture coordinates.
/** This method pre-calculates the texture coordinates for every quad.
 *  In fact, it tells every quad to calculate them himself, only giving
 *  him the right parameters.
 *
 * \param out_info An info structure that holds temporary informations needed
 *                 for the loading process.
 *
 * \TODO optimize, for example make it iteratively use the old normals,
 *       first calc it into a buffer and reuse.
 *
 * \author Pompei2
 */
void Terrain::precalcTexCoords(const SLoadingInfo &in_info)
{
    for(int y = 0, i = 0; y < m_usHeight; y++) {
        for(int x = 0; x < m_usWidth; x++, i++) {

            // The four edges.
            char cTL = in_info.pszLowerTilesBuffer[(y+0)*(m_usWidth+1) + (x+0)];
            char cTR = in_info.pszLowerTilesBuffer[(y+0)*(m_usWidth+1) + (x+1)];
            char cBL = in_info.pszLowerTilesBuffer[(y+1)*(m_usWidth+1) + (x+0)];
            char cBR = in_info.pszLowerTilesBuffer[(y+1)*(m_usWidth+1) + (x+1)];
            uint16_t cU = in_info.pUpperTilesBuffer[(y*m_usWidth + x)];

            m_pQuads[i].initTexCoords(m_pTileset, cTL, cTR, cBL, cBR, cU);
        }
    }
}

/// Pre-calculates the normals.
/** This method pre-calculates the normals of the four outter-most vertices
 *  of every quad and then tells every quad to calculate its inner normals
 *  if it is a complex quad.
 *
 * \TODO optimize, for example make it iteratively use the old normals,
 *       first calc it into a buffer and reuse.
 *
 * \author Pompei2
 */
#define _XY_(x,y) ((y)*(m_usWidth+1)+(x))
void Terrain::precalcNormals()
{
    // TODO: This is a very unoptimized version, you could probably
    //       Put all this stuff in 1 or max 2 loops !

    // Prepare: collect all data and store it in arrays.
    Vector *vVertices = new Vector[(m_usWidth+1)*(m_usHeight+1)];
    Vector *vNormals  = new Vector[(m_usWidth+1)*(m_usHeight+1)];

    float fXDecal = -m_usWidth * FTS_QUAD_SIZE / 2.0f;
    float fYDecal = m_usHeight * FTS_QUAD_SIZE / 2.0f;

    for(int y = 0, i = 0; y < m_usHeight; y++) {
        for(int x = 0; x < m_usWidth; x++, i++) {
            // Add the upper left vertex of that quad.
            vVertices[_XY_(x,y)] = Vector( x * FTS_QUAD_SIZE + fXDecal,
                                              -y * FTS_QUAD_SIZE + fYDecal,
                                              m_pQuads[i].getZ(0));
            vNormals[_XY_(x,y)] = Vector(0.0f,0.0f,0.0f);

            // Also add the right border vertices of the map.
            if(x == m_usWidth-1) {
                vVertices[_XY_(x+1,y)] = Vector( (x+1) * FTS_QUAD_SIZE + fXDecal,
                                                      -y   * FTS_QUAD_SIZE + fYDecal,
                                                     m_pQuads[i].getZ(1));
                vNormals[_XY_(x+1,y)] = Vector(0.0f,0.0f,0.0f);
            }

            // Also add the bottom border vertices of the map.
            if(y == m_usHeight-1) {
                vVertices[_XY_(x,y+1)] = Vector(    x   * FTS_QUAD_SIZE + fXDecal,
                                                     -(y+1) * FTS_QUAD_SIZE + fYDecal,
                                                     m_pQuads[i].getZ(2));
                vNormals[_XY_(x,y+1)] = Vector(0.0f,0.0f,0.0f);
            }

            // And the lower right edge.
            if(x == m_usWidth-1 && y == m_usHeight-1) {
                vVertices[_XY_(x+1,y+1)] = Vector( (x+1) * FTS_QUAD_SIZE + fXDecal,
                                                      -(y+1) * FTS_QUAD_SIZE + fYDecal,
                                                      m_pQuads[i].getZ(3));
                vNormals[_XY_(x+1,y+1)] = Vector(0.0f,0.0f,0.0f);
            }
        }
    }

    // Now, calculate the normal of every four edges of every quad, and add it
    // To the probably already calculated normal of that vertice, or to 0.
    Vector v1, v2;
    for(int y = 0, i = 0; y < m_usHeight; y++) {
        for(int x = 0; x < m_usWidth; x++, i++) {
            // Calculate the left upper normal:
            //       v2
            //    X----->X
            //    |
            // v1 |
            //    v
            //    X      X
            // v1 cross v2
            if(m_pQuads[i].isComplex()) {
                v1 = Vector(0.0f, -1.0f, m_pQuads[i].getCplxZ(5) - m_pQuads[i].getCplxZ(0));
                v2 = Vector(1.0f,  0.0f, m_pQuads[i].getCplxZ(1) - m_pQuads[i].getCplxZ(0));
            } else {
                v1 = vVertices[_XY_(x,y+1)] - vVertices[_XY_(x,y)];
                v2 = vVertices[_XY_(x+1,y)] - vVertices[_XY_(x,y)];
            }
            vNormals[_XY_(x,y)] += v1.cross(v2).normalize();

            // Calculate the right upper normal:
            //       v1
            //    X<-----X
            //           |
            //           | v2
            //           v
            //    X      X
            // v1 cross v2
            if(m_pQuads[i].isComplex()) {
                v1 = Vector(-1.0f, 0.0f, m_pQuads[i].getCplxZ(3) - m_pQuads[i].getCplxZ(4));
                v2 = Vector( 0.0f,-1.0f, m_pQuads[i].getCplxZ(9) - m_pQuads[i].getCplxZ(4));
            } else {
                v1 = vVertices[_XY_(x,y)] - vVertices[_XY_(x+1,y)];
                v2 = vVertices[_XY_(x+1,y+1)] - vVertices[_XY_(x+1,y)];
            }
            vNormals[_XY_(x+1,y)] += v1.cross(v2).normalize();

            // Calculate the lower left normal:
            //
            //    X      X
            //    ^
            // v2 |
            //    |
            //    X----->X
            //       v1
            // v1 cross v2
            if(m_pQuads[i].isComplex()) {
                v1 = Vector(1.0f, 0.0f, m_pQuads[i].getCplxZ(21) - m_pQuads[i].getCplxZ(20));
                v2 = Vector(0.0f, 1.0f, m_pQuads[i].getCplxZ(15) - m_pQuads[i].getCplxZ(20));
            } else {
                v1 = vVertices[_XY_(x+1,y+1)] - vVertices[_XY_(x,y+1)];
                v2 = vVertices[_XY_(x,y)] - vVertices[_XY_(x,y+1)];
            }
            vNormals[_XY_(x,y+1)] += v1.cross(v2).normalize();

            // Calculate the lower right normal:
            //
            //    X      X
            //           ^
            //           | v1
            //           |
            //    X<-----X
            //       v2
            // v1 cross v2
            if(m_pQuads[i].isComplex()) {
                v1 = Vector( 0.0f, 1.0f, m_pQuads[i].getCplxZ(19) - m_pQuads[i].getCplxZ(24));
                v2 = Vector(-1.0f, 0.0f, m_pQuads[i].getCplxZ(23) - m_pQuads[i].getCplxZ(24));
            } else {
                v1 = vVertices[_XY_(x+1,y)] - vVertices[_XY_(x+1,y+1)];
                v2 = vVertices[_XY_(x,y+1)] - vVertices[_XY_(x+1,y+1)];
            }
            vNormals[_XY_(x+1,y+1)] += v1.cross(v2).normalize();
        }
    }

    // Finally, normalize everything and send the information to the quads.
    for(int y = 0, i = 0; y < m_usHeight; y++) {
        for(int x = 0; x < m_usWidth; x++, i++) {
            m_pQuads[i].setupNormals(vNormals[_XY_(x,y)].normalize(),
                                     vNormals[_XY_(x+1,y)].normalize(),
                                     vNormals[_XY_(x,y+1)].normalize(),
                                     vNormals[_XY_(x+1,y+1)].normalize());
        }
    }

    SAFE_DELETE_ARR(vVertices);
    SAFE_DELETE_ARR(vNormals);
}
#undef _XY_

/// draws the terrain.
/** What can I say more ? It draws the terrain to a specified place using
 *  the current options for detailmap, tryes to clip it and only draw the
 *  currently visible part of the terrain.
 *
 * \param in_uiTicks The number of ticks that passed from the beginning of the game, in ms.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \TODO: Optimize, by for example VBO's, only display things in sight, maybe LOD ?
 *
 * \note The ticks have currently absolutely no influence but later they may !
 *
 * \author Pompei2
 */
int Terrain::draw(unsigned int in_uiTicks)
{
    // We select the tilemap as primary texture.
    glEnable(GL_TEXTURE_2D);
    m_pTileset->lower()->selectMap(0);

    if(m_bMultiTex) {
        // Apply the detailmap texture.
        m_pTileset->selectDetailMap(1);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        // Apply the uppertile texturemap.
        m_pTileset->upper()->selectMap(2);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    }

    glDisable(GL_BLEND);
//     glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    static float fXDecal = -m_usWidth * FTS_QUAD_SIZE / 2.0f;
    static float fYDecal = m_usHeight * FTS_QUAD_SIZE / 2.0f;

    // Draw all the quads.
    glBegin(GL_QUADS);
    glColor3f(1.0f,1.0f,1.0f);
    for(int y = 0, i = 0; y < m_usHeight; y++) {
        // This draws the line y of tiles.
        for(int x = 0; x < m_usWidth; x++, i++) {
            m_pQuads[i].draw(+ x * FTS_QUAD_SIZE + fXDecal,
                             - y * FTS_QUAD_SIZE + fYDecal,
                             0.0f, m_fMultiplier);
        }
    }
    glEnd();

    // If no multitexturing is enabled, we still need to see the uppertiles,
    // So we just make a second render pass.
    if(!m_bMultiTex) {
        m_pTileset->upper()->selectMap();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_EQUAL);
        glDepthMask(GL_FALSE);

        // Draw all the quads in a second step.
        glBegin(GL_QUADS);
        glColor3f(1.0f,1.0f,1.0f);
        for(int y = 0, i = 0; y < m_usHeight; y++) {
            // This draws the line y of tiles.
            for(int x = 0; x < m_usWidth; x++, i++) {
                m_pQuads[i].drawUppertile(+ x * FTS_QUAD_SIZE + fXDecal,
                                          - y * FTS_QUAD_SIZE + fYDecal,
                                          0.0f, m_fMultiplier);
            }
        }
        glEnd();
        glDisable(GL_BLEND);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
    }

    glFlush();

    // Disable the other textures now.
    //glActiveTexture(GL_TEXTURE0);
    if(m_bMultiTex) {
        glActiveTexture(GL_TEXTURE0+2);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0+1);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
    }

#ifdef DEBUG
    //extern bool g_bDrawNormals;
    if(g_bDrawNormals) {
        glBegin(GL_LINES);
        glColor3f(1.0f,1.0f,1.0f);
        for(int y = 0, i = 0; y < m_usHeight; y++) {
            // This draws the line y of tiles.
            for(int x = 0; x < m_usWidth; x++, i++) {
                m_pQuads[i].drawNormals(+ x * FTS_QUAD_SIZE + fXDecal,
                                        - y * FTS_QUAD_SIZE + fYDecal, 0.0f);
            }
        }
        glEnd();
    }
#endif

    verifGL("CTerrain::draw");
    return ERR_OK;
}
