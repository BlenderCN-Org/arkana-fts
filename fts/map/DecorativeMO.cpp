#include "DecorativeMO.h"

#include "3d/ModelInstance.h"
#include "graphic/Color.h"

FTS::DecorativeMO::DecorativeMO(std::unique_ptr<ModelInstance> in_pModelInst, const FTS::Vector& in_vPos, float in_fOrientation, const FTS::Vector& in_vScale)
    : MapObject(in_vPos, in_fOrientation, in_vScale)
    , m_pModelInst(std::move(in_pModelInst))
{

}

FTS::DecorativeMO::DecorativeMO(std::unique_ptr<ModelInstance> in_pModelInst, const FTS::Vector& in_vPos, const FTS::Quaternion& in_qRot, const FTS::Vector& in_vScale)
    : MapObject(in_vPos, in_qRot, in_vScale)
    , m_pModelInst(std::move(in_pModelInst))
{

}

FTS::DecorativeMO::~DecorativeMO()
{

}

FTS::DecorativeMO::DecorativeMO(DecorativeMO&& in_other)
{
    this->operator=(std::move(in_other));
}

FTS::DecorativeMO& FTS::DecorativeMO::operator=(DecorativeMO&& in_other)
{
    MapObject::operator=(std::move(in_other));

    m_pModelInst = std::move(in_other.m_pModelInst);

    return *this;
}

void FTS::DecorativeMO::render(const Color& in_playerColor)
{
    m_pModelInst->render(this->getModelMatrix(), in_playerColor);
}

FTS::ModelInstance* FTS::DecorativeMO::getModelInst() const
{
    return m_pModelInst.get();
}
