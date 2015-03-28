#include "server_log.h"
#include "constants.h"
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fstream>

using namespace FTS;
using namespace FTSSrv2;

void FTSSrv2::srvFlush(FILE * pFile)
{
    fprintf(pFile, "\n> ");
    fflush(pFile);
}

FTSSrv2::ServerLogger::ServerLogger( const FTS::String &in_sLogDir, bool in_bVerbose, int in_dbgLvl)
    : m_nPlayers(0)
    , m_nGames(0)
    , m_bDaemon(false)
    , m_bVerbose(in_bVerbose)
    , m_dbgLvl(in_dbgLvl)
{
    m_sLogFile     = tryFile(DSRV_LOGFILE_LOG,   in_sLogDir);
    m_sNetLogFile  = tryFile(DSRV_LOGFILE_NETLOG,in_sLogDir);
    m_sErrFile     = tryFile(DSRV_LOGFILE_ERR,   in_sLogDir);
    m_sPlayersFile = tryFile(DSRV_FILE_NPLAYERS, in_sLogDir);
    m_sGamesFile   = tryFile(DSRV_FILE_NGAMES,   in_sLogDir);

    m_nPlayers = 0;
    m_nGames = 0;
    m_bDaemon = false;
}

FTSSrv2::ServerLogger::~ServerLogger()
{
}

void FTSSrv2::ServerLogger::daemonized()
{
    this->message("Successfully daemonized.\n");
    m_mutex.lock();
    m_bDaemon = true;
    m_mutex.unlock();
}

// Finds a writable path for a file.
FTS::String FTSSrv2::ServerLogger::tryFile(const FTS::String &in_sFilename, const FTS::String &in_sDir) const
{
    FTS::String sDir = in_sDir;

    // Add a trailing slash if needed.
    if(sDir.getCharAt(sDir.len()-1) != '/') {
        sDir += "/";
    }

    FILE *pF = NULL;
    char *pszHome = getenv("HOME");
    FTS::String sFile = sDir + in_sFilename;

    // First try.
    fprintf(stdout, "Trying to access to the file at '%s' ... ", sFile.c_str());
    if(!(pF = fopen(sFile.c_str(), "a+"))) {
        fprintf(stdout, "Failed !\n");
    } else {
        fprintf(stdout, "Success !\n");
        fclose(pF);
        pF = NULL;
        return sFile;
    }

    // Second try.
    sFile = pszHome+String("/")+in_sFilename;
    fprintf(stdout, "Trying to access to the file at '%s' ... ", sFile.c_str());
    if(!(pF = fopen(sFile.c_str(), "a+"))) {
        fprintf(stdout, "Failed !\n");
        fprintf(stdout, "Logging will be disabled.");
        return FTS::String::EMPTY;
    } else {
        fprintf(stdout, "Success !\n");
        fclose(pF);
        pF = NULL;
        return sFile;
    }
}

// Creates a string of the current date.
FTS::String FTSSrv2::ServerLogger::timeString() const
{
    struct tm *newtime;
    time_t long_time;

    time(&long_time);
    newtime = localtime(&long_time);    // What a struggle to just get the time and date.

    return FTS::String::nr(newtime->tm_mday, 2)+"-"+FTS::String::nr(newtime->tm_mon + 1, 2)+"-"+
           FTS::String::nr(newtime->tm_year + 1900, 4)+"_"+FTS::String::nr(newtime->tm_hour, 2)+
           "-"+FTS::String::nr(newtime->tm_min, 2)+"-"+FTS::String::nr(newtime->tm_sec, 2);
}

int FTSSrv2::ServerLogger::logToFile(const FTS::String &in_sMessage, const FTS::String & in_sFile) const
{
    if(in_sFile.empty())
        return ERR_OK;

    FILE *pF = fopen(in_sFile.c_str(), "a+");
    if(pF == NULL) {
        fprintf(stderr, "HORROR: COULD NOT CREATE THE LOGFILE '%s': '%s' !\n", in_sFile.c_str(), strerror(errno));
        return -1;
    }

    fseek(pF, 0, SEEK_SET);
    fseek(pF, 0, SEEK_END);
    long size = ftell(pF);
    if(size > DSRV_MAX_LOGFILE_BYTES) {
        fclose(pF);
        system(("mv " + in_sFile + " " + in_sFile + "." + this->timeString()).c_str());

        pF = fopen(in_sFile.c_str(), "w+");
        if(pF == NULL) {
            fprintf(stderr, "HORROR: COULD NOT CREATE THE LOGFILE '%s': '%s' !\n", in_sFile.c_str(), strerror(errno));
            return -1;
        }
    }

    fprintf(pF, "%s", in_sMessage.c_str());
    SAFE_FCLOSE(pF);
    return ERR_OK;
}

FTS::String FTSSrv2::ServerLogger::formatMessage(const FTS::String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity,
            const FTS::String &in_sArg1,
            const FTS::String &in_sArg2,
            const FTS::String &in_sArg3,
            const FTS::String &in_sArg4,
            const FTS::String &in_sArg5,
            const FTS::String &in_sArg6,
            const FTS::String &in_sArg7,
            const FTS::String &in_sArg8,
            const FTS::String &in_sArg9) const
{
    return this->timeString() + ": " +
                   in_pszMsg.fmt(in_sArg1, in_sArg2, in_sArg3,in_sArg4,in_sArg5,
                                 in_sArg6, in_sArg7, in_sArg8,in_sArg9);

}

FTS::String FTSSrv2::ServerLogger::formatMessageDbg(const FTS::String &in_pszMsg, int in_iDbgLv,
                const FTS::String &in_sArg1,
                const FTS::String &in_sArg2,
                const FTS::String &in_sArg3,
                const FTS::String &in_sArg4,
                const FTS::String &in_sArg5,
                const FTS::String &in_sArg6,
                const FTS::String &in_sArg7,
                const FTS::String &in_sArg8,
                const FTS::String &in_sArg9) const
{
    return this->formatMessage(in_pszMsg, MsgType::Raw, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

FTS::String FTSSrv2::ServerLogger::formatI18nMessage(const FTS::String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity,
                const FTS::String &in_sArg1,
                const FTS::String &in_sArg2,
                const FTS::String &in_sArg3,
                const FTS::String &in_sArg4,
                const FTS::String &in_sArg5,
                const FTS::String &in_sArg6,
                const FTS::String &in_sArg7,
                const FTS::String &in_sArg8,
                const FTS::String &in_sArg9) const
{
    FTS::String sMsg = "Untranslated message \"" + in_pszMsgID + "\" with arguments: ";
    if(!in_sArg1.empty()) sMsg += in_sArg1;
    if(!in_sArg2.empty()) sMsg += in_sArg2;
    if(!in_sArg3.empty()) sMsg += in_sArg3;
    if(!in_sArg4.empty()) sMsg += in_sArg4;
    if(!in_sArg5.empty()) sMsg += in_sArg5;
    if(!in_sArg6.empty()) sMsg += in_sArg6;
    if(!in_sArg7.empty()) sMsg += in_sArg7;
    if(!in_sArg8.empty()) sMsg += in_sArg8;
    if(!in_sArg9.empty()) sMsg += in_sArg9;
    return sMsg;
}

FTS::String FTSSrv2::ServerLogger::formatI18nMessageDbg(const FTS::String &in_pszMsgID, int in_iDbgLv,
                    const FTS::String &in_sArg1,
                    const FTS::String &in_sArg2,
                    const FTS::String &in_sArg3,
                    const FTS::String &in_sArg4,
                    const FTS::String &in_sArg5,
                    const FTS::String &in_sArg6,
                    const FTS::String &in_sArg7,
                    const FTS::String &in_sArg8,
                    const FTS::String &in_sArg9) const
{
    return this->formatI18nMessage(in_pszMsgID, MsgType::Raw, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

int FTSSrv2::ServerLogger::message(const FTS::String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity,
                          const FTS::String &in_sArg1, const FTS::String &in_sArg2,
                          const FTS::String &in_sArg3, const FTS::String &in_sArg4,
                          const FTS::String &in_sArg5, const FTS::String &in_sArg6,
                          const FTS::String &in_sArg7, const FTS::String &in_sArg8,
                          const FTS::String &in_sArg9)
{
    m_mutex.lock();
    FTS::String sLogFile = FTS::String::EMPTY;
    bool bDaemon = m_bDaemon, bShow = m_bVerbose;

    switch(in_Gravity) {
    case MsgType::Warning:
    case MsgType::Error:
    case MsgType::Horror:
        sLogFile = m_sErrFile;
        break;
    case MsgType::WarningNoMB:
    case MsgType::GoodMessage:
    case MsgType::Message:
    case MsgType::MessageNoMB:
        sLogFile = m_sLogFile;
        break;
    case MsgType::Raw:
        bShow = true;
    default:
        sLogFile = FTS::String::EMPTY;
        break;
    }
    m_mutex.unlock();

    FTS::String sMsg = this->formatMessage(in_pszMsg, in_Gravity, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);

    // Write a message to the logfiles if wanted and possible.
    this->logToFile(sMsg + "\n", sLogFile);

    // Log the message if we are verbose unless we are a daemon.
    if(!bDaemon && bShow) {
        fprintf(stderr, "%s", sMsg.c_str());
        srvFlush(stderr);
    }

    return ERR_OK;
}

int FTSSrv2::ServerLogger::messageDbg(const FTS::String &in_pszMsg, int in_iDbgLv,
                             const FTS::String &in_sArg1, const FTS::String &in_sArg2,
                             const FTS::String &in_sArg3, const FTS::String &in_sArg4,
                             const FTS::String &in_sArg5, const FTS::String &in_sArg6,
                             const FTS::String &in_sArg7, const FTS::String &in_sArg8,
                             const FTS::String &in_sArg9)
{
    if( in_iDbgLv > m_dbgLvl )
        return 0;
    return this->message(in_pszMsg, MsgType::Raw,
                         in_sArg1, in_sArg2, in_sArg3, in_sArg4,
                         in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

int FTSSrv2::ServerLogger::i18nMessage(const FTS::String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity,
                              const FTS::String &in_sArg1, const FTS::String &in_sArg2,
                              const FTS::String &in_sArg3, const FTS::String &in_sArg4,
                              const FTS::String &in_sArg5, const FTS::String &in_sArg6,
                              const FTS::String &in_sArg7, const FTS::String &in_sArg8,
                              const FTS::String &in_sArg9)
{
    FTS::String sMsg = this->formatI18nMessage(in_pszMsgID, in_Gravity, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
    return this->message(sMsg, in_Gravity);
}

int FTSSrv2::ServerLogger::i18nMessageDbg(const FTS::String &in_pszMsgID, int in_iDbgLv,
                                 const FTS::String &in_sArg1, const FTS::String &in_sArg2,
                                 const FTS::String &in_sArg3, const FTS::String &in_sArg4,
                                 const FTS::String &in_sArg5, const FTS::String &in_sArg6,
                                 const FTS::String &in_sArg7, const FTS::String &in_sArg8,
                                 const FTS::String &in_sArg9)
{
    return this->i18nMessage(in_pszMsgID, MsgType::Raw,
                             in_sArg1, in_sArg2, in_sArg3, in_sArg4,
                             in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

void FTSSrv2::ServerLogger::netlog(const FTS::String &s)
{
    m_mutex.lock();
    FTS::String sFile = m_sNetLogFile;
#if defined(DEBUG)
    bool bDaemon = m_bDaemon, bVerbose = m_bVerbose;
#endif
    m_mutex.unlock();

    FTS::String sMsg = this->timeString() + ": " + s + "\n";
    this->logToFile(sMsg + "\n", sFile);

#if defined(DEBUG)
    if(bVerbose && !bDaemon) {
        fprintf(stdout, "Netlog: %s", sMsg.c_str());
        srvFlush(stdout);
    }
#endif
}

size_t FTSSrv2::ServerLogger::addPlayer()
{
    // Modify the statistics.
    Lock l(m_nPlayersMutex);

    m_nPlayers++;
    std::ofstream of(m_sPlayersFile.c_str(), std::ios_base::trunc);
    if(of) {
        of << m_nPlayers;
    }

    return m_nPlayers;
}

size_t FTSSrv2::ServerLogger::remPlayer()
{
    // Modify the statistics.
    Lock l(m_nPlayersMutex);
    m_nPlayers--;
    std::ofstream of(m_sPlayersFile.c_str(), std::ios_base::trunc);
    if(of) {
        of << m_nPlayers;
    }

    return m_nPlayers;
}

size_t FTSSrv2::ServerLogger::addGame()
{
    // Modify the statistics.
    Lock l(m_nGamesMutex);
    m_nGames++;
    std::ofstream of(m_sGamesFile.c_str(), std::ios_base::trunc);
    if(of) {
        of << m_nGames;
    }

    return m_nGames;
}

size_t FTSSrv2::ServerLogger::remGame()
{
    // Modify the statistics.
    Lock l(m_nGamesMutex);
    m_nGames--;
    std::ofstream of(m_sGamesFile.c_str(), std::ios_base::trunc);
    if(of) {
        of << m_nGames;
    }

    return m_nGames;
}

void FTSSrv2::ServerLogger::statAddSendPacket( int req )
{
    Lock l( m_mutex );
    ++m_totalPackets[req].first;
}

void FTSSrv2::ServerLogger::statAddRecvPacket( int req )
{
    Lock l( m_mutex );
    ++m_totalPackets[req].second;
}

void FTSSrv2::ServerLogger::clearStats()
{
    Lock l( m_mutex );
    m_totalPackets.clear();
}

