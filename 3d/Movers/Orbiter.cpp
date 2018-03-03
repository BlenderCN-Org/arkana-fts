#include "Orbiter.h"

#include "main/Clock.h"
#include "main/Exception.h"
#include "map/MapObject.h"

FTS::OrbiterVel::OrbiterVel(const std::weak_ptr<MapObject>& in_pWhom, const std::weak_ptr<MapObject>& in_vCenter, const Vector& in_vVelocity, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_vNormalizedAxis(in_vVelocity.normalized())
    , in_fRadPerSec(in_vVelocity.len())
    , m_pCenter(in_vCenter)
{
}

FTS::OrbiterVel::OrbiterVel(const std::weak_ptr<MapObject>& in_pWhom, const std::weak_ptr<MapObject>& in_vCenter, const Vector& in_vAxis, float in_fRadPerSec, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_vNormalizedAxis(in_vAxis.normalized())
    , in_fRadPerSec(in_fRadPerSec)
    , m_pCenter(in_vCenter)
{
}

FTS::OrbiterVel::~OrbiterVel()
{
}

bool FTS::OrbiterVel::update(const Clock& in_clock)
{
    if(!Mover::update(in_clock))
        return false;

    if(this->expired() || this->paused() || m_pCenter.expired())
        return true;

    // First, find out the current vector from center to obj.
    auto pObj = m_pWhom.lock();
    auto pCenter = m_pCenter.lock();
    Vector vDist = pObj->pos() - pCenter->pos(); // Current vector from center to obj.

    // Rotate that vector by the needed amount.
    Quaternion qAdditionalRotation = Quaternion::rotation(m_vNormalizedAxis, in_fRadPerSec*static_cast<float>(in_clock.getDeltaT())*this->speed());
    vDist = qAdditionalRotation.rotate(vDist);

    // Now, move it to the place the new distance vector shows.
    pObj->pos(pCenter->pos() + vDist);
    return true;
}
