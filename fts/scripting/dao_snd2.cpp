#include"dao_snd.h"
using namespace FTS;
#ifdef __cplusplus
extern "C"{
#endif

/*  hotkey.h */


static DaoNumItem dao_Hotkey_Nums[] =
{
  { NULL, 0, 0 }
};
static void dao_Hotkey_Hotkey_dao_3( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Hotkey_Hotkey_dao_2( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Hotkey_Hotkey( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Hotkey_addModifier( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Hotkey_remove( DaoContext *_ctx, DValue *_p[], int _n );

static DaoFuncItem dao_Hotkey_Meths[] =
{
  { dao_Hotkey_Hotkey_dao_3, "Hotkey( k : int, obj : IHotkey )=>Hotkey" },
  { dao_Hotkey_Hotkey_dao_2, "Hotkey( k : int, obj : IHotkey, function : string )=>Hotkey" },
  { dao_Hotkey_Hotkey, "Hotkey( k : int, function : string )=>Hotkey" },
  { dao_Hotkey_addModifier, "addModifier( self : Hotkey, obj : Hotkey )" },
  { dao_Hotkey_remove, "remove( self : Hotkey )" },
  { NULL, NULL }
};
static void Dao_Hotkey_Delete( void *self )
{
	delete (Hotkey*) self;
}
static DaoTypeBase Hotkey_Typer =
{ "Hotkey", NULL, dao_Hotkey_Nums, dao_Hotkey_Meths,
  { 0 }, Dao_Hotkey_Delete, NULL };
DaoTypeBase DAO_DLL_SND *dao_Hotkey_Typer = & Hotkey_Typer;
/* hotkey.h */
static void dao_Hotkey_Hotkey_dao_3( DaoContext *_ctx, DValue *_p[], int _n )
{
  int k= (int) _p[0]->v.i;
  if( k < Key::A || k >= MouseScroll::NoScroll) {
      DaoContext_RaiseException( _ctx, DAO_ERROR_VALUE, "Invalid Key const");
      return;
  }
  IHotkey* obj= (IHotkey*) DaoCData_GetData( _p[1]->v.cdata );
	Hotkey *_self = Dao_Hotkey_New( k, obj );
	DaoContext_PutCData( _ctx, _self, dao_Hotkey_Typer );
}
/* hotkey.h */
static void dao_Hotkey_Hotkey_dao_2( DaoContext *_ctx, DValue *_p[], int _n )
{
  int k= (int) _p[0]->v.i;
  if( k < Key::A || k >= MouseScroll::NoScroll) {
      DaoContext_RaiseException( _ctx, DAO_ERROR_VALUE, "Invalid Key const");
      return;
  }
  IHotkey* obj= (IHotkey*) DaoCData_GetData( _p[1]->v.cdata );
  char* function= (char*) DString_GetMBS( _p[2]->v.s );
	Hotkey *_self = Dao_Hotkey_New( k, obj, function );
	DaoContext_PutCData( _ctx, _self, dao_Hotkey_Typer );
}
/* hotkey.h */
static void dao_Hotkey_Hotkey( DaoContext *_ctx, DValue *_p[], int _n )
{
  int k= (int) _p[0]->v.i;
  if( k < Key::A || k >= MouseScroll::NoScroll) {
      DaoContext_RaiseException( _ctx, DAO_ERROR_VALUE, "Invalid Key const");
      return;
  }
  char* function= (char*) DString_GetMBS( _p[1]->v.s );
	Hotkey *_self = Dao_Hotkey_New( k, function );
	DaoContext_PutCData( _ctx, _self, dao_Hotkey_Typer );
}
/* hotkey.h */
static void dao_Hotkey_addModifier( DaoContext *_ctx, DValue *_p[], int _n )
{
  Hotkey* self= (Hotkey*) DaoCData_GetData( _p[0]->v.cdata );
  Hotkey* obj= (Hotkey*) DaoCData_GetData( _p[1]->v.cdata );
  self->Hotkey::addModifier( obj );
}
/* hotkey.h */
static void dao_Hotkey_remove( DaoContext *_ctx, DValue *_p[], int _n )
{
  Hotkey* self= (Hotkey*) DaoCData_GetData( _p[0]->v.cdata );
  self->Hotkey::remove(  );
}

/*  hotkey.h */


static DaoNumItem dao_IHotkey_Nums[] =
{
  { NULL, 0, 0 }
};
static void dao_IHotkey_IHotkey( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_IHotkey_exec( DaoContext *_ctx, DValue *_p[], int _n );

static DaoFuncItem dao_IHotkey_Meths[] =
{
  { dao_IHotkey_IHotkey, "IHotkey(  )=>IHotkey" },
  { dao_IHotkey_exec, "exec( self : IHotkey )=>int" },
  { NULL, NULL }
};
static void Dao_IHotkey_Delete( void *self )
{
	delete (IHotkey*) self;
}
static DaoTypeBase IHotkey_Typer =
{ "IHotkey", NULL, dao_IHotkey_Nums, dao_IHotkey_Meths,
  { 0 }, Dao_IHotkey_Delete, NULL };
DaoTypeBase DAO_DLL_SND *dao_IHotkey_Typer = & IHotkey_Typer;
static void dao_IHotkey_IHotkey( DaoContext *_ctx, DValue *_p[], int _n )
{
	DaoCxx_IHotkey *self = DaoCxx_IHotkey_New();
	DaoContext_PutResult( _ctx, (DaoBase*) self->cdata );
}
/* hotkey.h */
static void dao_IHotkey_exec( DaoContext *_ctx, DValue *_p[], int _n )
{
  IHotkey* self= (IHotkey*) DaoCData_GetData( _p[0]->v.cdata );
  int _exec = self->exec(  );
  DaoContext_PutInteger( _ctx, (int) _exec );
}

/*  Music.h */


static DaoNumItem dao_Music_Nums[] =
{
  { NULL, 0, 0 }
};
static void dao_Music_Music_ctor( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_Music( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_load( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_play( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_pause( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_resume( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_stop( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_unload( DaoContext *_ctx, DValue *_p[], int _n );
//static void dao_Music_volume( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_SETF_volume( DaoContext *_ctx, DValue *_p[], int _n );
static void dao_Music_SETF_type( DaoContext *_ctx, DValue *_p[], int _n );


static DaoFuncItem dao_Music_Meths[] =
{
    { dao_Music_Music_ctor, "Music( )=>Music" },
    { dao_Music_Music, "Music( fileName : string )=>Music" },
  { dao_Music_load, "load( self : Music, fileName : string )" },
  { dao_Music_play, "play( self : Music )" },
  { dao_Music_pause, "pause( self : Music )" },
  { dao_Music_resume, "resume( self : Music )" },
  { dao_Music_stop, "stop( self : Music )" },
  { dao_Music_unload, "unload( self : Music )" },
//  { dao_Music_volume, "volume( self : Music, v : float )" },
  { dao_Music_SETF_volume, ".volume=( self : Music, v : float )" },
  { dao_Music_SETF_type, ".type=( self : Music, t : int )" },
  { NULL, NULL }
};
static void Dao_Music_Delete( void *self )
{
	delete (Music*) self;
}
static DaoTypeBase Music_Typer =
{ "Music", NULL, dao_Music_Nums, dao_Music_Meths,
  { 0 }, Dao_Music_Delete, NULL };
DaoTypeBase DAO_DLL_SND *dao_Music_Typer = & Music_Typer;
/* Music.h */
static void dao_Music_Music_ctor( DaoContext *_ctx, DValue *_p[], int _n )
{
    DaoContext_RaiseException( _ctx, DAO_ERROR_SYNTAX, "Invalid call to private ctor");
}
static void dao_Music_Music( DaoContext *_ctx, DValue *_p[], int _n )
{
  char* fileName= (char*) DString_GetMBS( _p[0]->v.s );
	Music *_self = Dao_Music_New( fileName );
	DaoContext_PutCData( _ctx, _self, dao_Music_Typer );
}
/* Music.h */
static void dao_Music_load( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  char* fileName= (char*) DString_GetMBS( _p[1]->v.s );
  self->Music::load( fileName );
}
/* Music.h */
static void dao_Music_pause( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  self->Music::pause(  );
}
/* Music.h */
static void dao_Music_play( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  self->Music::play(  );
}
/* Music.h */
static void dao_Music_resume( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  self->Music::resume(  );
}
/* Music.h */
static void dao_Music_stop( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  self->Music::stop(  );
}
/* Music.h */
static void dao_Music_unload( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  self->Music::unload(  );
}
/* Music.h
static void dao_Music_volume( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  float v= (float) _p[1]->v.f;
  self->Music::volume( v );
}
*/
static void dao_Music_SETF_volume( DaoContext *_ctx, DValue *_p[], int _n )
{
  Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
  float v= (float) _p[1]->v.f;
  self->Music::volume( v );
}
static void dao_Music_SETF_type( DaoContext *_ctx, DValue *_p[], int _n )
{
    Music* self= (Music*) DaoCData_GetData( _p[0]->v.cdata );
    int t= (int) _p[1]->v.i;
    if( t < FTS::SndGroup::Music || t > FTS::SndGroup::All) {
        DaoContext_RaiseException( _ctx, DAO_ERROR_VALUE, "Invalid Sound type const");
        return;
    }
    self->Music::setType( t );
}


/*  InputConstants.h */

/*  InputConstants.h */

/*  InputConstants.h */

/*  InputConstants.h */

/*  InputConstants.h */

#ifdef __cplusplus
}
#endif

