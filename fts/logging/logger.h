#ifndef FTS_LOGGER_H
#define FTS_LOGGER_H

#include "main.h"

#include "utilities/command.h"
#include "dLib/dString/dString.h"
#include "utilities/Singleton.h"
#include "utilities/PolymorphicCopyable.h"
#include "logging/MsgType.h"

namespace FTS {

class Logger : public Singleton<Logger> {
protected:
    /// Protect constructor.
    Logger() {};

public:
    virtual ~Logger() {};

    virtual int loadConfig() = 0;

    virtual void stfu() = 0;
    virtual void sry() = 0;

    virtual void suppressNextDlg() {};
    virtual void suppressAllDlgs() {};
    virtual void stopSuppressingDlgs() {};

    virtual String formatMessage(const String &in_pszMsg,
                                 const MsgType::Enum& in_Gravity = MsgType::Raw,
                                 const String &in_sArg1 = String::EMPTY,
                                 const String &in_sArg2 = String::EMPTY,
                                 const String &in_sArg3 = String::EMPTY,
                                 const String &in_sArg4 = String::EMPTY,
                                 const String &in_sArg5 = String::EMPTY,
                                 const String &in_sArg6 = String::EMPTY,
                                 const String &in_sArg7 = String::EMPTY,
                                 const String &in_sArg8 = String::EMPTY,
                                 const String &in_sArg9 = String::EMPTY
                                ) const = 0;
    virtual String formatMessageDbg(const String &in_pszMsg, int in_iDbgLv,
                                    const String &in_sArg1 = String::EMPTY,
                                    const String &in_sArg2 = String::EMPTY,
                                    const String &in_sArg3 = String::EMPTY,
                                    const String &in_sArg4 = String::EMPTY,
                                    const String &in_sArg5 = String::EMPTY,
                                    const String &in_sArg6 = String::EMPTY,
                                    const String &in_sArg7 = String::EMPTY,
                                    const String &in_sArg8 = String::EMPTY,
                                    const String &in_sArg9 = String::EMPTY
                                   ) const = 0;

    virtual String formatI18nMessage(const String &in_pszMsgID,
                                     const MsgType::Enum& in_Gravity = MsgType::Raw,
                                     const String &in_sArg1 = String::EMPTY,
                                     const String &in_sArg2 = String::EMPTY,
                                     const String &in_sArg3 = String::EMPTY,
                                     const String &in_sArg4 = String::EMPTY,
                                     const String &in_sArg5 = String::EMPTY,
                                     const String &in_sArg6 = String::EMPTY,
                                     const String &in_sArg7 = String::EMPTY,
                                     const String &in_sArg8 = String::EMPTY,
                                     const String &in_sArg9 = String::EMPTY
                                    ) const = 0;
    virtual String formatI18nMessageDbg(const String &in_pszMsgID, int in_iDbgLv,
                                        const String &in_sArg1 = String::EMPTY,
                                        const String &in_sArg2 = String::EMPTY,
                                        const String &in_sArg3 = String::EMPTY,
                                        const String &in_sArg4 = String::EMPTY,
                                        const String &in_sArg5 = String::EMPTY,
                                        const String &in_sArg6 = String::EMPTY,
                                        const String &in_sArg7 = String::EMPTY,
                                        const String &in_sArg8 = String::EMPTY,
                                        const String &in_sArg9 = String::EMPTY
                                       ) const = 0;
    virtual int message(const String &in_pszMsg, const MsgType::Enum& in_Gravity = MsgType::Raw,
                        const String &in_sArg1 = String::EMPTY,
                        const String &in_sArg2 = String::EMPTY,
                        const String &in_sArg3 = String::EMPTY,
                        const String &in_sArg4 = String::EMPTY,
                        const String &in_sArg5 = String::EMPTY,
                        const String &in_sArg6 = String::EMPTY,
                        const String &in_sArg7 = String::EMPTY,
                        const String &in_sArg8 = String::EMPTY,
                        const String &in_sArg9 = String::EMPTY
                       ) = 0;
    virtual int messageDbg(const String &in_pszMsg, int in_iDbgLv,
                           const String &in_sArg1 = String::EMPTY,
                           const String &in_sArg2 = String::EMPTY,
                           const String &in_sArg3 = String::EMPTY,
                           const String &in_sArg4 = String::EMPTY,
                           const String &in_sArg5 = String::EMPTY,
                           const String &in_sArg6 = String::EMPTY,
                           const String &in_sArg7 = String::EMPTY,
                           const String &in_sArg8 = String::EMPTY,
                           const String &in_sArg9 = String::EMPTY
                          ) = 0;

    virtual int i18nMessage(const String &in_pszMsgID, const MsgType::Enum& in_Gravity = MsgType::Raw,
                            const String &in_sArg1 = String::EMPTY,
                            const String &in_sArg2 = String::EMPTY,
                            const String &in_sArg3 = String::EMPTY,
                            const String &in_sArg4 = String::EMPTY,
                            const String &in_sArg5 = String::EMPTY,
                            const String &in_sArg6 = String::EMPTY,
                            const String &in_sArg7 = String::EMPTY,
                            const String &in_sArg8 = String::EMPTY,
                            const String &in_sArg9 = String::EMPTY
                           ) = 0;
    virtual int i18nMessageDbg(const String &in_pszMsgID, int in_iDbgLv,
                               const String &in_sArg1 = String::EMPTY,
                               const String &in_sArg2 = String::EMPTY,
                               const String &in_sArg3 = String::EMPTY,
                               const String &in_sArg4 = String::EMPTY,
                               const String &in_sArg5 = String::EMPTY,
                               const String &in_sArg6 = String::EMPTY,
                               const String &in_sArg7 = String::EMPTY,
                               const String &in_sArg8 = String::EMPTY,
                               const String &in_sArg9 = String::EMPTY
                              ) = 0;
    virtual int doneConsoleMessage() = 0;
    virtual int failConsoleMessage() = 0;
};

} //namespace FTS

/// Short-hand name. Replaces the macro that had this name before, since macros are evil.
int FTS18N(const FTS::String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Raw, const FTS::String &in_sArg1 = FTS::String::EMPTY, const FTS::String &in_sArg2 = FTS::String::EMPTY, const FTS::String &in_sArg3 = FTS::String::EMPTY, const FTS::String &in_sArg4 = FTS::String::EMPTY, const FTS::String &in_sArg5 = FTS::String::EMPTY, const FTS::String &in_sArg6 = FTS::String::EMPTY, const FTS::String &in_sArg7 = FTS::String::EMPTY, const FTS::String &in_sArg8 = FTS::String::EMPTY, const FTS::String &in_sArg9 = FTS::String::EMPTY);

/// Short-hand name. Replaces the macro that had this name before, since macros are evil.
int FTSMSG(const FTS::String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Raw, const FTS::String &in_sArg1 = FTS::String::EMPTY, const FTS::String &in_sArg2 = FTS::String::EMPTY, const FTS::String &in_sArg3 = FTS::String::EMPTY, const FTS::String &in_sArg4 = FTS::String::EMPTY, const FTS::String &in_sArg5 = FTS::String::EMPTY, const FTS::String &in_sArg6 = FTS::String::EMPTY, const FTS::String &in_sArg7 = FTS::String::EMPTY, const FTS::String &in_sArg8 = FTS::String::EMPTY, const FTS::String &in_sArg9 = FTS::String::EMPTY);

/// Short-hand name. Replaces the macro that had this name before, since macros are evil.
int FTS18NDBG(const FTS::String &in_pszMsgID, int in_iDbgLv, const FTS::String &in_sArg1 = FTS::String::EMPTY, const FTS::String &in_sArg2 = FTS::String::EMPTY, const FTS::String &in_sArg3 = FTS::String::EMPTY, const FTS::String &in_sArg4 = FTS::String::EMPTY, const FTS::String &in_sArg5 = FTS::String::EMPTY, const FTS::String &in_sArg6 = FTS::String::EMPTY, const FTS::String &in_sArg7 = FTS::String::EMPTY, const FTS::String &in_sArg8 = FTS::String::EMPTY, const FTS::String &in_sArg9 = FTS::String::EMPTY);

/// Short-hand name. Replaces the macro that had this name before, since macros are evil.
int FTSMSGDBG(const FTS::String &in_pszMsg, int in_iDbgLv, const FTS::String &in_sArg1 = FTS::String::EMPTY, const FTS::String &in_sArg2 = FTS::String::EMPTY, const FTS::String &in_sArg3 = FTS::String::EMPTY, const FTS::String &in_sArg4 = FTS::String::EMPTY, const FTS::String &in_sArg5 = FTS::String::EMPTY, const FTS::String &in_sArg6 = FTS::String::EMPTY, const FTS::String &in_sArg7 = FTS::String::EMPTY, const FTS::String &in_sArg8 = FTS::String::EMPTY, const FTS::String &in_sArg9 = FTS::String::EMPTY);

namespace FTS {

class BaseLoggerCmd : public CommandBase, public PolymorphicCopyable {
public:
    BaseLoggerCmd() {};
    virtual ~BaseLoggerCmd() {};

    virtual bool exec() = 0;
    virtual bool exec() const = 0;
    virtual const char *getLogString() const = 0;
};

class DummyLoggerCmd : public BaseLoggerCmd {
public:
    DummyLoggerCmd() {};
    virtual ~DummyLoggerCmd() {};

    virtual bool exec() {return true;};
    virtual bool exec() const {return true;};
    virtual const char *getLogString() const {return "";};
    virtual DummyLoggerCmd* copy() const {return new DummyLoggerCmd(); };
};

class LoggerCmd : public BaseLoggerCmd {
protected:
    String m_sMessage;
    MsgType::Enum m_gravity;
    int m_iDbgLv;

public:
    LoggerCmd(String in_sMessage, const MsgType::Enum& in_Gravity)
        : m_sMessage(in_sMessage)
        , m_gravity(in_Gravity)
        , m_iDbgLv(0)
        {};
    LoggerCmd(String in_sMessage, const MsgType::Enum& in_Gravity, int in_iDbgLv)
        : m_sMessage(in_sMessage)
        , m_gravity(in_Gravity)
        , m_iDbgLv(in_iDbgLv)
        {};
    virtual ~LoggerCmd() {};

    bool exec() const {
        if(m_iDbgLv == 0)
            FTSMSG(m_sMessage, m_gravity);
        else
            FTSMSGDBG(m_sMessage, m_iDbgLv);
        return true;
    };

    bool exec() {
        if(m_iDbgLv == 0)
            FTSMSG(m_sMessage, m_gravity);
        else
            FTSMSGDBG(m_sMessage, m_iDbgLv);
        return true;
    };

    const char *getLogString() const {
        return m_sMessage.c_str();
    };

    virtual LoggerCmd* copy() const {
        return new LoggerCmd(m_sMessage, m_gravity, m_iDbgLv);
    };
};

class I18nLoggerCmd : public BaseLoggerCmd {
protected:
    mutable String m_sFormattedBuffer;
    String m_sMessage;
    MsgType::Enum m_gravity;
    int m_iDbgLv;
    String m_sArg1;
    String m_sArg2;
    String m_sArg3;
    String m_sArg4;
    String m_sArg5;
    String m_sArg6;
    String m_sArg7;
    String m_sArg8;
    String m_sArg9;

public:
    I18nLoggerCmd(String in_sMessage, const MsgType::Enum& in_Gravity,
                  const String &in_sArg1 = String::EMPTY,
                  const String &in_sArg2 = String::EMPTY,
                  const String &in_sArg3 = String::EMPTY,
                  const String &in_sArg4 = String::EMPTY,
                  const String &in_sArg5 = String::EMPTY,
                  const String &in_sArg6 = String::EMPTY,
                  const String &in_sArg7 = String::EMPTY,
                  const String &in_sArg8 = String::EMPTY,
                  const String &in_sArg9 = String::EMPTY
                 )
        : m_sMessage(in_sMessage)
        , m_gravity(in_Gravity)
        , m_iDbgLv(0)
        , m_sArg1(in_sArg1)
        , m_sArg2(in_sArg2)
        , m_sArg3(in_sArg3)
        , m_sArg4(in_sArg4)
        , m_sArg5(in_sArg5)
        , m_sArg6(in_sArg6)
        , m_sArg7(in_sArg7)
        , m_sArg8(in_sArg8)
        , m_sArg9(in_sArg9)
        {};
    I18nLoggerCmd(String in_sMessage, const MsgType::Enum& in_Gravity, int in_iDbgLv,
                  const String &in_sArg1 = String::EMPTY,
                  const String &in_sArg2 = String::EMPTY,
                  const String &in_sArg3 = String::EMPTY,
                  const String &in_sArg4 = String::EMPTY,
                  const String &in_sArg5 = String::EMPTY,
                  const String &in_sArg6 = String::EMPTY,
                  const String &in_sArg7 = String::EMPTY,
                  const String &in_sArg8 = String::EMPTY,
                  const String &in_sArg9 = String::EMPTY
                 )
        : m_sMessage(in_sMessage)
        , m_gravity(in_Gravity)
        , m_iDbgLv(in_iDbgLv)
        , m_sArg1(in_sArg1)
        , m_sArg2(in_sArg2)
        , m_sArg3(in_sArg3)
        , m_sArg4(in_sArg4)
        , m_sArg5(in_sArg5)
        , m_sArg6(in_sArg6)
        , m_sArg7(in_sArg7)
        , m_sArg8(in_sArg8)
        , m_sArg9(in_sArg9)
        {};
    virtual ~I18nLoggerCmd() {};

    bool exec() const {
        if(m_iDbgLv == 0)
            FTS18N(m_sMessage, m_gravity, m_sArg1, m_sArg2, m_sArg3, m_sArg4,
                                 m_sArg5, m_sArg6, m_sArg7, m_sArg8, m_sArg9);
        else
            FTS18NDBG(m_sMessage, m_iDbgLv, m_sArg1, m_sArg2, m_sArg3, m_sArg4,
                                   m_sArg5, m_sArg6, m_sArg7, m_sArg8, m_sArg9);
        return true;
    };

    bool exec() {
        if(m_iDbgLv == 0)
            FTS18N(m_sMessage, m_gravity, m_sArg1, m_sArg2, m_sArg3, m_sArg4,
                                 m_sArg5, m_sArg6, m_sArg7, m_sArg8, m_sArg9);
        else
            FTS18NDBG(m_sMessage, m_iDbgLv, m_sArg1, m_sArg2, m_sArg3, m_sArg4,
                                   m_sArg5, m_sArg6, m_sArg7, m_sArg8, m_sArg9);
        return true;
    };

    const char *getLogString() const {
        if(m_iDbgLv) {
            m_sFormattedBuffer = Logger::getSingleton().formatI18nMessageDbg(m_sMessage,
                                  m_iDbgLv, m_sArg1, m_sArg2, m_sArg3, m_sArg4,
                                  m_sArg5, m_sArg6, m_sArg7, m_sArg8, m_sArg9);
        } else {
            m_sFormattedBuffer = Logger::getSingleton().formatI18nMessage(m_sMessage, m_gravity,
                                  m_sArg1, m_sArg2, m_sArg3, m_sArg4,
                                  m_sArg5, m_sArg6, m_sArg7, m_sArg8, m_sArg9);
        }
        return m_sFormattedBuffer.c_str();
    };

    virtual I18nLoggerCmd* copy() const {
        return new I18nLoggerCmd(m_sMessage, m_gravity, m_iDbgLv, m_sArg1, m_sArg2, m_sArg3, m_sArg4, m_sArg5, m_sArg6, m_sArg7, m_sArg8, m_sArg9);
    };
};

} // Namespace FTS;

#endif                          /* FTS_LOGGER_H */
