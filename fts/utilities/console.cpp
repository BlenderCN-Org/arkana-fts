/**
 * \file console.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements util functions for the console.
 **/

#include "utilities/utilities.h"
#include "ui/ui.h"
#include "dLib/dConf/configuration.h"

using namespace FTS;

/* Win32 Console ... */
#if WINDOOF
enum WIN32_COLOR {
    /*  dark colors     */
    W32_BLACK,
    W32_BLUE,
    W32_GREEN,
    W32_CYAN,
    W32_RED,
    W32_MAGENTA,
    W32_BROWN,
    W32_LIGHTGRAY,
    /*  light colors    */
    W32_DARKGRAY,               /* "light black" */
    W32_LIGHTBLUE,
    W32_LIGHTGREEN,
    W32_LIGHTCYAN,
    W32_LIGHTRED,
    W32_LIGHTMAGENTA,
    W32_YELLOW,
    W32_WHITE
};

WIN32_COLOR DCol2Win32(const D_COLOR in_dCol)
{
    switch (in_dCol) {
    case D_BLACK:
        return W32_BLACK;
    case D_DARKRED:
        return W32_RED;
    case D_DARKGREEN:
        return W32_GREEN;
    case D_DARKYELLOW:
        return W32_YELLOW;
    case D_DARKBLUE:
        return W32_BLUE;
    case D_DARKMAGENTA:
        return W32_MAGENTA;
    case D_DARKCYAN:
        return W32_CYAN;
    case D_LIGHTGRAY:
        return W32_LIGHTGRAY;
    case D_GRAY:
        return W32_DARKGRAY;
    case D_RED:
        return W32_LIGHTRED;
    case D_GREEN:
        return W32_LIGHTGREEN;
    case D_YELLOW:
        return W32_YELLOW;
    case D_BLUE:
        return W32_LIGHTBLUE;
    case D_MAGENTA:
        return W32_LIGHTMAGENTA;
    case D_CYAN:
        return W32_LIGHTCYAN;
    case D_WHITE:
        return W32_WHITE;
    default:
        return W32_WHITE;
    }
}

static WIN32_COLOR __W32FOREGROUND = W32_LIGHTGRAY;
static WIN32_COLOR __W32BACKGROUND = W32_BLACK;

int W32ConsCol(const int in_attr)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), in_attr);
    return ERR_OK;
}

int W32ConsFG(const D_COLOR in_dCol)
{
    __W32FOREGROUND = DCol2Win32(in_dCol);
    W32ConsCol(__W32FOREGROUND | __W32BACKGROUND);
    return ERR_OK;
}

int W32ConsBG(const D_COLOR in_dCol)
{
    __W32BACKGROUND = DCol2Win32(in_dCol);
    W32ConsCol(__W32FOREGROUND | __W32BACKGROUND);
    return ERR_OK;
}

#endif                          /* WINDOOF */

/// Sets the console font attributes.

/**
 * \param in_Action The action to perform (see \c Enums.h ).
 * \param ...       The color to set (see \c Enums.h ), only if needed.
 *
 * \return If sucessfull: ERR_OK
 * \return If failed: an error code <0
 *
 * \note This is system dependant so it's possible that it doesn't work on
 *       UNIX unlike systems other than Windoof. If you have such a system
 *       and know how to do, please feel free to add this feature. \n
 *       Thanks to Matthew J. Glass for the original macros that I've adapted
 *       for use in FTS ! You can find out the original file in
 *       Originals/ansiscrn.h !
 * \author Pompei2
 */
int FTS::ConsAttr(const D_CONSOLEATTRIBUTE in_Action,
                  ... /*D_COLOR in_Color */ )
{
    /* See later.
     * Thanks to Matthew J. Glass for these 3 vars. You can find out the original
     * file in FTSSrcDir/Originals/ansiscrn.h
     * Pompei2
     */
    const char *_atrb_plt[] = {
        "0", "1", "4", "5", "7", "8"
    };

    const char *_fg_plt[] = {
        "0;30", "0;31", "0;32", "0;33", "0;34", "0;35", "0;36", "0;37",
        "1;30", "1;31", "1;32", "1;33", "1;34", "1;35", "1;36", "1;37"
    };

    const char *_bg_plt[] = {
        "40", "41", "42", "43", "44", "45", "46", "47"
    };

    va_list ap;
    D_COLOR in_Color;

    switch (in_Action) {
    case D_CHANGEFG:
        va_start(ap, in_Action);
        in_Color = (D_COLOR) va_arg(ap, int);

        if(in_Color <= D_WHITE)
#if WINDOOF
            W32ConsFG(in_Color);
#else
            printf("%c[%sm", 27, _fg_plt[in_Color]);
#endif
        else
            return -1;
        va_end(ap);
        break;
    case D_CHANGEBG:
        va_start(ap, in_Action);
        in_Color = (D_COLOR) va_arg(ap, int);

        if(in_Color <= D_LIGHTGRAY)
#if WINDOOF
            W32ConsBG(in_Color);
#else
            printf("%c[%sm", 27, _bg_plt[in_Color]);
#endif
        else
            return -2;
        va_end(ap);
        break;
#if WINDOOF
    case D_NORMAL:
        W32ConsFG(D_LIGHTGRAY);
        W32ConsBG(D_BLACK);
    default:
        return ERR_OK;
#else
    default:
        printf("%c[%sm", 27, _atrb_plt[in_Action]);
        break;
#endif
    }

    return ERR_OK;
}

int FTS::ForegroundConsole(const bool bFore)
{
#if WINDOOF
    if(!GUI::getSingletonPtr())
        return ERR_OK;
    Configuration conf ("conf.xml", ArkanaDefaultSettings());
    
    /* Don't do this in fullscreen mode ! */
    if(conf.getBool("Fullscreen"))
        return ERR_OK;

    if(bFore) {
        // The most important parts of this code comes from Joseph M. Newcomer,
        // found at the code project website:
        // http://www.codeproject.com/dialog/consoledialogs.asp?df=100&forumid=728&exp=0&select=95812
        static LPCTSTR temptitle =
            "{98C1C303-2A9E-11d4-9FF5-006067718D04}";
        TCHAR title[512];

        if(GetConsoleTitle(title, sizeof(title) / sizeof(TCHAR)) == 0)
            return -1;

        SetConsoleTitle(temptitle);
        HWND wnd = FindWindow(NULL, temptitle);

        SetConsoleTitle(title);
        SetForegroundWindow(wnd);
    } else {
        HWND wnd = FindWindow(NULL, FTS_WINDOW_TITLE);

        SetForegroundWindow(wnd);
    }
#endif

    return ERR_OK;
}

int FTS::EnableUTF8Console()
{
#if WINDOOF
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    //SetCurrentConsoleFontEx(...);
#endif

    // On linux, this is currently a noop as all modern linuxes use UTF8 by default.
    return ERR_OK;
}

 /* EOF */
