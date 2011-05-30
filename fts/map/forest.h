/**
 * \file forest.h
 * \author Pompei2
 * \date 19 June 2008
 * \brief This file contains the definition of the forest class.
 **/

#ifndef FTS_FOREST_H
#define FTS_FOREST_H

#include "main.h"
#include <vector>
#include <map>

#include "graphic/graphic.h"
#include "dLib/dConf/configuration.h"

namespace FTS {
    class Tree;

#define D_FOREST_TREE_DIST 0.0f

/// This class represents a forest. For more details, go see our dokuwiki.
class Forest {
private:
    unsigned char m_ucID; ///< The ID of the forest (see dokuwiki)
    String m_sType;      ///< The type of the forest (see dokuwiki)
    float m_fDiversity;   ///< The diversity of the forest (see dokuwiki)
    float m_fDensity;     ///< The density of the forest (see dokuwiki)
    String m_sHeight;     ///< The height of the forest ("low", "mid", "high", see dokuwiki)

    /// This is a map that contains every quad that this forest occupies, and
    /// for every quad it contains a vector of all trees in that quad.
    /// \todo: replace by a multmap?
    std::map<int, std::vector<Tree *> >m_QuadTrees;

    bool m_bLoaded; ///< Wether this forest is loaded or not.
        class Settings : public DefaultOptions {
    public:
        Settings() {
        }
    };
    
public:
    Forest(unsigned char in_ucID);
    virtual ~Forest();

    int load(const String &in_sConfFile);
    int unload();

    int plantForest(int in_iQuadID, float in_fQuadX, float in_fQuadY);

    static std::vector<String>getExistingTreeList();

    int draw(int in_iQuadID, unsigned int in_uiTicks);
    int drawAll(unsigned int in_uiTicks);
};

}

#endif                          /* FTS_FOREST_H */

 /* EOF */
