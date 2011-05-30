#ifndef FTS_DEFLOGGER_H
#define FTS_DEFLOGGER_H

#include "main.h"

#include "logging/logger.h"
#include "logging/Chronometer.h"

namespace FTS {

class DefaultLogger : public Logger {
private:
    FILE *m_pLogFile;
    bool m_bLastDbg;
    // GlobalDebugLevelLimit: The level up to wich the debug messages are shown.
    int m_iGDLL;

    String m_sLastMessage;
    MsgType::Enum m_lastMessageType;
    int m_iLastDbgLv;
    uint64_t m_nLastMessageRepeats;
    Chronometer m_lastMessageTime;

    mutable bool m_bIsInTranslation;

    bool m_bCEGUIReady;
    bool m_bSuppressNextDlg;
    bool m_bSuppressOnlyNextDlg;

    bool m_bMute;

    int createWindow(MsgType::Enum in_Gravity, const String &in_sMessage);
    int doMessage(const String &in_sMsg, const MsgType::Enum& in_Gravity, int in_iDbgLv = 0);

protected:
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
                                ) const;
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
                                   ) const;

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
                                    ) const;
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
                                       ) const;

public:
    DefaultLogger();
    virtual ~DefaultLogger();

    void stfu() {m_bMute = true;};
    void sry() {m_bMute = false;};

    int loadConfig();
    int message(const String &in_pszMsg, const MsgType::Enum& in_Gravity = MsgType::Raw,
                const String &in_sArg1 = String::EMPTY,
                const String &in_sArg2 = String::EMPTY,
                const String &in_sArg3 = String::EMPTY,
                const String &in_sArg4 = String::EMPTY,
                const String &in_sArg5 = String::EMPTY,
                const String &in_sArg6 = String::EMPTY,
                const String &in_sArg7 = String::EMPTY,
                const String &in_sArg8 = String::EMPTY,
                const String &in_sArg9 = String::EMPTY
               );
    int messageDbg(const String &in_pszMsg, int in_iDbgLv,
                   const String &in_sArg1 = String::EMPTY,
                   const String &in_sArg2 = String::EMPTY,
                   const String &in_sArg3 = String::EMPTY,
                   const String &in_sArg4 = String::EMPTY,
                   const String &in_sArg5 = String::EMPTY,
                   const String &in_sArg6 = String::EMPTY,
                   const String &in_sArg7 = String::EMPTY,
                   const String &in_sArg8 = String::EMPTY,
                   const String &in_sArg9 = String::EMPTY
                  );

    int i18nMessage(const String &in_pszMsgID, const MsgType::Enum& in_Gravity = MsgType::Raw,
                    const String &in_sArg1 = String::EMPTY,
                    const String &in_sArg2 = String::EMPTY,
                    const String &in_sArg3 = String::EMPTY,
                    const String &in_sArg4 = String::EMPTY,
                    const String &in_sArg5 = String::EMPTY,
                    const String &in_sArg6 = String::EMPTY,
                    const String &in_sArg7 = String::EMPTY,
                    const String &in_sArg8 = String::EMPTY,
                    const String &in_sArg9 = String::EMPTY
                   );
    int i18nMessageDbg(const String &in_pszMsgID, int in_iDbgLv,
                       const String &in_sArg1 = String::EMPTY,
                       const String &in_sArg2 = String::EMPTY,
                       const String &in_sArg3 = String::EMPTY,
                       const String &in_sArg4 = String::EMPTY,
                       const String &in_sArg5 = String::EMPTY,
                       const String &in_sArg6 = String::EMPTY,
                       const String &in_sArg7 = String::EMPTY,
                       const String &in_sArg8 = String::EMPTY,
                       const String &in_sArg9 = String::EMPTY
                      );
    int doneConsoleMessage();
    int failConsoleMessage();

    int setGDLL(int in_iValue);
    int getGDLL();

    inline void noticeCEGUIReady() {m_bCEGUIReady = true;};
    inline void noticeCEGUINoMoreReady() {m_bCEGUIReady = false;};

    void suppressNextDlg() {m_bSuppressNextDlg=m_bSuppressOnlyNextDlg=true;};
    void suppressAllDlgs() {m_bSuppressNextDlg=true;m_bSuppressOnlyNextDlg=false;};
    void stopSuppressingDlgs() {m_bSuppressNextDlg=m_bSuppressOnlyNextDlg=false;};
};

class DefaultLoggerDlg {
private:
    CEGUI::Window *m_pRoot;       ///< A pointer to this dialog's window.

    static uint64_t m_iCurrentID; ///< To allow multiple windows to open.

    CEGUI::String m_sLastActive;  ///< The window that was active before this dialog being created.
    bool m_bLastActiveWasModal;   ///< Whether the last window was modal or not.

    bool cbOk(const CEGUI::EventArgs & in_ea);

public:
    DefaultLoggerDlg(MsgType::Enum in_Gravity, const String &in_sMessage);
    virtual ~DefaultLoggerDlg();
};

} // namespace FTS

#endif                          /* FTS_DEFLOGGER_H */
