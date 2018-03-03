/**
 * \file load_rlv.cpp
 * \author Pompei2
 * \date 9 December 2008
 * \brief This file defines everything about the fts loading runlevel.
 **/

#include <chrono>
#include <thread>

#include <CEGUI.h>
#include <fts-net.h>

#include "main/load_fts_rlv.h"

#include "main/version.h" // For the version info.
#include "logging/ftslogger.h"
#include "utilities/console.h" 
#include "utilities/fps_calculator.h" // To init the FPSCalculator.
#include "graphic/graphic.h" // To draw the loading picture.
#include "graphic/cegui_ftsimg_codec.h" // To create/init the image codec.
#include "game/player.h" // To create "this" player. TODO FIXME: need this ?
#include "ui/ui.h" // Needed alot.
#include "ui/ui_menu.h" // To enter the main menu runlevel.
#include "ui/ui_commands.h"
#include "ui/cegui_items/progressbar/ArkanaProgressBarWR.h"
#include "ui/cegui_items/ArkanaResourceProvider.h"
#include "3d/3d.h"
#include "3d/Renderer.h"
#include "3d/Shader.h" // To init the shader manager
#include "scripting/DaoVm.h"
#include "sound/fts_Snd.h" // To init the sound system.

#include "dLib/dFile/dFile.h"
#include "dLib/dConf/configuration.h"

#include <openglrenderer.h>
#include <SDL.h>

using namespace FTS;

/// Default constructor.
FTS::LoadFTSRlv::LoadFTSRlv()
{
}

FTS::LoadFTSRlv::~LoadFTSRlv()
{
}

/** This method will be called during the loading of the runlevel.\n
 *  It does not actually load FTS, but it does load the loading runlevel.
 *  That means it loads everything that is needed to have the first thing
 *  displayed ; thus it loads the SDL and opens up a window with one image.
 *
 *  \return This method should return true only if it successfully loaded
 *          the whole runlevel. If it returns false, the previous runlevel
 *          will be backed up again.
 *  \author Pompei2
 */
bool FTS::LoadFTSRlv::load()
{
    FTS18N("Init");

    // First of all, write the current version into the versionfile.
    FILE *pVF = fopen("v.i", "w+");
    if(pVF != NULL)
        fprintf(pVF, "%s", getFTSVersionString().c_str());
    SAFE_FCLOSE(pVF);

    Logger::getSingletonPtr()->doneConsoleMessage();

    // Instantiate Dao VM
    new DaoVm();

    // Need to init the keyboard driver right at the beginning, as all other
    // things may create error messages, these may need to register their
    // callbacks in the error dialog.
    FTS18NDBG("InputDrvL", 1);
    new InputManager();
    // Agreed, this one is not that nice. I will make a constructor that
    // takes an initializer-list when this will be supported by compilers ;)
    InputManager::getSingleton().add((new InputCombo("Copy by Ctrl+c 1", Key::C, nullptr))->addModifier(new InputCombo("Copy by Ctrl+c 2", SpecialKey::Control, new CopyCmd())));
    InputManager::getSingleton().add((new InputCombo("Copy by Ctrl+Ins 1", Key::Insert, nullptr))->addModifier(new InputCombo("Copy by Ctrl+Ins 2", SpecialKey::Control, new CopyCmd())));
    InputManager::getSingleton().add((new InputCombo("Cut by Ctrl+x 1", Key::X, nullptr))->addModifier(new InputCombo("Cut by Ctrl+x 2", SpecialKey::Control, new CutCmd())));
    InputManager::getSingleton().add((new InputCombo("Paste by Ctrl+v 1", Key::V, nullptr))->addModifier(new InputCombo("Paste by Ctrl+v 2", SpecialKey::Control, new PasteCmd())));
    InputManager::getSingleton().add((new InputCombo("Paste by Shift+Ins 1", Key::Insert, nullptr))->addModifier(new InputCombo("Paste by Shift+Ins 2", SpecialKey::Shift, new PasteCmd())));

    Logger::getSingletonPtr()->doneConsoleMessage();

    //////////////////////////////////////////////////////////
    // Loading of the sound driver.                         //
    // Needed here because the main loop updates the sound. //
    FTS18NDBG("SoundDrvL", 1);
    ISndSys::createSoundSys();
    Logger::getSingletonPtr()->doneConsoleMessage();
    Console::Foreground(true);

    //////////////////////////////////////////////
    // Beginning of the graphics initialization //
    FTS18NDBG("GraphDrvL", 1);

    new Renderer;
    new GraphicManager;

    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    m_screenWidth  = conf.get<int>("HRes");
    m_screenHeight = conf.get<int>("VRes");

    // Load the logo and we are fine for our first display.
    String sLang = conf.get<std::string>("Language");
    m_sLogoFile = Path::datadir("Graphics/ui") + Path("Loading." + sLang + ".png");
    if(!FileUtils::fileExists(m_sLogoFile, File::Read)) {
        m_sLogoFile = Path::datadir("Graphics/ui") + Path("Loading.English.png");
    }
#if 0 // This is too slow, 3 secs, but you can try it to see imba mitchell algo :-)
    Graphic *pGraph = GraphicManager::getSingleton().getOrLoadGraphic(m_sLogoFile);
    float fWRatio = (float)globals->pLocalAcc->getW() / (float)pGraph->getW();
    float fHRatio = (float)globals->pLocalAcc->getH() / (float)pGraph->getH();
    float fRatio = std::min(fWRatio, fHRatio);
    GraphicManager::getSingleton().scaleGraphic(m_sLogoFile, fRatio);
#else
    GraphicManager::getSingleton().getOrLoadGraphic(m_sLogoFile);
#endif

    Logger::getSingletonPtr()->doneConsoleMessage();

    // The rest will be loaded in the tick method.
    return true;
}

/** This method will be called during the cleaning of the runlevel
 *  (when quitting).\n
 *  It does NOT unload everything that has been loaded by the load method,
 *  that would be nuts. It just unloads the stuff that has been displayed during
 *  the loading process.
 *
 *  \return This method should return true if it successfully unloaded
 *          the whole runlevel. If it returns false, nothing special is done
 *          (the runlevel is still unloaded).
 *  \author Pompei2
 */
bool FTS::LoadFTSRlv::unload()
{
    GraphicManager::getSingleton().destroyGraphic(m_sLogoFile);

    try {
        if(m_pRootWin != NULL)
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRootWin);
    } catch(CEGUI::Exception &) {
    }

    return true;
}

/** This method will be called once every frame, after the 3D rendering is done
 *  and all matrices are setup to render in two dimensions now.\n
 *  It will draw the FTS logo and below a progressbar and a text saying what is
 *  being loaded, as soon as CEGUI is available.
 *  \author Pompei2
 */
void FTS::LoadFTSRlv::render2D(const Clock&)
{
    // Draw the logo in the middle of the window.
    Graphic *pGraph = GraphicManager::getSingleton().getOrLoadGraphic(m_sLogoFile);
    float fWRatio = (float)getW()  / (float)pGraph->getW();
    float fHRatio = (float)getH() / (float)pGraph->getH();
    float fRatio = std::min(fWRatio, fHRatio);
    pGraph->drawZoomCentered(getW()/2, getH()/2, fRatio, fRatio);

    // We have no root window yet, that means no CEGUI is up yet. Stop here.
    if(m_pRootWin == NULL)
        return;

    // Try to render the whole GUI.
    try {
        CEGUI::System::getSingletonPtr()->renderGUI();
    } catch(CEGUI::Exception &) { }
}

/** This method will be called once every game tick, that is usually once every
 *  frame, right before the rendering is set up and done.\n
 *  This method will do the actual loading. It will always setup something, save
 *  its current state then return, letting FTS refresh the screen
 *  (progressbar and stuff), continuing its loading work next time it's called.
 *
 *  \author Pompei2
 */
bool FTS::LoadFTSRlv::update(const Clock&)
{
    auto timeBegin = std::chrono::steady_clock::now();
    switch(m_eNextTodo) {
    case LoadBeginning:
        // Return to the game.
        Console::Foreground(false);
        m_eNextTodo = LoadCEGUI;
        break;
    case LoadCEGUI:
        if(ERR_OK != this->initCEGUI())
            exit(1);
        m_eNextTodo = LoadGraphics;
        break;
    case LoadGraphics:
        new ShaderManager;
        m_eNextTodo = LoadSound;
        break;
    case LoadSound:
        FTS18NDBG("MusicL", 1);
        // Load all sounds used in the menu.
        DaoVm::getSingleton().execute(Path("loadMenuSounds.dao"));
        DaoVm::getSingleton().execute(Path("EasterEggs.dao"));
        // TODO: load this one by script too, as soon as setting the type is possible.
        ISndSys::getSingleton().CreateSndObj(SndGroup::Attention, "whisp_recv.ogg");
        ISndSys::getSingleton().CreateSndObj(SndGroup::UnitReaction, "scream1.ogg");
        Logger::getSingletonPtr()->doneConsoleMessage();
        m_eNextTodo = LoadNetwork;
        break;
    case LoadNetwork:
        FTS18NDBG("NetDrvL", 1);
        FTS::NetworkLibInit(3); // dbg level set to 4.
        Logger::getSingletonPtr()->doneConsoleMessage();
        m_eNextTodo = LoadFinal;
        break;
    case LoadFinal:
        new FTS::FPSCalculator();

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        m_eNextTodo = LoadDone;
        break;
    default:
        // We're done loading, prepare to enter the main menu runlevel.
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv());
        break;
    }

    this->updateProgressbar();

    // If the loading was faster then 200 ms, wait the rest of time so the
    // user sees the screen for 200 ms.
    auto TimeUsed = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - timeBegin ).count();
    if( TimeUsed < 200)
        std::this_thread::sleep_for( std::chrono::milliseconds( 200 - TimeUsed ) );

    return true;
}

/** This method needs to be overloaded. It has to return a unique name for
 *  the runlevel.
 *
 *  \author Pompei2
 */
String FTS::LoadFTSRlv::getName()
{
    return "FTS Startup";
}

/** This method loads up the whole CEGUI until the point the loading window can
 *  be displayed.\n
 *
 *  That means some little things like extra-imageset and stuff aren't loaded yet.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 *  \author Pompei2
 */
int FTS::LoadFTSRlv::initCEGUI()
{
    try {
        // Need to use the traditional new CEGUI is not our class.
        CEGUI::FTSImageCodec::init();
        CEGUI::ArkanaResourceProvider *pRP = new CEGUI::ArkanaResourceProvider;
        CEGUI::Renderer *pRenderer =
            new CEGUI::OpenGLRenderer(0, getW(), getH(),
                                      CEGUI::FTSImageCodec::getSingletonPtr());
        new CEGUI::System(pRenderer,
                          pRP,
                          (CEGUI::XMLParser *)0,
                          (CEGUI::ScriptModule *)0, /// \TODO TODO: Add Scripting.
                          /* configFile = */"",
                          /* logFile = */"CEGUI.log");

        pRP->setResourceGroupDirectory("imagesets", Path::datadir("Graphics/ui/imagesets"));
        pRP->setResourceGroupDirectory("fonts", Path::datadir("Graphics/ui/fonts"));
        pRP->setResourceGroupDirectory("layouts", Path::datadir("Graphics/ui/layouts"));
        pRP->setResourceGroupDirectory("looknfeels", Path::datadir("Graphics/ui/looknfeel"));
        pRP->setResourceGroupDirectory("schemes", Path::datadir("Graphics/ui/schemes"));

        CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Insane);

        // Adding our own stuff into CEGUI.
        CEGUI::WindowRendererManager::getSingleton().addFactory( &CEGUI::getArkanaProgressBarWRFactory() );

        // set the default resource groups to be used
        CEGUI::Imageset::setDefaultResourceGroup("imagesets");
        CEGUI::Font::setDefaultResourceGroup("fonts");
        CEGUI::WindowManager::setDefaultResourceGroup("layouts");
        CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
        CEGUI::ScriptModule::setDefaultResourceGroup( "lua_scripts" );
        CEGUI::Scheme::setDefaultResourceGroup("schemes");

        // Load the default font(s) before loading anything else.
        GUI::getSingleton().loadFont("normal");
        GUI::getSingleton().loadFont("old");
        GUI::getSingleton().loadFont("mono");

        // load in the scheme file, which auto-loads the ArkanaLook imageset
        CEGUI::SchemeManager::getSingleton().loadScheme(FTS_UI_SKIN_FILE);

        // Set the default cursor (we want nothing as a cursor)
        CEGUI::System::getSingleton().setDefaultMouseCursor(FTS_UI_SKIN, "OneTransparentPixel");
        CEGUI::MouseCursor::getSingleton().hide();

        // Set the default tooltip window type.
        CEGUI::System::getSingleton().setDefaultTooltip("ArkanaLook/Tooltip");
        CEGUI::System::getSingleton().getDefaultTooltip()->setHoverTime(0.75);
        CEGUI::System::getSingleton().getDefaultTooltip()->setDisplayTime(0);

        // Register some global callbacks.
        CEGUI::GlobalEventSet::getSingleton().
            subscribeEvent(CEGUI::Window::EventNamespace + "/" +
                           CEGUI::Window::EventActivated,
                           CEGUI::Event::Subscriber(&GUI::onWindowActivated, GUI::getSingletonPtr()));
        CEGUI::GlobalEventSet::getSingleton().
            subscribeEvent(CEGUI::Window::EventNamespace + "/" +
                           CEGUI::Window::EventDestructionStarted,
                           CEGUI::Event::Subscriber(&GUI::onWindowClosed, GUI::getSingletonPtr()));

        GUI::getSingleton().createRootWindow();

        // Notice the logger that we are ready to display messageboxes.
        DefaultLogger *pDefLog = dynamic_cast<DefaultLogger *>(Logger::getSingletonPtr());
        if(pDefLog)
            pDefLog->noticeCEGUIReady();

        // Now we may load the layout to display during the loading.
        m_pRootWin = GUI::getSingleton().loadLayout("loadfts");

        // TODO: The following stuff may be done a little later (below).
        CEGUI::ImagesetManager::getSingleton().createImageset("ftsui.imageset");

        // We need to register the CEGUI Updater.
        UpdateableManager::getSingleton().add("CEGUI Updater", new CEGUIUpdater());
    } catch(CEGUI::UnknownObjectException & ex) {
        FTS18N("CEGUI_Init", MsgType::Error, ex.getMessage());
        return -1;
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

void FTS::LoadFTSRlv::updateProgressbar()
{
    if(m_eNextTodo <= LoadCEGUI)
        return;

    String sName;
    switch(m_eNextTodo) {
    case LoadGraphics:   sName = "Loading 3D System ..."; break;
    case LoadSound:      sName = "Loading Sound System ..."; break;
    case LoadNetwork:    sName = "Loading Network System ..."; break;
    case LoadFinal:      sName = "Loading last things ..."; break;
    default:             sName = "Loading ..."; break;
    }

    try {
        FTSGetConvertWinMacro(CEGUI::ProgressBar,pPB, "loadfts/pgMe");
        float fPercent = static_cast<float>(m_eNextTodo-1)/static_cast<float>(LoadStageCount-2);
        pPB->setProgress(fPercent);
        CEGUI::WindowManager::getSingleton().getWindow("loadfts/lblState")->setText(sName);
    } catch(...) { }
}

 /* EOF */
