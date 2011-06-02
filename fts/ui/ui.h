/**
 * \file ui.h
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file contains all about the FTS User Interface.
 **/

#ifndef FTS_UI_H
#define FTS_UI_H

#include "main.h"

#include "input/input.h"
#include "utilities/command.h"
#include "utilities/Singleton.h"
#include "dLib/dString/dString.h"
#include "main/Updateable.h"

#include <CEGUIExceptions.h>
#include <CEGUIEvent.h>
#include <CEGUIEventArgs.h>
#include <CEGUIString.h>
#include <CEGUIWindowManager.h>

#include <list>

namespace CEGUI {
    class Window;
    class Imageset;
    class OpenGLRenderer;
    class Font;
};

namespace FTS {
#ifdef DEBUG
int printWindowHierarchy(CEGUI::Window * in_pRoot, int in_iIndent);
#endif
int disableAllChildren(CEGUI::Window *in_pRoot);

/// The FTS User Interface class
/** This is the user interface class, this interface controls nearly everything:
 *  the menus, the game ... just not the demos.
 **/
class GUI : public Singleton<GUI> {
private:
    // --- Variables --- //
    // CEGUI variables.
    CEGUI::Window *m_pActiveWidget;          ///< The currently active widget (that gots the focus).
    std::list<CEGUI::Window *>m_pWinHistory; ///< The history of the active windows.
    CEGUI::PopupMenu *m_pCurPopupMenu;       ///< The currently opened popup menu (or NULL).

    /// The possible OpenGL rendering modes
    typedef enum {
        Filled = 0,  ///< Render in filled polygons.
        Wireframe,   ///< Render in wireframe.
        Points,      ///< Render only the vertices as points.
    } PolyMode;

    /// The current OpenGL rendering mode (wireframe, ...)
    PolyMode m_currPolyMode;

    /// These are the possible information shown in the GUI at the top right,
    /// like FPS, memory used, ... for example.
    typedef enum {
        NoInfo = 0, ///< Shows nothing.
        FPS,        ///< Shows the current frames per seconds.
    } GUIInfo;

    /// The current info displayed on the top right of the GUI.
    GUIInfo m_currGUIInfo;

    /// An own clipboard in case the system's clipboard doesn't work.
    /// Most of the times, on linux, if Klipper is running and not empty, the
    /// SDL clipboard doesn't seem to work...
    FTS::String m_sOwnClipboard;

public:
    // ===== TEST ITEMS ===== //

public:
    // ===== FUNCTIONS ===== //
    GUI();                  ///< Constructor.
    virtual ~GUI();         ///< Destructor.

    void loadFont(const String &in_sFontName);
    void createRootWindow();

    // Other CEGUI related functions.
    CEGUI::Window * loadLayout(const String & in_sName,
                               bool in_bShow = true,
                               bool in_bActivate = true,
                               bool in_bModal = false);

    // These are used to keep track of the active window and to reactivate the previous one
    // If the currently active one is closed.
    bool onWindowActivated(const CEGUI::EventArgs & ea);
    bool onWindowClosed(const CEGUI::EventArgs & ea);

    // Handle popup menus.
    int openPopupMenu(CEGUI::PopupMenu *in_pPopup);
    int closeCurrentPopupMenu(void);
    inline CEGUI::PopupMenu *getCurrentPopupMenu(void) {return m_pCurPopupMenu;};

    void set(const PolyMode &in_mode);
    /// \return The GUI Info that is currently displayed.
    inline PolyMode getPolyMode() const {return m_currPolyMode;};
    PolyMode next(const PolyMode &in_mode) const;

    /// \param in_info The GUI Info you want to be displayed.
    inline void set(const GUIInfo &in_info) {m_currGUIInfo = in_info;};
    /// \return The GUI Info that is currently displayed.
    inline GUIInfo getGUIInfo() const {return m_currGUIInfo;};
    GUIInfo next(const GUIInfo &in_info) const;
    void updateGUIInfo();

    // Get/Set functions.
    inline CEGUI::Window *getActiveWin(void) {return m_pWinHistory.back();};
    inline CEGUI::Window *getActiveWidget(void) {return m_pActiveWidget;};

    void setActiveWidget(CEGUI::Window *w);
    void tabbing(bool in_bTabbing) {m_bTabbing = in_bTabbing;};
    bool tabbing() {return m_bTabbing;};

    void preChangeResolution(float in_w, float in_h);
    void postChangeResolution(float in_w, float in_h);

    // GUI related utility functions.
    static bool isContainer(CEGUI::Window *in_pWindow);
    static bool isChildRecursive(CEGUI::Window *in_pRoot, CEGUI::Window *in_pNeedle);
private:
    CEGUI::Font *loadFontFile(const String &in_sFileName, bool in_bColoured = true);
    bool m_bTabbing; ///< Is the user navigating through the GUI using Tab?
};

/// This class will do the time update CEGUI needs to update its internal states
/// that depend on time, like fading stuff, timing doubleclick etc.
class CEGUIUpdater : public Updateable {
public:
    bool update(const Clock&);
};

/// Little macro to write less.
/// \param x The name of the method to use, for example: CFTSUI::init
#define FTS_SUBS(x) (CEGUI::Event::Subscriber(&x, this))
#define FTS_SUBS_THIS(x, t) (CEGUI::Event::Subscriber(&x, t))

/// Get a CEGUI window and cast it safely to the type it has.
/** This is a little helper function that gets a window from the CEGUI system
 *  and casts it (dynamically) to a type you specify. If the window in fact
 *  isn't of that type, a CEGUI::Exception is thrown with the according error
 *  message in it.
 *
 * \param in_sWindowName The name of the window you want to get.
 * \param in_sTypeName The name of the type as a string (for the error message).
 *
 * \note You may use the FTSGetConvertWinMacro macro that is more easy to use.
 *
 * \author Pompei2
 */
template<class CEGUIType>
CEGUIType *FTSGetConvertWin(const String &in_sWindowName, const String &in_sTypeName)
{
    CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();
    if(pWM == NULL) {
        throw(CEGUI::InvalidRequestException("Want to get a window but CEGUI not initialised"));
    }

    CEGUIType *pWin = dynamic_cast<CEGUIType *>(pWM->getWindow(in_sWindowName));
    if(pWin == NULL) {
        throw(CEGUI::InvalidRequestException(String("Bad cast: The window named {1} should be of type {2}").fmt(in_sWindowName, in_sTypeName)));
    }

    return pWin;
}

/// Little macro to write less.
/** This is a macro so you need to write less if you want to use the
 *  FTSGetConvertWin function. See this function's documentation for more details.
 *
 * \param type The type of the window (without the * for pointer.)
 * \param var The name of the variable to create to hold the window in it.
 * \param name The name of the window to get.
 *
 * \throws CEGUI::Exception if the window with that name does not exist or is
 *                          not of the awaited type.
 *
 * \Example
 *       \code
 *        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "mdlviewer/panel/frmMoves/cbMove");
 *        cb->resetList();
 *       \endcode
 */
#define FTSGetConvertWinMacro(type, var, name) type *var = FTSGetConvertWin<type>(name, # type)

/// Little macro to write less.
/** This is a macro so you need to write less if you want to safely convert a
 *  CEGUI window * into something else.
 *
 * \param type The type you want to convert it to (without the * for pointer.)
 * \param var The name of the variable to create to hold the window in it.
 * \param win Pointer to the actial CEGUI::Window to convert.
 *
 * \throws CEGUI::Exception if the window with that name does not exist or is
 *                          not of the awaited type.
 *
 * \Example
 *       \code
 *        CEGUI::Window *pWin = something;
 *        FTSConvertWinMacro(CEGUI::Combobox, cb, pWin);
 *        cb->resetList();
 *       \endcode
 */
#define FTSConvertWinMacro(type, var, win) type *var = dynamic_cast<type *>(win); \
    if((var) == NULL) { \
        throw(CEGUI::InvalidRequestException("Bad cast: The window named "+(win)->getName()+" should be of type " # type)); \
    }

/// A dialog baseclass. You may subclass it to create your own dialog.
/// See the dokuwiki for more details.
class Dlg {
protected:
    CEGUI::Window * m_pRoot;       ///< A pointer to this dialog's window.

    CEGUI::String m_sLastActive;   ///< The window that was active before this dialog being created.
    bool m_bLastActiveWasModal;    ///< Whether the last window was modal or not.

    uint32_t m_nHotkeys;           ///< The amount of registered hotkeys for this dialog.

    void addShortcut(const Key::Enum &in_kCombo, CEGUI::Event::Subscriber in_subs);
    void addShortcut(const SpecialKey::Enum &in_kCombo, CEGUI::Event::Subscriber in_subs);

    virtual bool cbCancel(const CEGUI::EventArgs& in_ea);
    virtual bool cbOk(const CEGUI::EventArgs& in_ea);

public:
    Dlg(const String in_sLayoutName);
    virtual ~Dlg();
};

} // namespace FTS

#endif                          /* FTS_UI_H */

 /* EOF */
