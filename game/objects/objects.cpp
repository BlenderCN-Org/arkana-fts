/**
 * \file objects.cpp
 * \author Pompei2
 * \date 05 June 2008
 * \brief This file implements the default fts object methods.
 **/

#include "game/objects/objects.h"
#include "logging/logger.h"

using namespace FTS;

GameObject::GameObject()
{
    m_bLoaded = false;
    m_vPos = Vector(0.0f, 0.0f, 0.0f);
    m_vScale = Vector(1.0f, 1.0f, 1.0f);
    m_fOrientation = 0.0f;
}

GameObject::~GameObject()
{
    this->unload();
}

int GameObject::load(const String &in_sName, const Vector &in_vPos, const Vector &in_vScale, float in_fOrientation)
{
    if(m_bLoaded)
        this->unload();

    m_sName = in_sName;
    m_bLoaded = true;

    m_vPos = in_vPos;
    m_vScale = in_vScale;
    m_fOrientation = in_fOrientation;

    return ERR_OK;
}

int GameObject::unload()
{
    if(!m_bLoaded)
        return ERR_OK;

    m_sName = String::EMPTY;
    m_bLoaded = false;

    m_vPos = Vector(0.0f, 0.0f, 0.0f);
    m_vScale = Vector(1.0f, 1.0f, 1.0f);
    m_fOrientation = 0.0f;

    return ERR_OK;
}

File& GameObject::store(File& out_File) const
{
    return out_File << m_sName << m_vPos << m_vScale << m_fOrientation;
}

File& GameObject::restore(File& out_File)
{
    return out_File >> m_sName >> m_vPos >> m_vScale >> m_fOrientation;
}

int GameObject::draw(unsigned int in_uiTicks)
{
    return ERR_OK;
}
