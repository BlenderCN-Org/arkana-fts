/**
 * \file unit.h
 * \author Pompei2
 * \date 01 June 2008
 * \brief This file contains an abstract FTS game unit definition.
 **/

#ifndef FTS_UNIT_H
#define FTS_UNIT_H

#include "main.h"

#include "game/objects/objects.h"

namespace FTS {

class UnitBase : public GameObject {
public:
             UnitBase() {};
    virtual ~UnitBase() {};

    virtual int load(const String &in_sName) {GameObject::load(in_sName); return ERR_OK;};
    virtual int unload() {GameObject::unload(); return ERR_OK;};

    virtual int draw(unsigned int in_uiTicks) {GameObject::draw(in_uiTicks); return ERR_OK;};

protected:
    virtual File& store(File& in_File) const {return GameObject::store(in_File);};
    virtual File& restore(File& in_File) {return GameObject::restore(in_File);};
};

} // namespace FTS

#endif /* FTS_UNIT_H */

 /* EOF */
