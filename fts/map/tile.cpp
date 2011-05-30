/**
 * \file tile.cpp
 * \author Pompei2
 * \date 07 January 2007
 * \brief This file contains the tile and tileset class, a map is made of a lot of tiles and other objects.
 **/

#include "map/tile.h"

#include "3d/3d.h"

#include "logging/logger.h"
#include "utilities/Math.h"
#include "graphic/graphic.h"
#include "graphic/image.h"

#include "dLib/dArchive/dArchive.h"

using namespace FTS;


/// Constructor of a basic tileset.
/** This constructor doesn't already load the tileset, but it already
 *  keeps track of the tileset name, so the load function knows what
 *  to load withoute the need of any argument.
 *
 * \param in_pszShortName The short name of the tileset that will be loaded.
 *
 * \author Pompei2
 */
BasicTileset::BasicTileset()
    : m_pUpper(NULL)
    , m_nUpper(0)
    , m_wUpper(FTS_DEFAULT_UPPERTILE_W)
    , m_hUpper(FTS_DEFAULT_UPPERTILE_H)
    , m_wLower(0)
    , m_hLower(0)
    , m_pDetail(NULL)
{
}

/// Default destructor. Don't forget to call the unload method.
BasicTileset::~BasicTileset()
{
    this->unload();
}

/// Loads the basic tileset.
/** This loads all graphics for tiles, blendmasks and the detailmap
 *  defined in this tileset. All is defined in the info.conf file.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int BasicTileset::load(const String & in_sShortName)
{
    try {
        m_sShortName = in_sShortName;

        FTSMSGDBG("Loading basic tileset named \"" + m_sShortName + "\".", 3);

        // Load the archive of the tileset and get the info file out of it.
        Archive::Ptr pArch(Archive::loadArchive(Path::datadir("Graphics/tilesets") + Path(m_sShortName + ".tileset")));
        File& infoFile = pArch->getFile("info.xml");
        Configuration conf( infoFile, Settings());
        // Get all informations out of the info.conf file.
        m_sLongName = conf.get("LongName");
        uint16_t nLower = conf.getInt("LowerCount");
        uint16_t nBlend = conf.getInt("BlendCount");
        m_nUpper = conf.getInt("UpperCount");
        m_wUpper = conf.getInt("UpperW");
        m_hUpper = conf.getInt("UpperH");

        // Load the upper tileset.
        /// \todo: Fix it that the upper tiles can only be rendered using nearest_nearest filter ! That sucks hard.
        File& upperFile = pArch->getFile(m_sShortName+"_upper.png");
        m_pUpper = GraphicManager::getSingleton().getOrLoadGraphic(upperFile/*, Graphic::Nearest_Nearest*/);

        // Load the detailmap.
        File& detailmapFile = pArch->getFile(m_sShortName+"_detailmap.png");
        m_pDetail = GraphicManager::getSingleton().getOrLoadGraphic(detailmapFile);

        // Now, we try to load all the lowertiles.
        for(uint8_t i = 0 ; i < nLower ; ++i) {
            String sName = m_sShortName+"_lower_"+String::nr(i)+".png";
            m_mTiles[i] = this->loadTileFrom(*pArch, sName);
        }

        // Exactly the same for the blendmasks.
        for(uint8_t i = 0 ; i < nBlend ; ++i) {
            String sName = m_sShortName+"_blends_" + String::nr(i) + ".png";
            m_mBlends[i] = this->loadTileFrom(*pArch, sName);
        }

        return ERR_OK;
    } catch(const ArkanaException& e) {
        e.show();
        return -1;
    }
}

Graphic *BasicTileset::loadTileFrom(FTS::Archive& in_tileset, const String &in_sTileName)
{
    try {
        File& tileFile = in_tileset.getFile(in_sTileName);
        Graphic *pTile = GraphicManager::getSingleton().getOrLoadGraphic(tileFile);

        // Get the dimensions from the first lowertile.
        if(m_wLower == 0 || m_hLower == 0) {
            m_wLower = pTile->getW();
            m_hLower = pTile->getH();
        // For all other lowertiles check if their dimensions agree with the first one.
        } else if(pTile->getW() != m_wLower || pTile->getH() != m_hLower) {
            FTS18N("MAP_Tileset_TileBadSize", MsgType::Warning, m_sLongName, tileFile.getName(),
                String::nr(pTile->getW()), String::nr(pTile->getH()), String::nr(m_wLower), String::nr(m_hLower));

            // If they don't agree, resize them after having displayed a warning.
            pTile = GraphicManager::getSingleton().resizeGraphic(tileFile.getName(), m_wLower, m_hLower);
        }

        return pTile;
    } catch(const ArkanaException& e) {
        // If the tile does not exist within the archive, we log the error
        // and then get a resized error graphic.
         e.show();

        // In case the first tile can't load, default the size
        if(m_wLower == 0 || m_hLower == 0) {
            m_wLower = FTS_DEFAULT_LOWERTILE_W;
            m_hLower = FTS_DEFAULT_LOWERTILE_H;
        }
        FTS18N("MAP_Tileset_NoTileFile", MsgType::Warning, m_sLongName, in_sTileName);
        return GraphicManager::getSingleton().getOrCreateResizedGraphic("Error", m_wLower, m_hLower);
    }
}

/// Unloads the basic tileset.
/** This unloads everything that was loaded using the load function.
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int BasicTileset::unload()
{
    for(std::map<uint8_t,Graphic*>::iterator i = m_mTiles.begin() ; i != m_mTiles.end() ; ++i) {
        GraphicManager::getSingleton().destroyGraphic(i->second);
    }
    for(std::map<uint8_t,Graphic*>::iterator i = m_mBlends.begin() ; i != m_mBlends.end() ; ++i) {
        GraphicManager::getSingleton().destroyGraphic(i->second);
    }

    return ERR_OK;
}

Graphic *BasicTileset::getTile(uint8_t in_cName) const
{
    std::map<uint8_t,Graphic*>::const_iterator i = m_mTiles.find(in_cName);

    return i == m_mTiles.end() ? NULL : i->second;
}

Graphic *BasicTileset::getBlend(uint8_t in_cName) const
{
    std::map<uint8_t,Graphic*>::const_iterator i = m_mBlends.find(in_cName);

    return i == m_mBlends.end() ? NULL : i->second;
}

/// The constructor
/** This constructor only stores the four surrounding tiles and the blendmask name.
 *
 * \param in_cTopLeft     The name of the top left tile.
 * \param in_cTopRight    The name of the top right tile.
 * \param in_cBottomLeft  The name of the bottom left tile.
 * \param in_cBottomRight The name of the bottom right tile.
 * \param in_cBlendmask   The name of the blendmask.
 *
 * \author Pompei2
 */
Tile::Tile(uint8_t in_cTopLeft, uint8_t in_cTopRight, uint8_t in_cBottomLeft,
           uint8_t in_cBottomRight, uint8_t in_cBlendmask)
{
    m_cTopLeft = in_cTopLeft;
    m_cTopRight = in_cTopRight;
    m_cBottomLeft = in_cBottomLeft;
    m_cBottomRight = in_cBottomRight;
    m_cBlendmask = in_cBlendmask;
}

/// Default destructor, does nothing great ...
Tile::~Tile()
{
    this->unload();
}

#define XYR(x,y) (((y) * in_pTileset->getLowerW() * 4) + ((x) * 4)) + 0
#define XYG(x,y) (((y) * in_pTileset->getLowerW() * 4) + ((x) * 4)) + 1
#define XYB(x,y) (((y) * in_pTileset->getLowerW() * 4) + ((x) * 4)) + 2
#define XYA(x,y) (((y) * in_pTileset->getLowerW() * 4) + ((x) * 4)) + 3

/// Creates the tile.
/** This method creates the tile, that means creates a new picture using the
 *  four surrounding tiles and the blendmask (specified in the constructor).
 *  See the dokuwiki for more details.
 *
 * \param in_pTileset A pointer to the tileset to use during tile creation.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int Tile::load(const BasicTileset *in_pTileset)
{
    uint8_t *puszTopLeftPxs = NULL;
    uint8_t *puszTopRightPxs = NULL;
    uint8_t *puszBottomLeftPxs = NULL;
    uint8_t *puszBottomRightPxs = NULL;
    uint8_t *puszBlendmaskPxs = NULL;
    uint8_t *puszMe = NULL;
    Graphic *pGraph = NULL;

    // Try to get all the pixels of all four tiles and the blendmask.
    try {
        // The top left one.
        pGraph = in_pTileset->getTile(m_cTopLeft);
        if(!pGraph || !pGraph->isLoaded() || !(puszTopLeftPxs = pGraph->copyPixels(false)))
            throw(std::make_pair(m_cTopLeft, true));

        // The top right one.
        pGraph = in_pTileset->getTile(m_cTopRight);
        if(!pGraph || !pGraph->isLoaded() || !(puszTopRightPxs = pGraph->copyPixels(false)))
            throw(std::make_pair(m_cTopRight, true));

        // The bottom left one.
        pGraph = in_pTileset->getTile(m_cBottomLeft);
        if(!pGraph || !pGraph->isLoaded() || !(puszBottomLeftPxs = pGraph->copyPixels(false)))
            throw(std::make_pair(m_cBottomLeft, true));

        // The bottom right one.
        pGraph = in_pTileset->getTile(m_cBottomRight);
        if(!pGraph || !pGraph->isLoaded() || !(puszBottomRightPxs = pGraph->copyPixels(false)))
            throw(std::make_pair(m_cBottomRight, true));

        // The blendmask.
        pGraph = in_pTileset->getBlend(m_cBlendmask);
        if(!pGraph || !pGraph->isLoaded() || !(puszBlendmaskPxs = pGraph->copyPixels(false)))
            throw(std::make_pair(m_cBlendmask, false));

        puszMe = new uint8_t[4 * in_pTileset->getLowerW() * in_pTileset->getLowerH()];
    } catch(std::pair<uint8_t, bool> who) {
        FTS18N(who.second ? "MAP_Tileset_TileNotInSet" : "MAP_Tileset_MaskNotInSet",
               MsgType::WarningNoMB, in_pTileset->getName(), String::nr(who.first));
        SAFE_DELETE_ARR(puszTopLeftPxs);
        SAFE_DELETE_ARR(puszTopRightPxs);
        SAFE_DELETE_ARR(puszBottomLeftPxs);
        SAFE_DELETE_ARR(puszBottomRightPxs);
        SAFE_DELETE_ARR(puszBlendmaskPxs);
        return -1;
    }

    /* Now create the tile using color info from the four surrounding
     * tiles and the blendmask. For a more detailed algorith description,
     * take a look at the dokuwiki:
     * http://pompei2.cesar4.be/fts/dokuwiki/doku.php/dev:map:terrain.ftst
     */
    for(int y = 0; y < in_pTileset->getLowerH(); ++y) {
        for(int x = 0; x < in_pTileset->getLowerW(); ++x) {
            // Calculate the ratio every tile affects at this point.
            double dTopLeftRatio = (double)puszBlendmaskPxs[XYR(x, y)] / 255.0;
            double dTopRightRatio = (double)puszBlendmaskPxs[XYG(x, y)] / 255.0;
            double dBottomRightRatio = (double)puszBlendmaskPxs[XYB(x, y)] / 255.0;
            double dBottomLeftRatio = (255.0 - (double)puszBlendmaskPxs[XYA(x, y)]) / 255.0;

            // And calculate it.
            double dRed =
                (double)puszTopLeftPxs[XYR(x, y)] * dTopLeftRatio +
                (double)puszTopRightPxs[XYR(x, y)] * dTopRightRatio +
                (double)puszBottomRightPxs[XYR(x, y)] * dBottomRightRatio +
                (double)puszBottomLeftPxs[XYR(x, y)] * dBottomLeftRatio;
            double dGreen =
                (double)puszTopLeftPxs[XYG(x, y)] * dTopLeftRatio +
                (double)puszTopRightPxs[XYG(x, y)] * dTopRightRatio +
                (double)puszBottomRightPxs[XYG(x, y)] * dBottomRightRatio +
                (double)puszBottomLeftPxs[XYG(x, y)] * dBottomLeftRatio;
            double dBlue =
                (double)puszTopLeftPxs[XYB(x, y)] * dTopLeftRatio +
                (double)puszTopRightPxs[XYB(x, y)] * dTopRightRatio +
                (double)puszBottomRightPxs[XYB(x, y)] * dBottomRightRatio +
                (double)puszBottomLeftPxs[XYB(x, y)] * dBottomLeftRatio;
            double dAlpha =
                (double)puszTopLeftPxs[XYA(x, y)] * dTopLeftRatio +
                (double)puszTopRightPxs[XYA(x, y)] * dTopRightRatio +
                (double)puszBottomRightPxs[XYA(x, y)] * dBottomRightRatio +
                (double)puszBottomLeftPxs[XYA(x, y)] * dBottomLeftRatio;

            // Write it into the data.
            puszMe[XYR(x, y)] = (uint8_t)dRed;
            puszMe[XYG(x, y)] = (uint8_t)dGreen;
            puszMe[XYB(x, y)] = (uint8_t)dBlue;
            puszMe[XYA(x, y)] = (uint8_t)dAlpha;
        }
    }

    // Create a graphic with the calculated buffer.
    m_pGraphic = GraphicManager::getSingleton().createGraphicFromData(puszMe, in_pTileset->getLowerW(), in_pTileset->getLowerH());

#ifdef DEBUG
    // On debug version, write every tile to a tga file.
    char name[] = { (char)(m_cTopLeft+'a'), (char)(m_cTopRight+'a'), (char)(m_cBottomLeft+'a'), (char)(m_cBottomRight+'a'), (char)(m_cBlendmask+'a'), '\0' };
    m_pGraphic->toFTSImageFormat()->save(Path::userdir("Logfiles") + Path(String(name) + ".png"));
#endif

    SAFE_DELETE_ARR(puszTopLeftPxs);
    SAFE_DELETE_ARR(puszTopRightPxs);
    SAFE_DELETE_ARR(puszBottomLeftPxs);
    SAFE_DELETE_ARR(puszBottomRightPxs);
    SAFE_DELETE_ARR(puszBlendmaskPxs);
    SAFE_DELETE_ARR(puszMe);
    return ERR_OK;
}

/// Unloads the tile.
/** This unloads everything that was loaded using the load function.
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int Tile::unload()
{
    GraphicManager::getSingleton().destroyGraphic(m_pGraphic);
    return ERR_OK;
}

/// returns the graphic.
/** This returns a pointer to the graphic representing this tile.
 *
 * \return A pointer to the graphic representing this tile.
 *
 * \author Pompei2
 */
const Graphic *Tile::GetGraphic() const
{
    return m_pGraphic;
}

/// \return if both this and the other tile object look the same (have the same ground).
/// \param t The tile to compare this to.
bool Tile::operator ==(const Tile & t) const
{
    return (m_cTopLeft == t.m_cTopLeft)
        && (m_cTopRight == t.m_cTopRight)
        && (m_cBottomLeft == t.m_cBottomLeft)
        && (m_cBottomRight == t.m_cBottomRight)
        && (m_cBlendmask == t.m_cBlendmask);
}

/// Default constructor.
LowerTileset::LowerTileset(BasicTileset &in_base)
    : m_wTile(in_base.getLowerW())
    , m_hTile(in_base.getLowerH())
{
    m_TileInfos.empty();
}

/// Default destructor.
LowerTileset::~LowerTileset()
{
    GraphicManager::getSingleton().destroyGraphic(m_pTileMap);
}

/// Add a tile to the compile list.
/** This adds a tile to the current tile list. All tiles on the list will get compiled later.
 *
 * \param in_pTile The tile to add to the list.
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int LowerTileset::addTile(Tile * in_pTile)
{
    // Add one everywhere to avoid a 0 that would terminate the string!
    uint8_t sID[] = {(uint8_t)(in_pTile->getTL()+1), (uint8_t)(in_pTile->getTR()+1), (uint8_t)(in_pTile->getBL()+1), (uint8_t)(in_pTile->getBR()+1), (uint8_t)(in_pTile->getBMask()+1), 0};
    SLowerTileInfo lti;
    lti.sName = sID;
    lti.pTile = in_pTile;
    lti.pfTexCoords[0] = lti.pfTexCoords[1] = lti.pfTexCoords[2] = lti.pfTexCoords[3] = 0.0f;
    m_TileInfos[sID] = lti;
    return ERR_OK;
}

/// Check if a specific tile is already present.
/** This searches for a tile that was NOT YET COMPILED and created using
 *  given settings.
 *
 * \param in_cTopLeft     The top left tile used to create it.
 * \param in_cTopRight    The top right tile used to create it.
 * \param in_cBottomLeft  The bottom left tile used to create it.
 * \param in_cBottomRight The bottom right tile used to create it.
 * \param in_cBlendmask   The blendmask used to create it.
 *
 * \return Wether the (uncompiled) tile is present or not.
 *
 * \note This function searches the UNCOMPILED list, not the compiled one !
 *
 * \author Pompei2
 */
bool LowerTileset::isUncompiledTilePresent(uint8_t in_cTopLeft, uint8_t in_cTopRight,
                                           uint8_t in_cBottomLeft, uint8_t in_cBottomRight,
                                           uint8_t in_cBlendmask) const
{
    // Add one everywhere to avoid a 0 that would terminate the string!
    uint8_t sID[] = {(uint8_t)(in_cTopLeft+1), (uint8_t)(in_cTopRight+1), (uint8_t)(in_cBottomLeft+1), (uint8_t)(in_cBottomRight+1), (uint8_t)(in_cBlendmask+1), 0};
    return m_TileInfos.find(sID) != m_TileInfos.end();
}

/// This compiles the extended tileset.
/** Compile the extended tileset means draw all tiles on one big texture,
 *  with power of 2 sizes, and save the texture coordinates that will be
 *  used to acces every single tile.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note After the compilation, every tile's memory is freed !
 *
 * \author Pompei2
 */
int LowerTileset::compile()
{
    uint16_t iTileMapSize = 0;
    uint16_t iTileMapPixelW = 0;
    uint16_t iTileMapPixelH = 0;
    uint8_t *pData = NULL;
    int nTiles = m_TileInfos.size();

    // Calculate the size of the texture we need, based on the number of tiles.
    iTileMapSize = (uint16_t)sqrtf((float)nTiles);
    if(iTileMapSize * iTileMapSize != (uint16_t)nTiles)
        iTileMapSize += 1;
    iTileMapPixelW = power_of_two(iTileMapSize * m_wTile);
    iTileMapPixelH = power_of_two(iTileMapSize * m_hTile);

    /// \todo If the tilemap pixel size is bigger then the maximum texture size,
    ///       do something else then just cutting off some tiles ! That sux!
    iTileMapPixelW = static_cast<uint16_t>(std::min(static_cast<uint64_t>(iTileMapPixelW), GraphicManager::getSingleton().getMaxTextureSize()));
    iTileMapPixelH = static_cast<uint16_t>(std::min(static_cast<uint64_t>(iTileMapPixelH), GraphicManager::getSingleton().getMaxTextureSize()));

    // Alloc the memory to hold the whole map.
    pData = new uint8_t[iTileMapPixelW * iTileMapPixelH * 4];

    // Print all the tiles on this map, and keep track of their texCoords.
    uint32_t iXTile = 0, iYTile = 0;

    for(TileMapType::iterator i = m_TileInfos.begin() ; i != m_TileInfos.end(); ++i) {
        // Calculate the tile's position in pixel.
        uint32_t iX = iXTile * m_wTile;
        uint32_t iY = iYTile * m_hTile;

        // Fill in the informations structure.
        SLowerTileInfo &lti = i->second;

        /* We add and subtract a half pixel to the texCoords, so we are positioned just
         * in the middle of a pixel, and not at the corner of a pixel, that would lead
         * into texture filtering artifacts.
         */
        float fHalfPixelW = 1.0f / ((float)iTileMapPixelW * 2.0f);
        float fHalfPixelH = 1.0f / ((float)iTileMapPixelH * 2.0f);

        lti.pfTexCoords[0] = (float)iX / (float)iTileMapPixelW + fHalfPixelW;
        lti.pfTexCoords[1] = (float)iY / (float)iTileMapPixelH + fHalfPixelH;
        lti.pfTexCoords[2] = (float)iX / (float)iTileMapPixelW +
                             (float)m_wTile / (float)iTileMapPixelW - fHalfPixelW;
        lti.pfTexCoords[3] = (float)iY / (float)iTileMapPixelH +
                             (float)m_hTile / (float)iTileMapPixelH - fHalfPixelH;

        // Nothing to store.
        Tile *pTile = lti.pTile;

        // Copy the tiles onto the tileMap.
        uint8_t *pTileData = pTile->GetGraphic()->copyPixels(false);
        uint32_t iW = pTile->GetGraphic()->getW();
        uint32_t iH = pTile->GetGraphic()->getH();

        for(uint32_t y = 0; y < iH; ++y) {
            memcpy(&pData[iX * 4 + iY * iTileMapPixelW * 4 + y * iTileMapPixelW * 4],
                   &pTileData[y * iW * 4],
                   sizeof(uint8_t) * 4 * iW);
        }
        SAFE_FREE(pTileData);

        // Go to the next tile.
        iXTile++;
        if(iXTile >= iTileMapSize) {
            iXTile = 0;
            iYTile++;
        }
    }

    /* Create the graphic with the data we calculated, and FORCE the use of
     * the nearest filter, not te linear. If we'd use the linear or above filters,
     * we would get artifacts like one tile displaying one pixel from the surrounding
     * ones. That will make "error lines" in the map.
     */
    m_pTileMap = GraphicManager::getSingleton().createGraphicFromData(pData, iTileMapPixelW, iTileMapPixelH, Graphic::Nearest_Nearest);
    m_pTileMap->toFTSImageFormat()->save(Path::userdir("Logfiles") + Path("tilemap.png"));
    SAFE_DELETE_ARR(pData);

    // Free the original tiles saved until here.
    for(TileMapType::iterator i = m_TileInfos.begin() ; i != m_TileInfos.end() ; ++i) {
        SAFE_DELETE(i->second.pTile);
    }

    return ERR_OK;
}

/// This returns the texCoords of one tile.
/** Use this function to get the four texCoords needed to access to the tile
 *  specified in the parameters.
 *
 * \param out_pTexCoord   An array of 4 floats where the texcoords will be written.
 * \param in_cTopLeft     The top left tile to get the texCoords from.
 * \param in_cTopRight    The top right tile to get the texCoords from.
 * \param in_cBottomLeft  The bottom left tile to get the texCoords from.
 * \param in_cBottomRight The bottom right tile to get the texCoords from.
 * \param in_cBlendmask   The blendmask to get the texCoords from.
 *
 * \note The texCoords in tha array are as follows:
 *   out_pTexCoord[0] = left (X)
 *   out_pTexCoord[1] = top (Y)
 *   out_pTexCoord[2] = right (X+Width)
 *   out_pTexCoord[3] = bottom (Y+Height)
 *
 * \author Pompei2
 */
void LowerTileset::getTileTexCoords(float *out_pTexCoord,
                                    uint8_t in_cTopLeft,
                                    uint8_t in_cTopRight,
                                    uint8_t in_cBottomLeft,
                                    uint8_t in_cBottomRight,
                                    uint8_t in_cBlendmask) const
{
    // Add one everywhere to avoid a 0 that would terminate the string!
    uint8_t sID[] = {(uint8_t)(in_cTopLeft+1), (uint8_t)(in_cTopRight+1), (uint8_t)(in_cBottomLeft+1), (uint8_t)(in_cBottomRight+1), (uint8_t)(in_cBlendmask+1), 0};
    TileMapType::const_iterator i = m_TileInfos.find(sID);
    if(i != m_TileInfos.end()) {
        const SLowerTileInfo &lti = i->second;
        out_pTexCoord[0] = lti.pfTexCoords[0];
        out_pTexCoord[1] = lti.pfTexCoords[1];
        out_pTexCoord[2] = lti.pfTexCoords[2];
        out_pTexCoord[3] = lti.pfTexCoords[3];
    }
}

/// Default constructor.
UpperTileset::UpperTileset(const String &in_sName)
    : m_pTileMap(NULL)
    , m_sName(in_sName)
{
}

/// Default destructor.
UpperTileset::~UpperTileset()
{
    this->unload();
}

/// This loads the upper tiles from a file.
/** Use this function to load the upper tiles from a graphic file.
 *
 * \param in_sFile  The name of the file that contans the graphic.
 * \param in_iCount The amount of tiles that are presend in the graphic.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int UpperTileset::load(BasicTileset &in_base)
{
    m_pTileMap = in_base.getUpper();
    m_nTiles = in_base.getUpperCount();
    m_pTiles = new SUpperTileInfo[m_nTiles];

    /// \todo Problem: if data is ill-formated, ex:  tilemap=errortex.
    uint32_t iW = m_pTileMap->getW() / (in_base.getUpperW());
    uint32_t iH = m_pTileMap->getH() / (in_base.getUpperH());
    if(iW < 1) iW = 1;
    if(iH < 1) iH = 1;
    float fNormUpperW = m_pTileMap->getTexco()[2]/(float)iW;
    float fNormUpperH = m_pTileMap->getTexco()[3]/(float)iH;
    uint16_t i = 0;
    for(uint32_t y = 0 ; y < iH && i < m_nTiles ; ++y) {
        for(uint32_t x = 0 ; x < iW ; ++x, ++i) {
            m_pTiles[i].cName = i;
            m_pTiles[i].pfTexCoords[0] = (float)x*fNormUpperW;
            m_pTiles[i].pfTexCoords[1] = (float)y*fNormUpperH;
            m_pTiles[i].pfTexCoords[2] = m_pTiles[i].pfTexCoords[0] + fNormUpperW;
            m_pTiles[i].pfTexCoords[3] = m_pTiles[i].pfTexCoords[1] + fNormUpperH;
        }
    }

    // Fill remaining ones to be the transparent one.
    // There are only tiles remaining if we had ill-formatted input (iW<1 or iH<1)
    while(i < m_nTiles) {
        m_pTiles[i].cName = i;
        m_pTiles[i].pfTexCoords[0] = 0.0f;
        m_pTiles[i].pfTexCoords[1] = 0.0f;
        m_pTiles[i].pfTexCoords[2] = fNormUpperW;
        m_pTiles[i].pfTexCoords[3] = fNormUpperH;
        ++i;
    }

    return ERR_OK;
}

/// This unloads the upper tiles.
/** Use this function to unload everything the load function created.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int UpperTileset::unload()
{
    GraphicManager::getSingleton().destroyGraphic(m_pTileMap);
    SAFE_DELETE_ARR(m_pTiles);

    return ERR_OK;
}

/// This returns the texCoords of one tile.
/** Use this function to get the four texCoords needed to access to the tile
 *  specified in the parameters.
 *
 * \param out_pTexCoord   An array of 4 floats where the texcoords will be written.
 * \param in_cName        The name of the tile you want to access.
 *
 * \note The texCoords in tha array are as follows:
 *   out_pTexCoord[0] = left (X)
 *   out_pTexCoord[1] = top (Y)
 *   out_pTexCoord[2] = right (X+Width)
 *   out_pTexCoord[3] = bottom (Y+Height)
 *
 * \author Pompei2
 */
void UpperTileset::getTileTexCoords(float *out_pTexCoord, uint16_t in_uiName) const
{
    // For some wrong tilename, return the empty uppertile.
    if(in_uiName >= m_nTiles) {
        /// \todo: warn!
        FTS18N("MAP_Tileset_UpperNotInSet", MsgType::Warning, m_sName, String::nr(in_uiName));
        out_pTexCoord[0] = m_pTiles[0].pfTexCoords[0];
        out_pTexCoord[1] = m_pTiles[0].pfTexCoords[1];
        out_pTexCoord[2] = m_pTiles[0].pfTexCoords[2];
        out_pTexCoord[3] = m_pTiles[0].pfTexCoords[3];
        return;
    }

    out_pTexCoord[0] = m_pTiles[in_uiName].pfTexCoords[0];
    out_pTexCoord[1] = m_pTiles[in_uiName].pfTexCoords[1];
    out_pTexCoord[2] = m_pTiles[in_uiName].pfTexCoords[2];
    out_pTexCoord[3] = m_pTiles[in_uiName].pfTexCoords[3];
}

/// Default constructor.
Tileset::Tileset()
{
    m_pLower = NULL;
    m_pUpper = NULL;
}

/// Default destructor.
Tileset::~Tileset()
{
    SAFE_DELETE(m_pLower);
    SAFE_DELETE(m_pUpper);
    GraphicManager::getSingleton().destroyGraphic(m_pDetail);
}

 // EOF
