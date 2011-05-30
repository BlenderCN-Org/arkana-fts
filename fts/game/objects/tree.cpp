/**
 * \file tree.cpp
 * \author Pompei2
 * \date 05 June 2008
 * \brief This file contains the implementation of a tree in the game.
 **/

#include "game/objects/tree.h"
#include "utilities/Math.h"

using namespace FTS;

Tree::Tree()
{
}

Tree::~Tree()
{
    this->unload();
}

int Tree::load(const String &in_sName)
{
    if(m_bLoaded)
        this->unload();

    UnitBase::load(in_sName);

    return ERR_OK;
}

int Tree::unload()
{
    if(!m_bLoaded)
        return ERR_OK;

    UnitBase::unload();
    return ERR_OK;
}

File& Tree::store(File& out_File) const
{
    return UnitBase::store(out_File) << m_vScale;
}

File& Tree::restore(File& in_File)
{
    return UnitBase::restore(in_File) >> m_vScale;
}

int Tree::draw(unsigned int in_uiTicks)
{
    return ERR_OK;
}

/// Randomize this tree.
/** This gives the tree a controlled random scale and orientation.
 *
 * \return this
 *
 * \note This first chooses a random "main scale" factor between
 *       0.8 and 1.4 and then scales every direction for a random
 *       amount (between 0.0 and 0.1) around this "main scale". \n
 *       The orientation is just choosed randomly from [0,360]
 *
 * \author Pompei2
 */
Tree *Tree::randomize()
{
    // Choose a random "main scale" factor between 0.8 and 1.2 first:
    float fMainScale = 0.8f + ((float)random(0, 100) / 100.0f)*0.4f;

    // Now scale in every direction for a random amount (between 0.0 and 0.1%)
    // around this "main scale"
    this->setScale(Vector(fMainScale + ((float)random(0, 100) / 1000.0f),
                          fMainScale + ((float)random(0, 100) / 1000.0f),
                          fMainScale + ((float)random(0, 100) / 1000.0f)));
    this->setOrientation((float)random(0, 360));

    return this;
}

 /* EOF */
