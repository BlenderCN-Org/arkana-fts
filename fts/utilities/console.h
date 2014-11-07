/**
* \file console.cpp
* \author Pompei2
* \date unknown (very old)
* \brief This file implements util functions for the console.
**/

#ifndef FTS_CONSOLE_H
#define FTS_CONSOLE_H

namespace FTS {

    /* ==================== */
    /* CONSOLE MANIPULATION */
    /* ==================== */
    typedef enum
    {
        D_BLACK = 0,                /* Dark colors. */
        D_DARKRED,
        D_DARKGREEN,
        D_DARKYELLOW,
        D_DARKBLUE,
        D_DARKMAGENTA,
        D_DARKCYAN,
        D_LIGHTGRAY,                /* Bright colors. */
        D_GRAY,
        D_RED,
        D_GREEN,
        D_YELLOW,
        D_BLUE,
        D_MAGENTA,
        D_CYAN,
        D_WHITE
    } D_COLOR;

    typedef enum
    {
        D_NORMAL = 0,
        D_BOLD,
        D_ULINE,
        D_BLINK,
        D_REVERSE,
        D_INVIS,
        D_CHANGEBG,
        D_CHANGEFG
    } D_CONSOLEATTRIBUTE;

    int ConsAttr( D_CONSOLEATTRIBUTE in_Action, ... );
    int ForegroundConsole( bool bFore );
    int EnableUTF8Console();
}

#endif
