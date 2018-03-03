#include "Clock.h"

#include <functional>
#include <algorithm>
#include <chrono>

using namespace FTS;

FTS::Clock::Clock()
{
    m_startTime = std::chrono::steady_clock::now();
    m_lastTick = m_startTime;
    m_currentTime = m_lastTick;
}

FTS::Clock::~Clock()
{
}
 
void FTS::Clock::tick()
{
    m_lastTick = m_currentTime;
    m_currentTime = std::chrono::steady_clock::now();

    // Update the list of ticks in the last second: remove all ticks that are
    // older than one second.
    while ( !m_lastTicks.empty() && std::chrono::duration_cast< std::chrono::milliseconds >( m_currentTime - m_lastTicks.front() ).count() > 1000 )
        m_lastTicks.pop_front();

    // Add the current tick at the end.
    m_lastTicks.push_back(m_currentTime);
}

double FTS::Clock::getCurrentTime() const
{
    return std::chrono::duration<double>( m_currentTime - m_startTime ).count();
}

double FTS::Clock::getDeltaT() const
{
    return std::chrono::duration<double>( m_currentTime - m_lastTick ).count();
}

double FTS::Clock::getTPS() const
{
    // That's exactly how many ticks have been done in the last second.
    return (double)m_lastTicks.size();
}

