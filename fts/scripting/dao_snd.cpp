#include"dao_snd.h"

DAO_INIT_MODULE;
DaoVmSpace *__daoVmSpace = NULL;

#ifdef __cplusplus
extern "C"{
#endif
using namespace FTS;
static DaoNumItem constNumbers[] =
{

  { "false", DAO_INTEGER, 0 },
  { "true", DAO_INTEGER, 1 },
  { NULL, 0, 0 }
};
static DaoNumItem dao_Key_Nums[] =
{
  { "A", DAO_INTEGER, Key::A },
  { "Add", DAO_INTEGER, Key::Add },
  { "AppMenu", DAO_INTEGER, Key::AppMenu },
  { "ArrowDown", DAO_INTEGER, Key::ArrowDown },
  { "ArrowLeft", DAO_INTEGER, Key::ArrowLeft },
  { "ArrowRight", DAO_INTEGER, Key::ArrowRight },
  { "ArrowUp", DAO_INTEGER, Key::ArrowUp },
  { "B", DAO_INTEGER, Key::B },
  { "Backslash", DAO_INTEGER, Key::Backslash },
  { "Backspace", DAO_INTEGER, Key::Backspace },
  { "C", DAO_INTEGER, Key::C },
  { "Colon", DAO_INTEGER, Key::Colon },
  { "Comma", DAO_INTEGER, Key::Comma },
  { "D", DAO_INTEGER, Key::D },
  { "Decimal", DAO_INTEGER, Key::Decimal },
  { "Delete", DAO_INTEGER, Key::Delete },
  { "Divide", DAO_INTEGER, Key::Divide },
  { "E", DAO_INTEGER, Key::E },
  { "Eight", DAO_INTEGER, Key::Eight },
  { "End", DAO_INTEGER, Key::End },
  { "Equals", DAO_INTEGER, Key::Equals },
  { "Escape", DAO_INTEGER, Key::Escape },
  { "F", DAO_INTEGER, Key::F },
  { "F1", DAO_INTEGER, Key::F1 },
  { "F10", DAO_INTEGER, Key::F10 },
  { "F11", DAO_INTEGER, Key::F11 },
  { "F12", DAO_INTEGER, Key::F12 },
  { "F13", DAO_INTEGER, Key::F13 },
  { "F14", DAO_INTEGER, Key::F14 },
  { "F15", DAO_INTEGER, Key::F15 },
  { "F2", DAO_INTEGER, Key::F2 },
  { "F3", DAO_INTEGER, Key::F3 },
  { "F4", DAO_INTEGER, Key::F4 },
  { "F5", DAO_INTEGER, Key::F5 },
  { "F6", DAO_INTEGER, Key::F6 },
  { "F7", DAO_INTEGER, Key::F7 },
  { "F8", DAO_INTEGER, Key::F8 },
  { "F9", DAO_INTEGER, Key::F9 },
  { "Five", DAO_INTEGER, Key::Five },
  { "Four", DAO_INTEGER, Key::Four },
  { "G", DAO_INTEGER, Key::G },
  { "H", DAO_INTEGER, Key::H },
  { "Home", DAO_INTEGER, Key::Home },
  { "I", DAO_INTEGER, Key::I },
  { "Insert", DAO_INTEGER, Key::Insert },
  { "J", DAO_INTEGER, Key::J },
  { "K", DAO_INTEGER, Key::K },
  { "L", DAO_INTEGER, Key::L },
  { "Last", DAO_INTEGER, Key::Last },
  { "LeftAlt", DAO_INTEGER, Key::LeftAlt },
  { "LeftBracket", DAO_INTEGER, Key::LeftBracket },
  { "LeftControl", DAO_INTEGER, Key::LeftControl },
  { "LeftShift", DAO_INTEGER, Key::LeftShift },
  { "LeftWindows", DAO_INTEGER, Key::LeftWindows },
  { "M", DAO_INTEGER, Key::M },
  { "Minus", DAO_INTEGER, Key::Minus },
  { "Multiply", DAO_INTEGER, Key::Multiply },
  { "N", DAO_INTEGER, Key::N },
  { "Nine", DAO_INTEGER, Key::Nine },
  { "NumLock", DAO_INTEGER, Key::NumLock },
  { "Numpad0", DAO_INTEGER, Key::Numpad0 },
  { "Numpad1", DAO_INTEGER, Key::Numpad1 },
  { "Numpad2", DAO_INTEGER, Key::Numpad2 },
  { "Numpad3", DAO_INTEGER, Key::Numpad3 },
  { "Numpad4", DAO_INTEGER, Key::Numpad4 },
  { "Numpad5", DAO_INTEGER, Key::Numpad5 },
  { "Numpad6", DAO_INTEGER, Key::Numpad6 },
  { "Numpad7", DAO_INTEGER, Key::Numpad7 },
  { "Numpad8", DAO_INTEGER, Key::Numpad8 },
  { "Numpad9", DAO_INTEGER, Key::Numpad9 },
  { "NumpadEnter", DAO_INTEGER, Key::NumpadEnter },
  { "NumpadEquals", DAO_INTEGER, Key::NumpadEquals },
  { "O", DAO_INTEGER, Key::O },
  { "One", DAO_INTEGER, Key::One },
  { "P", DAO_INTEGER, Key::P },
  { "PageDown", DAO_INTEGER, Key::PageDown },
  { "PageUp", DAO_INTEGER, Key::PageUp },
  { "Pause", DAO_INTEGER, Key::Pause },
  { "Period", DAO_INTEGER, Key::Period },
  { "Power", DAO_INTEGER, Key::Power },
  { "Q", DAO_INTEGER, Key::Q },
  { "R", DAO_INTEGER, Key::R },
  { "Return", DAO_INTEGER, Key::Return },
  { "RightAlt", DAO_INTEGER, Key::RightAlt },
  { "RightBracket", DAO_INTEGER, Key::RightBracket },
  { "RightControl", DAO_INTEGER, Key::RightControl },
  { "RightShift", DAO_INTEGER, Key::RightShift },
  { "RightWindows", DAO_INTEGER, Key::RightWindows },
  { "S", DAO_INTEGER, Key::S },
  { "ScrollLock", DAO_INTEGER, Key::ScrollLock },
  { "Semicolon", DAO_INTEGER, Key::Semicolon },
  { "Seven", DAO_INTEGER, Key::Seven },
  { "Six", DAO_INTEGER, Key::Six },
  { "Slash", DAO_INTEGER, Key::Slash },
  { "Space", DAO_INTEGER, Key::Space },
  { "Subtract", DAO_INTEGER, Key::Subtract },
  { "SysRq", DAO_INTEGER, Key::SysRq },
  { "T", DAO_INTEGER, Key::T },
  { "Tab", DAO_INTEGER, Key::Tab },
  { "Three", DAO_INTEGER, Key::Three },
  { "Two", DAO_INTEGER, Key::Two },
  { "U", DAO_INTEGER, Key::U },
  { "V", DAO_INTEGER, Key::V },
  { "W", DAO_INTEGER, Key::W },
  { "X", DAO_INTEGER, Key::X },
  { "Y", DAO_INTEGER, Key::Y },
  { "Z", DAO_INTEGER, Key::Z },
  { "Zero", DAO_INTEGER, Key::Zero },
  { NULL, 0, 0 }
};
static DaoNumItem dao_MouseButton_Nums[] =
{
  { "Any", DAO_INTEGER, MouseButton::Any },
  { "Eighth", DAO_INTEGER, MouseButton::Eighth },
  { "Fifth", DAO_INTEGER, MouseButton::Fifth },
  { "Fourth", DAO_INTEGER, MouseButton::Fourth },
  { "Left", DAO_INTEGER, MouseButton::Left },
  { "Middle", DAO_INTEGER, MouseButton::Middle },
  { "NoButton", DAO_INTEGER, MouseButton::NoButton },
  { "Right", DAO_INTEGER, MouseButton::Right },
  { "Seventh", DAO_INTEGER, MouseButton::Seventh },
  { "Sixth", DAO_INTEGER, MouseButton::Sixth },
  { NULL, 0, 0 }
};
static DaoNumItem dao_MouseScroll_Nums[] =
{
  { "Down", DAO_INTEGER, MouseScroll::Down },
  { "Left", DAO_INTEGER, MouseScroll::Left },
  { "NoScroll", DAO_INTEGER, MouseScroll::NoScroll },
  { "Right", DAO_INTEGER, MouseScroll::Right },
  { "Up", DAO_INTEGER, MouseScroll::Up },
  { NULL, 0, 0 }
};
static DaoNumItem dao_SpecialKey_Nums[] =
{
  { "Alt", DAO_INTEGER, SpecialKey::Alt },
  { "Any", DAO_INTEGER, SpecialKey::Any },
  { "Control", DAO_INTEGER, SpecialKey::Control },
  { "Enter", DAO_INTEGER, SpecialKey::Enter },
  { "NoSpecial", DAO_INTEGER, SpecialKey::NoSpecial },
  { "Shift", DAO_INTEGER, SpecialKey::Shift },
  { NULL, 0, 0 }
};
static void dao__isKeyPressed( DaoContext *_ctx, DValue *_p[], int _n );
static void dao__isMousePressed( DaoContext *_ctx, DValue *_p[], int _n );
static void dao__mouseX( DaoContext *_ctx, DValue *_p[], int _n );
static void dao__mouseY( DaoContext *_ctx, DValue *_p[], int _n );

static DaoFuncItem dao_Funcs[] =
{
  { dao__isKeyPressed, "isKeyPressed( k : int )=>int" },
  { dao__isMousePressed, "isMousePressed( k : int )=>int" },
  { dao__mouseX, "mouseX(  )=>int" },
  { dao__mouseY, "mouseY(  )=>int" },
  { NULL, NULL }
};
/* hotkey.h */
static void dao__isKeyPressed( DaoContext *_ctx, DValue *_p[], int _n )
{
  int k= (int) _p[0]->v.i;
  if( k < Key::A || k >= SpecialKey::NoSpecial) {
      DaoContext_RaiseException( _ctx, DAO_ERROR_VALUE, "Invalid Key const");
      return;
  }

  bool _isKeyPressed = isKeyPressed( k );
  DaoContext_PutInteger( _ctx, (int) _isKeyPressed );
}
/* hotkey.h */
static void dao__isMousePressed( DaoContext *_ctx, DValue *_p[], int _n )
{
  int k= (int) _p[0]->v.i;
  if( k < MouseButton::Left || k >= MouseButton::NoButton) {
      DaoContext_RaiseException( _ctx, DAO_ERROR_VALUE, "Invalid Mouse button const");
      return;
  }

  bool _isMousePressed = isMousePressed( k );
  DaoContext_PutInteger( _ctx, (int) _isMousePressed );
}
/* hotkey.h */
static void dao__mouseX( DaoContext *_ctx, DValue *_p[], int _n )
{

  unsigned int _mouseX = mouseX(  );
  DaoContext_PutInteger( _ctx, (int) _mouseX );
}
/* hotkey.h */
static void dao__mouseY( DaoContext *_ctx, DValue *_p[], int _n )
{

  unsigned int _mouseY = mouseY(  );
  DaoContext_PutInteger( _ctx, (int) _mouseY );
}

int DaoOnLoad( DaoVmSpace *vms, DaoNameSpace *ns )
{
  DaoNameSpace *ns2;
  DaoTypeBase *typers[4];
  const char *aliases[1];
  __daoVmSpace = vms;
  typers[0] = dao_Hotkey_Typer,
  typers[1] = dao_IHotkey_Typer,
  typers[2] = dao_Music_Typer,
  typers[3] = NULL;
  aliases[0] = NULL;
  ns2 = DaoNameSpace_GetNameSpace( ns, "Key" );
  DaoNameSpace_TypeDefine( ns2, "int", "Enum" );
  ns2 = DaoNameSpace_GetNameSpace( ns, "MouseButton" );
  DaoNameSpace_TypeDefine( ns2, "int", "Enum" );
  ns2 = DaoNameSpace_GetNameSpace( ns, "MouseScroll" );
  DaoNameSpace_TypeDefine( ns2, "int", "Enum" );
  ns2 = DaoNameSpace_GetNameSpace( ns, "SpecialKey" );
  DaoNameSpace_TypeDefine( ns2, "int", "Enum" );
  DaoNameSpace_AddConstNumbers( ns, constNumbers );
  DaoNameSpace_WrapTypes( ns, typers );
  ns2 = DaoNameSpace_GetNameSpace( ns, "FTS" );
  ns2 = DaoNameSpace_GetNameSpace( ns, "Key" );
  DaoNameSpace_AddConstNumbers( ns2, dao_Key_Nums );
  ns2 = DaoNameSpace_GetNameSpace( ns, "MouseButton" );
  DaoNameSpace_AddConstNumbers( ns2, dao_MouseButton_Nums );
  ns2 = DaoNameSpace_GetNameSpace( ns, "MouseScroll" );
  DaoNameSpace_AddConstNumbers( ns2, dao_MouseScroll_Nums );
  ns2 = DaoNameSpace_GetNameSpace( ns, "SpecialKey" );
  DaoNameSpace_AddConstNumbers( ns2, dao_SpecialKey_Nums );
  DaoNameSpace_WrapFunctions( ns, dao_Funcs );
  return 0;
}
#ifdef __cplusplus
}
#endif

