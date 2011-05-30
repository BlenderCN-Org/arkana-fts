#include "Clock.h"

#include <functional>
#include <algorithm>
#if !WINDOOF
#include <sys/time.h>
#endif

using namespace FTS;

FTS::Clock::Clock()
    : m_dLastTick(0.0)
    , m_dCurrentTime(0.0)
{
#if !WINDOOF
    gettimeofday(&g_tvStart, NULL);
#endif
    m_dStartTime = static_cast<double>(getClockTicks())/1000.0;
}

FTS::Clock::~Clock()
{
}

void FTS::Clock::tick()
{
    m_dLastTick = m_dCurrentTime;
    m_dCurrentTime = static_cast<double>(getClockTicks())/1000.0 - m_dStartTime;

    // Update the list of ticks in the last second: remove all ticks that are
    // older than one second.
    while(!m_lastTicks.empty() && m_dCurrentTime - m_lastTicks.front() > 1.0)
        m_lastTicks.pop_front();

    // Add the current tick at the end.
    m_lastTicks.push_back(m_dCurrentTime);
}

double FTS::Clock::getCurrentTime() const
{
    return m_dCurrentTime;
}

double FTS::Clock::getDeltaT() const
{
    return m_dCurrentTime - m_dLastTick;
}

double FTS::Clock::getTPS() const
{
    // That's exactly how many ticks have been done in the last second.
    return m_lastTicks.size();
}

uint32_t FTS::Clock::getClockTicks()
{
#if WINDOOF
    return GetTickCount();
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec-g_tvStart.tv_sec)*1000 + (now.tv_usec-g_tvStart.tv_usec)/1000;
#endif
}