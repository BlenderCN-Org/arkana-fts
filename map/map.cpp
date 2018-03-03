#include <CEGUI.h>
#include <SDL_timer.h>

#include "map/map.h"
#include "map/mapinfo.h"
#include "map/terrain.h"
#include "map/forest.h"
#include "map/quad.h"

#include "ui/ui.h"
#include "graphic/graphic.h"
#include "logging/logger.h"
#include "utilities/utilities.h"

    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
#include "game/objects/tree.h"
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

using namespace FTS;

Map::Map()
    : m_pTerrain(new Terrain),
      m_pMapInfo(new MapInfo),
      m_ppForests(NULL),
      m_nForests(0),
      m_pForestsRegionsMap(NULL)
{
}

Map::~Map(void)
{
    this->unload();
}

    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
Tree *g_pTree;
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

/// Load all forests from the forest/config file combination.
/** This method loads all forests that are specified in the two files.
 *  It also creates all the trees from the forests.
 *
 * \param in_sForestsFile The name of the file that stores the forest regions.
 * \param in_sConfFile The name of the configuration file that stores the
 *                     forests propreties.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \note See our Dokuwiki for more details
 *  (http://fts.tuxfamily.org/dokuwiki/doku.php?id=design_documents:map:1_overview#layer_2the_vegetation_layer)
 *
 * \author Pompei2
 */
int Map::loadForests(const String &in_sForestsFile, const String &in_sConfFile)
{
    FILE *pFile = fopen(in_sForestsFile.c_str(), "rb");
    if(pFile == NULL) {
        FTS18N("File_FopenR", MsgType::Error, in_sForestsFile, strerror(errno));
        return -1;
    }

    char sID[7] = {0, 0, 0, 0, 0, 0, 0};
    unsigned short usW = 0, usH = 0, usOffset = 14;

    // Read the first header informations (ID, size, offset).
    fread(sID, sizeof(char), 6, pFile);
    fread(&usW, sizeof(unsigned short), 1, pFile);
    fread(&usH, sizeof(unsigned short), 1, pFile);
    fread(&m_nForests, sizeof(unsigned char), 1, pFile);
    fread(&usOffset, sizeof(unsigned short), 1, pFile);

    if(ferror(pFile) != 0) {
        FTS18N("File_Read", MsgType::Error, in_sForestsFile, strerror(errno));
        // Remove all forests if there was an error.
        m_nForests = 0;
        SAFE_FCLOSE(pFile);
        return -2;
    }

    // Alloc enough memory and then go read the regions.
    m_pForestsRegionsMap = new unsigned char[usW * usH];
    fseek(pFile, usOffset, SEEK_SET);
    fread(m_pForestsRegionsMap, sizeof(unsigned char), usW * usH, pFile);

    if(ferror(pFile) != 0) {
        FTS18N("File_Read", MsgType::Error, in_sForestsFile, strerror(errno));
        SAFE_FCLOSE(pFile);
        return -3;
    }

    SAFE_FCLOSE(pFile);

    // We got everything from the forests regions map, now go load the different forests.
    m_ppForests = new Forest*[m_nForests];
    for(int i = 0 ; i < m_nForests ; i++) {
        m_ppForests[i] = new Forest(i+1);
        m_ppForests[i]->load(in_sConfFile);
    }

    // After the forests are loaded, we can plant the trees of the forests into
    // The according quads.
    float fXDecal = -usW * FTS_QUAD_SIZE / 2.0f;
    float fYDecal =  usH * FTS_QUAD_SIZE / 2.0f;
    for(unsigned short y = 0, i = 0; y < usH ; y++) {
        for(unsigned short x = 0; x < usW ; x++, i++) {
            // 0 means no forest.
            if(m_pForestsRegionsMap[i] == 0)
                continue;

            this->getForest(m_pForestsRegionsMap[i])->plantForest(i, + x * FTS_QUAD_SIZE + fXDecal,
                                                                     - y * FTS_QUAD_SIZE + fYDecal);
        }
    }

    return ERR_OK;
}

/// Unload everything that has to do with forests.
/** This unloads and frees everything that has been loaded/allocated by
 *  the loadForests method.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int Map::unloadForests()
{
    for(int i = 0 ; i < m_nForests ; i++) {
        SAFE_DELETE(m_ppForests[i]);
    }
    SAFE_DELETE_ARR(m_ppForests);
    SAFE_DELETE_ARR(m_pForestsRegionsMap);
    m_nForests = 0;

    return ERR_OK;
}

/// Get a pointer to one of the forests.
/** This method returns a pointer to one of the Forests.
 *
 * \param in_ucID The ID of the forest you want to get.
 *
 * \return The forest with the corresponding ID, or NULL if there is none.
 *
 * \note The forests start with ID 1, as ID 0 means no forest.
 *
 * \author Pompei2
 */
Forest *Map::getForest(unsigned char in_ucID)
{
    return (in_ucID <= m_nForests && in_ucID > 0) ? m_ppForests[in_ucID-1] : NULL;
}

int Map::unload(void)
{
    this->unloadForests();
    SAFE_DELETE(m_pMapInfo);
    SAFE_DELETE(m_pTerrain);
    m_sFile = String::EMPTY;

    return ERR_OK;
}

int Map::draw(unsigned int in_uiTicks)
{
    m_pTerrain->draw(in_uiTicks);

    // Now draw the forests. TODO: Optimize: draw only the ones we see.
    for(int i = 0 ; i < m_nForests ; i++) {
        m_ppForests[i]->drawAll(in_uiTicks);
    }

    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    return  (g_pTree ? g_pTree->draw(in_uiTicks) : ERR_OK);
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
}
