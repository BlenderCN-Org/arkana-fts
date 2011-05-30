#ifndef D_MOVER_H
#define D_MOVER_H

#include "main/Updateable.h"
#include "utilities/command.h"

#include "dLib/dString/dString.h"

#include <memory>

namespace FTS {
    class MapObject;

class Mover : public Updateable, public CommandBase {
public:
    Mover(const std::weak_ptr<MapObject>& in_pWhom, float in_fDuration, const String& in_sName = String::EMPTY);
    virtual ~Mover() = 0;

    virtual bool update(const Clock&) = 0;
    virtual bool exec();

    Mover* planSelfDestruction();

    Mover* pause();
    Mover* resume();
    bool paused() const;

    Mover* speed(float in_fSpeed);
    float speed() const;

protected:
    bool expired() const;
    float percentDone() const;
    float durationLeft() const;
    float duration() const;

    std::weak_ptr<MapObject> m_pWhom;

private:
    float m_fDuration;
    float m_fDurationLeft;
    float m_fSpeed;
    float m_fOldSpeed;
    String m_sName;
    bool m_bSelfdestroy;
};

class MoverStopper : public CommandBase {
public:
    MoverStopper(Mover* in_pMover);

    virtual bool exec();

private:
    Mover* m_pMover;
};

} // namespace FTS

#endif // D_MOVER_H
