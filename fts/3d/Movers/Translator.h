#ifndef D_TRANSLATOR_H
#define D_TRANSLATOR_H

#include "Mover.h"

#include "3d/math/Vector.h"

namespace FTS {

class Translator : public Mover {
public:
    Translator(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vTo, float in_fDuration, const String& in_sName = String::EMPTY);
    Translator(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vFrom, const Vector& in_vTo, float in_fDuration, const String& in_sName = String::EMPTY);
    virtual ~Translator();

    virtual bool update(const Clock&);

private:
    Vector m_vFrom;
    Vector m_vTo;
};

class TranslatorVel : public Mover {
public:
    TranslatorVel(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vVelocity, float in_fDuration, const String& in_sName = String::EMPTY);
    virtual ~TranslatorVel();

    virtual bool update(const Clock&);

private:
    Vector m_vVel;
};

} // namespace FTS

#endif // D_TRANSLATOR_H
