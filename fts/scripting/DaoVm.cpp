/**
 * \file DaoVm.cpp
 * \author Klaus Beyer
 * \date 16 May 2010
 * \brief This file implements the Dao Virtual Machine interface
 **/
#include "DaoVm.h"
#include "dao.h"

#include "logging/logger.h"
#include "dLib/dString/dPath.h"
#include "DaoFunction.h"

#include <vector>
#include <assert.h>

#ifdef __cplusplus
extern "C"{
#endif
extern int DaoOnLoad( DaoVmSpace *vms, DaoNameSpace *ns );
#ifdef __cplusplus
}
#endif

namespace FTS {

ScriptEvalException::ScriptEvalException() throw()
    : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "ScriptEvalException() being called!"))
{
}

ScriptEvalException::ScriptEvalException(const Path& in_sFileName) throw()
    : LoggableException(new I18nLoggerCmd("ScriptEvalFile", MsgType::Horror, in_sFileName))
{
}

ScriptEvalException::ScriptEvalException(const String& in_sStatement) throw()
    : LoggableException(new I18nLoggerCmd("ScriptEvalStatement", MsgType::Horror, in_sStatement))
{
}

void DaoGetObjectMethod( DaoCData *cd, DaoObject **ob, DaoRoutine **ro, const char *name )
{
  DValue va;
  if( cd == NULL ) return;
  *ob = DaoCData_GetObject( cd );
  if( *ob == NULL ) return;
  va = DaoObject_GetField( *ob, name );
  if( va.t == DAO_ROUTINE ) *ro = va.v.routine;
}

void DaoWrite(DaoUserHandler * self, DString * output)
{
    FTSMSG("{1}", FTS::MsgType::Raw, DString_GetMBS(output));
    DaoVm::getSingleton().addOutput(DString_GetMBS(output));
}

DaoUserHandler handler = {
    NULL, // read
    DaoWrite,
    NULL, // flush
    NULL, // debug
    NULL, // breakpoint
    NULL, // call
    NULL // return
};

DaoVm::DaoVm(void)
: m_script(nullptr)
, m_vmSpace(nullptr)
, m_nameSpace(nullptr)
, m_vmProcess(nullptr)
, m_regExFeature(false)
{
    m_vmSpace = DaoInit();
    assert(m_vmSpace);
    m_nameSpace = DaoVmSpace_MainNameSpace(m_vmSpace);
    assert(m_nameSpace);
    m_vmProcess = DaoVmSpace_MainVmProcess(m_vmSpace);
    assert(m_vmProcess);
    DaoVmSpace_SetUserHandler(m_vmSpace, &handler);
    DaoVmSpace_SetOptions(m_vmSpace, DAO_EXEC_INTERUN);
    DaoNameSpace_SetOptions(m_nameSpace, DAO_NS_AUTO_GLOBAL);
    DaoVmSpace_AddPath(m_vmSpace, Path::getScriptPath().c_str());
    DaoNameSpace * ns = DaoVmSpace_GetNameSpace(m_vmSpace, "dao");
    DaoOnLoad(m_vmSpace, ns);
    m_script = DString_New(1);
    loadInternalScripts();
}

DaoVm::~DaoVm(void)
{
    DString_Delete(m_script);
    DaoQuit();
}

void DaoVm::loadInternalScripts()
{
    try {
        loadScript(Path("regex.dao"));
        execute();
        m_regExFeature = true;
    } catch( const NotExistException& ex ) {
        FTSMSG( ex.what() , MsgType::WarningNoMB );
    }
}
void DaoVm::loadScript(const FTS::Path& file)
{
    assert(file != NULL && !file.empty());
    String fullFileName = Path::getScriptPath() + file;
    File::Ptr fraw = File::open(fullFileName, File::Read);
    size_t size = static_cast<size_t>(fraw->getSize());
    std::vector<char> daoSource(size+1);
    fraw->readNoEndian(&daoSource[0], size);
    daoSource.back() = 0;
    // Prepare the Dao source codes:
    DString_SetMBS(m_script, &daoSource[0]);
}

bool DaoVm::execute()
{
    assert(m_script != nullptr);
    // Execute the Dao scripts:
    // Since the wrapped functions and types are imported into
    // namespace ns, it is need to access the wrapped functions and types
    // in the Dao scripts when it is executed:
    int res = DaoVmProcess_Eval(m_vmProcess, m_nameSpace, m_script, 1);
    return res > 0 ;
}

void DaoVm::execute(const FTS::Path& file)
{
    loadScript(file);
    if( !execute() ) {
        throw ScriptEvalException(file);
    }
}

void DaoVm::execute(const FTS::String& statement)
{
    // Prepare the Dao source codes:
    DString_SetMBS(m_script, statement.c_str());

    if( needsPrintln(statement) ) {
        DString_InsertMBS( m_script, "io.println(", 0, 0, 0 );
        DString_AppendMBS( m_script, ")" );
    }

    if( !execute() ) {
        throw ScriptEvalException(statement);
    }
}

void DaoVm::execRoutine(const char * name, DValue** parameter, int N)
{
    DValue value = DaoNameSpace_FindData(m_nameSpace, name );
    if( value.t == DAO_ROUTINE ){
        DaoVmProcess_Call( m_vmProcess, value.v.routine, NULL, parameter, N );
        return;
    } else {
        for( auto i = m_nsStack.rbegin() ; i != m_nsStack.rend() ; ++i ) {
            DValue valueTop = DaoNameSpace_FindData( *i, name);
            if( valueTop.t == DAO_ROUTINE ){
                DaoVmProcess_Call( m_vmProcess, valueTop.v.routine, NULL, parameter, N );
                return ;
            }
        }
    }
    FTSMSG("DaoVm::execRoutine {1} not found.\n", FTS::MsgType::Raw, name);
}

void DaoVm::callObjectMethod(DaoCData * cdata, const char * routine)
{
    DaoObject *_ob = NULL;
    DaoRoutine *_ro = NULL;

    DaoGetObjectMethod(cdata , & _ob, & _ro, routine );
    if( DaoVmProcess_Call( m_vmProcess, _ro, _ob, NULL, 0 ) ==0 )
        FTSMSG("{1} failed.\n", FTS::MsgType::Raw, routine);
}

DValue DaoVm::getReturn()
{
    return DaoVmProcess_GetReturned( m_vmProcess );
}

bool DaoVm::containsName(const char * name)
{
    DValue value = DaoNameSpace_FindData( m_nameSpace, name);
    if( value.t == DAO_NIL ) {
        for( auto i = m_nsStack.rbegin() ; i != m_nsStack.rend() ; ++i ) {
            value = DaoNameSpace_FindData( *i, name);
            if( value.t != DAO_NIL ) return true;
        }
    }
    return value.t != DAO_NIL ? true : false;
}

bool DaoVm::needsPrintln(const FTS::String& statement)
{
    if( !m_regExFeature )
        return false;

    DaoFunctionCall<int> regex("needPrintln");
    int rc = regex(statement);
    return rc == 1 ;
}

void DaoVm::pushContext()
{
    m_nsStack.push_back(m_nameSpace);
    m_vmpStack.push(m_vmProcess);
    m_nameSpace = DaoNameSpace_New(m_vmSpace);
    DaoNameSpace_SetOptions(m_nameSpace, DAO_NS_AUTO_GLOBAL);
    DaoNameSpace_AddParent(m_nameSpace, m_nsStack.back());
    m_vmProcess = DaoVmSpace_AcquireProcess(m_vmSpace);

}

void DaoVm::popContext()
{
    if (m_nsStack.empty())
    {
        throw new InvalidCallException("DaoVm::popContext: no previous context. Missing pushContext() call.");
    }
    DaoVmSpace_ReleaseProcess(m_vmSpace, m_vmProcess);
    m_nameSpace = m_nsStack.back();
    m_nsStack.pop_back();
    //m_nsStack.pop();
    m_vmProcess = m_vmpStack.top();
    m_vmpStack.pop();

}

}