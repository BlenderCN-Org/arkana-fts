#ifndef FTS_MAP_H
#define FTS_MAP_H

#include "main.h"
#include "dLib/dString/dString.h"

#include <list>

class CGraphic;

namespace FTS {
class Tile;
class Quad;
class Terrain;
class Forest;
class MapInfo;

class Map {
private:
    String m_sFile;             ///< The map file that I come from.
    Terrain *m_pTerrain;         ///< The terrain (heightfield+tiles).
    MapInfo *m_pMapInfo;         ///< More general informations about the map.
    std::list<String> m_lFiles; ///< A list of all files that have been extracted with this map.

    // Forest stuff.
    Forest  **m_ppForests;               ///< An array of all forests in this map.
    unsigned char m_nForests;            ///< The total amount of forests in the map.
    unsigned char *m_pForestsRegionsMap; ///< This will contain the map and for every quad the ID of the forest that is there.

    int unloadForests();
    Forest *getForest(unsigned char in_ucID);

public:
    friend class LoadGameRlv;

    Map();
    virtual ~Map();

    int loadForests(const String &in_sForestsFile, const String &in_sConfFile);
    int unload();

    inline MapInfo *getInfo() const { return m_pMapInfo; };

    int draw(unsigned int in_uiTicks);
};

}

#endif                          /* FTS_MAP_H */

 /* EOF */
