/**
 * \file game_rlv.h
 * \author Pompei2
 * \date 9 December 2008
 * \brief This file implements the game's main runlevel.
 **/

#include "game/game_rlv.h"

#include "dLib/dArchive/dArchive.h" // To unload the map archive
#include "dLib/dConf/configuration.h"
#include "graphic/graphic.h" // For the ICP and RB.
#include "input/input.h" // For the keyboard shortcuts.
#include "ui/ui.h" // For loadlayout.
#include "ui/confirm_dialog.h" // For leaving.
#include "ui/ui_menu.h" // For going back to the main menu.
#include "utilities/command.h"
#include "utilities/utilities.h"
#include "main/Clock.h"
#include "3d/light.h"
#include "3d/camera.h" // For the default camera.
#include "map/map.h"
#include "map/mapinfo.h"
#include "scripting/DaoVm.h"
#include "scripting/DaoFunction.h"
#include "sound/fts_Snd.h"

#include <CEGUIWindowManager.h> // For destroying the windows.
#include <CEGUIExceptions.h> // For destroying the windows.
#include <CEGUISystem.h> // For destroying the windows.

using namespace FTS;

GameRlv::GameRlv()
    : m_pRootWindow(NULL)
    , m_pMap(new Map)
    , m_pMapArchive(NULL)
#ifdef DEBUG
    , m_bRenderCoordinateSystem(false)
#endif
{
}

GameRlv::~GameRlv()
{
}

/** This method will be called during the loading of this runlevel.\n
 *  Please note that this method does NOT load the map, that is done during the
 *  LoadGame's runlevel.\n
 *
 *  This load method rather executes the map's script initialising function.
 *
 *  \return This method should return true only if it successfully loaded
 *          the whole runlevel. If it returns false, the previous runlevel
 *          will be backed up again.
 *  \author Pompei2
 * \TODO: Call the map's init script.
 */
bool GameRlv::load()
{
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    m_screenWidth = conf.getInt("HRes");
    m_screenHeight = conf.getInt("VRes");
    // Change the camera. TODO: Make this in the map's script.
    this->getMainCamera().resetOrientation();
    this->getMainCamera().position(Vector(0.0f,-60.0f,80.0f));
    this->getMainCamera().lookAt(Vector(0.0f,0.0f,0.0f));

    // This is temporary anyway.
    GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/ui/icp.png"));
    GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/ui/rb.png"));

    // Load the cursors for the game.
    this->loadDefaultCursor();

    // And setup the GUI.
    if((m_pRootWindow = GUI::getSingleton().loadLayout("game")) == NULL)
        throw ErrorAlreadyShownException();

    // And register my hotkeys.
    InputManager::getSingleton().pushContext();

    InputManager *pMgr = InputManager::getSingletonPtr();
    if(pMgr) {
        pMgr->registerDefaultMenuShortcuts(false);
        pMgr->add("Game/leave", Key::Escape,
                  new CallbackCommand(FTS_SUBS(GameRlv::cbLeave)));
#ifdef DEBUG
        pMgr->add("Game/DbgCoordSys", Key::F3,
                  new CallbackCommand(FTS_SUBS(GameRlv::cbDbgSwapShowCoordSys)));
#endif
    }
    DaoFunctionCall<>("OnLoadMap")(this->m_pMap->getInfo()->getName());
    return true;
}

/** This method will be called during the cleaning of the runlevel (when quitting).\n
 *  In contrast to the load method, this method DOES unload the map etc.
 *
 *  \return This method should return true if it successfully unloaded
 *          the whole runlevel. If it returns false, nothing special is done
 *          (the runlevel is still unloaded).
 *  \author Pompei2
 */
bool GameRlv::unload()
{
    GraphicManager::getSingleton().destroyGraphic(Path::datadir("Graphics/ui/icp.png"));
    GraphicManager::getSingleton().destroyGraphic(Path::datadir("Graphics/ui/rb.png"));

    // This might be done here. TODO: think about it.
    this->unloadDefaultCursor();

    // Remove all shortcuts. I mean, ALL.
    InputManager::getSingleton().popContext();
    ISndSys::getSingleton().popContext();
    DaoVm::getSingleton().popContext(); // Restore the vm state.
    // Unload the CEGUI layout.
    try {
        if(m_pRootWindow != NULL)
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRootWindow);
    } catch(CEGUI::Exception &) {
    }

    SAFE_DELETE(m_pMap);
    SAFE_DELETE(m_pMapArchive);
    return true;
}

/** This method will be called once every frame, after the 3D rendering is done
 *  and all matrices are setup to render in two dimensions now.\n
 *  Thus you should use the commands glVertex2i(x,y) to draw something, x
 *  and y being screen-space pixels (from 0 to w, 0 to h).\n
 *
 *  Actually, this only draws the gui.
 *
 *  \author Pompei2
 */
void GameRlv::render2D(const Clock& in_c)
{
    const Graphic *icp = GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/ui/icp.png"));
    const Graphic *rb = GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/ui/rb.png"));
    icp->draw(0,0);
    rb->draw(getW() - 3 - rb->getW(),
             getH() - 0 - rb->getH());

    CEGUI::System::getSingleton().renderGUI();

    // And draw the cursor at last.
    drawCursor(in_c, this->getActiveCursor());

    GUI::getSingleton().updateGUIInfo();
}

/** This method will be called once every frame, before the 2D rendering,
 *  when all matrices are still setup to render in three dimensions.\n
 *  You don't need to struggle with camera etc. as your currently active
 *  camera will already be selected and setup (rendered).\n
 *
 *  This actually renders the whole map, all units, buildings, etc.
 *
 *  \author Pompei2
 */
void GameRlv::render3D(const Clock& in_c)
{
    LightSystem::getSys()->renderAll((int)in_c.getCurrentTime());

    m_pMap->draw((unsigned int)(in_c.getCurrentTime()*1000.0));
}

/** This method needs to be overloaded. It has to return a unique name for
 *  the runlevel.
 *
 *  \author Pompei2
 */
String GameRlv::getName()
{
    return "Game";
}

/** This method is called when the user wants to leave the game. It asks if he
 *  really wants to leave.
 *
 * \return true
 *
 *  \author Pompei2
 */
bool GameRlv::cbLeave(const CEGUI::EventArgs &)
{
    // Open a confirmation dialog if the user really wants to quit.
    ConfirmDlg *pConfDlg = new ConfirmDlg();

    String sTxt = getTranslatedString("Really_Quit", "ui");
    pConfDlg->load(sTxt);
    pConfDlg->registerYesHandler(FTS_SUBS(GameRlv::cbLeaveYes));
    pConfDlg->show();

    return true;
}

/** This method is called when the user wants to leave the game and confirmed
 *  that. It prepares to enter the main menu runlevel.
 *
 * \return true
 *
 *  \author Pompei2
 */
bool GameRlv::cbLeaveYes(const CEGUI::EventArgs &)
{
    RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv);
    return true;
}

 /* EOF */
