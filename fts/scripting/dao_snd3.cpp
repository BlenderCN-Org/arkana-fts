#include"dao_snd.h"

void Dao_Get_Object_Method( DaoCData *cd, DaoObject **ob, DaoRoutine **ro, const char *name )
{
  DValue va;
  if( cd == NULL ) return;
  *ob = DaoCData_GetObject( cd );
  if( *ob == NULL ) return;
  va = DaoObject_GetField( *ob, name );
  if( va.t == DAO_ROUTINE ) *ro = va.v.routine;
}

int Function_00DA2180( DaoRoutine *_ro, DaoObject *_ob )
{
  DValue _res;
  DaoVmProcess *_vmp;
  int _exec= (int)0;
  if( _ro == NULL ) goto EndCall;
  _vmp = DaoVmSpace_AcquireProcess( __daoVmSpace );
  if( DaoVmProcess_Call( _vmp, _ro, _ob, NULL, 0 ) ==0 ) goto EndCall;
  _res = DaoVmProcess_GetReturned( _vmp );
  DaoVmSpace_ReleaseProcess( __daoVmSpace, _vmp );
  if( _res.t == DAO_INTEGER ) _exec= (int) _res.v.i;

EndCall:
  return _exec;
}


Hotkey* DAO_DLL_SND Dao_Hotkey_New( int k, IHotkey* obj )
{
	Hotkey *object = new Hotkey( k, obj );
	return object;
}
Hotkey* DAO_DLL_SND Dao_Hotkey_New( int k, IHotkey* obj, const char* function )
{
	Hotkey *object = new Hotkey( k, obj, function );
	return object;
}
Hotkey* DAO_DLL_SND Dao_Hotkey_New( int k, const char* function )
{
	Hotkey *object = new Hotkey( k, function );
	return object;
}


DaoCxx_IHotkey* DAO_DLL_SND DaoCxx_IHotkey_New(  )
{
	DaoCxx_IHotkey *self = new DaoCxx_IHotkey(  );
	self->DaoInitWrapper();
	return self;
}
void DaoCxxVirt_IHotkey::DaoInitWrapper( IHotkey *s, DaoCData *d )
{
	self = s;
	cdata = d;

}
DaoCxx_IHotkey::~DaoCxx_IHotkey()
{
	if( cdata ){
		DaoCData_SetData( cdata, NULL );
		DaoCData_SetExtReference( cdata, 0 );
	} 
}
void DaoCxx_IHotkey::DaoInitWrapper()
{
	cdata = DaoCData_New( dao_IHotkey_Typer, this );
	DaoCxxVirt_IHotkey::DaoInitWrapper( this, cdata );
}
int DaoCxxVirt_IHotkey::exec(  )
{
  DaoObject *_ob = NULL;
  DaoRoutine *_ro = NULL;
  int _exec= (int)0;
  Dao_Get_Object_Method( cdata, & _ob, & _ro, "exec" );
  if( _ro ==NULL || _ob ==NULL ) return _exec;
  return (int)Function_00DA2180( _ro, _ob );
}
int DaoCxx_IHotkey::exec(  )
{
  return DaoCxxVirt_IHotkey::exec(  );
}


Music* DAO_DLL_SND Dao_Music_New( const char* fileName )
{
	Music *object = new Music( fileName );
	return object;
}

