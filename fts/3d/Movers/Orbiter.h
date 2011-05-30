#ifndef D_ORBITER_H
#define D_ORBITER_H

#include "Mover.h"

#include "3d/math/Vector.h"

namespace FTS {

class OrbiterVel : public Mover {
public:
    OrbiterVel(const std::weak_ptr<MapObject>& in_pWhom, const std::weak_ptr<MapObject>& in_vCenter, const Vector& in_vVelocity, float in_fDuration, const String& in_sName = String::EMPTY);
    OrbiterVel(const std::weak_ptr<MapObject>& in_pWhom, const std::weak_ptr<MapObject>& in_vCenter, const Vector& in_vAxis, float in_fRadPerSec, float in_fDuration, const String& in_sName = String::EMPTY);
    virtual ~OrbiterVel();

    virtual bool update(const Clock&);

private:
    Vector m_vNormalizedAxis;
    float in_fRadPerSec;
    std::weak_ptr<MapObject> m_pCenter;
};

} // namespace FTS

#endif // D_ROTATOR_H
