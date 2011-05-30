#ifndef AXISALIGNEDBOUNDINGBOX_H
#define AXISALIGNEDBOUNDINGBOX_H

namespace FTS {
    class Vector;
    class AffineMatrix;

/// This class represents an axis-aligned bounding-box, that means that the six
/// planes of the bounding box are parallel to the main axes (X,Y,Z).
/// In our game, the "ground" plane is the (X,Y) plane and the Z axis is
/// pointing into the sky. The positive X shows to the right (East) and the
/// positive Y points away from us (to the North)
class AxisAlignedBoundingBox
{
public:
    /// Constructs a AABB with a starting point. The AABB that is being built
    /// this way is malformed: it is actually a single point.
    /// This constructor is mainly intended as a starting-point for building
    /// an AABB for a whole model.
    /// \param in_point The first point to use for the AABB.
    AxisAlignedBoundingBox(const FTS::Vector& in_point);

    /// Constructs a square AABB around the origin.
    /// \param in_fScale The distance from the origin to each plane.
    AxisAlignedBoundingBox(float in_fScale = 1.0f);

    /// Constructs an arbitrary AABB.
    /// \param in_fTop The coordinate of the top plane, in Z direction.
    /// \param in_fBottom The coordinate of the bottom plane, in Z direction.
    /// \param in_fLeft The coordinate of the left plane, in X direction.
    /// \param in_fRight The coordinate of the right plane, in X direction.
    /// \param in_fFront The coordinate of the front plane, in Y direction.
    /// \param in_fBack The coordinate of the back plane, in Y direction.
    AxisAlignedBoundingBox(float in_fTop, float in_fBottom, float in_fLeft, float in_fRight, float in_fFront, float in_fBack);

    /// \return The coordinate of the top plane, in Z direction.
    inline float top() const {return m_fTop;};
    /// \return The coordinate of the bottom plane, in Z direction.
    inline float bottom() const {return m_fBottom;};
    /// \return The coordinate of the left plane, in X direction.
    inline float left() const {return m_fLeft;};
    /// \return The coordinate of the right plane, in X direction.
    inline float right() const {return m_fRight;};
    /// \return The coordinate of the front plane, in Y direction.
    inline float front() const {return m_fFront;};
    /// \return The coordinate of the back plane, in Y direction.
    inline float back() const {return m_fBack;};

    /// \param in_fTop The coordinate of the top plane, in Z direction.
    /// \return a reference to *this, for chaining.
    inline AxisAlignedBoundingBox& top(float in_fTop) {m_fTop = in_fTop; return *this;};
    /// \param in_fBottom The coordinate of the bottom plane, in Z direction.
    /// \return a reference to *this, for chaining.
    inline AxisAlignedBoundingBox& bottom(float in_fBottom) {m_fBottom = in_fBottom; return *this;};
    /// \param in_fLeft The coordinate of the left plane, in X direction.
    /// \return a reference to *this, for chaining.
    inline AxisAlignedBoundingBox& left(float in_fLeft) {m_fLeft = in_fLeft; return *this;};
    /// \param in_fRight The coordinate of the right plane, in X direction.
    /// \return a reference to *this, for chaining.
    inline AxisAlignedBoundingBox& right(float in_fRight) {m_fRight = in_fRight; return *this;};
    /// \param in_fFront The coordinate of the front plane, in Y direction.
    /// \return a reference to *this, for chaining.
    inline AxisAlignedBoundingBox& front(float in_fFront) {m_fFront = in_fFront; return *this;};
    /// \param in_fBack The coordinate of the back plane, in Y direction.
    /// \return a reference to *this, for chaining.
    inline AxisAlignedBoundingBox& back(float in_fBack) {m_fBack = in_fBack; return *this;};

    /// Updates the bounding box with the given point. It means that, in case
    /// the point lies outside of it enlarges the AABB just enough to hold that
    /// point.
    /// \param in_point The point to make be inside the AABB.
    /// \return a reference to *this, for chaining.
    AxisAlignedBoundingBox& update(const Vector& in_point);

    AffineMatrix getModelMatrix() const;

private:
    /// +Z
    float m_fTop;
    /// -Z
    float m_fBottom;
    /// +X
    float m_fLeft;
    /// -X
    float m_fRight;
    /// -Y
    float m_fFront;
    /// +Y
    float m_fBack;
};

} // namespace FTS

#endif // AXISALIGNEDBOUNDINGBOX_H
