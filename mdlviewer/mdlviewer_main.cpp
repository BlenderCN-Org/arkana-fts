/**
 * \file mdlviewer_main.cpp
 * \author Pompei2
 * \date 02 July 2008
 * \brief This file contains all the main FTS Model Viewer parts.
 **/

#include <CEGUI.h>

#include "mdlviewer/mdlviewer_main.h"
#include "ui/ui.h"
#include "ui/ui_menu.h"
#include "ui/file_dialog.h"
#include "ui/cegui_items/simple_list_item.h"
#include "ui/cegui_items/ColorListItem.h"
#include "3d/camera.h"
#include "3d/ModelManager.h"
#include "3d/ModelInstance.h"
#include "3d/math/AxisAlignedBoundingBox.h"
#include "3d/Movers/Translator.h"
#include "3d/Movers/Rotator.h"
#include "3d/Resolution.h"
#include "graphic/Color.h"
#include "logging/logger.h"
#include "main/runlevels.h"
#include "map/DecorativeMO.h"
#include "utilities/utilities.h"
#include "scripting/DaoFunction.h"
#include "sound/SndSys.h"
#include "sound/SndObj.h"
#include "3d/Movers/Orbiter.h"
#include <bouge/CoreAnimation.hpp>

using namespace FTS;

/// Default constructor.
FTS::ModelViewerRlv::ModelViewerRlv()
    : m_pRoot(nullptr)
    , m_pModelManager(new ModelManager())
    , m_sModelName(m_pModelManager->ErrorModelName)
    , m_playerColor(FTS::getPlayerColors().front())
    , m_bShowAABB(false)
{
}

/// Default destructor.
FTS::ModelViewerRlv::~ModelViewerRlv()
{
    SAFE_DELETE(m_pModelManager);
}

/** This method needs to be overloaded. It will be called during the loading
 *  of the runlevel.\n
 *  Loads the whole GUI and sets up the keyboard shortcuts.
 *
 * \return This method should return true only if it successfully loaded
 *         the whole runlevel. If it returns false, the previous runlevel
 *         will be backed up again.
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::load()
{
    // Reset the camera.
    this->getMainCamera().resetOrientation();
    this->getMainCamera().position(Vector(0.0f,-10.0f,5.0f));
    this->getMainCamera().lookAt(Vector(0.0f,0.0f,0.0f));
    this->getMainCamera().moveRight(2.5f);

    this->loadDefaultCursor();

    if(ERR_OK != this->loadGUI())
        return false;

    this->setupGUI();

    // And register my hotkeys.
    InputManager::getSingleton().registerDefaultMenuShortcuts();

    // Load the models used to draw the coordinate system.
    m_pCoordSys = m_pModelManager->createInstance("Internal/CoordinateSystem");
    m_pAABB = m_pModelManager->createInstance("Internal/AABB");

    return true;
}

/** This method needs to be overloaded. It will be called during the cleaning
 *  of the runlevel (when quitting).\n
 *  You may for example unregister all your keyboard shortcuts here.
 *
 * \return This method should return true if it successfully unloaded
 *         the whole runlevel. If it returns false, nothing special is done
 *         (the runlevel is still unloaded).
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::unload()
{
    // Unregister my hotkeys.
    InputManager *pMgr = InputManager::getSingletonPtr();
    if(pMgr) {
        pMgr->unregisterDefaultMenuShortcuts();
        pMgr->delShortcut("MainMenu/nextGuiInfo");
        pMgr->delShortcut("Translator Test W");
        pMgr->delShortcut("Translator Test W Stopper");
        pMgr->delShortcut("Translator Test S");
        pMgr->delShortcut("Translator Test S Stopper");
        pMgr->delShortcut("Translator Test A");
        pMgr->delShortcut("Translator Test A Stopper");
        pMgr->delShortcut("Translator Test D");
        pMgr->delShortcut("Translator Test D Stopper");
        pMgr->delShortcut("Translator Test Space");
        pMgr->delShortcut("Rotator Test Q");
        pMgr->delShortcut("Rotator Test E");
        pMgr->delShortcut("Orbiter Test Y");
        pMgr->delShortcut("Orbiter Test X");
    }

    this->unloadDefaultCursor();
    this->unloadGUI();

    m_modelInsts.clear();

    SAFE_DELETE(m_pCoordSys);
    SAFE_DELETE(m_pAABB);

    // Again for our halloween   e a s t e r   e g g.
    // Play the normal menu background again.
    DaoFunctionCall<>("HalloweenEasterEgg")("SomeOtherModelName");

    return true;
}

/**  Loads the whole GUI, does NOT set any shortcuts or do anything else.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \author Pompei2
 */
int FTS::ModelViewerRlv::loadGUI()
{
    CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();
    if(!(m_pRoot = GUI::getSingleton().loadLayout("mdlviewer_main")))
        return false;

    // Now we go on and setup the callbacks for all the GUI elements.
    // Additionally, we set some of them to a default value.

    try {
        // Connect the events to the member functions.
        pWM->getWindow("mdlviewer/menubar/btnBack")
           ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(FTS::ModelViewerRlv::cbBack));
    } catch(CEGUI::Exception & e) {
        // This is the only error where we say we failed to load. On every other
        // error, we may still be somehow useful for the user.
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return false;
    }

    try {
        pWM->getWindow("mdlviewer/menubar/btnLoad")
           ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(FTS::ModelViewerRlv::cbLoad));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        pWM->getWindow("mdlviewer/panel/frmMoves/btnAddAsAction")
           ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(FTS::ModelViewerRlv::cbPlayAction));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        pWM->getWindow("mdlviewer/panel/frmMoves/btnAddAsCycle")
           ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(FTS::ModelViewerRlv::cbPlayCycle));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        pWM->getWindow("mdlviewer/panel/frmMoves/btnPlayPause")
           ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(FTS::ModelViewerRlv::cbPlayPause));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        pWM->getWindow("mdlviewer/panel/frmMoves/btnStop")
           ->subscribeEvent(CEGUI::PushButton::EventClicked, FTS_SUBS(FTS::ModelViewerRlv::cbStop));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "mdlviewer/panel/frmMoves/hsSpeed");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged, FTS_SUBS(FTS::ModelViewerRlv::cbhsScrollChanged));
        sb->setScrollPosition(100.0f);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "mdlviewer/panel/frmMoves/hsPrio");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged, FTS_SUBS(FTS::ModelViewerRlv::cbhsScrollChanged));
        sb->setScrollPosition(50.0f);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMisc/cbPlayerCol");
        cb->getEditbox()->subscribeEvent(CEGUI::Editbox::EventTextChanged, FTS_SUBS(FTS::ModelViewerRlv::cbPlayerColChanged));

        (new ColorListItem(FTS::getPlayerColors().front(), FTS::getPlayerColors().front().toString()))->addAsDefault(cb);
        for(auto i = FTS::getPlayerColors().begin() + 1 ; i != FTS::getPlayerColors().end() ; ++i) {
            cb->addItem(new ColorListItem(*i, i->toString()));
        }
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMisc/cbSkin");
        cb->getEditbox()->subscribeEvent(CEGUI::Editbox::EventTextChanged, FTS_SUBS(FTS::ModelViewerRlv::cbSkinChanged));
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Window, w1, "mdlviewer/panel/frmMisc/chkNormals");
        w1->disable();
        FTSGetConvertWinMacro(CEGUI::Window, w2, "mdlviewer/panel/frmMisc/chkAttPts");
        w2->disable();
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Checkbox, cb, "mdlviewer/panel/frmMisc/chkAABB");
        cb->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, FTS_SUBS(FTS::ModelViewerRlv::cbShowAABBChanged));
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "mdlviewer/panel/frmMass/hsX");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::ModelViewerRlv::cbhsScrollChanged));
        sb->setScrollPosition(1.0f);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "mdlviewer/panel/frmMass/hsY");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::ModelViewerRlv::cbhsScrollChanged));
        sb->setScrollPosition(1.0f);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return ERR_OK;
}

/** This method removes all CEGUI windows created.
 *
 * \return ERR_OK.
 *
 * \author Pompei2
 */
int FTS::ModelViewerRlv::unloadGUI()
{
    // Unload the GUI only if it is loaded.
    if(m_pRoot) {
        try {
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRoot);
        } catch(CEGUI::Exception & e) {
            FTS18N("CEGUI", MsgType::Error, e.getMessage());
        }
    }

    m_pRoot = NULL;
    return ERR_OK;
}

/** This method sets up the GUI to reflect the currently loaded model.
 *  That means it enables everything if a model has been loaded, disables
 *  everything if no model is loaded.\n
 *  If a model is loaded, it also shows its data, like the moves, ...
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \author Pompei2
 */
int FTS::ModelViewerRlv::setupGUI()
{
    // This will clear the moves combobox, and re-fill it if needed.
    this->setupMovesCombobox();
    this->setupSkinsCombobox();
    this->setupStatusbar();

    return ERR_OK;
}

/** This method needs to be overloaded. It will be called once every frame,
 *  after the 3D rendering is done and all matrices are setup to render
 *  in two dimensions now.\n
 *  Thus you should use the commands glVertex2i(x,y) to draw something, x
 *  and y being screen-space pixels (from 0 to w, 0 to h).
 * \author Pompei2
 */
void FTS::ModelViewerRlv::render2D(const Clock& in_c)
{
    // Draw the GUI.
    this->renderCEGUI();
    GUI::getSingleton().updateGUIInfo();

    // And draw the cursor at last.
    drawCursor(in_c, this->getActiveCursor());
}

/** This method needs to be overloaded. It will be called once every frame,
 *  before the 2D rendering, when all matrices are still setup to render in
 *  three dimensions.\n
 *  It renders a coordinate system and it all the models at their position.
 *
 * \author Pompei2
 */
void FTS::ModelViewerRlv::render3D(const Clock&)
{
    m_pCoordSys->render(Vector(), Color(0.0f, 0.0f, 0.0f));

    // Draw every model at its position.
    for(auto i = m_modelInsts.begin() ; i != m_modelInsts.end() ; ++i) {
        (*i)->render(m_playerColor);

        if(m_bShowAABB) {
            m_pAABB->render(AffineMatrix::translation((*i)->pos()) * (*i)->getModelInst()->restAABB().getModelMatrix(), Color(0.0f, 0.0f, 0.0f));
        }
    }
}

/** This method needs to be overloaded. It has to return a unique name for
 *  the runlevel.
 *
 * \author Pompei2
 */
String FTS::ModelViewerRlv::getName()
{
    return "Model Viewer";
}

/** This gets called on every mouse movement. It is responsible for the
 *  virtual trackball effect.
 *
 *  \param x The new mouse x pos, in pixels.
 *  \param y The new mouse y pos, in pixels.
 */
void FTS::ModelViewerRlv::onMouseMoved(uint16_t x, uint16_t y)
{
    static uint16_t prevx = x, prevy = y;

    if(InputManager::getSingleton().isMousePressed(MouseButton::Left)) {
        Resolution r;

        // First, normalize the amount of mouse move horizontally and vertically:
        float dx = static_cast<float>(prevx - x)/static_cast<float>(r.w);
        float dy = static_cast<float>(prevy - y)/static_cast<float>(r.h);

        // Next, convert this to an angle. 0.0 -> 0°, 1.0 -> 180°
        float alphax = dx*180.0f*deg2rad;
        float alphay = dy*180.0f*deg2rad;

        // Confused? XAxis = axis around which moving the mouse in Y rotates!
        Vector vXAxis = Vector(0.0f, 0.0f, 1.0f);
        Vector vYAxis = Vector(1.0f, 0.0f, 0.0f);

        // And rotate that around the camera axe. Note: rot the obj, not the cam
        // Also note multiplying it to the right makes the axes use the global
        // coordinate system, while multiplying from the left would use local.
        m_modelInsts.front()->rot(m_modelInsts.front()->rot() * Quaternion::rotation(vXAxis, alphax));
        m_modelInsts.front()->rot(m_modelInsts.front()->rot() * Quaternion::rotation(vYAxis, alphay));
    }

    prevx = x;
    prevy = y;
}

/** This fills the combobox with the moves of the current model. Sets the first
 *  move as the default. If no model is loaded, clears the combobox.
 *
 * \return ERR_OK if successful.
 * \return Error code < 0 if failed.
 *
 * \author Pompei2
 */
int FTS::ModelViewerRlv::setupMovesCombobox()
{
    try {
        // Empty the combobox.
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMoves/cbMove");
        cb->resetList();
        cb->setText("");

        // Now fill it with the available moves.
        const std::set<String>& moves = m_modelInsts.front()->getModelInst()->moves();

        if(!moves.empty()) {
            auto i = moves.begin();
            (new SimpleListItem(*i++))->addAsDefault(cb);
            for( ; i != moves.end() ; ++i) {
                cb->addItem(new SimpleListItem(*i));
            }
        }
    } catch(CEGUI::Exception &e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

int FTS::ModelViewerRlv::setupSkinsCombobox()
{
    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMisc/cbSkin");
        cb->resetList();
        cb->setText("");

        // And now fill it with the available skins:
        const std::set<String>& skins = m_modelInsts.front()->getModelInst()->skins();

        // We can assume that there is always at least one skin. Still, never trust.
        if(!skins.empty()) {
            auto i = skins.begin();
            (new SimpleListItem(*i++))->addAsDefault(cb);
            for( ; i != skins.end() ; ++i) {
                cb->addItem(new SimpleListItem(*i));
            }
        } else {
            (new SimpleListItem("Default"))->addAsDefault(cb);
        }
    } catch(CEGUI::Exception &e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

int FTS::ModelViewerRlv::setupStatusbar()
{
    try {
        CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();

        const ModelInstance* pInst = m_modelInsts.front()->getModelInst();

        pWM->getWindow("mdlviewer/statusbar/lblVertsVal")->setText(String::nr(pInst->vertexCount()));
        pWM->getWindow("mdlviewer/statusbar/lblFaceVal")->setText(String::nr(pInst->faceCount()));
        pWM->getWindow("mdlviewer/statusbar/lblMovesVal")->setText(String::nr(pInst->moves().size()));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

void FTS::ModelViewerRlv::destroyModelInstances()
{
    m_modelInsts.clear();
}

/** This first removes all old model instances that are currently created, then
 *  (if the model has been loaded,) re-creates the needed model instances and
 *  inserts them into the map.
 *
 * \return ERR_OK if successful.
 * \return Error code < 0 if failed.
 *
 * \author Pompei2
 */
int FTS::ModelViewerRlv::setupModelInstances()
{
    // First, we remove any currently existing model instances.
    this->destroyModelInstances();

    // Here begins the creation of all instanced.
    int nX = 1, nY = 1;

    // Get the amount of models in horizontal and vertical direction:
    try {
        CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();
        nX = atoi(pWM->getWindow("mdlviewer/panel/frmMass/hsXVal")->getText().c_str());
        nY = atoi(pWM->getWindow("mdlviewer/panel/frmMass/hsYVal")->getText().c_str());
    } catch(CEGUI::Exception &e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    // Get the width and depth of the model in resting pose.
    ModelInstance *pInst = m_pModelManager->createInstance(m_sModelName);
    float fModelW = pInst->restAABB().right() - pInst->restAABB().left();
    float fModelD = pInst->restAABB().back() - pInst->restAABB().front();
    SAFE_DELETE(pInst);

    // Compute an approximation of the distance we want between models.
    float fModelDX = fModelW * 1.5f;
    float fModelDY = fModelD * 1.5f;

    String sSkin = this->getSelectedSkinName();
    float yPos = -fModelDY*0.5f*static_cast<float>(nY-1);
    for(int y = 0 ; y < nY ; y++) {
        float xPos = -fModelDX*0.5f*static_cast<float>(nX-1);
        for(int x = 0 ; x < nX ; x++) {
            std::unique_ptr<ModelInstance> pInst(m_pModelManager->createInstance(m_sModelName));
            pInst->selectSkin(sSkin);
            m_modelInsts.push_back(std::make_shared<DecorativeMO>(std::move(pInst), Vector(xPos, yPos, 0.0f)));

            xPos += fModelDX;
        }
        yPos += fModelDY;
    }

    return ERR_OK;
}

/** Gets called on a click on the back button. \n
 *
 *  Enters the main menu runlevel.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbBack(const CEGUI::EventArgs & in_ea)
{
    // Go back to the main menu.
    RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv());
    return true;
}

/** Gets called on a click on the load model button. \n
 *
 *  Opens up a dialog that lets the user choose the model file to load.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbLoad(const CEGUI::EventArgs & in_ea)
{
    // Create the object.
    (new FileDlg())->registerHandler(FTS_SUBS(FTS::ModelViewerRlv::cbLoadDone))
                   ->loadOpenDlg("*.ftsmdl", Path::datadir("Models"), InterpretDirWithEntryAsFile::Ptr(new InterpretDirWithEntryAsFile("*.bx*")));

    return true;
}

/** Gets called when the dialog for opening a model file gets closed howsoever. \n
 *
 *  Loads the model and sets up the GUI with the propreties of that model.
 *
 * \param in_ea The CFileDlgEventArgs containing the opened file name.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbLoadDone(const CEGUI::EventArgs & in_ea)
{
    String sFile = static_cast<const FileDlgEventArgs &>(in_ea).getFile();

    // If he clicked on cancel, do nothing.
    if(sFile.empty()) {
        return true;
    }

    // In case a new model has been selected, we first unload all previous ones
    // because it might be the user changes them then reloads them. We want a
    // fresh reload.
    this->destroyModelInstances();
    m_pModelManager->removeModel(m_sModelName);
    m_sModelName = sFile;
    this->setupModelInstances();
    this->setupGUI();

    static std::shared_ptr<MapObject> dummy = std::make_shared<MapObject>(Vector(1.0f, 0.0f, 0.0f));

    InputManager::getSingleton().delShortcut("Translator Test W");
    InputManager::getSingleton().delShortcut("Translator Test W Stopper");
    InputManager::getSingleton().delShortcut("Translator Test S");
    InputManager::getSingleton().delShortcut("Translator Test S Stopper");
    InputManager::getSingleton().delShortcut("Translator Test A");
    InputManager::getSingleton().delShortcut("Translator Test A Stopper");
    InputManager::getSingleton().delShortcut("Translator Test D");
    InputManager::getSingleton().delShortcut("Translator Test D Stopper");
    InputManager::getSingleton().delShortcut("Translator Test Space");
    InputManager::getSingleton().delShortcut("Rotator Test Q");
    InputManager::getSingleton().delShortcut("Rotator Test E");
    InputManager::getSingleton().delShortcut("Orbiter Test Y");
    InputManager::getSingleton().delShortcut("Orbiter Test X");
    Mover* pMoverW = (new TranslatorVel(m_modelInsts.front(), Vector( 0.0f,  0.2f, 0.0f), 0.5f))->pause();
    Mover* pMoverS = (new TranslatorVel(m_modelInsts.front(), Vector( 0.0f, -0.2f, 0.0f), 0.5f))->pause();
    Mover* pMoverA = (new TranslatorVel(m_modelInsts.front(), Vector(-0.2f,  0.0f, 0.0f), 0.5f))->pause();
    Mover* pMoverD = (new TranslatorVel(m_modelInsts.front(), Vector( 0.2f,  0.0f, 0.0f), 0.5f))->pause();
    Mover* pMoverSpace = (new Translator(m_modelInsts.front(), Vector(-1.0f, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f), 10.0f))->pause();
    Mover* pRotatorE = (new Rotator(m_modelInsts.front(), Quaternion::rotation(Vector(1.0f, 0.0f, 0.0f), 90.0f * deg2rad), 1.0f))->pause();
    Mover* pRotatorQ = (new RotatorVel(m_modelInsts.front(), Vector(1.0f, 1.0f, 1.0f), 90.0f * deg2rad, 4.0f))->pause();
    Mover* pOrbiterX = (new OrbiterVel(m_modelInsts.front(), dummy, Vector(0.0f, 0.0f, 1.0f), 90.0f * deg2rad, 10.0f))->pause();
    Mover* pOrbiterY = (new OrbiterVel(m_modelInsts.front(), dummy, Vector(1.0f, 0.0f, 0.0f), 90.0f * deg2rad, 2.0f))->pause();
    InputManager::getSingleton().add("Orbiter Test X", Key::X, pOrbiterX);
    InputManager::getSingleton().add("Orbiter Test Y", Key::Y, pOrbiterY);
    InputManager::getSingleton().add("Rotator Test E", Key::E, pRotatorE);
    InputManager::getSingleton().add("Rotator Test Q", Key::Q, pRotatorQ);
    InputManager::getSingleton().add("Translator Test Space", Key::Space, pMoverSpace);
    InputManager::getSingleton().add("Translator Test W", Key::W, pMoverW);
    InputManager::getSingleton().add("Translator Test W Stopper", Key::W, new MoverStopper(pMoverW), false);
    InputManager::getSingleton().add("Translator Test S", Key::S, pMoverS);
    InputManager::getSingleton().add("Translator Test S Stopper", Key::S, new MoverStopper(pMoverS), false);
    InputManager::getSingleton().add("Translator Test A", Key::A, pMoverA);
    InputManager::getSingleton().add("Translator Test A Stopper", Key::A, new MoverStopper(pMoverA), false);
    InputManager::getSingleton().add("Translator Test D", Key::D, pMoverD);
    InputManager::getSingleton().add("Translator Test D Stopper", Key::D, new MoverStopper(pMoverD), false);

    try {
        CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();

        if(!m_modelInsts.empty()) {
            ModelInstance* pInst = (*m_modelInsts.begin())->getModelInst();

            // And also model-info like verts, frames.
            pWM->getWindow("mdlviewer/statusbar/lblVertsVal")->setText(String::nr(pInst->vertexCount()));
            pWM->getWindow("mdlviewer/statusbar/lblFaceVal")->setText(String::nr(pInst->faceCount()));
            pWM->getWindow("mdlviewer/statusbar/lblMovesVal")->setText(String::nr(pInst->moves().size()));
        }
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    // This is for our halloween e a s t e r   e g g.
    // When the pumpkin model is loaded, play the zombie music :)
    DaoFunctionCall<>("HalloweenEasterEgg")(m_sModelName);

    return true;
}

/** Returns the name of the currently selected move.
 *
 * \return The name of the currently selected move ; String::EMPTY if there was
 *         an error.
 *
 * \author Pompei2
 */
String FTS::ModelViewerRlv::getSelectedMoveName() const
{
    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMoves/cbMove");
        return cb->getText();
    } catch(CEGUI::Exception &e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return String::EMPTY;
}

/// \return The currently selected move-speed, normalized, that is 1.0 means 100%
float FTS::ModelViewerRlv::getSelectedMoveSpeed() const
{
    try {
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "mdlviewer/panel/frmMoves/hsSpeed");
        return sb->getScrollPosition() / 100.0f;
    } catch(CEGUI::Exception &e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return 1.0f;
}

/// \return The currently selected move-priority, normalized, that is 1.0 means 100%
float FTS::ModelViewerRlv::getSelectedMovePrio() const
{
    try {
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "mdlviewer/panel/frmMoves/hsPrio");
        return sb->getScrollPosition() / 100.0f;
    } catch(CEGUI::Exception &e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return 0.5f;
}

/** Returns the name of the currently selected skin.
 *
 * \return The name of the currently selected skin ; String::EMPTY if there was
 *         an error.
 *
 * \author Pompei2
 */
String FTS::ModelViewerRlv::getSelectedSkinName() const
{
    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMisc/cbSkin");
        return cb->getText();
    } catch(CEGUI::Exception &e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return String::EMPTY;
}

/** Gets called when one scrollbar changes its position. \n
 *
 *  This method adapts the data dynamically to scrollbar changes. That means
 *  first that it changes the number of the label called the same as the
 *  scrollbar that changed with a suffix "Val", then it does some action:\n
 *     - For the massive rendering scrollbars, it re-creates the instances.
 *     - For the movespeed scrollbar, it adapts the movespeed of all instanced.
 *
 * \param in_ea The CEGUI::WindowEventArgs that hold info about the scrollbar.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbhsScrollChanged(const CEGUI::EventArgs & in_ea)
{
    const CEGUI::WindowEventArgs &wea = static_cast<const CEGUI::WindowEventArgs &>(in_ea);

    try {
        FTSConvertWinMacro(CEGUI::Scrollbar, sb, wea.window);

        // Avoid values lower then one.
        if(sb->getScrollPosition() < 1.0f) {
            sb->setScrollPosition(1.0f);
            return true;
        }

        // Find the corresponding label and set its value.
        CEGUI::String sName = wea.window->getName();
        sName += "Val";
        sb->getParent()->getChild(sName)->setText(String::nr((int)sb->getScrollPosition()));

        if(sName == "mdlviewer/panel/frmMass/hsXVal" || sName == "mdlviewer/panel/frmMass/hsYVal") {
            // Add new models or delete some.
            this->setupModelInstances();
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

/** Gets called when the user clicks on the "play as action" button. \n
 *
 *  Plays the currently selected move as an action, i.e. overlays it to any
 *  currently played animation. Plays it only once, using the currently set
 *  animation speed.
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbPlayAction(const CEGUI::EventArgs &)
{
    // Get the currently selected move name and move speed.
    String sName = this->getSelectedMoveName();
    float fSpeed = this->getSelectedMoveSpeed();

    for(auto i = m_modelInsts.begin() ; i != m_modelInsts.end() ; ++i) {
        (*i)->getModelInst()->playAction(sName, fSpeed);
    }

    // Again, for our halloween   e a s t e r   e g g:
    // When playing the "Jump" of the "Pumpkin", scream!
    if(m_sModelName.contains("Pumpkin") && sName == "Jump") {
        ISndSys::getSingleton().getSndObj(SndGroup::UnitReaction, "scream1.ogg")->Play();
    }

    return true;
}

/** Gets called when the user clicks on the queue-move button \n
 *
 *  Inserts the currently selected move into the move queue of all the model
 *  instances, so that this move will be played when the model is done playing
 *  all the other moves that are already in the queue.
 *
 * \param in_ea unused.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbPlayCycle(const CEGUI::EventArgs & in_ea)
{
    // Get the currently selected move name and move speed.
    String sName = this->getSelectedMoveName();
    float fSpeed = this->getSelectedMoveSpeed();

    for(auto i = m_modelInsts.begin() ; i != m_modelInsts.end() ; ++i) {
        (*i)->getModelInst()->playCycle(sName, fSpeed);
    }

    return true;
}

/** Gets called when the user clicks on the play/pause button \n
 *
 *  This either pauses or plays all the model instances' current move.
 *
 * \param in_ea unused.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbPlayPause(const CEGUI::EventArgs & in_ea)
{
    for(auto i = m_modelInsts.begin() ; i != m_modelInsts.end() ; ++i) {
        if((*i)->getModelInst()->paused())
            (*i)->getModelInst()->resume();
        else
            (*i)->getModelInst()->pause();
    }

    // Now we need to change the icon on the button accordingly:
    try {
        CEGUI::Window* pButton = CEGUI::WindowManager::getSingletonPtr()->getWindow("mdlviewer/panel/frmMoves/btnPlayPause");
        if(m_modelInsts.front()->getModelInst()->paused()) {
            pButton->setProperty("ImageNormal", "set:FTSUI image:ResumeN");
            pButton->setProperty("ImageHover", "set:FTSUI image:ResumeH");
            pButton->setProperty("ImagePushed", "set:FTSUI image:ResumeN");
            pButton->setProperty("ImageDisabled", "set:FTSUI image:ResumeN");
        } else {
            pButton->setProperty("ImageNormal", "set:FTSUI image:PauseN");
            pButton->setProperty("ImageHover", "set:FTSUI image:PauseH");
            pButton->setProperty("ImagePushed", "set:FTSUI image:PauseN");
            pButton->setProperty("ImageDisabled", "set:FTSUI image:PauseN");
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

/** Gets called when the user clicks on the stop button \n
 *
 *  This stops all the model instances' current move and clears their move queue.
 *
 * \param in_ea unused.
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::ModelViewerRlv::cbStop(const CEGUI::EventArgs & in_ea)
{
    float fSpeed = this->getSelectedMoveSpeed();

    for(auto i = m_modelInsts.begin() ; i != m_modelInsts.end() ; ++i) {
        (*i)->getModelInst()->stopAll(fSpeed - 0.01f);
    }

    return true;
}

bool FTS::ModelViewerRlv::cbSkinChanged(const CEGUI::EventArgs& in_ea)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMisc/cbSkin");

        for(auto i = m_modelInsts.begin() ; i != m_modelInsts.end() ; ++i) {
            (*i)->getModelInst()->selectSkin(cb->getText());
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

bool FTS::ModelViewerRlv::cbPlayerColChanged(const CEGUI::EventArgs& in_ea)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMisc/cbPlayerCol");
        m_playerColor = Color(cb->getText());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

bool FTS::ModelViewerRlv::cbShowAABBChanged(const CEGUI::EventArgs& in_ea)
{
    try {
        FTSGetConvertWinMacro(CEGUI::Checkbox, cb, "mdlviewer/panel/frmMisc/chkAABB");
        m_bShowAABB = cb->isSelected();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}


 /* EOF */
