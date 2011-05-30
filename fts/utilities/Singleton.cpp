#include "Singleton.h"
#include "logging/logger.h"

using namespace FTS;

SingletonNotExistException::SingletonNotExistException(const String& in_sClassName) throw()
    : LoggableException(new I18nLoggerCmd("SingletonNotExist", MsgType::Horror, in_sClassName))
{
}

SingletonAlreadyExistException::SingletonAlreadyExistException(const String& in_sClassName) throw()
    : LoggableException(new I18nLoggerCmd("SingletonAlreadyExist", MsgType::Horror, in_sClassName))
{
}
