#include "Rotator.h"

#include "main/Clock.h"
#include "main/Exception.h"
#include "map/MapObject.h"

FTS::Rotator::Rotator(const std::weak_ptr<MapObject>& in_pWhom, const Quaternion& in_qTo, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_qFrom(in_pWhom.lock()->rot())
    , m_qTo(in_qTo)
{
    if(in_fDuration <= 0.0f || nearZero(in_fDuration))
        throw InvalidCallException("FTS::Rotator with negative or near zero duration: " + String::nr(in_fDuration));
}

FTS::Rotator::Rotator(const std::weak_ptr<MapObject>& in_pWhom, const Quaternion& in_qFrom, const Quaternion& in_qTo, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_qFrom(in_qFrom)
    , m_qTo(in_qTo)
{
    if(in_fDuration <= 0.0f || nearZero(in_fDuration))
        throw InvalidCallException("FTS::Rotator with negative or near zero duration: " + String::nr(in_fDuration));
}

FTS::Rotator::~Rotator()
{
}

bool FTS::Rotator::update(const Clock& in_clock)
{
    if(!Mover::update(in_clock))
        return false;

    if(this->expired() || this->paused())
        return true;

    // And update its rotation.
    MapObject* pObj = m_pWhom.lock().get();
    Quaternion result = m_qFrom.nlerp(m_qTo, this->percentDone());
    AxisAngle aa = result.toAxisAngle();
    pObj->rot(result);
    return true;
}

FTS::RotatorVel::RotatorVel(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vVelocity, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_vNormalizedAxis(in_vVelocity.normalized())
    , in_fRadPerSec(in_vVelocity.len())
{
}

FTS::RotatorVel::RotatorVel(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vAxis, float in_fRadPerSec, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_vNormalizedAxis(in_vAxis.normalized())
    , in_fRadPerSec(in_fRadPerSec)
{
}

FTS::RotatorVel::~RotatorVel()
{
}

bool FTS::RotatorVel::update(const Clock& in_clock)
{
    if(!Mover::update(in_clock))
        return false;

    if(this->expired() || this->paused())
        return true;

    // Get the differential angle we have to rotate.
    Quaternion qAdditionalRotation = Quaternion::rotation(m_vNormalizedAxis, in_fRadPerSec*static_cast<float>(in_clock.getDeltaT())*this->speed());

    // And update its rotation.
    MapObject* pObj = m_pWhom.lock().get();
    pObj->rot(qAdditionalRotation * pObj->rot());
    return true;
}
