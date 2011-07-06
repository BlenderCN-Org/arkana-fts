#include "AxisAlignedBoundingBox.h"

#include <bouge/Math.hpp>

FTS::AxisAlignedBoundingBox::AxisAlignedBoundingBox(const bouge::Vector& in_point)
    : m_fTop(in_point.z())
    , m_fBottom(in_point.z())
    , m_fLeft(in_point.x())
    , m_fRight(in_point.x())
    , m_fFront(in_point.y())
    , m_fBack(in_point.y())
{
}

FTS::AxisAlignedBoundingBox::AxisAlignedBoundingBox(float in_fScale)
    : m_fTop(in_fScale)
    , m_fBottom(-in_fScale)
    , m_fLeft(-in_fScale)
    , m_fRight(in_fScale)
    , m_fFront(-in_fScale)
    , m_fBack(in_fScale)
{
}

FTS::AxisAlignedBoundingBox::AxisAlignedBoundingBox(float in_fTop, float in_fBottom, float in_fLeft, float in_fRight, float in_fFront, float in_fBack)
    : m_fTop(in_fTop)
    , m_fBottom(in_fBottom)
    , m_fLeft(in_fLeft)
    , m_fRight(in_fRight)
    , m_fFront(in_fFront)
    , m_fBack(in_fBack)
{
}

FTS::AxisAlignedBoundingBox& FTS::AxisAlignedBoundingBox::update(const bouge::Vector& in_point)
{
    m_fTop    = std::max(m_fTop,    in_point.z());
    m_fBottom = std::min(m_fBottom, in_point.z());
    m_fLeft   = std::min(m_fLeft,   in_point.x());
    m_fRight  = std::max(m_fRight,  in_point.x());
    m_fFront  = std::min(m_fFront,  in_point.y());
    m_fBack   = std::max(m_fBack,   in_point.y());

    return *this;
}

bouge::AffineMatrix FTS::AxisAlignedBoundingBox::getModelMatrix() const
{
    float dx_2 = (this->right() - this->left())*0.5f;
    float dy_2 = (this->back() - this->front())*0.5f;
    float dz_2 = (this->top() - this->bottom())*0.5f;
    float xMid = this->right() - dx_2;
    float yMid = this->back() - dy_2;
    float zMid = this->top() - dz_2;

    return bouge::AffineMatrix::translation(xMid, yMid, zMid) * bouge::AffineMatrix::scale(dx_2, dy_2, dz_2);
}
