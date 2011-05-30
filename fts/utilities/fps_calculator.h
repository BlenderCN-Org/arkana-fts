#ifndef D_FPSCALCULATOR_H
#define D_FPSCALCULATOR_H

#include "main.h"

#include "utilities/Singleton.h"
#include "logging/Chronometer.h"
#include "main/Updateable.h"

namespace FTS {

class FPSCalculator : public Singleton<FPSCalculator>, public Updateable {
public:
    FPSCalculator();
    virtual ~FPSCalculator();

    bool update(const Clock&);
    inline double getFPS() const {return m_dLastFPS;};

private:
    Chronometer m_chrono;
    uint32_t m_uiFramesCounter;
    double m_dLastFPS;
};

} // namespace FTS

#endif // D_FPSCALCULATOR_H
