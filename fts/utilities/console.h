/**
* \file console.cpp
* \author Pompei2
* \date unknown (very old)
* \brief This file implements util functions for the console.
**/

#ifndef FTS_CONSOLE_H
#define FTS_CONSOLE_H

namespace FTS {
    class Console
    {
    public:

        /* ==================== */
        /* CONSOLE MANIPULATION */
        /* ==================== */
        enum class COLOR
        {
            BLACK = 0,                /* Dark colors. */
            DARKRED,
            DARKGREEN,
            DARKYELLOW,
            DARKBLUE,
            DARKMAGENTA,
            DARKCYAN,
            LIGHTGRAY,                /* Bright colors. */
            GRAY,
            RED,
            GREEN,
            YELLOW,
            BLUE,
            MAGENTA,
            CYAN,
            WHITE
        };

        enum class ATTRIBUTE
        {
            NORMAL = 0,
            BOLD,
            ULINE,
            BLINK,
            REVERSE,
            INVIS,
            CHANGEBG,
            CHANGEFG
        };

        static int Attr( ATTRIBUTE in_Action, COLOR in_Color = COLOR::WHITE);
        static int Foreground( bool bFore );
        static void EnableUTF8();
        static void Pause();
    };
}

#endif
