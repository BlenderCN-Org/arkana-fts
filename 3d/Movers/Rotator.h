#ifndef D_ROTATOR_H
#define D_ROTATOR_H

#include "Mover.h"

#include "3d/Math.h"

namespace FTS {

class Rotator : public Mover {
public:
    Rotator(const std::weak_ptr<MapObject>& in_pWhom, const Quaternion& in_qTo, float in_fDuration, const String& in_sName = String::EMPTY);
    Rotator(const std::weak_ptr<MapObject>& in_pWhom, const Quaternion& in_qFrom, const Quaternion& in_qTo, float in_fDuration, const String& in_sName = String::EMPTY);
    virtual ~Rotator();

    virtual bool update(const Clock&);

private:
    Quaternion m_qFrom;
    Quaternion m_qTo;
};

class RotatorVel : public Mover {
public:
    RotatorVel(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vVelocity, float in_fDuration, const String& in_sName = String::EMPTY);
    RotatorVel(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vAxis, float in_fRadPerSec, float in_fDuration, const String& in_sName = String::EMPTY);
    virtual ~RotatorVel();

    virtual bool update(const Clock&);

private:
    Vector m_vNormalizedAxis;
    float in_fRadPerSec;
};

} // namespace FTS

#endif // D_ROTATOR_H
