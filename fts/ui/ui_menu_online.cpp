/**
 * \file ui_menu_online.cpp
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file contains all the callbacks of the online login menu in FTS.
 **/

#include <CEGUI.h>

#include "ui/ui_menu_online.h"

#include "ui/ui.h"
#include "ui/dlg_online_acctInfo.h"
#include "ui/ui_menu.h" // To go back to the main menu.
#include "ui/ui_menu_online_main.h" // To enter the online runlevel.

#include "game/player.h"
#include "logging/logger.h"
#include "graphic/graphic.h" // For the BG image.
#include "graphic/Color.h"
#include "3d/light.h" // For the BG model.
#include "3d/camera.h" // For the BG model.
#include "3d/ModelManager.h"
#include "3d/ModelInstance.h"
#include "3d/Resolution.h"

#include "dLib/dConf/configuration.h"
#include "dLib/dBrowse/dBrowse.h"

using namespace FTS;

/// Default constructor.
FTS::LoginMenuRlv::LoginMenuRlv()
    : m_pRoot(NULL)
    , m_pgMenuBG(NULL)
    , m_pModelManager(0)
    , m_pMenuBGInst(0)
    , m_pConf(nullptr)
{
}

/// Default destructor.
FTS::LoginMenuRlv::~LoginMenuRlv()
{
    delete m_pConf;
}

/** This method will be called during the loading of the runlevel.\n
 *  The GUI is loaded here, other stuff is initialised too.
 *
 *  \return This method should return true only if it successfully loaded
 *          the whole runlevel. If it returns false, the previous runlevel
 *          will be backed up again.
 *  \author Pompei2
 */
bool FTS::LoginMenuRlv::load()
{
    if( m_pConf != nullptr ) {
        delete m_pConf;
    }
    m_pConf = new Configuration("conf.xml", ArkanaDefaultSettings());

    // Find the correct menu background image.
    std::list<Resolution> lAvail;
    PDBrowseInfo dbi = dBrowse_Open(Path::datadir("Graphics/ui/menubg"));
    for(String file = dBrowse_GetNextWithWildcard(dbi, "*.png") ; !file.isEmpty() ; file = dBrowse_GetNextWithWildcard(dbi, "*.png")) {
        lAvail.push_back(Resolution(Path(file).withoutExt()));
    }
    dBrowse_Close(dbi);

    String sClosestMatch = Resolution().bestFit(lAvail).toString(false);

    // Load the menu background image.
    m_pgMenuBG = GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/ui/menubg") + Path(sClosestMatch + ".png"));

    // And the menu background model.
    m_pModelManager = new ModelManager();
    m_pMenuBGInst = m_pModelManager->createInstance("Gaia/Fauna/Chicken");

    // Reset the camera.
    this->getMainCamera().resetOrientation();
    this->getMainCamera().position(Vector(0.0f,2.0f,10.0f));

    this->loadDefaultCursor();

    // Load the CEGUI menu layout. If that fails, quit ...
    if((m_pRoot = GUI::getSingleton().loadLayout("menu_online", true)) == NULL)
        return false;

    // Setup the callbacks for the mainmenu.
    try {
        m_pRoot->getChild("menu_online/btnLogin")
               ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::LoginMenuRlv::cbLogin));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    try {
        m_pRoot->getChild("menu_online/btnCreate")
               ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::LoginMenuRlv::cbCreate));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    try {
        m_pRoot->getChild("menu_online/btnBack")
               ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::LoginMenuRlv::cbBack));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    try {
        m_pRoot->getChild("menu_online/Nickname")
               ->setText(m_pConf->get("LastLogin"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    try {
        GUI::getSingleton().setActiveWidget(m_pRoot->getChild("menu_online/Password"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    // Setup keyboard shortcuts.
    InputManager *pMgr = InputManager::getSingletonPtr();
    if(pMgr) {
        // One for the return key to login and one for the escape key to go back to
        // the main menu.
        pMgr->add("Online_Login_Enter", SpecialKey::Enter, new CallbackCommand(FTS_SUBS(FTS::LoginMenuRlv::cbLogin)));
        pMgr->add("Online_Login_Back", Key::Escape, new CallbackCommand(FTS_SUBS(FTS::LoginMenuRlv::cbBack)));

        pMgr->registerDefaultMenuShortcuts();
    }

    // Set the correct version.
//     setVersionInfo(); ///< \TODO FIX THIS

    return true;
}

/** This method will be called during the cleaning of the runlevel (when quitting).\n
 *  It unloads the GUI, unregister all your keyboard shortcuts etc.
 *
 *  \return This method should return true if it successfully unloaded
 *          the whole runlevel. If it returns false, nothing special is done
 *          (the runlevel is still unloaded).
 *  \author Pompei2
 */
bool FTS::LoginMenuRlv::unload()
{
    this->unloadDefaultCursor();

    // Now that the dialog gets closed, we can remove both shortcuts from the system.
    InputManager::getSingleton().delShortcut("Online_Login_Enter");
    InputManager::getSingleton().delShortcut("Online_Login_Back");
    InputManager::getSingleton().unregisterDefaultMenuShortcuts();

    // Unload the menu background image and model.
    GraphicManager::getSingleton().destroyGraphic(m_pgMenuBG);
    SAFE_DELETE(m_pMenuBGInst);
    m_pModelManager->removeModel("Gaia/Fauna/Chicken");
    SAFE_DELETE(m_pModelManager);

    // Unload the CEGUI layout.
    try {
        if(m_pRoot != NULL) {
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRoot);
        }
    } catch(CEGUI::Exception &) {
    }
    delete m_pConf;
    m_pConf = nullptr;
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
 *  \author Pompei2
 */
void FTS::LoginMenuRlv::render2D(const Clock& c)
{
    // Draw menu background first.
    float fZoomX = (float)m_pConf->getInt("HRes") / (float)m_pgMenuBG->getW();
    float fZoomY = (float)m_pConf->getInt("VRes") / (float)m_pgMenuBG->getH();
    m_pgMenuBG->drawZoom(0, 0, fZoomX, fZoomY);

    // Now comes the GUI.
    this->renderCEGUI();

    // And draw the cursor at last.
    drawCursor(c, this->getActiveCursor());

    GUI::getSingleton().updateGUIInfo();
}

/** This method will be called once every frame,
 *  before the 2D rendering, when all matrices are still setup to render in
 *  three dimensions.\n
 *  You don't need to struggle with camera etc. as your currently active
 *  camera will already be selected and setup (rendered).
 *
 *  \author Pompei2
 */
void FTS::LoginMenuRlv::render3D(const Clock&)
{
    // We want to render light. Currently at 12 o'clock.
    LightSystem::getSys()->renderAll(12*60*60);

    // In debug mode draw a coordinate system.
#ifdef DEBUG
    this->renderCoordSys();
#endif

    m_pMenuBGInst->render(Vector(0.0, 0.0, 0.0), Color());
}

/** This method returns a unique name for the runlevel.
 *
 * \return The name of this runlevel.
 *
 *  \author Pompei2
 */
String FTS::LoginMenuRlv::getName()
{
    return "Login Menu";
}

/// Called on a click on the login button in the online login menu.
/** This callback is called when the user clicks the login button that
 *  is placed in the online login menu.
 *
 *  Tries to login the user on the master server, then enters the
 *  Online main runlevel.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::LoginMenuRlv::cbLogin(const CEGUI::EventArgs & in_ea)
{
    String sNick;
    String sPass;

    try {
        CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();
        sNick = pWM->getWindow("menu_online/Nickname")->getText();
        sPass = pWM->getWindow("menu_online/Password")->getText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    // First: Log me in.
    if(ERR_OK != g_pMeHacky->og_login(sNick, sPass))
        return true;

    // Second: TODO: start a keepalive thread.

    // When there was no error, save the login name.
    m_pConf->set("LastLogin", sNick);
    m_pConf->save();

    // Third: Enter into the menu.
    RunlevelManager::getSingleton().prepareRunlevelEntrance(new OnlineMenuRlv);

    return true;
}

/// Called on a click on the create button in the online login menu.
/** This callback is called when the user clicks the create button that
 *  is placed in the online login menu.
 *
 *  Opens a dialog for account creation on the master server.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::LoginMenuRlv::cbCreate(const CEGUI::EventArgs & in_ea)
{
    new DlgOnlineAcctInfo(DlgOnlineAcctInfo::ModeCreate, String::EMPTY, true);
    return true;
}

/// Called on a click on the back button in the online login menu.
/** This callback is called when the user clicks the back button that
 *  is palced in the online login menu.
 *
 *  Unloads this menu and enters the main menu runlevel.
 *
 * \param in_ea CEGUI Event arguments
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::LoginMenuRlv::cbBack(const CEGUI::EventArgs & in_ea)
{
    // This does all the stuff of loading etc.
    RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv());
    return true;
}

 /* EOF */
