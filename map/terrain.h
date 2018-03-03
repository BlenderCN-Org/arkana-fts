#ifndef FTS_TERRAIN_H
#define FTS_TERRAIN_H

#include "main.h"
#include "dLib/dString/dString.h"
#include "dLib/dFile/dFile.h"

#ifdef DEBUG
extern bool g_bDrawNormals;
#endif

namespace FTS {
    class Graphic;
    class Quad;
    class Tileset;
    class BasicTileset;

/// This class represents a Terrain, that is the textured heightmap of a map.
class Terrain {
private:
    bool m_bMultiTex; ///< Whether to use multitexturing or not.
    bool m_bComplex;  ///< Whether to use complex quads or not.

    uint16_t m_usWidth;  ///< The width of the map (number of quads)
    uint16_t m_usHeight; ///< The height of the map (number of quads)

    float m_fMultiplier; ///< The multiplier every height gets multiplied with.

    /// This is the short name (6 chars) of the tileset used for this terrain.
    String m_sShortTilesetName;

    Quad *m_pQuads;        ///< The array of all the quads.
    Tileset *m_pTileset;   ///< My tileset that encapsulates all tileset types.

public:
    /// Informations used for loading the terrain.
    struct SLoadingInfo {
        File::Ptr pFile;                 ///< The file that is being loaded.
        BasicTileset *pBaseTileset;  ///< The basic tileset of the terrain.
        String sMapName;            ///< The name of the map (for error msgs).
        uint16_t usOffsetQuads;      ///< Where in the file the quads are.
        uint16_t usOffsetLowerTiles; ///< Where in the file the lower tiles are.
        uint16_t usOffsetUpperTiles; ///< Where in the file the upper tiles are.
        char *pszLowerTilesBuffer;   ///< A buffer for the lowertiles names.
        uint16_t *pUpperTilesBuffer; ///< A buffer for the uppertiles names.

        SLoadingInfo(const String &in_sMapName);
        ~SLoadingInfo();
    };

    Terrain();
    virtual ~Terrain();

    int loadInfo(const String &in_sTerrainFile, SLoadingInfo &out_info);
    int loadQuads(SLoadingInfo &out_info);
    int loadLowerTiles(SLoadingInfo &out_info);
    int compileLowerTiles(SLoadingInfo &out_info);
    int loadUpperTiles(SLoadingInfo &out_info);
    void precalcTexCoords(const SLoadingInfo &in_info);
    void precalcNormals();

    int unload();

    int draw(unsigned int in_uiTicks);

    /// \return The width of the map (number of quads)
    inline uint16_t getW() {return m_usWidth;};
    /// \return The height of the map (number of quads)
    inline uint16_t getH() {return m_usHeight;};
};

}

#endif                          /* FTS_TERRAIN_H */

 /* EOF */
