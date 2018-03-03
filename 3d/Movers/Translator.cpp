#include "Translator.h"

#include "main/Clock.h"
#include "main/Exception.h"
#include "map/MapObject.h"

FTS::Translator::Translator(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vTo, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_vFrom(in_pWhom.lock()->pos())
    , m_vTo(in_vTo)
{
    if(in_fDuration <= 0.0f || nearZero(in_fDuration))
        throw InvalidCallException("FTS::Translator::to with negative or near zero duration: " + String::nr(in_fDuration));
}

FTS::Translator::Translator(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vFrom, const Vector& in_vTo, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_vFrom(in_vFrom)
    , m_vTo(in_vTo)
{
    if(in_fDuration <= 0.0f || nearZero(in_fDuration))
        throw InvalidCallException("FTS::Translator::fromto with negative or near zero duration: " + String::nr(in_fDuration));
}

FTS::Translator::~Translator()
{
}

bool FTS::Translator::update(const Clock& in_clock)
{
    if(!Mover::update(in_clock))
        return false;

    if(this->expired() || this->paused())
        return true;

    // And update its position.
    MapObject* pObj = m_pWhom.lock().get();
    pObj->pos(m_vFrom.lerp(m_vTo, this->percentDone()));
    return true;
}

FTS::TranslatorVel::TranslatorVel(const std::weak_ptr<MapObject>& in_pWhom, const Vector& in_vVelocity, float in_fDuration, const String& in_sName)
    : Mover(in_pWhom, in_fDuration, in_sName)
    , m_vVel(in_vVelocity)
{
}

FTS::TranslatorVel::~TranslatorVel()
{
}

bool FTS::TranslatorVel::update(const Clock& in_clock)
{
    if(!Mover::update(in_clock))
        return true;

    if(this->expired() || this->paused())
        return true;

    // And update its position.
    MapObject* pObj = m_pWhom.lock().get();
    pObj->pos(pObj->pos() + m_vVel*static_cast<float>(in_clock.getDeltaT())*this->speed());
    return true;
}
