#ifndef D_EXCEPTION_H
#define D_EXCEPTION_H

#include <exception>
#include <cstdint>
#include "logging/MsgType.h"
#include "main/defines.h"

/// \TODO Try to remove all these protected default constructors!

#ifndef D_NOCEGUI
namespace CEGUI {
    class Exception;
}
#endif // D_NOCEGUI

namespace FTS {
    class BaseLoggerCmd;
    class String;
    

    namespace Throw {
        enum Enum {
            Not,
            Do,
        };
    }

/// The base-class of any Arkana-related exception. It fully implements the
/// standard template library's exception and adds internationalization in the
/// Arkana-FTS fashion to it. Every Exception MUST be international!\n
/// Please note that an exception should always be an EXCEPTION, not a warning!
/// It has to be something irreparable (at least at the scope of the thrower).
class ArkanaException : public std::exception {
protected:
    virtual const BaseLoggerCmd& getLoggerCmd() const = 0;

public:
    ArkanaException() throw();
    virtual ~ArkanaException() throw();

    const char *what() const throw();
    void show() const;
};

/// Use this kind of exception everytime you encounter an error but the error
/// message has already been written (to log, GUI, ...) before and you thus
/// just want to throw an exception but not want to log any error.
class ErrorAlreadyShownException : public ArkanaException {
protected:
    virtual const BaseLoggerCmd& getLoggerCmd() const;
    BaseLoggerCmd* m_pCmd;

public:
    ErrorAlreadyShownException() throw();
    virtual ~ErrorAlreadyShownException() throw();

    /// \warning When overwriting this class, if for some reason, you implement
    ///          a copy-constructor, this one does not get called automatically!
    ///          You will have to call it explicitly.
    ErrorAlreadyShownException(const ErrorAlreadyShownException& in_other);

    /// \warning When overwriting this class, if for some reason, you implement
    ///          a = operator, this one does not get called automatically!
    ///          You will have to call it explicitly.
    const ErrorAlreadyShownException& operator=(const ErrorAlreadyShownException& in_other);
};

/// This is the base of all loggable exceptions. Note that those exceptions do
/// \b not automatically get logged upon throwal, but they automatically
/// implement the base class's requirements for a nicely logging exception.
class LoggableException : public ArkanaException {
    BaseLoggerCmd* m_pLoggerCmd; ///< The logger command object.

protected:
    const BaseLoggerCmd& getLoggerCmd() const;
    LoggableException(BaseLoggerCmd* in_pLogger) throw();

public:
    virtual ~LoggableException() throw();

    /// \warning When overwriting this class, if for some reason, you implement
    ///          a copy-constructor, this one does not get called automatically!
    ///          You will have to call it explicitly.
    LoggableException(const LoggableException& in_other);

    /// \warning When overwriting this class, if for some reason, you implement
    ///          a = operator, this one does not get called automatically!
    ///          You will have to call it explicitly.
    const LoggableException& operator=(const LoggableException& in_other);
};

#ifndef D_NOCEGUI
class CEGUIException : public virtual LoggableException {
protected:
    CEGUIException() throw();
public:
    /// \param ex The exception we got from CEGUI.
    /// \param in_gravity How severe is the error?
    CEGUIException(const CEGUI::Exception& ex, const MsgType::Enum& in_gravity = MsgType::Error) throw();
};
#endif // D_NOCEGUI

/// This is the base exception for no-exist-kind of exceptions. You can either
/// use it directly or subclass it to provide more detail.
class NotExistException : public virtual LoggableException {
protected:
    NotExistException() throw();
public:
    /// \param in_sObjectName What object does not exist?
    /// \param in_gravity How severe is the error?
    NotExistException(const String& in_sObjectName, const MsgType::Enum& in_gravity = MsgType::Error) throw();
    /// \param in_sObjectName What object does not exist?
    /// \param in_sContainerName And wherein doesn't it exist?
    /// \param in_gravity How severe is the error?
    NotExistException(const String& in_sObjectName, const String& in_sContainerName, const MsgType::Enum& in_gravity = MsgType::Error) throw();
};

/// This is the base exception for already-exist-kind of exceptions. Use this
/// (or subclass it to provide more detail) whenever you have the stupid problem
/// that a unique thing wants to exist twice!
class AlreadyExistException : public virtual LoggableException {
protected:
    AlreadyExistException() throw();
public:
    /// \param in_sObjectName What object does already exist?
    /// \param in_gravity How severe is the error?
    AlreadyExistException(const String& in_sObjectName, const MsgType::Enum& in_gravity = MsgType::Error) throw();
};

/// This is the base exception for already-exist-kind of exceptions. Use this
/// (or subclass it to provide more detail) whenever you have the stupid problem
/// that a unique thing wants to exist twice!
class SyscallException : public virtual LoggableException {
protected:
    SyscallException() throw();
public:
    /// \param in_sCall What syscall is it that we made and why did it result in
    ///                 an error?
    /// \param in_gravity How severe is the error?
    SyscallException(const String& in_sCall, const MsgType::Enum& in_gravity = MsgType::Error) throw();
};

/// This is the base exception for any invalid call to a method. By that, we mean
/// for example (\e really) unexpected NULL pointers, empty strings, ...
/// \warning Only use this for \e programmer errors please, not for possible
/// \e user errors!
class InvalidCallException : public virtual LoggableException {
protected:
    InvalidCallException() throw();
public:
    /// \param in_sInfo What a call has been made and why is it invalid?
    InvalidCallException(const String& in_sInfo) throw();
};

/// This is the base exception for anything that wants to be done but the needed
/// rights are missing.
class NoRightsException : public virtual LoggableException {
protected:
    NoRightsException() throw();
public:
    /// \param in_sInfo What is it that we don't have the rights for, and why?
    /// \param in_gravity How severe is the error?
    NoRightsException(const String& in_sInfo, const MsgType::Enum& in_gravity = MsgType::Error) throw();
};

/// This is the base exception for any error that comes from invalid data, this
/// may be an invalid image, sound, ... or even invalid script or config file.
class CorruptDataException : public virtual LoggableException {
protected:
    CorruptDataException() throw();
public:
    /// \param in_sData This might be either the content of the data (if small)
    ///                 or the name of the file the data originates from.
    /// \param in_sInfo What is wrong with the data?
    /// \param in_gravity How severe is the error?
    CorruptDataException(const String& in_sData, const String& in_sInfo, const MsgType::Enum& in_gravity = MsgType::Error) throw();
};

/// This exception is thrown when there is some hardware limit reached, making
/// the call fail, for example no more memory.
class HardwareLimitException : public virtual LoggableException {
protected:
    HardwareLimitException() throw();
public:
    /// \param in_sInfo What hardware limit are we talking about?
    /// \param in_uiNeeded How much would we need?
    /// \param in_uiLimit And what is the limit that the hardware imposes?
    HardwareLimitException(const String& in_sInfo, std::uint64_t in_uiNeeded, std::uint64_t in_uiLimit) throw();
};


} // namespace FTS

#endif // D_EXCEPTION_H
