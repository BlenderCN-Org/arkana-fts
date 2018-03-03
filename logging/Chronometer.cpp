#include "Chronometer.h"

#include "logging/logger.h"
#include "utilities/utilities.h"

using namespace FTS;

/// Builds up and starts the chronometer. The time is measured from the moment you
/// built this object.
FTS::Chronometer::Chronometer()
    : m_dIntegral(0.0)
{
    this->reset();
}

/// \return The time passed by from the last chronometer reset or the chronometer
///         construction. The time is give in seconds.
double FTS::Chronometer::measure()
{
    m_clock.tick();
    m_dIntegral += m_clock.getDeltaT();

    return m_dIntegral;
}

/// Resets the chronometer, meaning the next measure counts from that point on.
/// \return The time passed by from the last chronometer reset or the chronometer
///         construction. The time is given in seconds.
double FTS::Chronometer::reset()
{
    double ret = this->measure();
    m_dIntegral = 0.0;
    return ret;
}

/// Builds up and starts the chronometer. The time is measured from the moment you
/// built this object.
///
/// \param in_sDesc A description message that will be shown in paranthesis on
///                 every action the chronometer makes (start, measure, reset).
/// \param in_iDbgLv The debug level the message should have when displayed.
FTS::LoggingChronometer::LoggingChronometer(const String &in_sDesc, int in_iDbgLv)
    : m_sDesc(in_sDesc)
    , m_iDbgLv(in_iDbgLv)
{
    FTS18NDBG("ChronometerStarted", m_iDbgLv, m_sDesc);
}

/// This takes a measure and displays the time measured and the description message.
/// \return The time passed by from the last chronometer reset or the chronometer
///         construction. The time is give in seconds.
double FTS::LoggingChronometer::measure()
{
    double ret = Chronometer::measure();
    FTS18NDBG("ChronometerMeasured", m_iDbgLv, m_sDesc,
              String::nr(ret, 3));
    return ret;
}

/// This takes a measure and displays the time measured and the description
/// message AND an additional string.
///
/// \param in_sAdditionalInfo The additional information to be displayed.
///
/// \return The time passed by from the last chronometer reset or the chronometer
///         construction. The time is give in seconds.
double FTS::LoggingChronometer::measure(const String &in_sAdditionalInfo)
{
    double ret = Chronometer::measure();
    FTS18NDBG("ChronometerMeasuredAdd", m_iDbgLv, m_sDesc,
              String::nr(ret, 3), in_sAdditionalInfo);
    return ret;
}

/// Resets the chronometer, meaning the next measure counts from that point on.
/// Also loggs a message containing the time passed.
/// \return The time passed by from the last chronometer reset or the chronometer
///         construction. The time is give in seconds.
double FTS::LoggingChronometer::reset()
{
    double ret = Chronometer::reset();
    FTS18NDBG("ChronometerReset", m_iDbgLv, m_sDesc,
              String::nr(ret, 3));
    return ret;
}
