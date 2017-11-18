/**
 * \file forest.cpp
 * \author Pompei2
 * \date 07 January 2007
 * \brief This file contains the implementation of the forest class.
 **/

#include "map/forest.h"
#include "map/quad.h"

#include "3d/3d.h"
#include "logging/logger.h"
#include "game/objects/tree.h"
#include "utilities/Math.h"

using namespace FTS;

Forest::Forest(unsigned char in_ucID)
{
    m_ucID = in_ucID;
    m_sType = "oak";
    m_fDiversity = 0.2f;
    m_fDensity = 4.0f;
    m_sHeight = "mid";
    m_QuadTrees.clear();
    m_bLoaded = false;
}

Forest::~Forest()
{
    m_ucID = 0;
    this->unload();
}

/// Loads the forest, but not the trees.
/** This loads the informations about this forest into the class members,
 *  but it won't actually load the trees. You need to plant some trees onto
 *  a quad to load them.
 *
 * \param in_sConfFile The configuration file used to load the forest params.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int Forest::load(const String &in_sConfFile)
{
    if(m_bLoaded)
        return ERR_OK;

    if(in_sConfFile.empty()) {
        FTS18N("InvParam", MsgType::Horror, "CFTSForest::load");
        return -1;
    }
    // Get the forest parameters.
    String sPrefix = "Forest"+String::nr(m_ucID)+"%d_";
    Settings defaults;
    defaults.add(sPrefix + "Type", "oak");
    defaults.add(sPrefix + "Diversity", 0.2f);
    defaults.add(sPrefix + "Density", 0.4f);
    defaults.add(sPrefix + "Height", "mid");
    Configuration conf(in_sConfFile, defaults, false);
    m_sType = conf.get(sPrefix + "Type");
    m_fDiversity = conf.getFloat(sPrefix + "Diversity");
    m_fDensity = conf.getFloat(sPrefix + "Density");
    m_sHeight = conf.get(sPrefix + "Height");

    // Check these params for validity:
    if(m_fDiversity < 0.0f || m_fDiversity > 1.0f) {
        FTS18N("MAP_Forest_InvParam", MsgType::Warning, String::nr(m_ucID), "Diversity", "0.2");
        m_fDiversity = 0.2f;
    }
    if(m_fDensity < 0.0f || m_fDensity > 10.0f) {
        FTS18N("MAP_Forest_InvParam", MsgType::Warning, String::nr(m_ucID), "Density", "0.4");
        m_fDensity = 0.4f;
    }
    if(!(m_sHeight == "low" || m_sHeight == "mid" || m_sHeight == "high")) {
        FTS18N("MAP_Forest_InvParam", MsgType::Warning, String::nr(m_ucID), "Height", "mid");
        m_sHeight = "mid";
    }
    // TODO: Check the type parameter !!

    m_bLoaded = true;
    return ERR_OK;
}

/// Unloads the forest, and its trees.
/** This unloads all the things that have been loaded by the load method but it
 *  additionally unloads ALL trees of this forest !
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int Forest::unload()
{
    if(!m_bLoaded)
        return ERR_OK;

    // Go trough every quads that this forest belongs to.
    for(std::map<int, std::vector<Tree *> >::iterator i = m_QuadTrees.begin() ; i != m_QuadTrees.end() ; i++) {
        // Now go trough every tree of this forest.
        for(std::vector<Tree *>::iterator j = i->second.begin() ; j != i->second.end() ; j++) {
            Tree *pTree = (*j);
            SAFE_DELETE(pTree);
        }
    }

    m_sType = "oak";
    m_fDiversity = 0.2f;
    m_fDensity = 4.0f;
    m_sHeight = "mid";
    m_bLoaded = false;

    return ERR_OK;
}

/// Plants some trees of this forest into a quad.
/** This plants some trees of this forest into a given quad. That means it
 *  creates some trees according to this forest's parameters.
 *
 * \param in_iQuadID The ID of the quad where to plant some trees. The quad ID
 *                   is it's index in the quads array, aka y*map_w + x.
 * \param in_fQuadX The X position of the top left corner of that quad.
 * \param in_fQuadY The Y position of the top left corner of that quad.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \note For more details about the tree generation, take a look at our dokuwuki.
 *
 * \author Pompei2
 */
int Forest::plantForest(int in_iQuadID, float in_fQuadX, float in_fQuadY)
{
    // Find out the number of trees that go into this quad.
    int nTrees = (int)m_fDensity;

    // A certain % chance to get an additional tree.
    float fRest = m_fDensity - (float)nTrees;
    if(((float)random(0,100) / 100.0f) <= fRest) {
        nTrees++;
    }

    std::vector<Tree *> &vTreeVec = m_QuadTrees[in_iQuadID];

    for(int i = 0 ; i < nTrees ; i++) {
        // Calculate the name of the tree to load. We are sure
        // the file exists because of the validation in the load method.
        String sTreeName = m_sType;
        if(((float)random(0,100) / 100.0f) <= m_fDiversity) {
            // Pick a random tree.
            std::vector<String> vsAllTrees = Forest::getExistingTreeList();
            sTreeName = vsAllTrees[random<int>(0,(int)vsAllTrees.size()-1)];
        }

        // Calculate a random position in the quad, but keep some distance from the other trees.
        Vector vPos(in_fQuadX, in_fQuadY, 3.0f);
        bool bCorrect = false;
        while(!bCorrect) {
            // TODO: The Z Pos. Overall, it would be better to already store the X/Y pos
            // Directly into the quads and give this forest really a quad !
            vPos.x(in_fQuadX + ((float)random(0,100) / 100.0f)*FTS_QUAD_SIZE)
                .y(in_fQuadY - ((float)random(0,100) / 100.0f)*FTS_QUAD_SIZE)
                .z(3.0f);
            bCorrect = true;

            // Check agains all trees in this quad if there is a minimum distance.
            for(std::vector<Tree *>::iterator iTree = vTreeVec.begin() ; iTree != vTreeVec.end() ; iTree++) {
                Tree *pOther = *iTree;
                // If there is one tree with a too little distance, stop looking and break.
                // TODO: Make the tree distance dependent on the density.
                if((vPos - pOther->getPos()).len() < D_FOREST_TREE_DIST) {
                    bCorrect = false;
                    break;
                }
            }
        }

        // Load, position and randomize the tree.
        Tree *pTree = new Tree();
        pTree->load(Path::datadir("Models/Gaia/Flora/Trees") + sTreeName + FTS_DIR_SEPARATOR + sTreeName + "_" + m_sHeight + ".ftsmodel");
        pTree->setPos(vPos);
        vTreeVec.push_back(pTree->randomize());
    }

    return ERR_OK;
}

/// Get a list of all existing trees.
/** This method goes look into the right directories what trees are disponible
 *  and then puts their names into a vector of strings.
 *
 * \return A vector of all existing tree names.
 *
 * \todo Automatically generate the list.
 *
 * \author Pompei2
 */
std::vector<String> Forest::getExistingTreeList()
{
    // TODO: look for folders !
    std::vector<String> vRet;
    vRet.push_back("beech");
//     vRet.push_back("oak");
    return vRet;
}

/// Draws all trees of this forest that are in one quad.
/** This draws all trees that are in one certain quad and belong to this forest.
 *  If the quad does not have this forest on it, this method does nothing.
 *
 * \param in_iQuadID The ID of the quad to draw the forest.
 * \param in_uiTicks The number of ticks that passed from the beginning of the game, in ms.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
int Forest::draw(int in_iQuadID, unsigned int in_uiTicks)
{
    std::map<int, std::vector<Tree *> >::iterator i = m_QuadTrees.find(in_iQuadID);

    // This quad does not possess this forest.
    if(i == m_QuadTrees.end())
        return ERR_OK;

    // Found some trees, draw them !
    for(std::vector<Tree *>::iterator j = i->second.begin() ; j != i->second.end() ; j++) {
        Tree *pTree = *j;
        pTree->draw(in_uiTicks);
    }

    return ERR_OK;
}

/// Draws ALL trees of this forest.
/** This draws ALL trees that belong to this forest. You should not use it !
 *
 * \param in_uiTicks The number of ticks that passed from the beginning of the game, in ms.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \todo Do not need this method anymore !
 *
 * \author Pompei2
 */
int Forest::drawAll(unsigned int in_uiTicks)
{
    // Go trough every quads that this forest belongs to.
    for(std::map<int, std::vector<Tree *> >::iterator i = m_QuadTrees.begin() ; i != m_QuadTrees.end() ; i++) {
        // Now go trough every tree of this forest.
        for(std::vector<Tree *>::iterator j = i->second.begin() ; j != i->second.end() ; j++) {
            Tree *pTree = (*j);
            pTree->draw(in_uiTicks);
        }
    }

    return ERR_OK;
}

 /* EOF */
