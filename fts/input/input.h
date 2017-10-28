#ifndef FTS_INPUT_H
#define FTS_INPUT_H

#include "main.h"

#include "InputConstants.h"

#include "dLib/dString/dString.h"
#include "utilities/command.h"
#include "utilities/Singleton.h"
#include "main/Updateable.h"
#include "main/Stackable.h"

#include <CEGUIInputEvent.h>
#include <SDL_events.h>

#include <list>
#include <set>
#include <stack>

namespace FTS {
class String;
class Graphic;
class Anim;

#define FTS_CURSOR_IMAGES 4

/// This structure defines a cursor.
struct SCursor {
    /** If the cursor is animated, this contains the animations.
     *  This is an array because you can specify a different animation
     *  For the different states.
     */
    FTS::Anim *pAnim[FTS_CURSOR_IMAGES];
    /** If the cursor isn't animated, this contains the graphics.
     *  This is an array because you can specify a different graphics
     *  For the different states.
     */
    FTS::Graphic *pGraph[FTS_CURSOR_IMAGES];
    /** whether each cursor is animated or not. */
    bool      bAnimated[FTS_CURSOR_IMAGES];
    /** The cursor's current position. Not absolutely the cursor's graphic's position. */
    int       iX, iY;
    /** The cursor's hotspot, that is in it's graphic, where is "the point that clicks". */
    int       iXHS, iYHS;
    /** This is the state of the mouse, the indexes are the constants from the
     *  FTSMouseButton enum. true means that the button is pressed.
     */
    bool      pbState[MouseButton::NoButton - SpecialKey::NoSpecial];
};
using PCursor = SCursor*;

/// This class represents a key combination. It is a recursive data type, thus
/// one object of this class may either represent the base-key (for example 'a')
/// or a modifier key applied to it (for example 'Ctrl'+a) or a modifier key for
/// another modifier (for example 'Shift'+Ctrl+a) etc.
class InputCombo : public NonCopyable {
public:
    friend class InputComboManager;

    InputCombo(const String &in_sName, const Key::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress = true);
    InputCombo(const String &in_sName, const SpecialKey::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress = true);
    InputCombo(const String &in_sName, const MouseButton::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress = true);
    InputCombo(const String &in_sName, const MouseScroll::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress = true);

    /// Destructor.
    virtual ~InputCombo();

    InputCombo *addModifier(InputCombo *out_pCombo);
    FTS::String print(uint8_t in_uiLevel = 0) const;
    void getAllNames(std::list<FTS::String> &out_list) const;

    /// Execute the associated command.
    /** This executes the command associated to this key combo if there is one.
     *  The command is associated to the key combo in the constructor.
     *
     * \return The return code of the command if it has been executed, false if
     *         there is no command or it couldn't be executed.
     *
     * \author Pompei2
     */
    inline bool exec() {return m_pCommand ? m_pCommand->exec() : false;};

    /// Get the base-KeyCombo of this set.
    /** This returns the base key combo of this key combo set. That is, say you
     *  have a key combo for the key 'a', a modifier for Ctrl+a and a modifier
     *  to this modifier to make Ctrl+Shift+a, if You call this method on any
     *  of these three key combo objects, it will always return a pointer to the
     *  key combo object for the key 'a', this is the base.
     *
     * \return The base key combo of this key combo set.
     *
     * \author Pompei2
     */
    inline InputCombo *getBase() {return m_pModified ? m_pModified->getBase() : this;};

    /// Get a number associated to this combo level.
    /** This returns a number that makes this key combo object. For modifiers, it
     *  is the key that does modify.
     *
     * \return The number of the key associated to this key combo. This may be
     *         any correct value for a Key variable as well as any correct
     *         value of a SpecialKey variable or a MouseButton or MouseScroll.
     *
     * \author Pompei2
     */
    inline uint32_t getNumber() const {
        // If anyone knows how to make that better (unions, maybe?), please
        // tell me how! I currently have no good idea.
        if(m_key == Key::Last) {
            if(m_specialKey == SpecialKey::NoSpecial) {
                if(m_mouseButton == MouseButton::NoButton) {
                    return m_mouseScroll;
                }
                return m_mouseButton;
            }
            return m_specialKey;
        }
        return m_key;
    };

    /** Returns the name of this specific key combo object. Not the name of the
     *  whole set (a set doesn't have a name actually).
     *
     * \return The name of this key combo object.
     *
     * \author Pompei2
     */
    inline FTS::String getName() const {return m_sName;};

    /// Gets the highest modifier that is currently triggered.
    /** Beginning at this object, this method first checks if the key associated
     *  to this object is currently being pressed. If this isn't the case, it
     *  returns NULL. If it is the case, it recursively does this for all
     *  modifiers registered for this object. It then returns the "highest" (or
     *  "deepest") modifier object that still is currently being triggered.
     *
     * \param in_scroll If this search is triggered by a mouse scroll event,
     *                  this is the event's data. Thus we will suppose this
     *                  action to have happened and modifiers needing this
     *                  event will trigger.
     *
     * \return The "highest" modifier object that still is currently being
     *         triggered.
     *
     * \author Pompei2
     */
    inline InputCombo *getHighestActiveModifier(const MouseScroll::Enum& in_scroll) {
        uint32_t tmp = 0;
        return this->getHighestActiveModifier(tmp, in_scroll);
    };

protected:
    Key::Enum m_key;                 ///< The key I represent.
    SpecialKey::Enum m_specialKey;   ///< Or maybe I represent a special key ?
    MouseButton::Enum m_mouseButton; ///< Or a mouse button ?
    MouseScroll::Enum m_mouseScroll; ///< Or even a mouse scroll?

    /// If I am a modifier, this is NON-NULL and points to the Key-combo I do
    /// modify. For example: if I am Ctrl+Up, this points to Up, as I am Ctrl
    /// and I do modify Up.
    InputCombo *m_pModified;

    /// This is a list of keycombo objects that modify me.
    std::list<InputCombo *>m_pModifiers;

    /// The command associated to me.
    CommandBase *m_pCommand;

    /// Every key or key modifier has a (unique) name.
    String m_sName;

    /// Whether to act on press or release of that key.
    bool m_bOnPress;

    InputCombo *getHighestActiveModifier(uint32_t &out_uiLevel, const MouseScroll::Enum& in_scroll);
};

class InputComboManager : NonCopyable {
public:
    InputComboManager();
    virtual ~InputComboManager();

    int add(InputCombo *in_pCombo);
    int rem(const String &in_sName);
    void detach(const String &in_sName);
    void remAll();
    InputCombo *get(const String &in_sName);

    bool handle(const Key::Enum &in_k);
    bool handle(const MouseButton::Enum& in_button);
    bool handle(const MouseScroll::Enum& in_scroll);

protected:
    /// An array that stores all combos for fast access.
    /// Every slot of the array is a list so we may register several key combos
    /// for the exact same key but only use one at a time.
    /// \note These KeyCombo are pointers to the same key combo objects as
    /// in /// \a m_pCombosMap !
    std::list<InputCombo *>m_pCombosArray[MouseScroll::NoScroll];

    /// A map that stores all combos associated to their name, for another fast
    /// access. \note These are pointers to the same key combo objects as in
    /// \a m_pCombosArray !
    std::map<FTS::String, InputCombo *>m_pCombosMap;

    int addNamedRecursively(InputCombo *in_pCombo);
    bool handleKey(int in_k, const MouseScroll::Enum& in_scroll = MouseScroll::NoScroll);
};

class InputManager : public Singleton<InputManager>, public Updateable, public Stackable {
public:
    InputManager();
    virtual ~InputManager();

    bool handleEvent(const SDL_Event& ev);
    bool update(const Clock&);

    int add(InputCombo *in_pCombo);
    int add(const String &in_sName, const Key::Enum& in_k, CommandBase *in_pCmd, bool in_bOnPress = true);
    int add(const String &in_sName, const SpecialKey::Enum& in_k, CommandBase *in_pCmd, bool in_bOnPress = true);
    int add(const String &in_sName, const MouseButton::Enum& in_k, CommandBase *in_pCmd, bool in_bOnPress = true);
    int add(const String &in_sName, const MouseScroll::Enum& in_k, CommandBase *in_pCmd);
    int delShortcut(const String &in_sName);
    void detachShortcut(const String &in_sName);
    void delAllShortcuts();
    int registerDefaultMenuShortcuts(bool in_bWithCamera = true);
    int unregisterDefaultMenuShortcuts(bool in_bWithCamera = true);

    bool isKeyPressed(Key::Enum in_k) const;
    bool isKeyPressed(SpecialKey::Enum in_k) const;
    bool isMousePressed(MouseButton::Enum in_k) const;

    bool isKeyRepeating(Key::Enum in_k) const;

    inline uint16_t getMouseX() const {return m_uiCursorX;};
    inline uint16_t getMouseY() const {return m_uiCursorY;};

    void simulateMouseMove(uint16_t in_X, uint16_t in_Y);
    void simulateMouseClick(MouseButton::Enum in_button);
    void simulateMouseClick(MouseButton::Enum in_button, uint16_t in_X, uint16_t in_Y);
    void simulateKeyPress(Key::Enum in_key);
    void simulateKeyDown(Key::Enum in_key);
    void simulateKeyUp(Key::Enum in_key);
    void pushContext();
    void popContext();
protected:
    struct InputInfo {
        /// True if that key is currently being hold down by the user, false else.
        bool bPressed = false;
        /// If it has already been repeated or not.
        bool bRepeating = false;
        /// How much sec to wait before starting the first key repetition.
        double dRepeatDelay = 0.5;
        /// How much sec to wait between every key repetition.
        double dRepeatInterval =0.05;
        /// The last time this key has been triggered.
        double dLastTrigger = 0.;
        /// The unicode-character that was triggered by this key.
        uint32_t utf32 = 0;

        bool checkForRepeat(double in_currTime);
    };

    /// A table for all keys, if they are pressed or not.
    InputInfo m_keyTab[Key::Last];

    /// A set that stores all currently pressed keys.
    std::set<Key::Enum>m_pressedKeys;

    /// A set that stores all currently pressed mouse buttons.
    std::set<MouseButton::Enum>m_pressedButtons;

    uint16_t m_uiCursorX; ///< The X position of the mouse.
    uint16_t m_uiCursorY; ///< The Y position of the mouse.

    /// The keyboard shortcuts manager that I use to handle shortcuts.
    InputComboManager* m_ComboMgr;

    void handleUTF32(uint32_t in_iCharcode);
    void handleKeyDown(Key::Enum in_Key);
    void handleKeyUp(Key::Enum in_Key);
    bool handleKeyDownGUI(Key::Enum in_Key);
    void handleMouseButtonPress(MouseButton::Enum in_button);
    void handleMouseButtonRelease(MouseButton::Enum in_button);
    void handleMouseScroll(MouseScroll::Enum in_direction);
    void handleMouseMove(uint16_t in_iX, uint16_t in_iY);

    bool injectToCEGUI(MouseButton::Enum in_button, bool in_bPress);

private:
    bool needForwardToCEGUI(Key::Enum k);
    std::stack<InputComboManager*> m_ctxInputComboManagers;
    Key::Enum m_LastPressedKey = Key::Last ;
};

} // namespace FTS

FTS::PCursor loadCursor(const FTS::String &in_sFile);
int unloadCursor(FTS::PCursor in_pCursor);
int deleteCursor(FTS::PCursor in_pCursor);
int drawCursor(const FTS::Clock&, const FTS::PCursor in_pCursor);

/* ====================== */
/* KEY CODES MANIPULATION */
/* ====================== */
CEGUI::uint FTSKeyToCEGUIKey(FTS::Key::Enum key);
FTS::MouseButton::Enum SDLMouseToFTSMouse(int btn);
FTS::MouseScroll::Enum SDLMouseToFTSScroll(const SDL_MouseWheelEvent& ev);
/// Returns the key code as a readable string.
/** This function converts a \c Key into a human readable string.
 *
 * \param in_Key The \c Key whose name to get.
 *
 * \return If successful: The key code.
 * \return If failed:      "Unknown key code".
 *
 * \note The returned value is sometimes more then one word: "left shift"
 *
 * \author Pompei2
 */
const char *getFTSKeyName(FTS::Key::Enum in_Key);

/// Returns the key code as a readable string.
/** This function converts a \c SpecialKey into a human readable string.
 *
 * \param in_Key The \c SpecialKey whose name to get.
 *
 * \return If successful: The key code.
 * \return If failed:      "Unknown key code".
 *
 * \note The returned value is sometimes more then one word: "left shift"
 *
 * \author Pompei2
 */
const char *getFTSKeyName(FTS::SpecialKey::Enum in_Key);

const char *getFTSButtonName(FTS::MouseButton::Enum in_Button);

#endif /* FTS_INPUT_H */

/* EOF */
