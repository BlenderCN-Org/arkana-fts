/**
 * \file console.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements util functions for the console.
 **/

#include "utilities/console.h"
#include "ui/ui.h"
#include "dLib/dConf/configuration.h"

using namespace FTS;

#if WINDOOF

/* Win32 Console ... */
enum class WIN32_COLOR
{
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

WIN32_COLOR DCol2Win32(const FTS::Console::COLOR in_dCol)
{
    using FTS::Console;

    switch (in_dCol) {
        case FTS::Console::COLOR::BLACK:
            return WIN32_COLOR::W32_BLACK;
        case FTS::Console::COLOR::DARKRED:
            return WIN32_COLOR::W32_RED;
        case FTS::Console::COLOR::DARKGREEN:
            return WIN32_COLOR::W32_GREEN;
        case FTS::Console::COLOR::DARKYELLOW:
            return WIN32_COLOR::W32_YELLOW;
        case FTS::Console::COLOR::DARKBLUE:
            return WIN32_COLOR::W32_BLUE;
        case FTS::Console::COLOR::DARKMAGENTA:
            return WIN32_COLOR::W32_MAGENTA;
        case FTS::Console::COLOR::DARKCYAN:
            return WIN32_COLOR::W32_CYAN;
        case FTS::Console::COLOR::LIGHTGRAY:
            return WIN32_COLOR::W32_LIGHTGRAY;
        case FTS::Console::COLOR::GRAY:
            return WIN32_COLOR::W32_DARKGRAY;
        case FTS::Console::COLOR::RED:
            return WIN32_COLOR::W32_LIGHTRED;
        case FTS::Console::COLOR::GREEN:
            return WIN32_COLOR::W32_LIGHTGREEN;
        case FTS::Console::COLOR::YELLOW:
            return WIN32_COLOR::W32_YELLOW;
        case FTS::Console::COLOR::BLUE:
            return WIN32_COLOR::W32_LIGHTBLUE;
        case FTS::Console::COLOR::MAGENTA:
            return WIN32_COLOR::W32_LIGHTMAGENTA;
        case FTS::Console::COLOR::CYAN:
            return WIN32_COLOR::W32_LIGHTCYAN;
        case FTS::Console::COLOR::WHITE:
            return WIN32_COLOR::W32_WHITE;
        default:
            return WIN32_COLOR::W32_WHITE;
    }
}

static WIN32_COLOR __W32FOREGROUND = WIN32_COLOR::W32_LIGHTGRAY;
static WIN32_COLOR __W32BACKGROUND = WIN32_COLOR::W32_BLACK;

int W32ConsCol(const int in_attr)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), in_attr);
    return ERR_OK;
}

int ConsFG( const FTS::Console::COLOR in_dCol )
{
    __W32FOREGROUND = DCol2Win32(in_dCol);
    W32ConsCol((int)__W32FOREGROUND | (int)__W32BACKGROUND);
    return ERR_OK;
}

int ConsBG( const FTS::Console::COLOR in_dCol )
{
    __W32BACKGROUND = DCol2Win32(in_dCol);
    W32ConsCol((int)__W32FOREGROUND | (int)__W32BACKGROUND);
    return ERR_OK;
}

int ConsDefault( const FTS::Console::ATTRIBUTE in_Action )
{
    if ( in_Action == FTS::Console::ATTRIBUTE::NORMAL )
    {
        ConsFG( FTS::Console::COLOR::LIGHTGRAY );
        ConsBG( FTS::Console::COLOR::BLACK );
    }
    return ERR_OK;
}

#else /* WINDOOF */

int ConsFG( const FTS::Console::COLOR in_Color )
{
    const char *_fg_plt[] = {
        "0;30", "0;31", "0;32", "0;33", "0;34", "0;35", "0;36", "0;37",
        "1;30", "1;31", "1;32", "1;33", "1;34", "1;35", "1;36", "1;37"
    };

    printf( "%c[%sm", 27, _fg_plt[(int)in_Color] );

    return ERR_OK;
}

int ConsBG( const FTS::Console::COLOR in_Color )
{
    const char *_bg_plt[] = {
        "40", "41", "42", "43", "44", "45", "46", "47"
    };

    printf( "%c[%sm", 27, _bg_plt[(int)in_Color] );

    return ERR_OK;
}

int ConsDefault(const FTS::Console::ATTRIBUTE in_Action)
{
    const char *_atrb_plt[] = {
        "0", "1", "4", "5", "7", "8"
    };
    printf( "%c[%sm", 27, _atrb_plt[(int)in_Action] );
    return ERR_OK;
}
#endif 

/// Sets the console font attributes.

/**
 * \param in_Action The action to perform (see \c Enums.h ).
 * \param ...       The color to set (see \c Enums.h ), only if needed.
 *
 * \return If successful: ERR_OK
 * \return If failed: an error code <0
 *
 * \note This is system defendant so it's possible that it doesn't work on
 *       UNIX unlike systems other than Windoof. If you have such a system
 *       and know how to do, please feel free to add this feature. \n
 *       Thanks to Matthew J. Glass for the original macros that I've adapted
 *       for use in FTS ! You can find out the original file in
 *       Originals/ansiscrn.h !
 * \author Pompei2
 */
int FTS::Console::Attr( const ATTRIBUTE in_Action, COLOR in_Color )
{
    /* See later.
     * Thanks to Matthew J. Glass for these 3 vars. You can find out the original
     * file in FTSSrcDir/Originals/ansiscrn.h
     * Pompei2
     */

    switch ( in_Action )
    {
        case ATTRIBUTE::CHANGEFG:
            if ( in_Color <= COLOR::WHITE )
                ConsFG( in_Color );
            else
                return -1;
            break;
        case ATTRIBUTE::CHANGEBG:
            if ( in_Color <= COLOR::LIGHTGRAY )
                ConsBG( in_Color );
            else
                return -2;
            break;
        default:
            ConsDefault( in_Action );
            break;
    }

    return ERR_OK;
}

int FTS::Console::Foreground( const bool bFore )
{
#if WINDOOF
    if(!GUI::getSingletonPtr())
        return ERR_OK;

    Configuration conf ("conf.xml", ArkanaDefaultSettings());
    
    /* Don't do this in fullscreen mode ! */
    if(conf.get<bool>("Fullscreen"))
        return ERR_OK;

    if(bFore) {
        // The most important parts of this code comes from Joseph M. Newcomer,
        // found at the code project website:
        // http://www.codeproject.com/dialog/consoledialogs.asp?df=100&forumid=728&exp=0&select=95812
        static LPCTSTR temptitle = "{98C1C303-2A9E-11d4-9FF5-006067718D04}";
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

void FTS::Console::EnableUTF8()
{
#if WINDOOF
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // On linux, this is currently a noop as all modern linuxes use UTF8 by default.
}

void FTS::Console::Pause()
{
#if WINDOOF && defined(DEBUG)
    std::cin.get();
#endif

}


 /* EOF */
