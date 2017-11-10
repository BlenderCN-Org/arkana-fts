/**
 * \file ui_menu.h
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file contains all the callbacks of the main menu in FTS.
 **/

#include <CEGUI.h>
#include "packet.h"
#include "connection.h"

#include "ui/ui.h"
#include "ui/ui_menu.h"
#include "ui/ui_menu_online.h"
#include "ui/dlg_options.h"
#include "ui/file_dialog.h"

#include "map/map.h"
#include "3d/camera.h"
#include "3d/light.h"
#include "3d/ModelManager.h"
#include "3d/ModelInstance.h"
#include "3d/Resolution.h"
#include "logging/logger.h"
#include "utilities/utilities.h"
#include "utilities/DataContainer.h"
#include "main/runlevels.h"
#include "main/version.h"
#include "mdlviewer/mdlviewer_main.h"
#include "game/loadgame_rlv.h"
#include "graphic/graphic.h"
#include "dLib/dConf/configuration.h"
#include "dLib/dString/dTranslation.h"

#include "dLib/dProcess/dProcess.h"


using namespace FTS;

/// Sets the version text on the menu.
/** Sets the version text on the menu.
 *
 * \author Pompei2
 */
void FTS::setVersionInfo()
{
/// \TODO TODO Place somewhere !
    try {
        CEGUI::WindowManager::getSingleton().getWindow("Version")->
            setText(String("version ") + getFTSVersionString());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Default constructor
FTS::MainMenuRlv::MainMenuRlv()
    : m_pRoot(nullptr)
    , m_pModelManager(new ModelManager())
    , m_pgMenuBG(nullptr)
    , m_pMenuBGInst(0)
{
    loadSettingsFromConf();
}

/// Default destructor
FTS::MainMenuRlv::~MainMenuRlv()
{
    SAFE_DELETE(m_pModelManager);
}

void FTS::MainMenuRlv::loadSettingsFromConf()
{
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    m_iScreenHeigth = conf.getInt("VRes");
    m_iScreenWidth = conf.getInt("HRes");
}

/** This method will be called during the loading
 *  of the runlevel.\n
 *  You may for example register all your keyboard shortcuts here.
 *
 *  \return This method should return true only if it successfully loaded
 *          the whole runlevel. If it returns false, the previous runlevel
 *          will be backed up again.
 *  \author Pompei2
 */
bool FTS::MainMenuRlv::load()
{
    // load the screen dimensions in case they have changed in the meanwhile.
    loadSettingsFromConf();

    // Find the correct menu background image.
    std::list<Resolution> lAvail;
    std::vector<String> fileNames = dBrowse(Path::datadir("Graphics/ui/menubg"), "*.png");
    for(auto&& file : fileNames) {
        lAvail.push_back(Resolution(Path(file).withoutExt()));
    }

    String sClosestMatch = Resolution().bestFit(lAvail).toString(false);

    // Load the menu background image.
    m_pgMenuBG = GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/ui/menubg") + Path(sClosestMatch + ".png"));

    // And the menu background model.
    m_pMenuBGInst = m_pModelManager->createInstance("Gaia/Fauna/Chicken");

    // Reset the camera.
    this->getMainCamera().resetOrientation();
    this->getMainCamera().position(Vector(0.0f,2.0f,10.0f));

    this->loadDefaultCursor();
    this->loadCEGUI(); // Throws on problem.

    // And register my hotkeys.
    InputManager::getSingleton().registerDefaultMenuShortcuts();

    return true;
}

bool FTS::MainMenuRlv::loadCEGUI()
{
    // Load the CEGUI menu layout. If that fails, quit ...
    if((m_pRoot = GUI::getSingleton().loadLayout("menu_main", true)) == NULL)
        throw ErrorAlreadyShownException();

    // Setup the callbacks for the mainmenu.
    try {
        if(m_pRoot->isChild("menu_main/btnNewGame"))
            m_pRoot->getChild("menu_main/btnNewGame")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbNewGame));
        if(m_pRoot->isChild("menu_main/btnOnlineGame"))
            m_pRoot->getChild("menu_main/btnOnlineGame")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbOnlineGame));
        if(m_pRoot->isChild("menu_main/btnOptions"))
            m_pRoot->getChild("menu_main/btnOptions")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbOptions));
        if(m_pRoot->isChild("menu_main/btnOpen"))
            m_pRoot->getChild("menu_main/btnOpen")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbMdlViewer));
        if(m_pRoot->isChild("menu_main/btnUpdate"))
            m_pRoot->getChild("menu_main/btnUpdate")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbUpdate));
        if(m_pRoot->isChild("menu_main/btnInfo"))
            m_pRoot->getChild("menu_main/btnInfo")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbInfo));
        if(m_pRoot->isChild("menu_main/btnQuit"))
            m_pRoot->getChild("menu_main/btnQuit")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbQuit));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    // Set the correct version.
    setVersionInfo();

    // If there was an error during init, give that dialogbox the focus.
    try {
        if(CEGUI::WindowManager::getSingleton().isWindowPresent("dlg_message 0")) {
            CEGUI::WindowManager::getSingleton().getWindow("dlg_message 0")->activate();
        }
    } catch(CEGUI::Exception &) { }

    return true;
}

bool FTS::MainMenuRlv::unloadCEGUI()
{
    // Unload the CEGUI layout.
    try {
        if(m_pRoot != NULL) {
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRoot);
        }
    } catch(CEGUI::Exception &) {
    }

    return true;
}

/** This method will be called during the cleaning
 *  of the runlevel (when quitting).\n
 *  You may for example unregister all your keyboard shortcuts here.
 *
 *  \return This method should return true if it successfully unloaded
 *          the whole runlevel. If it returns false, nothing special is done
 *          (the runlevel is still unloaded).
 *  \author Pompei2
 */
bool FTS::MainMenuRlv::unload()
{
    this->unloadDefaultCursor();

    // Unregister my hotkeys.
    InputManager *pMgr = InputManager::getSingletonPtr();
    if(pMgr) {
        pMgr->unregisterDefaultMenuShortcuts();
        pMgr->delShortcut("MainMenu/nextGuiInfo");
    }

    // Unload the menu background image and model.
    SAFE_DELETE(m_pMenuBGInst);
    m_pModelManager->removeModel("Gaia/Fauna/Chicken");
    GraphicManager::getSingleton().destroyGraphic(m_pgMenuBG);

    this->unloadCEGUI();

    return true;
}

/** This method will be called once every frame,
 *  after the 3D rendering is done and all matrices are setup to render
 *  in two dimensions now.\n
 *  Thus you should use the commands glVertex2i(x,y) to draw something, x
 *  and y being screen-space pixels (from 0 to w, 0 to h).
 *
 * \todo TODO Only update the GUI Info every 500 ms or so.
 *
 * \author Pompei2
 */
void FTS::MainMenuRlv::render2D(const Clock& in_c)
{
    // Draw menu background first. Stretch it to fill the screen resolution.
    float fZoomX = (float)m_iScreenWidth / (float)m_pgMenuBG->getW();
    float fZoomY = (float)m_iScreenHeigth / (float)m_pgMenuBG->getH();
    m_pgMenuBG->drawZoom(0, 0, fZoomX, fZoomY);

    // Now comes the GUI.
    this->renderCEGUI();

    // And draw the cursor at last.
    drawCursor(in_c, this->getActiveCursor());

    GUI::getSingleton().updateGUIInfo();
}

/** This method needs will be called once every frame,
 *  before the 2D rendering, when all matrices are still setup to render in
 *  three dimensions.\n
 *  You don't need to struggle with camera etc. as your currently active
 *  camera will already be selected and setup (rendered).
 *
 *  \author Pompei2
 */
void FTS::MainMenuRlv::render3D(const Clock&)
{
    // We want to render light. Currently at 12 o'clock.
    LightSystem::getSys()->renderAll(12*60*60);

    m_pMenuBGInst->render(Vector(0.0, 0.0, 0.0), Color());
}

/** This method returns a unique name for the runlevel.
 *
 * \return The name of this runlevel.
 *
 * \author Pompei2
 */
String FTS::MainMenuRlv::getName()
{
    return "Main menu";
}

/// Called on a click on the new game button.
/** This callback is called when the user clicks the new game button.
 *  But you shouldn't place game initialisations here, instead place
 *  them in the according runlevel's load method.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbNewGame(const CEGUI::EventArgs & in_ea)
{
    (new FileDlg())->registerHandler(FTS_SUBS(MainMenuRlv::cbNewGameOpened))
                   ->loadOpenDlg("*.ftsm", Path::datadir("Maps"));

    return true;
}

/// Called when the user chose a map to open.
/** This callback is called when the user has chosen a map
 *  in the file open dialog that he wants to play.
 *
 * \param in_pF The file dialog object that called me.
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbNewGameOpened(const CEGUI::EventArgs &in_ea)
{
    Path sFile = static_cast<const FileDlgEventArgs &>(in_ea).getFile();

    // If he clicked on cancel, do nothing.
    if(sFile.empty()) {
        return true;
    }

    // This does all the stuff of loading etc.
    /// \TODO: TODO dynamic settings with a menu before loading the game.
    Runlevel *pRlv = new LoadGameRlv(sFile, 12);
    RunlevelManager::getSingleton().prepareRunlevelEntrance(pRlv);
    return true;
}

/// Called on a click on the online game button.
/** This callback is called when the user clicks the online game button.
 *  This will just load the online menu runlevel thus any initialisation should
 *  be placed into that runlevel's init code.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbOnlineGame(const CEGUI::EventArgs & in_ea)
{
    Runlevel *pRlv = new LoginMenuRlv();
    RunlevelManager::getSingleton().prepareRunlevelEntrance(pRlv);
    return true;
}

/// Called on a click on the options button.
/** This callback is called when the user clicks the options button.
 *  It just opens up the options menu.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbOptions(const CEGUI::EventArgs & in_ea)
{
    new MenuOptions();
    return true;
}

/// Called on a click on the Modelviewer button.
/** This callback is called when the user clicks the modelviewer button.
 *
 *  It will create and enter the modelviewer runlevel, that is a kind of little
 *  application right within the game.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbMdlViewer(const CEGUI::EventArgs & in_ea)
{
    Runlevel *pRlv = new ModelViewerRlv();
    RunlevelManager::getSingleton().prepareRunlevelEntrance(pRlv);
    return true;
}

/// Called on a click on the search for updates button.
/** This callback is called when the user clicks the search for updates
 *  button in the main menu.\n
 *  This checks if there is a new version available, if yes it lets the user
 *  choose to start the updater or not.
 *
 * \param in_ea CEGUI Event arguments
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbUpdate(const CEGUI::EventArgs & in_ea)
{
    try {
        CEGUI::Window *wUpdate = GUI::getSingleton().loadLayout("dlg_Update");
        wUpdate->getChild("dlg_Update/Cancel")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbUpdateCancel));
        wUpdate->getChild("dlg_Update/Ok")->
            subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(MainMenuRlv::cbUpdateDoIt));
        wUpdate->
            subscribeEvent(CEGUI::FrameWindow::EventCloseClicked, FTS_SUBS(MainMenuRlv::cbUpdateCancel));
        wUpdate->getChild("dlg_Update/Ok")->setEnabled(false);

        Translation trans( "ui" );
        // TODO Fix DataContainer usage afte the fts-net lib doesn't use it anymore.
        std::vector<uint8_t> Data ;
        FTSC_ERR err = getHTTPFile(Data, "arkana-fts.sourceforge.net", "/actual_release", 1000*10);
        if( err != FTSC_ERR::OK ) {
            wUpdate->getChild( "dlg_Update/Desc" )->setText( trans.get( "Update_Error" ) );
            return true;
        }
        RawDataContainer NewestVersion ( Data.size() );
        memcpy( NewestVersion.getData(), Data.data(), Data.size() );

        // Extract the version information.
        int iVersion[3] = {0};
        sscanf(StreamedConstDataContainer(&NewestVersion).readstr().c_str(), "%d.%d.%d", &iVersion[0], &iVersion[1], &iVersion[2]);

        // Look if we have a new version.
        if(makeFTSVersionUInt32(iVersion[0],iVersion[1],iVersion[2]) > getFTSVersionUInt32()) {
            wUpdate->getChild("dlg_Update/Desc")->setText(trans.get("Update_Found"));
            wUpdate->getChild("dlg_Update/Ok")->setEnabled(true);
        } else {
            wUpdate->getChild("dlg_Update/Desc")->setText(trans.get("Update_None"));
        }
    }
    catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

/// Called when the user cancels the update.
/** This callback is called when the user clicked on the cancel button
 *  in the update dialog.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbUpdateCancel(const CEGUI::EventArgs & in_ea)
{
    try {
        CEGUI::WindowManager::getSingleton().destroyWindow("dlg_Update");
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    return true;
}

/// Called when the user wants to update.
/** This callback is called when the user clicks the update now button
 *  in the update dialog.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbUpdateDoIt(const CEGUI::EventArgs & ea)
{
    // Close the update dialog first.
    this->cbUpdateCancel(ea);
#if 0
    // Then start the updater and quit fts.
    const char *ppszArgs[] = { "fts_updater.exe", "fts_updater.exe" };
    spawnv_async(ppszArgs[0], ppszArgs);
    exit(EXIT_SUCCESS);
#else
    FTS18N("NotImpYet", MsgType::Warning);
#endif
    return true;
}

/// Called on a click on the info button.
/** This callback is called when the user clicks the info button.
 *
 * It will show some informations about Arkana-FTS.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbInfo(const CEGUI::EventArgs &)
{
    FTS18N("Info", MsgType::Message);
    return true;
}

/// Called on a click on the quit button.
/** This callback is called when the user clicks the quit button.
 *
 * It will call the exit function, starting to de-init Arkana-FTS.
 *
 * \return never returns
 *
 * \author Pompei2
 */
bool FTS::MainMenuRlv::cbQuit(const CEGUI::EventArgs &)
{
    exit(EXIT_SUCCESS);
}

 /* EOF */
