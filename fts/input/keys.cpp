#include "input/input.h"

using namespace FTS;

CEGUI::uint FTSKeyToCEGUIKey(Key::Enum in_key)
{
    switch (in_key) {
    case Key::Backspace:
        return CEGUI::Key::Backspace;
    case Key::Tab:
        return CEGUI::Key::Tab;
    case Key::Return:
        return CEGUI::Key::Return;
    //case Key::Pause:
        //return CEGUI::Key::Pause;
    case Key::Escape:
        return CEGUI::Key::Escape;
    case Key::Space:
        return CEGUI::Key::Space;
    case Key::Comma:
        return CEGUI::Key::Comma;
    case Key::Minus:
        return CEGUI::Key::Minus;
    case Key::Period:
        return CEGUI::Key::Period;
    case Key::Slash:
        return CEGUI::Key::Slash;
    case Key::Zero:
        return CEGUI::Key::Zero;
    case Key::One:
        return CEGUI::Key::One;
    case Key::Two:
        return CEGUI::Key::Two;
    case Key::Three:
        return CEGUI::Key::Three;
    case Key::Four:
        return CEGUI::Key::Four;
    case Key::Five:
        return CEGUI::Key::Five;
    case Key::Six:
        return CEGUI::Key::Six;
    case Key::Seven:
        return CEGUI::Key::Seven;
    case Key::Eight:
        return CEGUI::Key::Eight;
    case Key::Nine:
        return CEGUI::Key::Nine;
    case Key::Colon:
        return CEGUI::Key::Colon;
    case Key::Semicolon:
        return CEGUI::Key::Semicolon;
    case Key::Equals:
        return CEGUI::Key::Equals;
    case Key::LeftBracket:
        return CEGUI::Key::LeftBracket;
    case Key::Backslash:
        return CEGUI::Key::Backslash;
    case Key::RightBracket:
        return CEGUI::Key::RightBracket;
    case Key::A:
        return CEGUI::Key::A;
    case Key::B:
        return CEGUI::Key::B;
    case Key::C:
        return CEGUI::Key::C;
    case Key::D:
        return CEGUI::Key::D;
    case Key::E:
        return CEGUI::Key::E;
    case Key::F:
        return CEGUI::Key::F;
    case Key::G:
        return CEGUI::Key::G;
    case Key::H:
        return CEGUI::Key::H;
    case Key::I:
        return CEGUI::Key::I;
    case Key::J:
        return CEGUI::Key::J;
    case Key::K:
        return CEGUI::Key::K;
    case Key::L:
        return CEGUI::Key::L;
    case Key::M:
        return CEGUI::Key::M;
    case Key::N:
        return CEGUI::Key::N;
    case Key::O:
        return CEGUI::Key::O;
    case Key::P:
        return CEGUI::Key::P;
    case Key::Q:
        return CEGUI::Key::Q;
    case Key::R:
        return CEGUI::Key::R;
    case Key::S:
        return CEGUI::Key::S;
    case Key::T:
        return CEGUI::Key::T;
    case Key::U:
        return CEGUI::Key::U;
    case Key::V:
        return CEGUI::Key::V;
    case Key::W:
        return CEGUI::Key::W;
    case Key::X:
        return CEGUI::Key::X;
    case Key::Y:
        return CEGUI::Key::Y;
    case Key::Z:
        return CEGUI::Key::Z;
    case Key::Delete:
        return CEGUI::Key::Delete;
    case Key::Numpad0:
        return CEGUI::Key::Numpad0;
    case Key::Numpad1:
        return CEGUI::Key::Numpad1;
    case Key::Numpad2:
        return CEGUI::Key::Numpad2;
    case Key::Numpad3:
        return CEGUI::Key::Numpad3;
    case Key::Numpad4:
        return CEGUI::Key::Numpad4;
    case Key::Numpad5:
        return CEGUI::Key::Numpad5;
    case Key::Numpad6:
        return CEGUI::Key::Numpad6;
    case Key::Numpad7:
        return CEGUI::Key::Numpad7;
    case Key::Numpad8:
        return CEGUI::Key::Numpad8;
    case Key::Numpad9:
        return CEGUI::Key::Numpad9;
    case Key::Decimal:
        return CEGUI::Key::Decimal;
    case Key::Divide:
        return CEGUI::Key::Divide;
    case Key::Multiply:
        return CEGUI::Key::Multiply;
    case Key::Subtract:
        return CEGUI::Key::Subtract;
    case Key::Add:
        return CEGUI::Key::Add;
    case Key::NumpadEnter:
        return CEGUI::Key::NumpadEnter;
    case Key::NumpadEquals:
        return CEGUI::Key::NumpadEquals;
    case Key::ArrowUp:
        return CEGUI::Key::ArrowUp;
    case Key::ArrowDown:
        return CEGUI::Key::ArrowDown;
    case Key::ArrowRight:
        return CEGUI::Key::ArrowRight;
    case Key::ArrowLeft:
        return CEGUI::Key::ArrowLeft;
    case Key::Insert:
        return CEGUI::Key::Insert;
    case Key::Home:
        return CEGUI::Key::Home;
    case Key::End:
        return CEGUI::Key::End;
    case Key::PageUp:
        return CEGUI::Key::PageUp;
    case Key::PageDown:
        return CEGUI::Key::PageDown;
    case Key::F1:
        return CEGUI::Key::F1;
    case Key::F2:
        return CEGUI::Key::F2;
    case Key::F3:
        return CEGUI::Key::F3;
    case Key::F4:
        return CEGUI::Key::F4;
    case Key::F5:
        return CEGUI::Key::F5;
    case Key::F6:
        return CEGUI::Key::F6;
    case Key::F7:
        return CEGUI::Key::F7;
    case Key::F8:
        return CEGUI::Key::F8;
    case Key::F9:
        return CEGUI::Key::F9;
    case Key::F10:
        return CEGUI::Key::F10;
    case Key::F11:
        return CEGUI::Key::F11;
    case Key::F12:
        return CEGUI::Key::F12;
    case Key::F13:
        return CEGUI::Key::F13;
    case Key::F14:
        return CEGUI::Key::F14;
    case Key::F15:
        return CEGUI::Key::F15;
    //case Key::NumLock:
        //return CEGUI::Key::NumLock;
    //case Key::ScrollLock:
        //return CEGUI::Key::ScrollLock;
    case Key::RightShift:
        return CEGUI::Key::RightShift;
    case Key::LeftShift:
        return CEGUI::Key::LeftShift;
    case Key::RightControl:
        return CEGUI::Key::RightControl;
    case Key::LeftControl:
        return CEGUI::Key::LeftControl;
    case Key::RightAlt:
        return CEGUI::Key::RightAlt;
    case Key::LeftAlt:
        return CEGUI::Key::LeftAlt;
    //case Key::LeftWindows:
        //return CEGUI::Key::LeftWindows;
    //case Key::RightWindows:
        //return CEGUI::Key::RightWindows;
    //case Key::SysReq:
        //return CEGUI::Key::SysRq;
    //case Key::AppMenu:
        //return CEGUI::Key::AppMenu;
    //case Key::Power:
        //return CEGUI::Key::Power;
    default:
        return 0;
    }
    return 0;
}

bool SDLIsMouseScroll(int btn)
{
    return btn == SDL_BUTTON_WHEELUP || btn == SDL_BUTTON_WHEELDOWN;
}

MouseButton::Enum SDLMouseToFTSMouse(int btn)
{
    switch(btn) {
    case SDL_BUTTON_LEFT:
        return MouseButton::Left;
    case SDL_BUTTON_MIDDLE:
        return MouseButton::Middle;
    case SDL_BUTTON_RIGHT:
        return MouseButton::Right;
#ifdef SDL_BUTTON_X1
    case SDL_BUTTON_X1:
        return MouseButton::Fourth;
#endif
#ifdef SDL_BUTTON_X2
    case SDL_BUTTON_X2:
        return MouseButton::Fifth;
#endif
    default:
        return MouseButton::NoButton;
    }
}

MouseScroll::Enum SDLMouseToFTSScroll(int btn)
{
    switch(btn) {
    case SDL_BUTTON_WHEELUP:
        return MouseScroll::Up;
    case SDL_BUTTON_WHEELDOWN:
        return MouseScroll::Down;
    default:
        return MouseScroll::NoScroll;
    }
}

const char *getFTSKeyName(Key::Enum in_Key)
{
    switch (in_Key) {
        /* The alphabet keys. */
    case Key::A:
        return "a";
    case Key::B:
        return "b";
    case Key::C:
        return "c";
    case Key::D:
        return "d";
    case Key::E:
        return "e";
    case Key::F:
        return "f";
    case Key::G:
        return "g";
    case Key::H:
        return "h";
    case Key::I:
        return "i";
    case Key::J:
        return "j";
    case Key::K:
        return "k";
    case Key::L:
        return "l";
    case Key::M:
        return "m";
    case Key::N:
        return "n";
    case Key::O:
        return "o";
    case Key::P:
        return "p";
    case Key::Q:
        return "q";
    case Key::R:
        return "r";
    case Key::S:
        return "s";
    case Key::T:
        return "t";
    case Key::U:
        return "u";
    case Key::V:
        return "v";
    case Key::W:
        return "w";
    case Key::X:
        return "x";
    case Key::Y:
        return "y";
    case Key::Z:
        return "z";
        /* The number keys. */
    case Key::Zero:
        return "0";
    case Key::One:
        return "1";
    case Key::Two:
        return "2";
    case Key::Three:
        return "3";
    case Key::Four:
        return "4";
    case Key::Five:
        return "5";
    case Key::Six:
        return "6";
    case Key::Seven:
        return "7";
    case Key::Eight:
        return "8";
    case Key::Nine:
        return "9";
        /* Keypad. */
    case Key::Numpad0:
        return "keypad 0";
    case Key::Numpad1:
        return "keypad 1";
    case Key::Numpad2:
        return "keypad 2";
    case Key::Numpad3:
        return "keypad 3";
    case Key::Numpad4:
        return "keypad 4";
    case Key::Numpad5:
        return "keypad 5";
    case Key::Numpad6:
        return "keypad 6";
    case Key::Numpad7:
        return "keypad 7";
    case Key::Numpad8:
        return "keypad 8";
    case Key::Numpad9:
        return "keypad 9";
    case Key::Decimal:
        return "keypad period";
    case Key::Divide:
        return "keypad divide";
    case Key::Multiply:
        return "keypad multiply";
    case Key::Minus:
        return "keypad minus";
    case Key::Add:
        return "keypad plus";
    case Key::NumpadEnter:
        return "keypad enter";
        /* The "point signs". */
    case Key::Period:
        return "point";
    case Key::Comma:
        return "comma";
    case Key::Colon:
        return "colon";
    case Key::Semicolon:
        return "semicolon";
    //case Key::Exclamation:
        //return "excalmation mark";
    //case Key::Question:
        //return "interrogation mark";
    case Key::Slash:
        return "slash";
        /* Special signs. */
    case Key::LeftBracket:
        return "left bracket";
    case Key::RightBracket:
        return "right bracket";
    case Key::Backslash:
        return "backslash";
        /* The math signs. */
    case Key::NumpadEquals:
        return "equals to";
    //case Key::Add:
        //return "plus";
    //case Key::Minus:
        //return "minus";
    //case Key::Multiply:
        //return "multiply";
        /* The "Fat" keys (they take much place on the common keyboards). */
    case Key::LeftShift:
        return "left shift";
    case Key::RightShift:
        return "right shift";
    case Key::LeftControl:
        return "left control";
    case Key::RightControl:
        return "right control";
    case Key::LeftAlt:
        return "left alt";
    case Key::RightAlt:
        return "right alt";
    //case Key::LeftSuper:
        //return "left windows";
    //case Key::RightSuper:
        //return "right windows";
    case Key::Space:
        return "space";
    case Key::Return:
        return "return";
    case Key::Tab:
        return "tabulation";
    case Key::Escape:
        return "escape";
        /* The 'Lock' keys. */
    //case Key::Numlock:
        //return "numlock";
    //case Key::Capslock:
        //return "capslock";
    //case Key::Scrollock:
        //return "scrollock";
        /* The function keys. */
    case Key::F1:
        return "function 1";
    case Key::F2:
        return "function 2";
    case Key::F3:
        return "function 3";
    case Key::F4:
        return "function 4";
    case Key::F5:
        return "function 5";
    case Key::F6:
        return "function 6";
    case Key::F7:
        return "function 7";
    case Key::F8:
        return "function 8";
    case Key::F9:
        return "function 9";
    case Key::F10:
        return "function 10";
    case Key::F11:
        return "function 11";
    case Key::F12:
        return "function 12";
    case Key::F13:
        return "function 13";
    case Key::F14:
        return "function 14";
    case Key::F15:
        return "function 15";
        /* The Direction keys. */
    case Key::ArrowUp:
        return "up";
    case Key::ArrowDown:
        return "down";
    case Key::ArrowLeft:
        return "left";
    case Key::ArrowRight:
        return "right";
        /* The Home/End pad */
    case Key::Insert:
        return "insert";
    case Key::Delete:
        return "delete";
    case Key::Home:
        return "home";
    case Key::End:
        return "end";
    case Key::PageUp:
        return "page up";
    case Key::PageDown:
        return "page down";
        /* Others. */
    case Key::Backspace:
        return "backspace";
    //case Key::Clear:
        //return "clear";
    //case Key::Pause:
        //return "pause";
    //case Key::Mode:
        //return "mode shift";
    //case Key::Help:
        //return "help";
    //case Key::Print:
        //return "print-screen";
    //case Key::SysReq:
        //return "SysRq";
    //case Key::Break:
        //return "break";
    //case Key::Menu:
        //return "menu";
    //case Key::Power:
        //return "power";
    default:
        return "Unknown key code";
    }
}

const char *getFTSKeyName(SpecialKey::Enum in_Key)
{
    switch(in_Key) {
    case SpecialKey::Alt: return "left or right Alt key";
    case SpecialKey::Control: return "left or right Ctrl key";
    case SpecialKey::Shift: return "left or right Shift key";
    case SpecialKey::Enter: return "enter key";
    case SpecialKey::Any: return "any key";
    default: return "no special key";
    }
}

const char *getFTSButtonName(MouseButton::Enum in_Button)
{
    switch(in_Button) {
    case MouseButton::Left: return "left mouse button";
    case MouseButton::Middle: return "middle mouse button";
    case MouseButton::Right: return "right mouse button";
    case MouseButton::Fourth: return "fourth mouse button (some special one)";
    case MouseButton::Fifth: return "fifth mouse button (some special one)";
    case MouseButton::Sixth: return "sixth mouse button (some special one)";
    case MouseButton::Seventh: return "seventh mouse button (some special one)";
    case MouseButton::Eighth: return "eighth mouse button (some special one)";
    case MouseButton::Any: return "any mouse button";
    default: return "no mouse button";
    }
}

const char *getFTSButtonName(MouseScroll::Enum in_Button)
{
    switch(in_Button) {
    case MouseScroll::Up: return "mouse scroll up";
    case MouseScroll::Down: return "mouse scroll down";
    case MouseScroll::Left: return "mouse scroll left";
    case MouseScroll::Right: return "mouse scroll right";
    default: return "no mouse scroll";
    }
}

 /* EOF */
