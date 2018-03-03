#ifndef FTS_INPUT_CONSTANTS_H
#define FTS_INPUT_CONSTANTS_H

#include <SDL_keycode.h>

namespace FTS {

namespace Key {
    enum Enum {
        A = SDL_SCANCODE_A,
        B = SDL_SCANCODE_B,
        C = SDL_SCANCODE_C,
        D = SDL_SCANCODE_D,
        E = SDL_SCANCODE_E,
        F = SDL_SCANCODE_F,
        G = SDL_SCANCODE_G,
        H = SDL_SCANCODE_H,
        I = SDL_SCANCODE_I,
        J = SDL_SCANCODE_J,
        K = SDL_SCANCODE_K,
        L = SDL_SCANCODE_L,
        M = SDL_SCANCODE_M,
        N = SDL_SCANCODE_N,
        O = SDL_SCANCODE_O,
        P = SDL_SCANCODE_P,
        Q = SDL_SCANCODE_Q,
        R = SDL_SCANCODE_R,
        S = SDL_SCANCODE_S,
        T = SDL_SCANCODE_T,
        U = SDL_SCANCODE_U,
        V = SDL_SCANCODE_V,
        W = SDL_SCANCODE_W,
        X = SDL_SCANCODE_X,
        Y = SDL_SCANCODE_Y,
        Z = SDL_SCANCODE_Z,
        Zero  = SDL_SCANCODE_0,
        One   = SDL_SCANCODE_1,
        Two   = SDL_SCANCODE_2,
        Three = SDL_SCANCODE_3,
        Four  = SDL_SCANCODE_4,
        Five  = SDL_SCANCODE_5,
        Six   = SDL_SCANCODE_6,
        Seven = SDL_SCANCODE_7,
        Eight = SDL_SCANCODE_8,
        Nine  = SDL_SCANCODE_9,
        Backspace = SDL_SCANCODE_BACKSPACE,
        Tab       = SDL_SCANCODE_TAB,
        Return    = SDL_SCANCODE_RETURN,
        Pause     = SDL_SCANCODE_PAUSE,
        Escape    = SDL_SCANCODE_ESCAPE,
        Space     = SDL_SCANCODE_SPACE,
        Comma     = SDL_SCANCODE_COMMA,
        Minus     = SDL_SCANCODE_MINUS,
        Period    = SDL_SCANCODE_PERIOD,
        Slash     = SDL_SCANCODE_SLASH,
//TODO No mapping to a scan code, only as Key code. Use temporary the last plus one.
        Colon     = SDL_SCANCODE_AUDIOFASTFORWARD + 1,
        Semicolon = SDL_SCANCODE_SEMICOLON,
        Equals    = SDL_SCANCODE_EQUALS,
        LeftBracket  = SDL_SCANCODE_LEFTBRACKET,
        Backslash    = SDL_SCANCODE_BACKSLASH,
        RightBracket = SDL_SCANCODE_RIGHTBRACKET,
        Delete  = SDL_SCANCODE_DELETE,
        Numpad0 = SDL_SCANCODE_KP_0,
        Numpad1 = SDL_SCANCODE_KP_1,
        Numpad2 = SDL_SCANCODE_KP_2,
        Numpad3 = SDL_SCANCODE_KP_3,
        Numpad4 = SDL_SCANCODE_KP_4,
        Numpad5 = SDL_SCANCODE_KP_5,
        Numpad6 = SDL_SCANCODE_KP_6,
        Numpad7 = SDL_SCANCODE_KP_7,
        Numpad8 = SDL_SCANCODE_KP_8,
        Numpad9 = SDL_SCANCODE_KP_9,
        Decimal = SDL_SCANCODE_KP_PERIOD,
        Divide          = SDL_SCANCODE_KP_DIVIDE,
        Multiply        = SDL_SCANCODE_KP_MULTIPLY,
        Subtract        = SDL_SCANCODE_KP_MINUS,
        Add             = SDL_SCANCODE_KP_PLUS,
        NumpadEnter     = SDL_SCANCODE_KP_ENTER,
        NumpadEquals    = SDL_SCANCODE_KP_EQUALS,
        ArrowUp         = SDL_SCANCODE_UP,
        ArrowDown       = SDL_SCANCODE_DOWN,
        ArrowRight      = SDL_SCANCODE_RIGHT,
        ArrowLeft       = SDL_SCANCODE_LEFT,
        Insert          = SDL_SCANCODE_INSERT,
        Home            = SDL_SCANCODE_HOME,
        End             = SDL_SCANCODE_END,
        PageUp          = SDL_SCANCODE_PAGEUP,
        PageDown        = SDL_SCANCODE_PAGEDOWN,
        F1              = SDL_SCANCODE_F1,
        F2              = SDL_SCANCODE_F2,
        F3              = SDL_SCANCODE_F3,
        F4              = SDL_SCANCODE_F4,
        F5              = SDL_SCANCODE_F5,
        F6              = SDL_SCANCODE_F6,
        F7              = SDL_SCANCODE_F7,
        F8              = SDL_SCANCODE_F8,
        F9              = SDL_SCANCODE_F9,
        F10             = SDL_SCANCODE_F10,
        F11             = SDL_SCANCODE_F11,
        F12             = SDL_SCANCODE_F12,
        F13             = SDL_SCANCODE_F13,
        F14             = SDL_SCANCODE_F14,
        F15             = SDL_SCANCODE_F15,
        NumLock         = SDL_SCANCODE_NUMLOCKCLEAR,
        ScrollLock      = SDL_SCANCODE_SCROLLLOCK,
        RightShift      = SDL_SCANCODE_RSHIFT,
        LeftShift       = SDL_SCANCODE_LSHIFT,
        RightControl    = SDL_SCANCODE_RCTRL,
        LeftControl     = SDL_SCANCODE_LCTRL,
        RightAlt        = SDL_SCANCODE_RALT,
        LeftAlt         = SDL_SCANCODE_LALT,
        LeftWindows     = SDL_SCANCODE_LGUI,
        RightWindows    = SDL_SCANCODE_RGUI,
        SysRq           = SDL_SCANCODE_SYSREQ,
        AppMenu         = SDL_SCANCODE_MENU,
        Power           = SDL_SCANCODE_POWER,
        Last            = SDL_NUM_SCANCODES,
    };
}

/// These are some special keys that are used as meta-keys. That means Alt
/// will be mapped to LeftAlt OR RightAlt etc. Use them to construct new
/// KeyCombo objects.
namespace SpecialKey {
    enum Enum {
        Alt = FTS::Key::Last+1, ///< The right or left alt key.
        Control,                ///< The right or left control key.
        Shift,                  ///< The right or left shift key.
        Enter,                  ///< Any enter key, for example return or keypad enter.
        Any,                    ///< Any key.

        NoSpecial               ///< Nothing.
    };
}

/// The different mouse buttons that may be used.
namespace MouseButton {
    enum Enum {
        Left = SpecialKey::NoSpecial, ///< Left mouse button.
        Middle,                       ///< Middle mouse button.
        Right,                        ///< Right mouse button.
        Fourth,                       ///< Bonus mouse button 1.
        Fifth,                        ///< Bonus mouse button 2.
        Sixth,                        ///< Bonus mouse button 3.
        Seventh,                      ///< Bonus mouse button 4.
        Eighth,                       ///< Bonus mouse button 5.
        Any,                          ///< Any of the mouse buttons.

        NoButton                      ///< Nothing.
    };
}

/// The different mouse scrolling directions that may be used.
namespace MouseScroll {
    enum Enum {
        Up = MouseButton::NoButton, ///< Scrolling up with the main wheel.
        Down,                       ///< Scrolling down with the main wheel.
        Left,                       ///< Scrolling left with the second wheel.
        Right,                      ///< Scrolling right with the second wheel.

        NoScroll                    ///< Nothing.
    };
}


} // namespace FTS.

#endif // FTS_INPUT_CONSTANTS_H
