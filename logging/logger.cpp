#include "logging/logger.h"

int FTS18N(const FTS::String &in_pszMsgID, const FTS::MsgType::Enum& in_Gravity, const FTS::String &in_sArg1, const FTS::String &in_sArg2, const FTS::String &in_sArg3, const FTS::String &in_sArg4, const FTS::String &in_sArg5, const FTS::String &in_sArg6, const FTS::String &in_sArg7, const FTS::String &in_sArg8, const FTS::String &in_sArg9)
{
    return FTS::Logger::getSingleton().i18nMessage(in_pszMsgID, in_Gravity, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

int FTSMSG(const FTS::String &in_pszMsg, const FTS::MsgType::Enum& in_Gravity, const FTS::String &in_sArg1, const FTS::String &in_sArg2, const FTS::String &in_sArg3, const FTS::String &in_sArg4, const FTS::String &in_sArg5, const FTS::String &in_sArg6, const FTS::String &in_sArg7, const FTS::String &in_sArg8, const FTS::String &in_sArg9)
{
    return FTS::Logger::getSingleton().message(in_pszMsg, in_Gravity, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

int FTS18NDBG(const FTS::String &in_pszMsgID, int in_iDbgLv, const FTS::String &in_sArg1, const FTS::String &in_sArg2, const FTS::String &in_sArg3, const FTS::String &in_sArg4, const FTS::String &in_sArg5, const FTS::String &in_sArg6, const FTS::String &in_sArg7, const FTS::String &in_sArg8, const FTS::String &in_sArg9)
{
    return FTS::Logger::getSingleton().i18nMessageDbg(in_pszMsgID, in_iDbgLv, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

int FTSMSGDBG(const FTS::String &in_pszMsg, int in_iDbgLv, const FTS::String &in_sArg1, const FTS::String &in_sArg2, const FTS::String &in_sArg3, const FTS::String &in_sArg4, const FTS::String &in_sArg5, const FTS::String &in_sArg6, const FTS::String &in_sArg7, const FTS::String &in_sArg8, const FTS::String &in_sArg9)
{
    return FTS::Logger::getSingleton().messageDbg(in_pszMsg, in_iDbgLv, in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}
