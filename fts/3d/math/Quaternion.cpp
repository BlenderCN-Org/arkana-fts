/**
 * \file quaternion.cpp
 * \author Pompei2
 * \date 2 February 2010
 * \brief This file contains the implementation of the quaternion class. This class defines a 4-component vector.
 **/

#include "Quaternion.h"

#include "Vector.h"
#include "main/Exception.h"
#include "dLib/dString/dString.h"
#include "dLib/dFile/dFile.h"

using namespace FTS;

////////////////////////////////////////////
// Constructors and assignment operators. //
////////////////////////////////////////////

FTS::Quaternion::Quaternion()
    : m_q(new float[4])
{
    m_q[0] = 0.0f;
    m_q[1] = 0.0f;
    m_q[2] = 0.0f;
    m_q[3] = 1.0f;
}

FTS::Quaternion::Quaternion(float in_v[4])
    : m_q(new float[4])
{
    m_q[0] = in_v[0];
    m_q[1] = in_v[1];
    m_q[2] = in_v[2];
    m_q[3] = in_v[3];
}

FTS::Quaternion::Quaternion(const Quaternion& in_q)
    : m_q(new float[4])
{
    m_q[0] = in_q.x();
    m_q[1] = in_q.y();
    m_q[2] = in_q.z();
    m_q[3] = in_q.w();
}

const Quaternion& FTS::Quaternion::operator=(const Quaternion& in_q)
{
    m_q[0] = in_q.x();
    m_q[1] = in_q.y();
    m_q[2] = in_q.z();
    m_q[3] = in_q.w();

    return *this;
}

FTS::Quaternion::Quaternion(float in_fX, float in_fY, float in_fZ, float in_fW)
    : m_q(new float[4])
{
    m_q[0] = in_fX;
    m_q[1] = in_fY;
    m_q[2] = in_fZ;
    m_q[3] = in_fW;
}

FTS::Quaternion::Quaternion(Quaternion&& in_q)
{
    this->operator=(std::move(in_q));
}

const Quaternion& FTS::Quaternion::operator=(Quaternion&& in_q)
{
    m_q = in_q.m_q;
    in_q.m_q = nullptr;

    return *this;
}

Quaternion::~Quaternion()
{
    if(m_q) delete [] m_q;
}

//////////////////////////////////////
// Special Quaternion constructors. //
//////////////////////////////////////

Quaternion FTS::Quaternion::rotation(float in_fX, float in_fY, float in_fZ, float in_fRadians)
{
    return Quaternion::rotation(Vector(in_fX, in_fY, in_fZ), in_fRadians);
}

Quaternion FTS::Quaternion::rotation(const Vector& in_v, float in_fRadians)
{
    float omega = 0.5f*in_fRadians;
    Vector v = sin(omega) * in_v.normalized();
    return Quaternion(v.x(), v.y(), v.z(), cos(omega));
}

Quaternion FTS::Quaternion::rotation(const AxisAngle& in_rot)
{
    return Quaternion::rotation(in_rot.axis, in_rot.angle);
}

///////////////////////////////////////
// Conversion methods and operators. //
///////////////////////////////////////

String FTS::Quaternion::toString(unsigned int in_iDecimalPlaces) const
{
    return "("+String::nr(this->x(), in_iDecimalPlaces)+", "
              +String::nr(this->y(), in_iDecimalPlaces)+", "
              +String::nr(this->z(), in_iDecimalPlaces)+", "
              +String::nr(this->w(), in_iDecimalPlaces)+")";
}

FTS::Quaternion::operator String() const
{
    return this->toString();
}

FTS::AxisAngle FTS::Quaternion::toAxisAngle() const
{
    float factor = 1.0f/sqrt(1-this->w()*this->w());
    return AxisAngle(Vector(this->x()*factor,
                            this->y()*factor,
                            this->z()*factor).normalize(),
                     2.0f*acos(this->w()));
}

#ifdef D_CAL3D_INTEGRATION
FTS::Quaternion::Quaternion(const CalQuaternion& o)
    : m_q(new float[4])
{
    m_q[0] = o.x;
    m_q[1] = o.y;
    m_q[2] = o.z;
    m_q[3] = o.w;
}

FTS::Quaternion::operator CalQuaternion() const
{
    return CalQuaternion(this->x(), this->y(), this->z(), this->w());
}
#endif

/////////////////////////////////////
// Accessors, getters and setters. //
/////////////////////////////////////

float& FTS::Quaternion::operator[](unsigned int idx)
{
    if(idx > 3)
        throw FTS::NotExistException("Index " + String::nr(idx), "Vector");

    return m_q[idx];
}

////////////////////////////////////
// Basic Quaternion calculations. //
////////////////////////////////////

Quaternion FTS::Quaternion::operator -() const
{
    return Quaternion(-this->x(),
                      -this->y(),
                      -this->z(),
                      -this->w());
}

Quaternion FTS::Quaternion::operator +(const Quaternion& in_q) const
{
    return Quaternion(this->x() + in_q.x(),
                      this->y() + in_q.y(),
                      this->z() + in_q.z(),
                      this->w() + in_q.w());
}

Quaternion FTS::Quaternion::operator -(const Quaternion& in_q) const
{
    return (*this) + (-in_q);
}

Quaternion FTS::Quaternion::operator *(float in_f) const
{
    return Quaternion(this->x() * in_f,
                      this->y() * in_f,
                      this->z() * in_f,
                      this->w() * in_f);
}

Quaternion FTS::Quaternion::operator /(float in_f) const
{
    return Quaternion(this->x() / in_f,
                      this->y() / in_f,
                      this->z() / in_f,
                      this->w() / in_f);
}

void FTS::Quaternion::operator +=(const Quaternion& in_q)
{
    this->operator=((*this) + in_q);
}

void FTS::Quaternion::operator -=(const Quaternion& in_q)
{
    this->operator+=(-in_q);
}

void FTS::Quaternion::operator *=(float in_f)
{
    this->operator=((*this) * in_f);
}

Quaternion FTS::Quaternion::operator *(const Quaternion& in_q) const
{
    return Quaternion(this->w() * in_q.x() + this->x() * in_q.w() + this->y() * in_q.z() - this->z() * in_q.y(),
                      this->w() * in_q.y() + this->y() * in_q.w() + this->z() * in_q.x() - this->x() * in_q.z(),
                      this->w() * in_q.z() + this->z() * in_q.w() + this->x() * in_q.y() - this->y() * in_q.x(),
                      this->w() * in_q.w() - this->x() * in_q.x() - this->y() * in_q.y() - this->z() * in_q.z());
}

Quaternion FTS::Quaternion::operator /(const Quaternion & in_q) const
{
    return *this * this->inv();
}

float FTS::Quaternion::dot(const Quaternion& in_q) const
{
    return this->x() * in_q.x()
         + this->y() * in_q.y()
         + this->z() * in_q.z()
         + this->w() * in_q.w();
}

Quaternion FTS::Quaternion::cnj() const
{
    return Quaternion(-this->x(),
                      -this->y(),
                      -this->z(),
                       this->w());
}

Quaternion Quaternion::inv() const
{
    float l = this->len();
    return this->cnj()/(l*l);
}

///////////////////////////////////////////
// Quaternion length related operations. //
///////////////////////////////////////////

float FTS::Quaternion::len() const
{
    return sqrt(this->x()*this->x()
              + this->y()*this->y()
              + this->z()*this->z()
              + this->w()*this->w());
}

Quaternion& FTS::Quaternion::normalize()
{
    // The zero-quaternion stays the zero-quaternion.
    if(nearZero(this->x()) &&
       nearZero(this->y() == 0.0f) &&
       nearZero(this->z() == 0.0f) &&
       nearZero(this->w() == 0.0f) ) {
        return this->setX(0.0f).setY(0.0f).setZ(0.0f).setW(0.0f);
    }

    float l = this->len();

    // Very little quaternion will be stretched to a unit quaternion in one direction.
    if(nearZero(l)) {
        if((this->x() >= this->y())
        && (this->x() >= this->z())
        && (this->x() >= this->w())
        && (this->x() >= 0.0f)) {
            return this->setX(1.0f).setY(0.0f).setZ(0.0f).setW(0.0f);
        } else if((this->x() <= this->y())
               && (this->x() <= this->z())
               && (this->x() <= this->w())
               && (this->x() <= 0.0f)) {
            return this->setX(-1.0f).setY(0.0f).setZ(0.0f).setW(0.0f);
        } else {
            if(this->y() >= this->z()
            && this->y() >= this->w()
            && this->y() >= 0.0f) {
                return this->setX(0.0f).setY(1.0f).setZ(0.0f).setW(0.0f);
            } else if(this->y() <= this->z()
                   && this->y() <= this->w()
                   && this->y() <= 0.0f) {
                return this->setX(0.0f).setY(-1.0f).setZ(0.0f).setW(0.0f);
            } else {
                if(this->z() >= this->w()
                && this->z() >= 0.0f) {
                    return this->setX(0.0f).setY(0.0f).setZ(1.0f).setW(0.0f);
                } else if(this->z() <= this->w()
                       && this->z() <= 0.0f) {
                    return this->setX(0.0f).setY(0.0f).setZ(-1.0f).setW(0.0f);
                } else {
                    return this->setX(0.0f).setY(0.0f).setZ(0.0f).setW(this->w() >= 0.0f ? 1.0f : -1.0f);
                }
            }
        }
    } else {
        // Follows the usual normalization rule.
        float m = 1.0f / l;
        return this->setX(this->x()*m).setY(this->y()*m).setZ(this->z()*m).setW(this->w()*m);
    }
}

Quaternion FTS::Quaternion::normalized() const
{
    Quaternion copy(*this);
    return copy.normalize();
}

//////////////////////////////////////////
// Quaternion interpolation operations. //
//////////////////////////////////////////
Quaternion FTS::Quaternion::nlerp(const Quaternion& q2, float between) const
{
    return (*this + (q2 - *this)*between).normalize();
}

Quaternion FTS::Quaternion::slerp(const Quaternion& q2, float between) const
{
    float cosTheta = this->dot(q2);
    cosTheta = std::min(cosTheta, 1.0f);
    cosTheta = std::max(cosTheta, -1.0f); // Clamp to [-1, 1] for the acos.
    float theta    = acos(cosTheta);
    float sinTheta = sin(theta);

    float w1, w2;

    if(sinTheta > D_FTS_EPSILON || sinTheta < -D_FTS_EPSILON)
    {
        w1 = float(sin((1.0f-between)*theta) / sinTheta);
        w2 = float(sin(between*theta) / sinTheta);
    } else {
        // Quaternions a and b are nearly the same, do linear interpolation.
        w1 = 1.0f - between;
        w2 = between;
    }

    return ((*this)*w1 + q2*w2).normalize();
}

//////////////////////////////
// Rotating by quaternions. //
//////////////////////////////

Vector FTS::Quaternion::rotate(const Vector& in_v)
{
    Quaternion vOrig = Quaternion(in_v.x(), in_v.y(), in_v.z(), 0.0f);
    Quaternion vRotated = *this * vOrig * this->cnj();
    return Vector(vRotated.x(), vRotated.y(), vRotated.z());
}

///////////////////////////////
// Quaternion serialization. //
///////////////////////////////

File& FTS::operator<<(File& f, const Quaternion& v) {
    return f << v.x() << v.y() << v.z() << v.w();
}

File& FTS::operator>>(File& f, Quaternion& v) {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;
    f >> x >> y >> z >> w;
    v.setX(x).setY(y).setZ(z).setW(w);
    return f;
}
