#include "main.h"
#include "toolcompat.h"
#include <thread>

#if WINDOOF
#  include <time.h>
#else
#  include <sys/time.h>
#endif

using namespace FTSTools;
using namespace FTS;

/// Own logger for the tools
MinimalLogger::MinimalLogger(int in_iDbgLv)
    : m_iDbgLv(in_iDbgLv)
    , m_bMuted(false)
{
}

MinimalLogger::~MinimalLogger()
{
}

String MinimalLogger::formatMessage(const String &in_pszMsg,
                                    const FTS::MsgType::Enum& in_Gravity,
                                    const String &in_sArg1,
                                    const String &in_sArg2,
                                    const String &in_sArg3,
                                    const String &in_sArg4,
                                    const String &in_sArg5,
                                    const String &in_sArg6,
                                    const String &in_sArg7,
                                    const String &in_sArg8,
                                    const String &in_sArg9
                                   ) const
{
    return in_pszMsg.fmt(in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                         in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

String MinimalLogger::formatMessageDbg(const String &in_pszMsg, int in_iDbgLv,
                                       const String &in_sArg1,
                                       const String &in_sArg2,
                                       const String &in_sArg3,
                                       const String &in_sArg4,
                                       const String &in_sArg5,
                                       const String &in_sArg6,
                                       const String &in_sArg7,
                                       const String &in_sArg8,
                                       const String &in_sArg9
                                      ) const
{
    return in_pszMsg.fmt(in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                         in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

String MinimalLogger::formatI18nMessage(const String &in_pszMsgID,
                                        const FTS::MsgType::Enum& in_Gravity,
                                        const String &in_sArg1,
                                        const String &in_sArg2,
                                        const String &in_sArg3,
                                        const String &in_sArg4,
                                        const String &in_sArg5,
                                        const String &in_sArg6,
                                        const String &in_sArg7,
                                        const String &in_sArg8,
                                        const String &in_sArg9
                                       ) const
{
    return "Untranslated message: " + in_pszMsgID + " with args "
                    + in_sArg1 + " " + in_sArg2 + " " + in_sArg3 + " "
                    + in_sArg4 + " " + in_sArg5 + " " + in_sArg6 + " "
                    + in_sArg7 + " " + in_sArg8 + " " + in_sArg9;
}

String MinimalLogger::formatI18nMessageDbg(const String &in_pszMsgID, int in_iDbgLv,
                                           const String &in_sArg1,
                                           const String &in_sArg2,
                                           const String &in_sArg3,
                                           const String &in_sArg4,
                                           const String &in_sArg5,
                                           const String &in_sArg6,
                                           const String &in_sArg7,
                                           const String &in_sArg8,
                                           const String &in_sArg9
                                          ) const
{
    return "Untranslated message: " + in_pszMsgID + " with args "
                    + in_sArg1 + " " + in_sArg2 + " " + in_sArg3 + " "
                    + in_sArg4 + " " + in_sArg5 + " " + in_sArg6 + " "
                    + in_sArg7 + " " + in_sArg8 + " " + in_sArg9;
}

int MinimalLogger::message(const String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity,
                           const String &in_sArg1, const String &in_sArg2,
                           const String &in_sArg3, const String &in_sArg4,
                           const String &in_sArg5, const String &in_sArg6,
                           const String &in_sArg7, const String &in_sArg8,
                           const String &in_sArg9)
{
    String sRes = this->formatMessage(in_pszMsg, in_Gravity, in_sArg1, in_sArg2,
                                       in_sArg3, in_sArg4, in_sArg5, in_sArg6,
                                       in_sArg7, in_sArg8, in_sArg9);
    if(in_Gravity != FTS::MsgType::Raw)
        sRes += "\n";

    if(!m_bMuted) {
        fprintf(stderr, "%s", sRes.c_str());
        fflush(stderr);
    }

    return ERR_OK;
}

int MinimalLogger::messageDbg(const String &in_pszMsg, int in_iDbgLv,
                   const String &in_sArg1,
                   const String &in_sArg2,
                   const String &in_sArg3,
                   const String &in_sArg4,
                   const String &in_sArg5,
                   const String &in_sArg6,
                   const String &in_sArg7,
                   const String &in_sArg8,
                   const String &in_sArg9)
{
#if defined(DEBUG)
    if(in_iDbgLv > m_iDbgLv)
        return ERR_OK;

    String sRes = this->formatMessageDbg(in_pszMsg, in_iDbgLv, in_sArg1, in_sArg2,
                                          in_sArg3, in_sArg4, in_sArg5, in_sArg6,
                                          in_sArg7, in_sArg8, in_sArg9);

    if(!m_bMuted) {
        fprintf(stderr, "%s\n", sRes.c_str());
        fflush(stderr);
    }
#endif
    return ERR_OK;
}

int MinimalLogger::i18nMessage(const String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity,
                    const String &in_sArg1,
                    const String &in_sArg2,
                    const String &in_sArg3,
                    const String &in_sArg4,
                    const String &in_sArg5,
                    const String &in_sArg6,
                    const String &in_sArg7,
                    const String &in_sArg8,
                    const String &in_sArg9)
{
    String sRes = this->formatI18nMessage(in_pszMsgID, in_Gravity, in_sArg1,
                                           in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                                           in_sArg6, in_sArg7, in_sArg8, in_sArg9);
    if(in_Gravity != FTS::MsgType::Raw)
        sRes += "\n";

    if(!m_bMuted) {
        fprintf(stderr, "%s", sRes.c_str());
        fflush(stderr);
    }

    return ERR_OK;
}

int MinimalLogger::i18nMessageDbg(const String &in_pszMsgID, int in_iDbgLv,
                       const String &in_sArg1,
                       const String &in_sArg2,
                       const String &in_sArg3,
                       const String &in_sArg4,
                       const String &in_sArg5,
                       const String &in_sArg6,
                       const String &in_sArg7,
                       const String &in_sArg8,
                       const String &in_sArg9)
{
#if defined(DEBUG)
    if(in_iDbgLv > m_iDbgLv)
        return ERR_OK;

    String sRes = this->formatI18nMessageDbg(in_pszMsgID, in_iDbgLv, in_sArg1,
                                           in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                                           in_sArg6, in_sArg7, in_sArg8, in_sArg9);

    if(!m_bMuted) {
        fprintf(stderr, "%s\n", sRes.c_str());
        fflush(stderr);
    }
#endif
    return ERR_OK;
}

int MinimalLogger::doneConsoleMessage()
{
    if(!m_bMuted) {
        printf("done\n");
        fflush(stderr);
    }
    return ERR_OK;
}

int MinimalLogger::failConsoleMessage()
{
    if(!m_bMuted) {
        printf("failed\n");
        fflush(stderr);
    }
    return ERR_OK;
}

uint32_t FTS::dGetTicks()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

/// Sleeps the thread for a certain amount of time.
/** This function only returns after a certain amount of milliseconds.
 *
 * \param in_ulMilliseconds The amount of msec to sleep.
 *
 * \author Pompei2
 */
void FTS::dSleep(unsigned long in_ulMilliseconds)
{
    std::this_thread::sleep_for( std::chrono::milliseconds( in_ulMilliseconds ) );
}

void FTSTools::init()
{
    srand((unsigned)time(NULL));
    setlocale(LC_ALL, "C");
}

String FTS::getTranslatedString(const String &in_s, const String &, bool *)
{
    return in_s;
}
