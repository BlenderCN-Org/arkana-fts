#ifndef FTS_INPUT_CONSTANTS_H
#define FTS_INPUT_CONSTANTS_H

#include <SDL_keysym.h>

namespace FTS {

namespace Key {
    enum Enum {
        A = SDLK_a,
        B = SDLK_b,
        C = SDLK_c,
        D = SDLK_d,
        E = SDLK_e,
        F = SDLK_f,
        G = SDLK_g,
        H = SDLK_h,
        I = SDLK_i,
        J = SDLK_j,
        K = SDLK_k,
        L = SDLK_l,
        M = SDLK_m,
        N = SDLK_n,
        O = SDLK_o,
        P = SDLK_p,
        Q = SDLK_q,
        R = SDLK_r,
        S = SDLK_s,
        T = SDLK_t,
        U = SDLK_u,
        V = SDLK_v,
        W = SDLK_w,
        X = SDLK_x,
        Y = SDLK_y,
        Z = SDLK_z,
        Zero = SDLK_0,
        One = SDLK_1,
        Two = SDLK_2,
        Three = SDLK_3,
        Four = SDLK_4,
        Five = SDLK_5,
        Six = SDLK_6,
        Seven = SDLK_7,
        Eight = SDLK_8,
        Nine = SDLK_9,
        Backspace = SDLK_BACKSPACE,
        Tab = SDLK_TAB,
        Return = SDLK_RETURN,
        Pause = SDLK_PAUSE,
        Escape = SDLK_ESCAPE,
        Space = SDLK_SPACE,
        Comma = SDLK_COMMA,
        Minus = SDLK_MINUS,
        Period = SDLK_PERIOD,
        Slash = SDLK_SLASH,
        Colon = SDLK_COLON,
        Semicolon = SDLK_SEMICOLON,
        Equals = SDLK_EQUALS,
        LeftBracket = SDLK_LEFTBRACKET,
        Backslash = SDLK_BACKSLASH,
        RightBracket = SDLK_RIGHTBRACKET,
        Delete = SDLK_DELETE,
        Numpad0 = SDLK_KP0,
        Numpad1 = SDLK_KP1,
        Numpad2 = SDLK_KP2,
        Numpad3 = SDLK_KP3,
        Numpad4 = SDLK_KP4,
        Numpad5 = SDLK_KP5,
        Numpad6 = SDLK_KP6,
        Numpad7 = SDLK_KP7,
        Numpad8 = SDLK_KP8,
        Numpad9 = SDLK_KP9,
        Decimal = SDLK_KP_PERIOD,
        Divide = SDLK_KP_DIVIDE,
        Multiply = SDLK_KP_MULTIPLY,
        Subtract = SDLK_KP_MINUS,
        Add = SDLK_KP_PLUS,
        NumpadEnter = SDLK_KP_ENTER,
        NumpadEquals = SDLK_KP_EQUALS,
        ArrowUp = SDLK_UP,
        ArrowDown = SDLK_DOWN,
        ArrowRight = SDLK_RIGHT,
        ArrowLeft = SDLK_LEFT,
        Insert = SDLK_INSERT,
        Home = SDLK_HOME,
        End = SDLK_END,
        PageUp = SDLK_PAGEUP,
        PageDown = SDLK_PAGEDOWN,
        F1 = SDLK_F1,
        F2 = SDLK_F2,
        F3 = SDLK_F3,
        F4 = SDLK_F4,
        F5 = SDLK_F5,
        F6 = SDLK_F6,
        F7 = SDLK_F7,
        F8 = SDLK_F8,
        F9 = SDLK_F9,
        F10 = SDLK_F10,
        F11 = SDLK_F11,
        F12 = SDLK_F12,
        F13 = SDLK_F13,
        F14 = SDLK_F14,
        F15 = SDLK_F15,
        NumLock = SDLK_NUMLOCK,
        ScrollLock = SDLK_SCROLLOCK,
        RightShift = SDLK_RSHIFT,
        LeftShift = SDLK_LSHIFT,
        RightControl = SDLK_RCTRL,
        LeftControl = SDLK_LCTRL,
        RightAlt = SDLK_RALT,
        LeftAlt = SDLK_LALT,
        LeftWindows = SDLK_LSUPER,
        RightWindows = SDLK_RSUPER,
        SysRq = SDLK_SYSREQ,
        AppMenu = SDLK_MENU,
        Power = SDLK_POWER,
        Last = SDLK_LAST+1,
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

// void bla(FTS::Key::Enum e);

} // namespace FTS.

#endif // FTS_INPUT_CONSTANTS_H
