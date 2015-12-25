#ifndef CLOCK_H
#define CLOCK_H
#include "main.h"

#include <list>
#include <chrono>

namespace FTS {

class Clock
{
public:
    Clock();
    virtual ~Clock();

    virtual void tick();

    /*! Returns the time difference in seconds since the last call of tick().
    */
    double getDeltaT() const;

    /*! Returns the time in seconds since program start.
    */
    double getCurrentTime() const;
    
    /*! Returns the ticks of the current second.
    */
    double getTPS() const;

protected:
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_lastTick;
    std::chrono::steady_clock::time_point m_currentTime;

    std::list<std::chrono::steady_clock::time_point> m_lastTicks;

private:
};

}; // namespace FTS

#endif // CLOCK_H
