/**
 * \file objects.h
 * \author Pompei2
 * \date 01 June 2008
 * \brief This file contains an abstract FTS game object definition.
 **/

#ifndef FTS_OBJECT_H
#define FTS_OBJECT_H

#include "main.h"

#include "3d/3d.h"
#include "3d/Math.h"
#include "dLib/dString/dString.h"
#include "dLib/dFile/dFile.h"

namespace FTS {

class GameObject {
    friend inline File& operator<<(File& o, const GameObject& obj);
    friend inline File& operator>>(File& i, GameObject& obj);
public:
    virtual ~GameObject();

    virtual int load(const String &in_sName,
                     const Vector &in_vPos   = Vector(0.0f, 0.0f, 0.0f),
                     const Vector &in_vScale = Vector(1.0f, 1.0f, 1.0f),
                     float in_fOrientation = 0.0f);
    virtual int unload();

    virtual int draw(unsigned int in_uiTicks);

    virtual const Vector& getPos() const {return m_vPos;};
    virtual const Vector& getScale() const {return m_vScale;};
    virtual float getOrientation() const {return m_fOrientation;};

    virtual GameObject *setPos(const Vector &v) {m_vPos = v; return this;};
    virtual GameObject *setScale(const Vector&v) {m_vScale = v; return this;};
    virtual GameObject *setOrientation(float f) {m_fOrientation = f; return this;};

protected:
    bool m_bLoaded;      ///< Wether the object has been successfully loaded or not.
    String m_sName;     ///< The name of the object (could be anything, but mostly a filename).

    Vector m_vPos;    ///< The current position of the object.
    Vector m_vScale;  ///< This scales the object in all directions (or not).
    float m_fOrientation; ///< The orientation (in degrees) of the object, 0.0f meaning no rotation.

    GameObject();

    virtual File& store(File&) const;
    virtual File& restore(File&);
};

inline File& operator<<(File& o, const GameObject& obj) {
    return obj.store(o);
}

inline File& operator>>(File& i, GameObject& obj) {
    return obj.restore(i);
}

} // namespace FTS

#endif /* FTS_OBJECT_H */

 /* EOF */
