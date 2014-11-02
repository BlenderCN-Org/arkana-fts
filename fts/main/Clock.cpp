#include "Clock.h"

#include <functional>
#include <algorithm>
#include <chrono>

using namespace FTS;

FTS::Clock::Clock()
{
    m_dStartTime = std::chrono::steady_clock::now();
    m_dLastTick = m_dStartTime;
    m_dCurrentTime = m_dLastTick;
}

FTS::Clock::~Clock()
{
}

void FTS::Clock::tick()
{
    m_dLastTick = m_dCurrentTime;
    m_dCurrentTime = std::chrono::steady_clock::now();

    // Update the list of ticks in the last second: remove all ticks that are
    // older than one second.
    while ( !m_lastTicks.empty() && std::chrono::duration_cast< std::chrono::milliseconds >( m_dCurrentTime - m_lastTicks.front() ).count() > 1000 )
        m_lastTicks.pop_front();

    // Add the current tick at the end.
    m_lastTicks.push_back(m_dCurrentTime);
}

double FTS::Clock::getCurrentTime() const
{
    return std::chrono::duration_cast< std::chrono::milliseconds >( m_dCurrentTime - m_dStartTime ).count() / 1000.0;
}

double FTS::Clock::getDeltaT() const
{
    return std::chrono::duration_cast< std::chrono::milliseconds >( m_dCurrentTime - m_dLastTick ).count() / 1000.0 ;
}

double FTS::Clock::getTPS() const
{
    // That's exactly how many ticks have been done in the last second.
    return m_lastTicks.size();
}

