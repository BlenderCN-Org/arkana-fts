/**
 * \file tile.h
 * \author Pompei2
 * \date 07 January 2007
 * \brief This file contains the tile and tileset class, a map is made of a lot of tiles and other objects.
 **/

#ifndef FTS_TILE_H
#define FTS_TILE_H

#include "main.h"
#include <map>
#include <vector>

#include "graphic/graphic.h"
#include "dLib/dConf/configuration.h"

/**********************
 * The tileset        *
 **********************/
namespace FTS {
    class Archive;
/** What is a basic tileset ? This is a set just containing all different
 *  tiles, without any transition from one tile to the other.
 *  This tileset is used to create an extended tileset. (see below).
 */
class BasicTileset {
private:
    #define FTS_DEFAULT_UPPERTILE_W 64
    #define FTS_DEFAULT_UPPERTILE_H 64
    #define FTS_DEFAULT_LOWERTILE_W 64
    #define FTS_DEFAULT_LOWERTILE_H 64

    String m_sShortName; ///< The internal name of the tileset.
    String m_sLongName;  ///< The public/official name of the tileset.

    Graphic *m_pUpper; ///< The upper tileset image.
    uint16_t m_nUpper; ///< The count of uppertiles in the set.
    uint16_t m_wUpper; ///< The width of an upper tile.
    uint16_t m_hUpper; ///< The height of an upper tile.
    uint16_t m_wLower; ///< The width of an lower tile.
    uint16_t m_hLower; ///< The height of an lower tile.

    std::map<uint8_t,Graphic*> m_mTiles;  ///< All the tiles mapped to their name.
    std::map<uint8_t,Graphic*> m_mBlends; ///< All blendmasks mapped to their name.
    Graphic *m_pDetail;                   ///< The detailmap.

    Graphic *loadTileFrom(Archive& in_tileset, const String &in_sTileName);
    class Settings : public DefaultOptions {
    public:
        Settings() {
            add("LongName", "Default Tileset name");
            add( "LowerCount", 1);
            add( "BlendCount", 1);
            add( "UpperCount", 1);
            add( "UpperW", FTS_DEFAULT_UPPERTILE_W);
            add( "UpperH", FTS_DEFAULT_UPPERTILE_H);
        }
    };

public:
    BasicTileset();
    virtual ~BasicTileset();

    int load(const String & in_sShortName);
    int unload();

    /// \return Returns the long (official, public) name of this tileset.
    inline String getName() const {return m_sLongName;};

    /// \return Returns the upper tileset graphic.
    inline Graphic *getUpper() {return m_pUpper;};
    /// \return Returns the number of upper tiles present in the upper tileset.
    inline uint16_t getUpperCount() const {return m_nUpper;};
    /// \return The width of an upper tile.
    inline uint16_t getUpperW() const {return m_wUpper;};
    /// \return The height of an upper tile.
    inline uint16_t getUpperH() const {return m_hUpper;};
    /// \return The width of an lower tile.
    inline uint16_t getLowerW() const {return m_wLower;};
    /// \return The height of an lower tile.
    inline uint16_t getLowerH() const {return m_hLower;};

    Graphic *getTile(uint8_t in_cName) const;
    Graphic *getBlend(uint8_t in_cName) const;

    /// \return The detailmap graphic.
    inline Graphic *getDetailmap() const {return m_pDetail;};
};

/** A Tile is one texture, built up using the surrounding four tiles.
 *  These four are taken from the basic tileset and blend together
 *  using the blendmask, then stored into the extended tileset.
 */
class Tile {
    friend class LowerTileset;

private:
    uint8_t m_cTopLeft;      ///< The name of the top left tile composing this one.
    uint8_t m_cTopRight;     ///< The name of the top right tile composing this one.
    uint8_t m_cBottomLeft;   ///< The name of the bottom left tile composing this one.
    uint8_t m_cBottomRight;  ///< The name of the bottom right tile composing this one.

    uint8_t m_cBlendmask;    ///< The name of the blendmask used to compose this tile.

    Graphic *m_pGraphic;   ///< The graphic of the blended tile.

public:
    Tile(uint8_t in_cTopLeft, uint8_t in_cTopRight, uint8_t in_cBottomLeft,
         uint8_t in_cBottomRight, uint8_t in_cBlendmask);
    virtual ~Tile();

    int load(const BasicTileset *in_pTileset);
    int unload();

    const Graphic *GetGraphic() const;
    inline uint8_t getTL() const {return m_cTopLeft;};
    inline uint8_t getTR() const {return m_cTopRight;};
    inline uint8_t getBL() const {return m_cBottomLeft;};
    inline uint8_t getBR() const {return m_cBottomRight;};
    inline uint8_t getBMask() const {return m_cBlendmask;};

    bool operator ==(const Tile & t) const;
};

/** The Lower tileset is really much like the basic tileset, but
 *  It contains ALL tiles, also the blended/transition ones, and all
 *  compiled into one big texture (the Tilemap).
 */
class LowerTileset {
    friend class Tile;

private:

    /** This structure holds all informations about one tile. */
    typedef struct _SLowerTileInfo_ {
        float pfTexCoords[4];   ///< Its coordinates in the one big image (When compiled).
        Tile *pTile;            ///< A pointer to the original tile I represented, before compilation.
        String sName;          ///< The name, composed of the topleft+topright+bottomleft+bottomright+blendmask.
    } SLowerTileInfo, *PLowerTileInfo;

    /// This map shall contain every tile, before it gets compiled
    /// And also after it is compiled.
    typedef std::map<String, SLowerTileInfo, cstring_ltcomp> TileMapType;
    TileMapType m_TileInfos;

    /// This image contais all tiles compiled together into one big tilemap.
    Graphic *m_pTileMap;

    uint16_t m_wTile; ///< The width of lowertiles.
    uint16_t m_hTile; ///< The height of lowertiles.

public:
    LowerTileset(BasicTileset &in_base);
    virtual ~LowerTileset();

    int addTile(Tile * in_pTile);
    bool isUncompiledTilePresent(uint8_t in_cTopLeft, uint8_t in_cTopRight,
                                 uint8_t in_cBottomLeft, uint8_t in_cBottomRight,
                                 uint8_t in_cBlendmask) const;
    void getTileTexCoords(float *out_pTexCoord, uint8_t in_cTopLeft,
                          uint8_t in_cTopRight, uint8_t in_cBottomLeft,
                          uint8_t in_cBottomRight, uint8_t in_cBlendmask) const;

    /// Selects the detailMap as the current OpenGL Texture.
    inline void selectMap(uint8_t in_uiTexUnit = 0) const {m_pTileMap->select(in_uiTexUnit);};

    int compile();
};

/** The Upper tileset contains the picture with all upper tiles.
 *  in it and functions to access every single tile.
 */
class UpperTileset {
private:

    /** This structure holds all informations about one tile. */
    typedef struct _SUpperTileInfo_ {
        uint16_t cName;       ///< The name of this tile.
        float pfTexCoords[4]; ///< Its coordinates in the one big image.
    } SUpperTileInfo, *PUpperTileInfo;

    Graphic *m_pTileMap; ///< Contains all the tiles compiled into one picture.
    PUpperTileInfo m_pTiles; ///< The informations about all tiles.
    uint16_t m_nTiles; ///< The amount of tiles in the array.
    String m_sName; ///< The name of this tileset, for displaying errors.

public:
    UpperTileset(const String &in_sName);
    virtual ~UpperTileset();

    int load(BasicTileset &in_base);
    int unload();
    void getTileTexCoords(float *out_pTexCoord, uint16_t in_cName) const;

    /// Selects the detailMap as the current OpenGL Texture.
    inline void selectMap(uint8_t in_uiTexUnit = 0) const {m_pTileMap->select(in_uiTexUnit);};
};

/** This is the real tileset class that ENCAPSULATES everything the
 *  tileset needs: the upper and the lower tileset as well as the
 *  detailmap and the name of the tileset.
 */
class Tileset {
private:
    LowerTileset *m_pLower; ///< The lower tileset.
    UpperTileset *m_pUpper; ///< The upper tileset.
    Graphic *m_pDetail;     ///< The detailmap.
    String m_sName;        ///< The long name of the tileset.

public:
    Tileset();
    virtual ~Tileset();

    ///< \param in_pLower the new lower tileset.
    inline void setName(const String &in_sName) {m_sName = in_sName;};
    ///< \return the long name of this tileset.
    inline String name() {return m_sName;};

    ///< \param in_pLower the new lower tileset.
    inline void setLower(LowerTileset *in_pLower) {m_pLower = in_pLower;};
    ///< \param in_pUpper the new lower tileset.
    inline void setUpper(UpperTileset *in_pUpper) {m_pUpper = in_pUpper;};
    ///< \return the lower tileset.
    inline const LowerTileset *lower() const {return m_pLower;};
    ///< \return the upper tileset.
    inline const UpperTileset *upper() const {return m_pUpper;};

    /// Selects the detailMap as the current OpenGL Texture.
    inline void selectDetailMap(uint8_t in_uiTexUnit = 0) const {m_pDetail->select(in_uiTexUnit);};
    /// \param in_g The new graphic to use for the detailmap.
    inline void setDetailMap(Graphic *in_g) {m_pDetail = in_g;};
};

}

#endif                          /* FTS_TILE_H */

 /* EOF */
