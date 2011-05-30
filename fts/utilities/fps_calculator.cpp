#include "fps_calculator.h"

#include "utilities/utilities.h"

using namespace FTS;

FTS::FPSCalculator::FPSCalculator()
    : m_uiFramesCounter(0)
    , m_dLastFPS(0.0f)
{
    UpdateableManager::getSingleton().add("FPSCalculator", this);
}

FTS::FPSCalculator::~FPSCalculator()
{
    UpdateableManager::getSingleton().rem("FPSCalculator");
}

bool FTS::FPSCalculator::update(const Clock& c)
{
    m_uiFramesCounter++;

    // If less then one second passed, we do nothing.
    if(m_chrono.measure() < 1.0)
        return true;

    // But if one or more seconds passed by, calculate the FPS:
    m_dLastFPS = static_cast<double>(m_uiFramesCounter)/m_chrono.reset();

    // And reset the frames counter.
    m_uiFramesCounter = 0;
    return true;
}
