#ifndef D_SRVLOG_H
#  define D_SRVLOG_H

#  include "dLib/dString/dString.h"
#  include "logging/logger.h"
#  include "utilities/threading.h"

#  include <stdio.h>

namespace FTSSrv2 {

void srvFlush(FILE * pFile);

class ServerLogger : public FTS::Logger {
protected:
    /// Protect from copying.
    ServerLogger(const ServerLogger &) {};

    size_t m_nPlayers;         ///< Number of players logged in.
    FTS::Mutex m_nPlayersMutex; ///< Protects the number of players attribute.
    size_t m_nGames;           ///< Number of games currently opened.
    FTS::Mutex m_nGamesMutex;   ///< Protects the number of games attribute.

    FTS::Mutex m_mutex;         ///< Protects from threaded calls.
    FTS::String m_sErrFile;        ///< The file to write error messages to.
    FTS::String m_sLogFile;        ///< The file to write logging messages to.
    FTS::String m_sNetLogFile;     ///< The file to log network traffic to.
    FTS::String m_sPlayersFile;    ///< The file to write the actual player count to.
    FTS::String m_sGamesFile;      ///< The file to write the actual games count to.
    bool m_bDaemon;            ///< Whether I'm started as a daemon or not.
    bool m_bVerbose;           ///< Whether I'm verbose or not.

    FTS::String tryFile(const FTS::String &in_sFilename, const FTS::String &in_sDir) const;
    FTS::String timeString() const;

    int logToFile(const FTS::String &in_sMessage, const FTS::String & in_sFile) const;

public:
    ServerLogger(const FTS::String &in_sLogDir, bool in_bVerbose);
    virtual ~ServerLogger();

    int loadConfig() {return ERR_OK;};
    void daemonized();

    // We _never_ want to shut up here.
    void stfu() {};
    void sry() {};

    FTS::String formatMessage(const FTS::String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Message,
                const FTS::String &in_sArg1 = FTS::String::EMPTY,
                const FTS::String &in_sArg2 = FTS::String::EMPTY,
                const FTS::String &in_sArg3 = FTS::String::EMPTY,
                const FTS::String &in_sArg4 = FTS::String::EMPTY,
                const FTS::String &in_sArg5 = FTS::String::EMPTY,
                const FTS::String &in_sArg6 = FTS::String::EMPTY,
                const FTS::String &in_sArg7 = FTS::String::EMPTY,
                const FTS::String &in_sArg8 = FTS::String::EMPTY,
                const FTS::String &in_sArg9 = FTS::String::EMPTY) const;
    FTS::String formatMessageDbg(const FTS::String &in_pszMsg, int in_iDbgLv,
                   const FTS::String &in_sArg1 = FTS::String::EMPTY,
                   const FTS::String &in_sArg2 = FTS::String::EMPTY,
                   const FTS::String &in_sArg3 = FTS::String::EMPTY,
                   const FTS::String &in_sArg4 = FTS::String::EMPTY,
                   const FTS::String &in_sArg5 = FTS::String::EMPTY,
                   const FTS::String &in_sArg6 = FTS::String::EMPTY,
                   const FTS::String &in_sArg7 = FTS::String::EMPTY,
                   const FTS::String &in_sArg8 = FTS::String::EMPTY,
                   const FTS::String &in_sArg9 = FTS::String::EMPTY) const;
    FTS::String formatI18nMessage(const FTS::String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Message,
                    const FTS::String &in_sArg1 = FTS::String::EMPTY,
                    const FTS::String &in_sArg2 = FTS::String::EMPTY,
                    const FTS::String &in_sArg3 = FTS::String::EMPTY,
                    const FTS::String &in_sArg4 = FTS::String::EMPTY,
                    const FTS::String &in_sArg5 = FTS::String::EMPTY,
                    const FTS::String &in_sArg6 = FTS::String::EMPTY,
                    const FTS::String &in_sArg7 = FTS::String::EMPTY,
                    const FTS::String &in_sArg8 = FTS::String::EMPTY,
                    const FTS::String &in_sArg9 = FTS::String::EMPTY) const;
    FTS::String formatI18nMessageDbg(const FTS::String &in_pszMsgID, int in_iDbgLv,
                       const FTS::String &in_sArg1 = FTS::String::EMPTY,
                       const FTS::String &in_sArg2 = FTS::String::EMPTY,
                       const FTS::String &in_sArg3 = FTS::String::EMPTY,
                       const FTS::String &in_sArg4 = FTS::String::EMPTY,
                       const FTS::String &in_sArg5 = FTS::String::EMPTY,
                       const FTS::String &in_sArg6 = FTS::String::EMPTY,
                       const FTS::String &in_sArg7 = FTS::String::EMPTY,
                       const FTS::String &in_sArg8 = FTS::String::EMPTY,
                       const FTS::String &in_sArg9 = FTS::String::EMPTY) const;

    int message(const FTS::String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Message,
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
    int i18nMessage(const FTS::String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity = FTS::MsgType::Message,
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
    void netlog(const FTS::String &in_sMsg);
    int doneConsoleMessage() {return ERR_OK;};
    int failConsoleMessage() {return ERR_OK;};

    inline FTS::String getLogfilename(void) const {return m_sLogFile;};
    inline FTS::String getErrfilename(void) const {return m_sErrFile;};
    inline FTS::String getNetLogfilename(void) const {return m_sNetLogFile;};
    inline FTS::String getPlayersfilename(void) const {return m_sPlayersFile;};
    inline FTS::String getGamesfilename(void) const {return m_sGamesFile;};

    size_t addPlayer(void);
    size_t remPlayer(void);
    inline size_t getPlayerCount(void) const {return m_nPlayers;};
    size_t addGame(void);
    size_t remGame(void);
    inline size_t getGameCount(void) const {return m_nGames;};

    inline bool getVerbose() const {return m_bVerbose;};
    inline bool setVerbose(bool in_b) {bool bOld = m_bVerbose; m_bVerbose = in_b; return bOld;};
};

} // namespace FTSSrv2

#endif                          /* D_SRVLOG_H */
