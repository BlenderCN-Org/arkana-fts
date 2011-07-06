#ifndef D_MAP_OBJECT_H
#define D_MAP_OBJECT_H

#include "3d/Math.h"

namespace FTS {
    class Color;

/// This abstract class represents an (3d) object in space. This object may be
/// moved around or whatever. It has a position, orientation (rotation) and scale.
/// Orientation is the facing direction of the object (north, south, ...)
class MapObject {
public:
    /// \param in_vPos The position of the object.
    /// \param in_fOrientation The rotation around Z-axis (in radians).
    /// \param in_vScale How the object is scaled, in the three directions.
    MapObject(const Vector& in_vPos = Vector(), float in_fOrientation = 0.0f, const Vector& in_vScale = Vector(1.0f, 1.0f, 1.0f));

    /// \param in_vPos The position of the object.
    /// \param in_qRot The (arbitrary) rotation of the object.
    /// \param in_vScale How the object is scaled, in the three directions.
    MapObject(const Vector& in_vPos, const Quaternion& in_qRot, const Vector& in_vScale = Vector(1.0f, 1.0f, 1.0f));

    virtual ~MapObject();

    /// \param in_other The other object to be copied.
    /// \return A reference to *this.
    MapObject(const MapObject& in_other);
    /// \param in_other The other object to be copied.
    /// \return A reference to *this.
    MapObject& operator=(const MapObject& in_other);

    /// \param in_other The other object to be moved.
    /// \return A reference to *this.
    MapObject(MapObject&& in_other);
    /// \param in_other The other object to be moved.
    /// \return A reference to *this.
    MapObject& operator=(MapObject&& in_other);

    /// Renders this map object exactly as it should appear, i.e. at a certain
    /// position, with a certain scale and orientation.
    virtual void render(const Color& in_playerColor);

    /// \param in_vNewPos The new position the object gets "teleported" to.
    virtual void pos(const Vector& in_vNewPos);
    /// \return The current position of the object.
    Vector pos() const;

    /// \param in_fOrientation The new orientation of the object, again in radians.
    virtual void rot(float in_fOrientation);
    /// \return The current orientation (in radians) of the object. If the object
    ///         is subject to an arbitrary rotation, this returns 0.
    float orientation() const;

    /// \param in_qNewRot The new (arbitrary) rotation of the object.
    virtual void rot(const Quaternion& in_qNewRot);
    /// \return The current arbitrary rotation of the object.
    Quaternion rot() const;

    virtual void scale(const Vector& in_vNewScale);
    Vector scale() const;

protected:
    AffineMatrix getModelMatrix() const;

private:
    Vector m_vPos;
    Quaternion m_qRot;
    Vector m_vScale;
};

}; // namespace FTS

#endif // D_MAP_OBJECT_H
