#ifndef D_CHRONOMETER_H
#define D_CHRONOMETER_H

#include "main.h"

#include "main/Clock.h"
#include "dLib/dString/dString.h"

namespace FTS {

/// This class can be used to measure the time that passed between two specific
/// events.
class Chronometer {
    Clock m_clock;
    double m_dIntegral;

public:
    Chronometer();

    virtual double measure();
    virtual double reset();
};

/// This is a specialisation of the chronometer. It does exactly the same as the
/// chronometer but in addition, it loggs a message when being measured.
class LoggingChronometer : public Chronometer {
    String m_sDesc; ///< The description that will be shown on every measuring.
    int m_iDbgLv;    ///< The debug level the message should be logged in.

public:
    LoggingChronometer(const String &in_sDesc, int in_iDbgLv);

    virtual double measure();
    virtual double measure(const String &in_sAdditionalInfo);
    virtual double reset();
};

};

#endif // D_CHRONOMETER_H
