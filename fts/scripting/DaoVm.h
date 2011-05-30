/*
 * \file DaoVm.h
 *
 * \author kby (Klaus Beyer)
 * \brief Interface file to the dao scripting language virtual machine.
 * \detail The DaoVm class encapsulate the access to the dao vm. It is
 *         anchor for all dao scripts to be executed in the game. 
 */

#pragma once
#include <stack>
#include <utilities/Singleton.h>
#include "dLib/dString/dPath.h"
#include "dLib/dFile/dFile.h"
#include "dLib/dString/dString.h"
#include "main/Stackable.h"

#include "dao.h"

namespace FTS {

/// This is the exception for errors found in the script. 
class ScriptEvalException : public virtual LoggableException {
protected:
    ScriptEvalException() throw();
public:
    /// \param in_sFileName What a script file has been evaluated?
    ScriptEvalException(const class Path& in_sFileName) throw();
    /// \param in_sStatement What a statement has been evaluated?
    ScriptEvalException(const String& in_sStatement) throw();
};
    
    
class DaoVm : public Singleton<DaoVm>, public Stackable
{
public:
    DaoVm(void) ;
    virtual ~DaoVm(void);
    void execute(const FTS::Path& file);
    void execute(const FTS::String& statement);
    void callObjectMethod(DaoCData * cdata, const char * routine);
    void addOutput(const String& output) {m_daoOutput += output;}
    String getOutput() { return m_daoOutput; }
    void clearOutput() { m_daoOutput = ""; }
    void execRoutine(const char * name, DValue** parameter, int N);
    DValue getReturn();
    bool containsName(const char * name);
    void pushContext();
    void popContext();
protected:
    void loadScript(const FTS::Path& file);
    bool execute();
private:
    DString *m_script;
    DaoVmSpace *m_vmSpace;
    DaoNameSpace *m_nameSpace;
    DaoVmProcess *m_vmProcess;
    String m_daoOutput;
    bool m_regExFeature;
    bool needsPrintln(const String& statement );
    void loadInternalScripts();
    //std::stack<DaoNameSpace*> m_nsStack;
    std::list<DaoNameSpace*> m_nsStack;
    std::stack<DaoVmProcess*> m_vmpStack;
};

};