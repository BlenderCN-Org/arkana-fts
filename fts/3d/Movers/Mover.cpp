#include "Mover.h"

#include "main/Clock.h"
#include "utilities/Math.h"

FTS::Mover::Mover(const std::weak_ptr<MapObject>& in_pWhom, float in_fDuration, const String& in_sName)
    : m_pWhom(in_pWhom)
    , m_fDuration(in_fDuration)
    , m_fDurationLeft(in_fDuration)
    , m_fSpeed(1.0f)
    , m_fOldSpeed(1.0f)
    , m_sName(in_sName)
    , m_bSelfdestroy(false)
{
    // If a name was given, use this for the updateable.
    if(!m_sName.isEmpty())
        UpdateableManager::getSingleton().add(m_sName, this);
    else
        UpdateableManager::getSingleton().add(this);
}

FTS::Mover::~Mover()
{
    if(!m_sName.isEmpty())
        UpdateableManager::getSingleton().rem(m_sName);
    else
        UpdateableManager::getSingleton().rem(this);
}

bool FTS::Mover::update(const Clock& in_clock)
{
    m_fDurationLeft -= static_cast<float>(in_clock.getDeltaT()) * m_fSpeed;
    return !(this->expired() && m_bSelfdestroy);
}

bool FTS::Mover::exec()
{
    m_fDurationLeft = m_fDuration;
    this->resume();
    return true;
}

FTS::Mover* FTS::Mover::planSelfDestruction()
{
    m_bSelfdestroy = true;
    return this;
}

FTS::Mover* FTS::Mover::pause()
{
    m_fOldSpeed = m_fSpeed;
    m_fSpeed = 0.0f;
    return this;
}

FTS::Mover* FTS::Mover::resume()
{
    m_fSpeed = m_fOldSpeed;
    return this;
}

bool FTS::Mover::paused() const
{
    return nearZero(m_fSpeed);
}

FTS::Mover* FTS::Mover::speed(float in_fSpeed)
{
    m_fOldSpeed = m_fSpeed = in_fSpeed;
    return this;
}

float FTS::Mover::speed() const
{
    return m_fSpeed;
}

bool FTS::Mover::expired() const
{
    return (m_fDurationLeft <= 0.0f && m_fDuration > 0.0f) || m_pWhom.expired();
}

float FTS::Mover::percentDone() const
{
    return (this->duration() - this->durationLeft())/this->duration();
}

float FTS::Mover::durationLeft() const
{
    return m_fDurationLeft;
}

float FTS::Mover::duration() const
{
    return m_fDuration;
}

FTS::MoverStopper::MoverStopper(Mover* in_pMover)
    : m_pMover(in_pMover)
{
}

bool FTS::MoverStopper::exec()
{
    m_pMover->pause();
    return true;
}
