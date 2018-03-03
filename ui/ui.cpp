#include "ui/ui.h"

#include "3d/3d.h" // For the cegui renderer included below.

#include "ui/ui_menu.h"
#include "ui/ui_menu_online.h"
#include "ui/ui_menu_online_main.h"
#include "ui/ui_commands.h"

#include "logging/logger.h"
#include "main/Clock.h"
#include "graphic/graphic.h"
#include "utilities/utilities.h"
#include "utilities/fps_calculator.h"

#include <CEGUI.h>
#include <openglrenderer.h>

using namespace FTS;

FTS::GUI::GUI()
    : m_pActiveWidget(NULL)
    , m_pCurPopupMenu(NULL)
    , m_currPolyMode(Filled)
    , m_currGUIInfo(NoInfo)
    , m_bTabbing(false)
{
    this->setActiveWidget(NULL);
}

FTS::GUI::~GUI()
{
}

/// This loads a font for use in FTS.
/** To be able to completely load, the font must be present in 6 files:\n
 *  FontNameX.font where X ranges from 1-5, that is the font in sizes from
 *  little to big, 2 being the normal size.\n
 *  FontNameNoCol.font where the font is, in normal size, to be used when
 *  no colours are allowed (usually the same as FontName2.font).
 *
 * \param in_sFontName The name of the font to load.
 *
 * \author Pompei2
 **/
void FTS::GUI::loadFont(const String &in_sFontName)
{
    // Load 2 before 1, so 2 will be the default.
    this->loadFontFile(in_sFontName + "2.font");
    this->loadFontFile(in_sFontName + "1.font");
    this->loadFontFile(in_sFontName + "3.font");
    this->loadFontFile(in_sFontName + "4.font");
    this->loadFontFile(in_sFontName + "5.font");
    this->loadFontFile(in_sFontName + "NoCol.font", false);
}

/// For internal use only. Better use "loadFont" function.
/** Loads one CEGUI font file and sets its attributes.
 *
 * \param in_sFileName The name of the font CEGUI file to load.
 * \param in_bColoured Enable colours (with |cXXXXXXXX) in the font or not ?
 * \param in_bTabbed Enable tabs (with \t) in the font or not ?
 *
 * \author Pompei2
 **/
CEGUI::Font *FTS::GUI::loadFontFile(const String &in_sFileName, bool in_bColoured)
{
    CEGUI::Font *f;

    // First, just load the font.
    try {
        f = CEGUI::FontManager::getSingleton().createFont(in_sFileName);
    } catch(CEGUI::Exception& e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return NULL;
    }

    // Add support for colour if wanted.
    try {
        if(in_bColoured) {
            f->setTriggersEnabled(true);
            f->addTrigger(new CEGUI::ColourStartFontTrigger);
            f->addTrigger(new CEGUI::ColourStopFontTrigger);
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return f;
}

/** This creates the CEGUI root window (just an empty default window.).
 *
 * \author Pompei2
 **/
void FTS::GUI::createRootWindow()
{
    // Create THE root window (just a default empty window that takes the
    // whole screen space and draws nothing (only its children)).
    CEGUI::Window *pTHERootWin = CEGUI::WindowManager::getSingleton().
                                    createWindow("DefaultWindow", "root");
    m_pWinHistory.push_back(pTHERootWin);
    CEGUI::System::getSingleton().setGUISheet(pTHERootWin);
}

bool cbCEGUIProp(CEGUI::Window* window,
                 CEGUI::String& propname,
                 CEGUI::String& propvalue,
                 void* userdata)
{
    return true;
}

/// Load a CEGUI window (.layout file)
/** This function loads the layout file specified by in_sName, and if you want
 *  it, shows it too. in_sName Is the pure name, withoute extension. The real
 *  file that will be loaded is LAYOUT_DIR/in_sName.[lang].layout
 *
 * \param in_sName The pure name of the layout to load.
 *
 * \return If successfull: A pointer to the CEGUI window that gots loaded.
 * \return If failed:      NULL
 *
 * \author Pompei2
 */
CEGUI::Window * FTS::GUI::loadLayout(const String & in_sName,
                                   bool in_bShow,
                                   bool in_bActivate,
                                   bool in_bModal)
{
    CEGUI::Window * w = NULL;

    if(!in_sName) {
        FTS18N("InvParam", MsgType::Horror, "FTS::GUI::loadLayout");
        return NULL;
    }

    try {
        w = CEGUI::WindowManager::getSingleton().
            loadWindowLayout(in_sName/*, "", "", &cbCEGUIProp, 0*/);

        // This may happen for empty files for example.
        if(w == NULL) {
            FTS18N("InvParam", MsgType::Horror, "FTS::GUI::loadLayout");
            return NULL;
        }

        // And put the window into the system.
        w->setVisible(in_bShow);
        CEGUI::System::getSingleton().getGUISheet()->addChildWindow(w);
        if(in_bActivate) {
            w->activate();
            this->setActiveWidget(NULL);
        }
        w->setModalState(in_bModal);
    }
    catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_LoadLayout", MsgType::Error, in_sName, e.getMessage());
        return NULL;
    }

    return w;
}

/// Called everytime a window gets activated.
/** This function is called by CEGUI every time that a window gets the focus.
 *
 *  Currently, we use this function to keep track of the order that windows where
 *  active, so when closing a window we can activate the last one, as this isn't
 *  done automatically by CEGUI.
 *
 * \param e The event arguments.
 *
 * \return Always true.
 *
 * \author Pompei2
 */
bool FTS::GUI::onWindowActivated(const CEGUI::EventArgs & ea)
{
    try {
        const CEGUI::WindowEventArgs & we =
            static_cast < const CEGUI::WindowEventArgs & >(ea);
        CEGUI::Window * pLastWin = m_pWinHistory.back();

        bool bDoAlpha = true;

        // We only work with FrameWindows.
        if(!we.window->testClassName(CEGUI::FrameWindow::EventNamespace)) {

            // If it is not a frame window, we need to check if we want to set the
            // Widget as an active (for tab ordering)
            if(we.window->getID() == 0 ||
               !we.window->isVisible() ||
               we.window->isDisabled() ||
               we.window->getName().find(CEGUI::Window::AutoWidgetNameSuffix) != CEGUI::String::npos ||
               FTS::GUI::isContainer(we.window) )
                return true;

            // Here we don't call this->setActiveWidget to avoid infinite recursion.
            m_pActiveWidget = we.window;
            return true;
        }

        // If it is the same window as before, ignore it.
        if(pLastWin == we.window)
            return true;

#ifdef DEBUG
        String sBlaa = we.window->getName();

        FTSMSGDBG("Activating window " + sBlaa, 4);
        if( pLastWin )
            sBlaa = pLastWin->getName();
#endif

        if(bDoAlpha) {
            if( pLastWin != CEGUI::System::getSingleton().getGUISheet() ) {
                // The inactive windows gets 25% alpha, as long as it isn't the root window.
                pLastWin->setProperty("Alpha", "0.25");
            }

            // The active window gets 100% alpha.
            we.window->setProperty("InheritsAlpha", "false");
            we.window->setProperty("Alpha", "1.0");
        }

        m_pWinHistory.push_back(we.window);

        // TABBING SETTINGS
        this->setActiveWidget(NULL);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Called everytime a window gets closed.
/** This function is called by CEGUI just before a window gets closed.
 *
 *  Currently, we use this function to activate the previous window
 *  when closing this one, as it's not done automatically by CEGUI.
 *
 * \param e The event arguments.
 *
 * \return Always true.
 *
 * \author Pompei2
 */
bool FTS::GUI::onWindowClosed(const CEGUI::EventArgs & ea)
{
    try {
        const CEGUI::WindowEventArgs & we =
            static_cast < const CEGUI::WindowEventArgs & >(ea);
        CEGUI::Window * pLastWin = NULL;

        // We only work with FrameWindows.
        if(!we.window->testClassName(CEGUI::FrameWindow::EventNamespace))
            return true;

#ifdef DEBUG
        String sBlaa = we.window->getName();

        FTSMSGDBG("Closing window " + sBlaa, 4);
#endif

        // Delete the current window from the list.
        m_pWinHistory.remove(we.window);

        // Get the last window, activate it and reset it's alpha to full opacity.
        pLastWin = m_pWinHistory.back();
        pLastWin->activate();
        pLastWin->setProperty("Alpha", "1.0");
#ifdef DEBUG
        sBlaa = pLastWin->getName();
#endif

        this->setActiveWidget(NULL);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// Opens a popup menu and closes the previous one, if there was one.
/** This function closes the popup menu that has been previously opened,
 *  and opens this popup menu.
 *
 * \param in_pPopup The menu to open.
 *
 * \return ERR_OK.
 *
 * \author Pompei2
 */
int FTS::GUI::openPopupMenu(CEGUI::PopupMenu *in_pPopup)
{
    this->closeCurrentPopupMenu();

    in_pPopup->openPopupMenu(false);
    m_pCurPopupMenu = in_pPopup;
    return ERR_OK;
}

/// Closes the current popup menu.
/** This closes the current popup menu.
 *
 * \return ERR_OK.
 *
 * \author Pompei2
 */
int FTS::GUI::closeCurrentPopupMenu()
{
    if(m_pCurPopupMenu) {
        m_pCurPopupMenu->closePopupMenu(false);
        m_pCurPopupMenu = NULL;
    }

    return ERR_OK;
}

/// \param in_mode The Polygon mode you want to activate.
void FTS::GUI::set(const PolyMode &in_mode)
{
    switch (in_mode) {
    case Filled:    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  break;
    case Wireframe: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  break;
    case Points:    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
    default: return;
    }
    m_currPolyMode = in_mode;
}

/// \return The next Polygon mode that has to be displayed.
FTS::GUI::PolyMode FTS::GUI::next(const FTS::GUI::PolyMode &in_mode) const
{
    switch (in_mode) {
    case Filled: return Wireframe;
    case Wireframe: return Points;
    case Points: return Filled;
    default: return Filled;
    }
}

/// \return The next GUI info that has to be displayed.
FTS::GUI::GUIInfo FTS::GUI::next(const GUIInfo &in_info) const
{
    switch(in_info) {
    case NoInfo: return FPS;
    case FPS: return NoInfo;
    };
    return FPS;
}

/// Updates the GUI Info field of CEGUI, if there is one.
extern size_t g_nAllocs;
extern size_t g_iTotalMem;
void FTS::GUI::updateGUIInfo()
{
    CEGUI::Window *pWin = NULL;

    // First try if that window does exist. if it doesn't, just stop here.
    try {
        pWin = CEGUI::WindowManager::getSingleton().getWindow("Info");
    } catch(CEGUI::Exception & ) {
        return;
    }

    CEGUI::String sCInfo;
    String sInfo;

    switch(this->getGUIInfo()) {
    case NoInfo:
        break;
    case FPS:
        {
            sInfo = "FPS : "+String::nr(FPSCalculator::getSingleton().getFPS(),1);
            sCInfo = sInfo;
            sCInfo.replace(4, 1, 1, 0x2245);
        }
        break;
    }

    try {
        if(pWin && pWin->getText() != sCInfo) {
            pWin->setText(sCInfo);
        }
    } catch(CEGUI::Exception & ) {
    }
}

/// Selects a new active widget.
/** This function sets the given widget as being the currently active widget,
 *  the one with the blue dotted line around it and getting all keyboard input.
 *
 * \param in_pWidget The widget to set as active. May also be NULL to set no active
 *                   widget.
 *
 * \author Pompei2
 */
void FTS::GUI::setActiveWidget(CEGUI::Window *in_pWidget)
{
    if(in_pWidget != NULL)
        in_pWidget->activate();
    else if(m_pActiveWidget != NULL)
        m_pActiveWidget->deactivate();
    m_pActiveWidget = in_pWidget;
}

/** The last chance for the GUI to grab its data before a resolution change.
 *
 * \throws CEGUI::Exception for anything that might go wrong.
 */
void FTS::GUI::preChangeResolution(float /*in_fW*/, float /*in_fH*/)
{
    if(CEGUI::System::getSingletonPtr()) {
        CEGUI::OpenGLRenderer *gl = (CEGUI::OpenGLRenderer *) CEGUI::System::getSingleton().getRenderer();
        gl->grabTextures();
    }
}

/** Chance for the GUI to restore its data after a resolution change.
 *
 * \throws CEGUI::Exception for anything that might go wrong.
 */
void FTS::GUI::postChangeResolution(float in_fW, float in_fH)
{
    if(CEGUI::System::getSingletonPtr()) {
        CEGUI::OpenGLRenderer *gl = (CEGUI::OpenGLRenderer *) CEGUI::System::getSingleton().getRenderer();
        if(gl) {
            gl->restoreTextures();
            gl->setDisplaySize(CEGUI::Size(in_fW, in_fH));
        }
    }
}

#ifdef DEBUG
/// Prints all windows to the console.
/** This function recursively prints a tree of all child windows of in_pRoot,
 *  or if it is NULL, of all windows present in the scene.
 *
 * \param in_pRoot   the window you want the childs to be print.
 * \param in_iIndent the spaces to put before it.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 */
#include <iostream>
int FTS::printWindowHierarchy(CEGUI::Window * in_pRoot, int in_iIndent)
{
    if(in_pRoot == NULL) {
        CEGUI::Window *pWin = CEGUI::System::getSingleton().getGUISheet();
        CEGUI::WindowManager::getSingleton().
            writeWindowLayoutToStream( *pWin, std::cout, true);
    } else {
        CEGUI::WindowManager::getSingleton().
            writeWindowLayoutToStream(*in_pRoot, std::cout, true);
    }

    return ERR_OK;
}
#endif // DEBUG

/// Checks if the window is a container style window or not.
/** This function looks if the window is a container style window, this is currently
 *  a StaticFrame, a DefaultWindow or a DefaultGUISheet.\n
 *  If yes, returns true, else returns false.
 *
 * \param in_pWindow The window to check.
 *
 * \return true if it is a container window, else false.
 *
 * \author Pompei2
 */
bool FTS::GUI::isContainer(CEGUI::Window *in_pWindow)
{
    if((in_pWindow->getType().find(CEGUI::String("StaticFrame")) != CEGUI::String::npos) ||
       (in_pWindow->getType().find(CEGUI::String("DefaultWindow")) != CEGUI::String::npos) ||
       (in_pWindow->getType().find(CEGUI::String("DefaultGUISheet")) != CEGUI::String::npos) ||
       (in_pWindow->getType().find(CEGUI::String("ContainerBox")) != CEGUI::String::npos) ||
       (in_pWindow->getType().find(CEGUI::String("PopupMenu")) != CEGUI::String::npos)
      ) {
        return true;
    }

    return false;
}

/// Recursively checks if a window is the child of another window.
/** This function recurses into in_pRoot and compares all its childs
 *  recursively with in_pNeedle. If the window is found, it returns true.
 *
 * \param in_pRoot The window where to begin the search.
 * \param in_pNeedle The window to search.
 *
 * \return true if in_pRoot contains in_pNeedle, else false.
 *
 * \author Pompei2
 */
bool FTS::GUI::isChildRecursive(CEGUI::Window *in_pRoot, CEGUI::Window *in_pNeedle)
{
    for(size_t i = 0 ; i < in_pRoot->getChildCount() ; i++) {
        CEGUI::Window *pChild = in_pRoot->getChildAtIdx(i);

        if(pChild == in_pNeedle)
            return true;

        if(isChildRecursive(pChild, in_pNeedle))
            return true;
    }

    return false;
}

bool CEGUIUpdater::update(const FTS::Clock& c)
{
    // inject the time that passed since the last loop
    try {
        CEGUI::System::getSingleton().injectTimePulse(static_cast<float>(c.getDeltaT()));
    } catch(CEGUI::Exception &) { }
    return true;
}

/// Add a shortcut for this dialog.
/** This is a little helper method that lets you add a key to trigger a callback,
 *  if this window has the focus.
 *
 * \param in_kCombo The key you want to register as a shortcut.
 * \param in_subs the callback method to call.
 *
 * \note You may use the FTS_SUBS macro to specify a callback.
 *
 * \author Pompei2
 */
void FTS::Dlg::addShortcut(const Key::Enum &in_kCombo, CEGUI::Event::Subscriber in_subs)
{
    InputManager *pMgr = InputManager::getSingletonPtr();
    ActiveWindowCheckCmd *pCond = NULL;
    CallbackCommand *pCbCmd = NULL;

    // Determine the name of the hotkey.
    String sName = m_pRoot->getName() + " " + String::nr(m_nHotkeys);
    m_nHotkeys++;

    // The action shall be conditional, only execute if this window is active.
    pCond = new ActiveWindowCheckCmd(m_pRoot->getName());

    // The action should callback the event subscriber given as argument.
    pCbCmd = new CallbackCommand(in_subs);

    // Add it.
    pMgr->add(sName, in_kCombo, new ConditionalCommand(pCond, pCbCmd));
}

/// Add a shortcut for this dialog.
/** This is a little helper method that lets you add a key to trigger a callback,
 *  if this window has the focus.
 *
 * \param in_kCombo The key you want to register as a shortcut.
 * \param in_subs the callback method to call.
 *
 * \note You may use the FTS_SUBS macro to specify a callback.
 *
 * \author Pompei2
 */
void FTS::Dlg::addShortcut(const SpecialKey::Enum &in_kCombo, CEGUI::Event::Subscriber in_subs)
{
    InputManager *pMgr = InputManager::getSingletonPtr();
    ActiveWindowCheckCmd *pCond = NULL;
    CallbackCommand *pCbCmd = NULL;

    // Determine the name of the hotkey.
    String sName = m_pRoot->getName() + " " + String::nr(m_nHotkeys);
    m_nHotkeys++;

    // The action shall be conditional, only execute if this window is active.
    pCond = new ActiveWindowCheckCmd(m_pRoot->getName());

    // The action should callback the event subscriber given as argument.
    pCbCmd = new CallbackCommand(in_subs);

    // Add it.
    pMgr->add(sName, in_kCombo, new ConditionalCommand(pCond, pCbCmd));
}

/// Default constructor
/** This is the default constructor that creates the dialog, sets
 *  up all callbacks etc.
 *
 * \param in_sLayoutName The name of the layout to load (the filename but
 *                       without language and extension)
 *
 * \note You should check the value of m_pRoot for NULL in the constructor
 *       of your derived class to see if the loading went fine.
 *
 * \author Pompei2
 */
FTS::Dlg::Dlg(const String in_sLayoutName)
{
    m_nHotkeys = 0;

    CEGUI::Window *pLastActive = GUI::getSingleton().getActiveWin();
    m_sLastActive = pLastActive ? pLastActive->getName() : "None";
    m_bLastActiveWasModal = pLastActive ? pLastActive->getModalState() : false;

    m_pRoot = GUI::getSingleton().loadLayout(in_sLayoutName, true, true, true);

    this->addShortcut(SpecialKey::Enter, FTS_SUBS(FTS::Dlg::cbOk));
    this->addShortcut(Key::Escape, FTS_SUBS(FTS::Dlg::cbCancel));
}

/// Default destructor
/** This is the default destructor that deletes the dialog
 *  and maybe gives the modal state back to the parent.
 *  It also removes all shortcuts (hotkeys) that were registered by this dialog.
 *
 * \author Pompei2
 */
FTS::Dlg::~Dlg()
{
    if(!m_pRoot)
        return ;

    // Unregister all shortcuts.
    String sName;
    for(uint32_t i = 0 ; i < m_nHotkeys ; i++) {
        sName = m_pRoot->getName() + " " + String::nr(i);
        InputManager::getSingleton().delShortcut(sName);
    }

    try {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_pRoot);
        if(m_sLastActive != "None" && m_bLastActiveWasModal) {
            try {
                CEGUI::WindowManager::getSingleton().getWindow(m_sLastActive)->setModalState(true);
            } catch(CEGUI::Exception &) {
                ;
            }
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

bool Dlg::cbCancel(const CEGUI::EventArgs& in_ea)
{
    delete this;
    return true;
}

bool Dlg::cbOk(const CEGUI::EventArgs& in_ea)
{
    return this->cbCancel(in_ea);
}

/// Utility function to disable all children of a widget non-recursively.
/** This function disables all children of a widget non-recursively, but does not
 *  disable the widget itself. Also, it will not disable MultiLineEditboxes,
 *  but set them to read only so the user is still able to scroll around.
 *
 * \return ERR_OK on success, an error code < 0 on failure.
 *
 * \author Pompei2
 **/
int FTS::disableAllChildren(CEGUI::Window *in_pRoot)
{
    try {
        // Disable all children ...
        for(size_t i = 0; i < in_pRoot->getChildCount(); i++) {
            CEGUI::Window *pChild = in_pRoot->getChildAtIdx(i);

            // ... but the editboxes and the ...
//             if(pChild->getType() == "ArkanaLook/Editbox") {
//                 FTSConvertWinMacro(CEGUI::Editbox, pE, pChild);
//                 pE->setReadOnly(true);
            // ... multiline editboxes should be enabled but read only so that
            // one can still scroll.
            /*} else */if(pChild->getType() == "ArkanaLook/MultiLineEditbox") {
                FTSConvertWinMacro(CEGUI::MultiLineEditbox, pMLE, pChild);
                pMLE->setReadOnly(true);
            } else {
                pChild->disable();
            }
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

 /* EOF */
