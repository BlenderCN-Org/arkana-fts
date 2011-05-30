/**
 * \file Quaternion.h
 * \author Pompei2
 * \date 2 February 2010
 * \brief This file contains the quaternion class definition. This class defines a 4-component vector.
 **/

#ifndef D_QUATERNION_H
#define D_QUATERNION_H

#include "Vector.h"

#include "utilities/Math.h"

#ifdef D_CAL3D_INTEGRATION
#  include <cal3d/quaternion.h>
#endif

namespace FTS {
    class String;
    class File;
    class Quaternion;

    /// This class represents an axis-angle description of an arbitrary rotation.
    /// The angle is stored in radians and the axis is normalized.
    struct AxisAngle {
        Vector axis;
        float angle;

        /// \param in_axis The axis of rotation.
        /// \param in_angle The angle of rotation, in radians.
        AxisAngle(const Vector& in_axis, float in_angle) : axis(in_axis.normalized()), angle(in_angle) {}
    };

    ///////////////////////////////
    // Quaternion serialization. //
    ///////////////////////////////
    /// Serializes a quaternion to a file. Use it like this: f << q1 << q2 << etc.
    /// \param f The file to write the quaternion to.
    /// \param q The quaternion to write to the file.
    /// \return a reference to the file to allow chaining.
    File& operator<<(File& f, const Quaternion& q);
    /// Extracts a quaternion from a file. Use it like this: f >> q1 >> q2 >> etc.
    /// \param f The file to read the quaternion from.
    /// \param q The quaternion to write the read values to.
    /// \return a reference to the file to allow chaining.
    File& operator>>(File& f, Quaternion& q);
class Quaternion {
    friend File& operator<<(File& f, const Quaternion& q);
    friend File& operator>>(File& f, Quaternion& q);

public:
    ////////////////////////////////////////////
    // Constructors and assignment operators. //
    ////////////////////////////////////////////

    /// Creates a quaternion with all components set to 0
    Quaternion();
    /// Creates a quaternion based on the contents of a float array.
    /// \param in_q The four components of the quaternion.
    Quaternion(float in_q[4]);
    /// Creates a quaternion.
    /// \param in_fX The value of the first component of the quaternion.
    /// \param in_fY The value of the second component of the quaternion.
    /// \param in_fZ The value of the third component of the quaternion.
    /// \param in_fW The value of the fourth component of the quaternion.
    Quaternion(float in_fX, float in_fY, float in_fZ, float in_fW = 1.0f);
    /// Copies a quaternion.
    /// \param in_q The quaternion to be copied.
    Quaternion(const Quaternion& in_q);
    /// Copies a quaternion.
    /// \param in_q The quaternion to be copied.
    /// \return a const reference to myself that might be used as a rvalue.
    const Quaternion& operator=(const Quaternion& in_q);
    /// Moves a quaternion.
    /// \param in_q The quaternion to be moved.
    Quaternion(Quaternion&& in_q);
    /// Moves a quaternion.
    /// \param in_q The quaternion to be moved.
    /// \return a const reference to myself that might be used as a rvalue.
    const Quaternion& operator=(Quaternion&& in_q);
    ~Quaternion();

    //////////////////////////////////////
    // Special Quaternion constructors. //
    //////////////////////////////////////

    /// Creates a quaternion that represents a rotation of \a in_fPhi radians
    /// about an arbitrary axis going from the origin to the point (\a in_fX,
    /// \a in_fY, \a in_fZ).
    /// \param in_fX The x-coordinate of the endpoint of the rotation axis.
    /// \param in_fY The y-coordinate of the endpoint of the rotation axis.
    /// \param in_fZ The z-coordinate of the endpoint of the rotation axis.
    /// \param in_fRadians The angle of rotation, in \e radians.
    static Quaternion rotation(float in_fX, float in_fY, float in_fZ, float in_fRadians);
    /// Creates a quaternion that represents a rotation of \a in_fPhi radians
    /// about an arbitrary axis going from the origin to the point \a in_v.
    /// \param in_v The other endpoint of the rotation axis.
    /// \param in_fRadians The angle of rotation, in \e radians.
    static Quaternion rotation(const Vector& in_v, float in_fRadians);
    /// Creates a quaternion that represents a rotation described in axis-angle notation.
    /// \param in_rot The axis-angle description of the rotation.
    static Quaternion rotation(const AxisAngle& in_rot);

    ///////////////////////////////////////
    // Conversion methods and operators. //
    ///////////////////////////////////////

    /// \return A read-only array of four floats holding the values of the
    ///         four components of this quaternion.
    inline const float *array4f() const {return m_q;};
    /// \return A read-only array of four floats holding the values of the
    ///         four components of this quaternion.
    inline operator const float*() const {return m_q;};

    /// \return A string-representation of the quaternion.
    /// \param in_iDecimalPlaces The amount of numbers to print behind the dot.
    String toString(unsigned int in_iDecimalPlaces = 2) const;
    /// \return A string-representation of the quaternion.
    operator String() const;

    /// \return An axis-angle representation of the rotation.
    AxisAngle toAxisAngle() const;

#ifdef D_CAL3D_INTEGRATION
    Quaternion(const CalQuaternion&);
    operator CalQuaternion() const;
#endif // D_CAL3D_INTEGRATION

    /////////////////////////////////////
    // Accessors, getters and setters. //
    /////////////////////////////////////

    /// \return The X coordinate of the quaternion.
    inline float x() const { return m_q[0]; };
    /// \return The Y coordinate of the quaternion.
    inline float y() const { return m_q[1]; };
    /// \return The Z coordinate of the quaternion.
    inline float z() const { return m_q[2]; };
    /// \return The W coordinate of the quaternion.
    inline float w() const { return m_q[3]; };
    /// \param in_fX The new X coordinate of the quaternion.
    inline Quaternion& setX(float in_fX) { m_q[0] = in_fX; return *this; };
    /// \param in_fY The new Y coordinate of the quaternion.
    inline Quaternion& setY(float in_fY) { m_q[1] = in_fY; return *this; };
    /// \param in_fZ The new Z coordinate of the quaternion.
    inline Quaternion& setZ(float in_fZ) { m_q[2] = in_fZ; return *this; };
    /// \param in_fW The new W coordinate of the quaternion.
    inline Quaternion& setW(float in_fW) { m_q[3] = in_fW; return *this; };

    /// Access the elements of this quaternion.
    /// \param idx The index of the element of this quaternion. This may only be a
    ///            value between 0 and 3.
    /// \throws NotExistException if \a idx is >3.
    float& operator[](unsigned int idx);

    ////////////////////////////////////
    // Basic Quaternion calculations. //
    ////////////////////////////////////

    /// \return A negated copy of this quaternion.
    Quaternion operator -() const;
    /// Adds two quaternions.
    /// \param in_q The quaternion to add to this quaternion.
    /// \returns the quaternion resulting from this + \a in_q
    Quaternion operator +(const Quaternion& in_q) const;
    /// Subtracts two quaternion.
    /// \param in_q The quaternion to subtract from this quaternion.
    /// \returns the quaternion resulting from this - \a in_q
    Quaternion operator -(const Quaternion& in_q) const;
    /// Creates a scaled quaternion.
    /// \param in_f The scaling factor.
    /// \returns the quaternion resulting from this * \a in_f (this multiplied component-wise by f).
    Quaternion operator *(float in_f) const;
    /// Creates a shrinked quaternion.
    /// \param in_f The shrinking factor.
    /// \returns the quaternion resulting from this / \a in_f (this divided component-wise by f).
    Quaternion operator /(float in_f) const;

    /// \param in_q The quaternion to add to this quaternion. The result is stored in this quaternion.
    void operator +=(const Quaternion & in_q);
    /// \param in_q The quaternion to subtract from this quaternion. The result is stored in this quaternion.
    void operator -=(const Quaternion & in_q);
    /// \param in_f The factor to scale this quaternion. The result is stored in this quaternion.
    void operator *=(float in_f);

    /// Calculate the product of two quaternions. This concatenates their rotation.
    /// Note that the order matters, just as with matrices.
    /// \param in_q The second quaternion of the product.
    /// \return The resulting quaternion from *this * \a in_q.
    Quaternion operator *(const Quaternion & in_q) const;
    /// Calculate the quotient of this divided by \a in_q. This is the same as
    /// multiplying this by the inverse of \a in_q.
    /// \param in_q The denominator. (Lower part of the fraction.)
    /// \return The resulting quaternion from *this / \a in_q.
    Quaternion operator /(const Quaternion & in_q) const;
    /// Calculate the dot product of two quaternions.
    /// \param in_q The second quaternion of the dot product.
    /// \return The resulting quaternion from *this DOT \a in_q.
    float dot(const Quaternion & in_q) const;
    /// Calculate the conjugate of this quaternion. That is the first three components negated.
    /// i.e. The rotation axis is inverted and thus it rotates the other way around.
    /// \return The conjugate of this quaternion.
    Quaternion cnj() const;
    /// Calculate the inverse of this quaternion. This is the conjugate, but with
    /// length 1/L where L is the length of this. Thus for unit quaternions
    /// it is the same as the conjugate.
    Quaternion inv() const;

    ///////////////////////////////////////////
    // Quaternion length related operations. //
    ///////////////////////////////////////////

    /// \return The length of this quaternion, using the euclides norm.
    float len() const;
    /// Normalizes this quaternion: makes it have unit length.
    /// \return a reference to *this
    Quaternion& normalize();
    /// \return A normalized copy of this quaternion. It has unit length.
    Quaternion normalized() const;

    //////////////////////////////////////////
    // Quaternion interpolation operations. //
    //////////////////////////////////////////

    /// Normalized Linear interpolation between this and v2
    /// \param v2 The other quaternion with which to interpolate.
    /// \param between The time of interpolation. 0.0f results in this, 1.0f results in \a v2.
    /// \return A quaternion resulting from the normalized linear interpolation of this and \a v2, at time \a between.
    ///         This is especially useful to interpolate softly between two rotation angles.
    /// \note nlerp travels along the curve with non-constant speed but it IS commutative
    ///       and it is FAST to compute.
    Quaternion nlerp(const Quaternion& v2, float between) const;

    /// Spherical Linear interpolation between this and v2
    /// \param v2 The other quaternion with which to interpolate.
    /// \param between The time of interpolation. 0.0f results in this, 1.0f results in \a v2.
    /// \return A quaternion resulting from the spherical linear interpolation of this and \a v2, at time \a between.
    ///         This is especially useful to interpolate softly between two rotation angles.
    /// \note Slerp travels along the curve with constant speed but it is NOT
    ///       commutative and it is SLOW. Prefer using nlerp. See this link to know why:
    ///       http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/
    Quaternion slerp(const Quaternion& v2, float between) const;

    ///////////////////////////////////////
    // Quaternion comparison operations. //
    ///////////////////////////////////////

    /// \return true if this is longer than \a in_q.
    inline bool operator >(const Quaternion &in_q) const {return this->len() > in_q.len();};
    /// \return true if this is shorter than \a in_q.
    inline bool operator <(const Quaternion &in_q) const {return this->len() < in_q.len();};
    /// \return true if this is longer or has the same length as \a in_q.
    inline bool operator >=(const Quaternion &in_q) const {return this->len() >= in_q.len();};
    /// \return true if this is shorter or has the same length as \a in_q.
    inline bool operator <=(const Quaternion &in_q) const {return this->len() <= in_q.len();};
    /// \return true if this is \e nearly the same as \a in_q.
    inline bool operator ==(const Quaternion &in_q) const {
        Quaternion diff = *this - in_q;
        return nearZero(diff.x()) && nearZero(diff.y()) && nearZero(diff.z());
    };
    /// \return true if this is \e not \e nearly the same as \a in_q.
    inline bool operator !=(const Quaternion &in_q) const {return !this->operator==(in_q);};

    //////////////////////////////
    // Rotating by quaternions. //
    //////////////////////////////

    /// Rotates a vector by this quaternion.
    /// \param in_v The vector to rotate.
    /// \return A new vector that is the result of having rotated the given
    ///         vector by this quaternion. (ret = this * in_v * this.inv)
    Vector rotate(const Vector& in_v);

private:
    /// The four components of the quaternion.
    float* m_q;
};

} // namespace FTS

#endif // D_QUATERNION_H
