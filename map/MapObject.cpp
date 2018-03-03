#include "map/MapObject.h"

FTS::MapObject::MapObject(const FTS::Vector& in_vPos, float in_fOrientation, const FTS::Vector& in_vScale)
    : m_vPos(in_vPos)
    , m_qRot(Quaternion::rotation(Vector(0.0f, 0.0f, 1.0f), in_fOrientation))
    , m_vScale(in_vScale)
{

}

FTS::MapObject::MapObject(const FTS::Vector& in_vPos, const FTS::Quaternion& in_qRot, const FTS::Vector& in_vScale)
    : m_vPos(in_vPos)
    , m_qRot(in_qRot)
    , m_vScale(in_vScale)
{

}

FTS::MapObject::~MapObject()
{

}

FTS::MapObject::MapObject(const FTS::MapObject& in_other)
    : m_vPos(in_other.pos())
    , m_qRot(in_other.rot())
    , m_vScale(in_other.scale())
{
}

FTS::MapObject& FTS::MapObject::operator=(const FTS::MapObject& in_other)
{
    // Protect against self-assignment.
    if(&in_other == this) return *this;

    m_vPos = in_other.pos();
    m_qRot = in_other.rot();
    m_vScale = in_other.scale();

    return *this;
}

FTS::MapObject::MapObject(FTS::MapObject&& in_other)
{
    this->operator=(std::move(in_other));
}

FTS::MapObject& FTS::MapObject::operator=(FTS::MapObject&& in_other)
{
    m_vPos = std::move(in_other.m_vPos);
    m_qRot = std::move(in_other.m_qRot);
    m_vScale = std::move(in_other.m_vScale);

    return *this;
}

void FTS::MapObject::render(const FTS::Color& in_playerColor)
{
}

void FTS::MapObject::pos(const FTS::Vector& in_vNewPos)
{
    m_vPos = in_vNewPos;
}

FTS::Vector FTS::MapObject::pos() const
{
    return m_vPos;
}

void FTS::MapObject::rot(float in_fOrientation)
{
    m_qRot = Quaternion::rotation(Vector(0.0f, 0.0f, 1.0f), in_fOrientation);
}

float FTS::MapObject::orientation() const
{
    Vector axis = m_qRot.axis();
    float angle = m_qRot.angle();

    if(axis == Vector(0.0f, 0.0f, 1.0f))
        return angle;
    else if(axis == Vector(0.0f, 0.0f, -1.0f))
        return -angle;

    return 0.0f;
}

void FTS::MapObject::rot(const FTS::Quaternion& in_qNewRot)
{
    m_qRot = in_qNewRot;
}

FTS::Quaternion FTS::MapObject::rot() const
{
    return m_qRot;
}

void FTS::MapObject::scale(const FTS::Vector& in_vNewScale)
{
    m_vScale = in_vNewScale;
}

FTS::Vector FTS::MapObject::scale() const
{
    return m_vScale;
}

FTS::AffineMatrix FTS::MapObject::getModelMatrix() const
{
    return AffineMatrix::translation(m_vPos) * AffineMatrix::rotation(m_qRot) * AffineMatrix::scale(m_vScale);
}
