/**
 * \file tree.h
 * \author Pompei2
 * \date 01 June 2008
 * \brief This file contains the definition of a tree in the FTS game.
 **/

#ifndef FTS_TREE_H
#define FTS_TREE_H

#include "main.h"

#include "game/objects/unit.h"

namespace FTS {

class Tree : public UnitBase {
public:
             Tree();
    virtual ~Tree();

    virtual int load(const String &in_sName);
    virtual int unload();

    virtual int draw(unsigned int in_uiTicks);

    virtual Tree *randomize();

protected:
//    ModelInstance *m_pModelInstance;  ///< An instance of the 3D model of the tree.

    virtual File& store(File& out_File) const;
    virtual File& restore(File& in_File);
};

} // namespace FTS

#endif /* FTS_TREE_H */

 /* EOF */
