/**
 * \file exception.cpp
 * \author Pompei2
 * \date 9 December 2008
 * \brief This file defines everything about the fts loading runlevel.
 **/

#include "Exception.h"
#include "logging/logger.h"
#include "dLib/dString/dPath.h"

#ifndef D_NOCEGUI
#  include <CEGUIExceptions.h>
#endif // D_NOCEGUI

#include <errno.h>

using namespace FTS;

FTS::ArkanaException::ArkanaException() throw()
{
}

FTS::ArkanaException::~ArkanaException() throw()
{
}

void FTS::ArkanaException::show() const
{
    this->getLoggerCmd().exec();
}

const char *FTS::ArkanaException::what() const throw()
{
    try {
        return this->getLoggerCmd().getLogString();
    } catch(const std::exception& e) {
        return e.what();
    }
}

FTS::ErrorAlreadyShownException::ErrorAlreadyShownException() throw()
    : m_pCmd(new DummyLoggerCmd)
{
}

FTS::ErrorAlreadyShownException::~ErrorAlreadyShownException() throw()
{
    SAFE_DELETE(m_pCmd);
}

FTS::ErrorAlreadyShownException::ErrorAlreadyShownException(const ErrorAlreadyShownException& in_other)
    : m_pCmd(dynamic_cast<BaseLoggerCmd*>(in_other.m_pCmd->copy()))
{
}

const ErrorAlreadyShownException& FTS::ErrorAlreadyShownException::operator=(const ErrorAlreadyShownException& in_other)
{
    // Protect against self-assignment.
    if(&in_other == this) {
        return *this;
    }

    // Do the deep copy.
    m_pCmd = dynamic_cast<BaseLoggerCmd*>(in_other.m_pCmd->copy());
    return *this;
}

const BaseLoggerCmd& FTS::ErrorAlreadyShownException::getLoggerCmd() const
{
    return *m_pCmd;
}

FTS::LoggableException::LoggableException(BaseLoggerCmd* in_pLogger) throw()
    : m_pLoggerCmd(in_pLogger)
{
}

const BaseLoggerCmd& FTS::LoggableException::getLoggerCmd() const
{
    return *m_pLoggerCmd;
}

FTS::LoggableException::~LoggableException() throw()
{
    SAFE_DELETE(m_pLoggerCmd);
}

FTS::LoggableException::LoggableException(const LoggableException& in_other)
    : m_pLoggerCmd(dynamic_cast<BaseLoggerCmd*>(in_other.m_pLoggerCmd->copy()))
{
}

const LoggableException& FTS::LoggableException::operator=(const LoggableException& in_other)
{
    // Protect against self-assignment.
    if(&in_other == this) {
        return *this;
    }

    // Do the deep copy.
    m_pLoggerCmd = dynamic_cast<BaseLoggerCmd*>(in_other.m_pLoggerCmd->copy());
    return *this;
}

#ifndef D_NOCEGUI
FTS::CEGUIException::CEGUIException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "CEGUIException() being called!"))
{
}

FTS::CEGUIException::CEGUIException(const CEGUI::Exception& in_sEx, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("CEGUI", in_gravity, in_sEx.getMessage()))
{
}
#endif // D_NOCEGUI

FTS::NotExistException::NotExistException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "NotExistException() being called!"))
{
}

FTS::NotExistException::NotExistException(const String& in_sObjectName, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("ObjectNotExist", in_gravity, in_sObjectName))
{
}

FTS::NotExistException::NotExistException(const String& in_sObjectName, const String& in_sContainerName, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("ObjectNotExistIn", in_gravity, in_sObjectName, in_sContainerName))
{
}

FTS::AlreadyExistException::AlreadyExistException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "AlreadyExistException() being called!"))
{
}

FTS::AlreadyExistException::AlreadyExistException(const String& in_sObjectName, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("ObjectAlreadyExist", in_gravity, in_sObjectName))
{
}

FTS::SyscallException::SyscallException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "SyscallException() being called!"))
{
}

FTS::SyscallException::SyscallException(const String& in_sCall, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("SyscallFailure", in_gravity, in_sCall, String::nr(errno), strerror(errno)))
{
}

FTS::InvalidCallException::InvalidCallException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "InvalidCallException() being called!"))
{
}

FTS::InvalidCallException::InvalidCallException(const String& in_sInfo) throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, in_sInfo))
{
}

FTS::NoRightsException::NoRightsException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "NoRightsException() being called!"))
{
}

FTS::NoRightsException::NoRightsException(const String& in_sInfo, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("NoRights", in_gravity, in_sInfo))
{
}

FTS::CorruptDataException::CorruptDataException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "CorruptDataException() being called!"))
{
}

FTS::CorruptDataException::CorruptDataException(const String& in_sData, const String& in_sInfo, const MsgType::Enum& in_gravity) throw()
    : LoggableException(new I18nLoggerCmd("CorruptData", in_gravity, in_sData, in_sInfo))
{
}

FTS::HardwareLimitException::HardwareLimitException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "HardwareLimit() being called!"))
{
}

FTS::HardwareLimitException::HardwareLimitException(const String& in_sInfo, uint64_t in_uiActual, uint64_t in_uiLimit) throw()
    : LoggableException(new I18nLoggerCmd("HardwareLimit", MsgType::Error, in_sInfo, String::nr(in_uiActual), String::nr(in_uiLimit)))
{
}
