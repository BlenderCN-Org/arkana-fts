#ifndef FTS_MINLOGGER_H
#define FTS_MINLOGGER_H

#include "logging/logger.h"

namespace FTS {
    class String;

/// This is a completely minimalistic logger with no dependencies
/// (except the string class) but it also does nothing special at all.
class MinimalLogger : public FTS::Logger {
protected:
    int m_iDbgLv = 5;
    bool m_bMuted = false;

    /// Protect from copying.
    MinimalLogger(const MinimalLogger&) {};

public:
    MinimalLogger(int in_iDbgLv = 5);
    virtual ~MinimalLogger();

    int loadConfig() {return 0;};

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

} // namespace FTS

#endif // FTS_MINLOGGER_H
