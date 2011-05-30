#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<dao.h>
#include "Music.h"
#include "hotkey.h"
#include "input/InputConstants.h"
#include "sound/SndGrp.h"

#ifndef DAO_SND_STATIC
#ifndef DAO_DLL_SND
#define DAO_DLL_SND DAO_DLL_EXPORT
#endif
#else
#define DAO_DLL_SND
#endif

extern DaoVmSpace *__daoVmSpace;

#ifdef __cplusplus
extern "C"{
#endif

extern DaoTypeBase *dao_Hotkey_Typer;
extern DaoTypeBase *dao_IHotkey_Typer;
extern DaoTypeBase *dao_Music_Typer;
#ifdef __cplusplus
}
#endif



Hotkey* DAO_DLL_SND Dao_Hotkey_New( int k, IHotkey* obj );
Hotkey* DAO_DLL_SND Dao_Hotkey_New( int k, IHotkey* obj, const char* function );
Hotkey* DAO_DLL_SND Dao_Hotkey_New( int k, const char* function );

class DAO_DLL_SND DaoCxxVirt_IHotkey 
{
	public:
	DaoCxxVirt_IHotkey(){ self = 0; cdata = 0; }
	void DaoInitWrapper( IHotkey *self, DaoCData *d );
	IHotkey *self;
	DaoCData *cdata;
	int exec(  );

};
class DAO_DLL_SND DaoCxx_IHotkey : public IHotkey, public DaoCxxVirt_IHotkey
{ 
	public:
	~DaoCxx_IHotkey();
	void DaoInitWrapper();
	int exec(  );
};
DaoCxx_IHotkey* DAO_DLL_SND DaoCxx_IHotkey_New(  );


Music* DAO_DLL_SND Dao_Music_New( const char* fileName );
