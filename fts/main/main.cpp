#include "main.h"
#include <CEGUI.h>
#include <SDL.h>
#include <time.h>

#include "ui/ui.h"
#include "logging/ftslogger.h"
#include "game/player.h"
#include "graphic/graphic.h"
#include "utilities/console.h"
#include "main/runlevels.h"
#include "main/load_fts_rlv.h"
#include "main/Clock.h"
#include "main/Updateable.h"
#include "main/version.h" // To write the version into a file.
#include "3d/Renderer.h" // to create/delete the renderer singleton.
#include "3d/Shader.h" // to create/delete the shader manager.
#include "3d/light.h" // To deinit the light system.
#include "graphic/cegui_ftsimg_codec.h" // To deinit the image codec.
#include "sound/fts_Snd.h"
#include "scripting/DaoVm.h"

#include "dLib/dConf/configuration.h"
#include "dLib/dMem/dMem.h"
#include "dLib/dFile/dFile.h"


using namespace FTS;

/* -------------------------- */
/* Local prototypes           */
/* -------------------------- */
void printCmdHelp(char *in_pszAppName);
int enterMainLoop();
int cleanFTS(void);
void exitfunc(void);

int run_tests(int argc, const char* argv[]);

/* -------------------------- */
/* Var's                      */
/* -------------------------- */

String PRINT_FUN =
"       _____________   ____         \n"
"       \\_   _____/  |_/ __         \n"
"        |   ___) \\  __\\____       \n"
"        |  |     |  |   __ \\       \n"
"        \\__|     |__|  ____/ " + String(getFTSVersionString()) + "\n"
"                                    \n"
"                  ###               \n"
"                 #o#o#              \n"
"                 #vvv#              \n"
"                #  v  #             \n"
"       LINUX   ##     ##  RULES !   \n"
"               +###   #+            \n"
"              +++#   #+++           \n"
"               +++###+++            \n"
"                ++   ++             \n"
"                                    \n"
" By:                                \n"
"   - Pompei2 (Pompei2@gmail.com)    \n"
" From:                              \n"
"   - The dProggers                  \n"
"                                    \n";
#if defined(_DEBUG)
String PRINT_FUN_DBG =
"                                    \n"
" DEBUG:                             \n"
" ------                             \n"
"Version " + String(getFTSVersionString()) + "\n";
#else
String PRINT_FUN_DBG = "";
#endif // DEBUG

int main(int argc, char *argv[])
{
    try {
        printf("CWD: %s\n", Path::wd().c_str());

#ifdef SIGPIPE
        signal(SIGPIPE, SIG_IGN); /* Ignore broken pipe */
#endif /* SIGPIPE */

        // In debug mode, always run the tests on startup!
#ifdef DEBUG
        // The -tDaoVm disables the DaoVm tests, as currently the Dao VM cannot be
        // initialized more than one time. This is the fault of Dao, not Arkana.
        const char* test_argv[] = {"./tests", "fts", "-tDaoVm", "-Shaders"};
        int failures = run_tests(sizeof(test_argv)/sizeof(test_argv[0]), test_argv);
        if(failures > 0) {
            Console::Pause();
            return failures;
        }

        if( argc == 2 && std::string(argv[1]) == "run-test-only" ) {
            Console::Pause();
            return 0;
        }
#endif

        if(!FileUtils::dirExists(DATA)) {
            std::cout << "FTS not running in the correct directory: " << std::endl
                      << "no data (" DATA ") directory found !" << std::endl
                      << std::endl
                      << "You are most probably a developer using Visual C++ but " << std::endl
                      << "you forgot to change the project's working directory." << std::endl
                      << std::endl
                      << "You have to set it to '..' (without the quotes)" << std::endl
                      << std::endl
                      << "Press any key to quit ... ";
            std::cin.get();
            return 0;
        }

        // If we do this, we get problems with SDL unicode keys.
        //      setlocale( LC_ALL, globals->pLocalAcc->getOptString( _S("Language"), _S("English") ).c_str( ) );
        setlocale(LC_NUMERIC, "C");
        Console::EnableUTF8();
        std::puts("\x00\xe9");

        srand((unsigned)time(NULL));

        // We need this one ready here already in order to be able to get things
        // like timer, best video mode, ...
        if(-1 == SDL_Init(SDL_INIT_VIDEO)) {
            FTS18N("SDL_Init", MsgType::Error, SDL_GetError());
            return -1;
        }

        // Init the logging system.
        DefaultLogger* pDefLog = new DefaultLogger;

        // We have to make that after glfwInit because glfwInit registers an atexit
        // that destroys the OpenGL context.
        atexit(exitfunc);

        // Counter-intuitively, this does NOT create a GUI. Hehe.
        // It is more a helper-glue-class for GUI-related stuff.
        new GUI();

        // The arguments loop.
        for(int i = 1; i < argc; i++) {
            if(argv[i][0] == '-') {
                switch (argv[i][1]) {
                    // The arg begins with a --
                case '-':
                    // The user needs help
                    if(!strcmp("help", &argv[i][2])) {
                        printCmdHelp(argv[0]);
                        // Set the debug level to the number given in the next arg.
                    } else if(!strcmp("debug", &argv[i][2])) {
                        pDefLog->setGDLL(atoi(argv[++i]));
                    }
                    break;
                    // The user needs help
                case 'h':
                case 'H':
                    printCmdHelp(argv[0]);
                    break;
                    // Set the debug level to the number given in the next arg.
                case 'd':
                    if(argc >= i + 1)
                        pDefLog->setGDLL(atoi(argv[++i]));
                    break;
                }
            }
        }

        // Load the debug level from the logger.
        Logger::getSingleton().loadConfig();

        // Print a lil nice message.
        std::puts(PRINT_FUN.c_str());
        std::puts(PRINT_FUN_DBG.c_str());

        // And get the stone rolling ...
        new RunlevelManager();
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new LoadFTSRlv());

        enterMainLoop();
    } catch(const std::exception& ex) {
        std::cout << "Uncaught exception: " << ex.what() << std::endl;
        std::ofstream fCritLog("CriticalError.txt");
        fCritLog << ex.what();
        Console::Foreground(true);
        return 1;
    }

    return 0;
}

// General FTS deinitialization function
int cleanFTS(void)
{
    Console::Foreground(true);
    FTS18N("Deinit");

    DefaultLogger *pLog = dynamic_cast<DefaultLogger *>(Logger::getSingletonPtr());
    if(pLog)
        pLog->noticeCEGUINoMoreReady();

    // Deinit the runlevelmanager, that also unloads+deletes the current rlv.
    delete RunlevelManager::getSingletonPtr();
    delete ShaderManager::getSingletonPtr();
    delete GraphicManager::getSingletonPtr();
    delete Renderer::getSingletonPtr();

    // Deinit the scripting subsystem last, because the other subsystems above
    // may use clean-up scripts!
    delete DaoVm::getSingletonPtr();

    try {
        if(CEGUI::System::getSingletonPtr()) {
            // Also deinits Image codec.
            delete CEGUI::System::getSingletonPtr()->getRenderer();
            /// \TODO: Debug why here is a pure virtual function call!
            //delete CEGUI::System::getSingletonPtr();
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    SAFE_DELETE(g_pMeHacky);

    FTS18NDBG("GraphDrvU", 1);

    LightSystem::deinit();

    try {
        Logger::getSingleton().doneConsoleMessage();
    } catch(const ArkanaException&) { }

    delete GUI::getSingletonPtr();
    delete ISndSys::getSingletonPtr();

    SDL_Quit();

    delete Logger::getSingletonPtr();

    return ERR_OK;
}

int enterMainLoop()
{
    Runlevel *pRlv = RunlevelManager::getSingleton().getCurrRunlevel();
    SDL_Event sdlEvent;
    bool bCont = true;

    // Enter the initial run level.
    pRlv = RunlevelManager::getSingleton().realEnterRunlevel();

    Clock c;

    while(bCont) {
        while(SDL_PollEvent(&sdlEvent)) {
            if(InputManager::getSingleton().handleEvent(sdlEvent))
                continue;

            // Not handled by the input manager?
            switch (sdlEvent.type) {
            case SDL_WINDOWEVENT:
                break;
            case SDL_QUIT:
                bCont = false;
                break;
            case SDL_SYSWMEVENT:
                break;
            case SDL_USEREVENT:
                break;
            default:
                break;
            }
        }

        // Check for runlevel change and execute a game tick.
        pRlv = RunlevelManager::getSingleton().realEnterRunlevel();

        c.tick();

        // Update everybody who wants that!
        UpdateableManager::getSingleton().doUpdates(c);

        // Set up the camera of that runlevel and then let it render its stuff.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::getSingleton().enter3DMode(pRlv->getActiveCamera());
        pRlv->render3D(c);
        Renderer::getSingleton().enter2DMode(Renderer::getSingleton().getDefault2DCamera());
        pRlv->render2D(c);

        glFinish();
        glFlush();
        SDL_GL_SwapWindow(Renderer::getSingleton().getWindow());
        verifGL("Main");
    }

    return ERR_OK;
}

/* printCmdHelp: Prints the help text.
 *
 * param in_pszAppName: the name of the app (argv[0]).
 *
 * return: type is void
 * Pompei2
 *
 * Comments:
 */
void printCmdHelp(char *in_pszAppName)
{
    std::cout << "usage: " << in_pszAppName << " [options]\n\n";
    std::puts("options:");
    std::puts("\t-h, --help  Prints this help");
    std::puts("\t-d, --debug Sets the debug level to LEVEL(1->5)");
    std::puts("-------------------------------------------------");
    std::puts("This software is distributed under the GNU/GPL license v2 or higher.");
    std::puts("See LICENSE.txt for more details.");
    std::puts("");
    std::puts("Bug report to <pompei2@gmail.com>. For more infos about how to");
    std::puts("report your bug, see README.txt chapter bug report.");

    delete GUI::getSingletonPtr();
    exit(EXIT_SUCCESS);
    return;
}

void exitfunc(void)
{
    cleanFTS();
#ifdef DEBUG
#  if WINDOOF
    // Give the user a chance to see what's up, in debug mode.
    std::cout << "We're done, press any key to quit. This is only shown in debug mode ..." << std::endl;
    Console::Pause();
#  endif /* WINDOOF */
#endif /* DEBUG defined */
}
