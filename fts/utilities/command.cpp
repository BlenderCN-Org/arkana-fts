/**
 * \file command.cpp
 * \author Pompei2
 * \date 21 Nov 2008
 * \brief This file contains implementations of the basic command classes for the Command design pattern.
 **/

#include "utilities/command.h"

#include "ui/ui.h"

using namespace FTS;

ConditionalCommand::ConditionalCommand(CommandBase *in_pCond, CommandBase *in_pCommand)
{
    m_pCond = in_pCond;
    m_pCommand = in_pCommand;
}

ConditionalCommand::~ConditionalCommand()
{
    SAFE_DELETE(m_pCond);
    SAFE_DELETE(m_pCommand);
}

bool ConditionalCommand::exec()
{
    if(m_pCond->exec())
        return m_pCommand->exec();
    return false;
}

#ifndef D_NOCEGUI
/** Builds a CallbackCommand that calls back a method of an object with
 *  an event args object as argument to the method.
 *
 * \param in_subs The method and object that get called back.
 * \param in_ea The arguments passed to the callback method upon callback.
 *
 * \author Pompei2
 **/
CallbackCommand::CallbackCommand(const CEGUI::Event::Subscriber &in_subs, CEGUI::EventArgs in_ea)
    : m_subs(in_subs),
      m_ea(in_ea)
{
    m_pfn = NULL;
    m_pArg = NULL;
}

/** Builds a CallbackCommand that calls back a method of an object.
 *
 * \param in_subs The method and object that get called back.
 *
 * \author Pompei2
 **/
CallbackCommand::CallbackCommand(const CEGUI::Event::Subscriber &in_subs)
    : m_subs(in_subs)
{
    m_pfn = NULL;
    m_pArg = NULL;
}
#endif

/** Builds a CallbackCommand that calls back a function with a pointer
 *  as argument to the function.
 *
 * \param in_pfn A pointer to the function to call back.
 * \param in_pArg The arguments that shall be passed to the function upon callback.
 *
 * \author Pompei2
 **/
CallbackCommand::CallbackCommand(CallBackFunction in_pfn, void *in_pArg)
{
    m_pfn = in_pfn;
    m_pArg = in_pArg;
}

/** Builds a CallbackCommand that calls back a function.
 *
 * \param in_pfn A pointer to the function to call back.
 *
 * \author Pompei2
 **/
CallbackCommand::CallbackCommand(CallBackFunction in_pfn)
{
    m_pfn = in_pfn;
    m_pArg = NULL;
}

/// Copy constructor.
CallbackCommand::CallbackCommand(const CallbackCommand &o)
    : m_subs(o.m_subs),
      m_pfn(o.m_pfn)
{
}

/// Calls the callback function or method with the argumets given in the constructor.
bool CallbackCommand::exec()
{
    if(m_pfn)
        return m_pfn(m_pArg);
#ifndef D_NOCEGUI
    else
        return m_subs(m_ea);
#endif
}

 /* EOF */
