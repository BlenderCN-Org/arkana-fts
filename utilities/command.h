/**
 * \file command.h
 * \author Pompei2
 * \date 21 Nov 2008
 * \brief This file contains the basic command classes for the Command design pattern.
 **/

#ifndef FTS_COMMAND_H
#define FTS_COMMAND_H


#ifndef D_NOCEGUI
#  include <CEGUIEvent.h>
#  include <CEGUIEventArgs.h>
#endif

namespace FTS {

/// This is the base class of any command whatsoever.
class CommandBase {
protected:
    CommandBase() {};
    CommandBase(const CommandBase &) {};

public:
    virtual ~CommandBase() {};

    /// Any command shall return true if it executed successfully and false if not.
    virtual bool exec() = 0;
};

/// This is the base class of any command that is also undoable.
class UndoableCommandBase : public CommandBase {
protected:
    UndoableCommandBase() {};
    UndoableCommandBase(const UndoableCommandBase &) {};

public:
    virtual ~UndoableCommandBase() {};

    virtual bool exec() = 0;
    virtual bool unexec() = 0;
};

/// This class represents a conditional command, that is a command that only
/// executes if a condition is met. The condition is just another command.
class ConditionalCommand : public CommandBase {
protected:
    CommandBase *m_pCond = nullptr;    ///< The condition command.
    CommandBase *m_pCommand = nullptr; ///< The execution command.

    /// Copy constructor blocked because of mem-free issues upon copy.
    ConditionalCommand(const ConditionalCommand &) = delete;

public:
    ConditionalCommand(CommandBase *in_pCond, CommandBase *in_pCommand);
    virtual ~ConditionalCommand();

    bool exec() override;
};

/// This is a command class that calls a callback method or function upon execution.
class CallbackCommand : public CommandBase {
public:
    /// The type of function that may be called back.
    using CallBackFunction = bool (*)(void *);

protected:
#ifndef D_NOCEGUI
    CEGUI::Event::Subscriber m_subs;
    CEGUI::EventArgs m_ea;
#endif

    CallbackCommand::CallBackFunction m_pfn = nullptr;
    void *m_pArg = nullptr;

public:
#ifndef D_NOCEGUI
    CallbackCommand(const CEGUI::Event::Subscriber &in_subs);
    CallbackCommand(const CEGUI::Event::Subscriber &in_subs, CEGUI::EventArgs in_ea);
#endif
    CallbackCommand(CallBackFunction in_pfn);
    CallbackCommand(CallBackFunction in_pfn, void *in_pArg);
    CallbackCommand(const CallbackCommand &o);
    CallbackCommand& operator=(const CallbackCommand& o);
    
    /// Destructor.
    virtual ~CallbackCommand() {};

    bool exec() override;
};

} // namespace FTS

#endif                          /* FTS_COMMAND_H */

/* EOF */
