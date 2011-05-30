#ifndef CLOCK_H
#define CLOCK_H
#include "main.h"

#include <list>

namespace FTS {

class Clock
{
public:
    Clock();
    ~Clock();

    virtual void tick();

    double getDeltaT() const;
    double getCurrentTime() const;
    double getTPS() const;

protected:
    double m_dStartTime;
    double m_dLastTick;
    double m_dCurrentTime;

    std::list<double> m_lastTicks;

private:
    uint32_t getClockTicks();
#if !WINDOOF
    struct timeval g_tvStart;
#endif
};

}; // namespace FTS

#endif // CLOCK_H
