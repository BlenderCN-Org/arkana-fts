#ifndef FTS_TOOLCOMPAT_H
#define FTS_TOOLCOMPAT_H

#include <time.h>
#include <stdlib.h>
#include <locale.h>

// Common Arkana-FTS Headers.
#include "logging/logger.h"

namespace FTSTools {

void init();

/// Own logger for the tools
class MinimalLogger : public FTS::Logger {
protected:
    int m_iDbgLv;
    bool m_bMuted;

    /// Protect from copying.
    MinimalLogger(const MinimalLogger&) {};

public:
    MinimalLogger(int in_iDbgLv = 5);
    virtual ~MinimalLogger();

    int loadConfig() {return ERR_OK;};

    void stfu() {m_bMuted = true;};
    void sry() {m_bMuted = false;};

    virtual FTS::String formatMessage(const FTS::String &in_pszMsg,
                                      const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Raw,
                                      const FTS::String &in_sArg1 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg2 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg3 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg4 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg5 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg6 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg7 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg8 = FTS::String::EMPTY,
                                      const FTS::String &in_sArg9 = FTS::String::EMPTY
                                     ) const;
    virtual FTS::String formatMessageDbg(const FTS::String &in_pszMsg, int in_iDbgLv,
                                         const FTS::String &in_sArg1 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg2 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg3 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg4 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg5 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg6 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg7 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg8 = FTS::String::EMPTY,
                                         const FTS::String &in_sArg9 = FTS::String::EMPTY
                                        ) const;
    virtual FTS::String formatI18nMessage(const FTS::String &in_pszMsgID,
                                          const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Raw,
                                          const FTS::String &in_sArg1 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg2 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg3 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg4 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg5 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg6 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg7 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg8 = FTS::String::EMPTY,
                                          const FTS::String &in_sArg9 = FTS::String::EMPTY
                                         ) const;
    virtual FTS::String formatI18nMessageDbg(const FTS::String &in_pszMsgID, int in_iDbgLv,
                                             const FTS::String &in_sArg1 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg2 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg3 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg4 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg5 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg6 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg7 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg8 = FTS::String::EMPTY,
                                             const FTS::String &in_sArg9 = FTS::String::EMPTY
                                            ) const;
    int message(const FTS::String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Raw,
                const FTS::String &in_sArg1 = FTS::String::EMPTY,
                const FTS::String &in_sArg2 = FTS::String::EMPTY,
                const FTS::String &in_sArg3 = FTS::String::EMPTY,
                const FTS::String &in_sArg4 = FTS::String::EMPTY,
                const FTS::String &in_sArg5 = FTS::String::EMPTY,
                const FTS::String &in_sArg6 = FTS::String::EMPTY,
                const FTS::String &in_sArg7 = FTS::String::EMPTY,
                const FTS::String &in_sArg8 = FTS::String::EMPTY,
                const FTS::String &in_sArg9 = FTS::String::EMPTY);
    int messageDbg(const FTS::String &in_pszMsg, int in_iDbgLv,
                   const FTS::String &in_sArg1 = FTS::String::EMPTY,
                   const FTS::String &in_sArg2 = FTS::String::EMPTY,
                   const FTS::String &in_sArg3 = FTS::String::EMPTY,
                   const FTS::String &in_sArg4 = FTS::String::EMPTY,
                   const FTS::String &in_sArg5 = FTS::String::EMPTY,
                   const FTS::String &in_sArg6 = FTS::String::EMPTY,
                   const FTS::String &in_sArg7 = FTS::String::EMPTY,
                   const FTS::String &in_sArg8 = FTS::String::EMPTY,
                   const FTS::String &in_sArg9 = FTS::String::EMPTY);
    int i18nMessage(const FTS::String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Raw,
                    const FTS::String &in_sArg1 = FTS::String::EMPTY,
                    const FTS::String &in_sArg2 = FTS::String::EMPTY,
                    const FTS::String &in_sArg3 = FTS::String::EMPTY,
                    const FTS::String &in_sArg4 = FTS::String::EMPTY,
                    const FTS::String &in_sArg5 = FTS::String::EMPTY,
                    const FTS::String &in_sArg6 = FTS::String::EMPTY,
                    const FTS::String &in_sArg7 = FTS::String::EMPTY,
                    const FTS::String &in_sArg8 = FTS::String::EMPTY,
                    const FTS::String &in_sArg9 = FTS::String::EMPTY);
    int i18nMessageDbg(const FTS::String &in_pszMsgID, int in_iDbgLv,
                       const FTS::String &in_sArg1 = FTS::String::EMPTY,
                       const FTS::String &in_sArg2 = FTS::String::EMPTY,
                       const FTS::String &in_sArg3 = FTS::String::EMPTY,
                       const FTS::String &in_sArg4 = FTS::String::EMPTY,
                       const FTS::String &in_sArg5 = FTS::String::EMPTY,
                       const FTS::String &in_sArg6 = FTS::String::EMPTY,
                       const FTS::String &in_sArg7 = FTS::String::EMPTY,
                       const FTS::String &in_sArg8 = FTS::String::EMPTY,
                       const FTS::String &in_sArg9 = FTS::String::EMPTY);
    int doneConsoleMessage();
    int failConsoleMessage();
};

} // namespace FTSTools;

namespace FTS {
#ifndef FTS_UTILITIES_H
uint32_t dGetTicks();
void dSleep(unsigned long in_ulMilliseconds);
String getTranslatedString(const String &in_s, const String &, bool *);
#endif
} // namespace FTS

#endif /* FTS_TOOLCOMPAT_H */
