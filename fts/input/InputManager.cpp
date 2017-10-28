#include "input/input.h"

#include "3d/3d.h"
#include "3d/camera.h"
#include "3d/Renderer.h" // For the screen res for the mouse wrapping.
#include "ui/ui.h"
#include "ui/ui_commands.h"
#include "ui/ScriptConsole.h"
#include "game/player.h"
#include "graphic/anim.h" // For the cursor. To be removed.
#include "logging/logger.h"
#include "main/runlevels.h"
#include "main/Clock.h"

#include <CEGUISystem.h>
#include <CEGUIExceptions.h>
#include <elements/CEGUICheckbox.h>
#include <elements/CEGUICombobox.h>
#include <elements/CEGUIListbox.h>
#include <elements/CEGUIListboxItem.h>
#include <elements/CEGUIRadioButton.h>
#include <elements/CEGUISpinner.h>
#include <elements/CEGUIScrollbar.h>
#include <elements/CEGUIPopupMenu.h>

#include <algorithm>
#include <array>

using namespace FTS;

#define MOUSE_SCROLL_LIMIT 5

#ifdef DEBUG
bool g_bDrawNormals = false;
class SwitchGlobalDrawNormalsCmd : public CommandBase {
public:
    bool exec() {g_bDrawNormals = !g_bDrawNormals; return true;};
};

class DebugPringWindowHiearachyCmd : public CommandBase {
public:
    bool exec() {printWindowHierarchy(NULL, 0); return true;};
};
#endif // DEBUG

#define DEBUG_INPUT_MGR 1

/// Creates a key combo object.
/** This creates a key combo object. It can either function as a basic key
 *  or as a modifier, this makes no difference.
 *
 *  \remarks To see more details about how this is done, refer to the Dokuwiki
 *  http://wiki.arkana-fts.org/doku.php?id=design_documents:ui:key_shortcuts
 *  The category design_documents/ui/key_shortcuts.
 *
 * \param in_sName The name this key combo object should have.
 * \param in_k The key that this key combo object is bound to.
 * \param in_pCommand The command to be executed if this key combo is triggered.
 *
 * \author Pompei2
 */
InputCombo::InputCombo(const String &in_sName, const Key::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress)
    : m_key(in_k)
    , m_specialKey(SpecialKey::NoSpecial)
    , m_mouseButton(MouseButton::NoButton)
    , m_mouseScroll(MouseScroll::NoScroll)
    , m_pModified(nullptr)
    , m_pCommand(in_pCommand)
    , m_sName(in_sName)
    , m_bOnPress(in_bOnPress)
{
}

/// Constructor for a meta key.
InputCombo::InputCombo(const String &in_sName, const SpecialKey::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress)
    : m_key(Key::Last)
    , m_specialKey(in_k)
    , m_mouseButton(MouseButton::NoButton)
    , m_mouseScroll(MouseScroll::NoScroll)
    , m_pModified(nullptr)
    , m_pCommand(in_pCommand)
    , m_sName(in_sName)
    , m_bOnPress(in_bOnPress)
{
}

/// Constructor for a mouse button.
InputCombo::InputCombo(const String &in_sName, const MouseButton::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress)
    : m_key(Key::Last)
    , m_specialKey(SpecialKey::NoSpecial)
    , m_mouseButton(in_k)
    , m_mouseScroll(MouseScroll::NoScroll)
    , m_pModified(nullptr)
    , m_pCommand(in_pCommand)
    , m_sName(in_sName)
    , m_bOnPress(in_bOnPress)
{
}

/// Constructor for a mouse scroll.
InputCombo::InputCombo(const String &in_sName, const MouseScroll::Enum &in_k, CommandBase *in_pCommand, bool in_bOnPress)
    : m_key(Key::Last)
    , m_specialKey(SpecialKey::NoSpecial)
    , m_mouseButton(MouseButton::NoButton)
    , m_mouseScroll(in_k)
    , m_pModified(nullptr)
    , m_pCommand(in_pCommand)
    , m_sName(in_sName)
    , m_bOnPress(in_bOnPress)
{
}

InputCombo::~InputCombo()
{
    while(!m_pModifiers.empty()) {
        InputCombo *pFront = m_pModifiers.front();
        delete pFront;
        m_pModifiers.pop_front();
    }

    delete m_pCommand;
}

InputCombo *InputCombo::addModifier(InputCombo *out_pCombo)
{
    if(out_pCombo == nullptr) {
        FTS18N("InvParam", MsgType::Horror, "InputCombo::addModifier(NULL)");
        return this;
    }

    // Check if there is not already a modifier with that name in the combo.

    // Get the names of all things bound to that key currently:
    std::list<String>lAllNames;
    this->getBase()->getAllNames(lAllNames);

    // Check if there is already something named like the one we want to add.
    for(std::list<String>::iterator i = lAllNames.begin() ; i != lAllNames.end() ; ++i) {
        if(*i == out_pCombo->getName()) {
            // Something is twice in the combo, stop !
            FTS18N("DoubleShortcut", MsgType::Horror, *i);
            return this;
        }
    }

    // If nothing went wrong, we may finally add the modifier.
    m_pModifiers.push_back(out_pCombo);
    out_pCombo->m_pModified = this;
    return this;
}

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
InputCombo *InputCombo::getHighestActiveModifier(uint32_t &out_uiLevel, const MouseScroll::Enum& in_scroll)
{
    // Am I active ? if not, we can stop here and go back home :)
    // Special treatement if I am a special key.
    if(m_key == Key::Last) {
        if(m_specialKey == SpecialKey::Any) {
            // Check if any key activates.
            if(InputManager::getSingleton().isKeyPressed(SpecialKey::Any) != m_bOnPress)
                return nullptr;
        } else if(m_specialKey == SpecialKey::NoSpecial) {
            if(m_mouseButton == MouseButton::Any) {
                // Check if any mouse button activates.
                if(InputManager::getSingleton().isMousePressed(MouseButton::Any) != m_bOnPress)
                    return nullptr;
            } else if(m_mouseButton == MouseButton::NoButton) {
                // check if we are asking for mouse scrolling.
                if(m_mouseScroll == MouseScroll::NoScroll) {
                    return nullptr;
                } else {
                    if(m_mouseScroll != in_scroll) {
                        return nullptr;
                    }
                }
            } else {
                // Check if we are asking for a mouse button.
                if(InputManager::getSingleton().isMousePressed(m_mouseButton) != m_bOnPress)
                    return nullptr;
            }
        } else {
            // Check if one of both alt, ctrl or shift keys activates.
            Key::Enum k1 = Key::Last, k2 = Key::Last;
            switch(m_specialKey) {
            case SpecialKey::Alt: k1 = Key::LeftAlt; k2 = Key::RightAlt; break;
            case SpecialKey::Control: k1 = Key::LeftControl; k2 = Key::RightControl; break;
            case SpecialKey::Shift: k1 = Key::LeftShift; k2 = Key::RightShift; break;
            case SpecialKey::Enter: k1 = Key::Return; k2 = Key::NumpadEnter; break;
            default: return nullptr;
            }

            // None of both keys activate?
            if((InputManager::getSingleton().isKeyPressed(k1) != m_bOnPress) &&
               (InputManager::getSingleton().isKeyPressed(k2) != m_bOnPress)
              ) {
                return nullptr; // If none is, we're out.
            }
        }
    } else {
        // Check if the key activates this part of the combo. If not, leave.
        if(InputManager::getSingleton().isKeyPressed(m_key) != m_bOnPress) {
            return nullptr;
        }
    }

    out_uiLevel++;

    // No more guys left ? I am the highest in this way.
    if(m_pModifiers.empty()) {
        return this;
    }

    // Try all of my modifiers to see who goes the highest.
    uint32_t uiMax = 0;
    InputCombo *pMax = nullptr;
    for(std::list<InputCombo *>::const_iterator i = m_pModifiers.begin() ; i != m_pModifiers.end() ; ++i) {
        auto uiNow = out_uiLevel;
        auto pNow = (*i)->getHighestActiveModifier(uiNow, in_scroll);
        // We got a new highest active modifier. >= To give priority to the
        // one that was added the last in case of a duelling.
        if((pNow != nullptr) && (uiNow >= uiMax)) {
            uiMax = uiNow;
            pMax = pNow;
        }
    }

    if(pMax == nullptr) {
        return this;
    } else {
        out_uiLevel += uiMax;
        return pMax;
    }
}

void InputCombo::getAllNames(std::list<String> &out_list) const
{
    out_list.push_back(this->getName());

    for(std::list<InputCombo *>::const_iterator i = m_pModifiers.begin() ; i != m_pModifiers.end() ; ++i) {
        (*i)->getAllNames(out_list);
    }
}

String InputCombo::print(uint8_t in_uiLevel) const
{
    String sResult;
    for(uint8_t i = 0 ; i < in_uiLevel ; ++i) {
        sResult += " ";
    }

    if(m_key != Key::Last) {
        sResult += getFTSKeyName(m_key) + String("\n");
    } else {
        sResult += getFTSKeyName(m_specialKey) + String("\n");
    }

    for(std::list<InputCombo *>::const_iterator i = m_pModifiers.begin() ; i != m_pModifiers.end() ; ++i) {
        for(uint8_t j = 0 ; j < in_uiLevel ; ++j) {
            sResult += " ";
        }
        sResult += (*i)->print(in_uiLevel+2);
    }
    return sResult;
}

InputComboManager::InputComboManager()
{
}

InputComboManager::~InputComboManager()
{
    // We delete all combos using the array, because when deleting a base
    // combo, it deletes all its modifers itself, thus if we would delete every
    // combo in the map, we would delete some twice as the map contains base
    // combos as well as combo modifiers.
    for(int k = 0 ; k < MouseScroll::NoScroll ; ++k) {
        while(!m_pCombosArray[k].empty()) {
            InputCombo *pComb = *(m_pCombosArray[k].begin());
            delete pComb;
            m_pCombosArray[k].erase(m_pCombosArray[k].begin());
        }
    }
}

int InputComboManager::addNamedRecursively(InputCombo *in_pCombo)
{
    if(in_pCombo == NULL) {
        FTS18N("InvParam", MsgType::Horror, "InputComboManager::addNamedRecursively(NULL)");
        return ERR_OK;
    }

    // Add myself to the name map.
    m_pCombosMap.insert(std::pair<String, InputCombo *>(in_pCombo->getName(),in_pCombo));

    // And add all my modifiers and their modifiers etc. recursively.
    int iRet = ERR_OK;
    for(std::list<InputCombo *>::iterator i = in_pCombo->m_pModifiers.begin();
        i != in_pCombo->m_pModifiers.end() ; i++) {
        // If one fails to add, still continue, but keep in mind at least one failed.
        if(ERR_OK != this->addNamedRecursively(*i))
            iRet = -1;
    }

    return iRet;
}

int InputComboManager::add(InputCombo *in_pCombo)
{
    if(in_pCombo == nullptr) {
        FTS18N("InvParam", MsgType::Horror, "InputComboManager::add(NULL)");
        return ERR_OK;
    }

    // Begin at the base :)
    InputCombo *pBase = in_pCombo->getBase();

    // First, check if the combo or one of its modifiers has already been added,
    // we forbit the adding of the whole packet. (Check done by name).
    std::list<String>lComboNames;
    in_pCombo->getAllNames(lComboNames);

    for(std::list<String>::iterator i = lComboNames.begin() ; i != lComboNames.end() ; ++i) {
        // If one is already in it, stop immediately.
        if(this->get(*i) != nullptr) {
            FTS18N("DoubleShortcut", MsgType::Horror, *i);
            return -1;
        }
    }

    // If we come here, that means none of the combos is already added.

    // First, add the base at the front of the list for this key, so freshly
    // added key combos will be executed before older are. (The older may not be
    // executed at all.)
    m_pCombosArray[pBase->getNumber()].push_front(pBase);

    // And add it and its modifiers recursively onto the name map.
    return this->addNamedRecursively(pBase);
}

int InputComboManager::rem(const String &in_sName)
{
    InputCombo *pToRem = this->get(in_sName);

    if(pToRem == nullptr) {
        // No need for an error, a log is enough.
        FTSMSGDBG("InvParam", 3, "InputComboManager::rem: "+in_sName+" not existent.");
        return ERR_OK;
    }

    // We need to remove the keycombo and all its modifiers too as they make
    // no more sense without the keycombo they modify. Agree ?

    std::list<String>meAndMyModifiers;
    pToRem->getAllNames(meAndMyModifiers);

    // Removing from map is easy:
    for(std::list<String>::iterator i = meAndMyModifiers.begin() ;
        i != meAndMyModifiers.end() ; ++i) {
        // Need to erase the string, not the iterator!
        m_pCombosMap.erase(*i);
    }

    // Now, we must remove all of them from the array, a little more tricky ...
    InputCombo *pBase = pToRem->getBase();

    // We need an extra treatment if the combo we want to remove is the base:
    std::list<InputCombo *> *l = nullptr;
    if(pToRem == pBase) {
        // Need to remove it from the combos array.
        l = &m_pCombosArray[pToRem->getNumber()];
    } else {
        // If the one we want to remove is not the base, but a modifier,
        // Remove it from the modifier list of the one it modifies.
        l = &pToRem->m_pModified->m_pModifiers;
    }

    // Remove it from the list:
    for(std::list<InputCombo *>::iterator i = l->begin() ; i != l->end() ; ++i) {
        if((*i) == pToRem) {
            l->erase(i);
            break;
        }
    }

    // Now we may delete it.
    delete pToRem;
    return ERR_OK;
}

void InputComboManager::detach(const String &in_sName)
{
    InputCombo *pToDetach= this->get(in_sName);

    if(pToDetach == nullptr) {
        // No need for an error, a log is enough.
        FTSMSGDBG("InvParam", 3, "InputComboManager::detach: "+in_sName+" not existent.");
        return ;
    }


    // Now, we must remove all of them from the array, a little more tricky ...
    InputCombo *pBase = pToDetach->getBase();

    // We need an extra treatment if the combo we want to remove is the base:
    std::list<InputCombo *> *l = nullptr;
    if(pToDetach == pBase) {
        // Need to remove it from the combos array.
        l = &m_pCombosArray[pToDetach->getNumber()];
    } else {
        // If the one we want to remove is not the base, but a modifier,
        // Remove it from the modifier list of the one it modifies.
        l = &pToDetach->m_pModified->m_pModifiers;
    }

    // Remove it from the list:
    for(std::list<InputCombo *>::iterator i = l->begin() ; i != l->end() ; ++i) {
        if((*i) == pToDetach) {
            l->erase(i);
            break;
        }
    }
}

void InputComboManager::remAll()
{
    while(!m_pCombosMap.empty()) {
        this->rem(m_pCombosMap.begin()->first);
    }
}

InputCombo *InputComboManager::get(const String &in_sName)
{
    std::map<String,InputCombo *>::iterator i = m_pCombosMap.find(in_sName);

    return (i == m_pCombosMap.end()) ? nullptr : i->second;
}

bool InputComboManager::handle(const Key::Enum &in_k)
{
    // Handle the pressed key. If it triggers, we're done.
    if(this->handleKey(in_k)) {
        return true;
    }

    // If it didn't trigger, check for the special keys.

    if(in_k == Key::LeftControl || in_k == Key::RightControl) {
        if(this->handleKey(SpecialKey::Control)) {
            return true;
        }
    }

    if(in_k == Key::LeftAlt || in_k == Key::RightAlt) {
        if(this->handleKey(SpecialKey::Alt)) {
            return true;
        }
    }

    if(in_k == Key::LeftShift || in_k == Key::RightShift) {
        if(this->handleKey(SpecialKey::Shift)) {
            return true;
        }
    }

    if(in_k == Key::Return || in_k == Key::NumpadEnter) {
        if(this->handleKey(SpecialKey::Enter)) {
            return true;
        }
    }

    // Nothing triggered, try to trigger the "any key" callback.
    // It only triggers if it is registered.
    if(this->handleKey(SpecialKey::Any)) {
        return true;
    }

    // No key handled.
    return false;
}

bool InputComboManager::handle(const MouseButton::Enum& in_button)
{
    // Handle the pressed button. If it triggers, we're done.
    if(this->handleKey(in_button)) {
        return true;
    }

    // If it didn't trigger, check for the special "any" button.
    if(this->handleKey(MouseButton::Any)) {
        return true;
    }

    // No button handled.
    return false;
}

bool InputComboManager::handle(const MouseScroll::Enum& in_scroll)
{
    return this->handleKey(in_scroll, in_scroll);
}

bool InputComboManager::handleKey(int in_k, const MouseScroll::Enum& in_scroll)
{
    // If there is no combo with that key as a base, quit.
    if(m_pCombosArray[in_k].empty())
        return false;

    // Find the first combo registered with that base key that triggers
    // successfully.
    for(auto i = m_pCombosArray[in_k].begin() ; i != m_pCombosArray[in_k].end() ; ++i) {

        InputCombo *pNow = (*i)->getHighestActiveModifier(in_scroll);
        if(pNow && pNow->exec()) {
            // If it triggered successfully, we stop for this key
            // (don't wanna execute two things for one keypress).
            return true;
        }

        // Didn't trigger successfully, try the next one.
    }

    // Nothing triggered, we're gone.
    return false;
}

/// Constructor of the keyboard manager. Inits the copy/paste engine too.
InputManager::InputManager()
    : m_uiCursorX(0)
    , m_uiCursorY(0)
{
    m_ComboMgr = new InputComboManager();
    // m_keyTab being initialized correctly by its constructor.
    SDL_ShowCursor(SDL_DISABLE);

    UpdateableManager::getSingleton().add("Input Manager", this);
}

InputManager::~InputManager()
{
    // We do not have to unregister ourselves as
    UpdateableManager::getSingleton().rem("Input Manager");
    SDL_ShowCursor(SDL_ENABLE);
    delete m_ComboMgr;
}

static std::uint32_t utf8_32(std::array<uint8_t,4> buf)
{
    if(!(buf[0] & 0x80)) {
        return buf[0];
    }

    auto followers = 0;
    auto ch = buf[0];
    while (ch & 0x80) { followers++; ch <<= 1; }
    followers--;
    if(followers == 1) {
        return (buf[1] & 0x3f) | ((buf[0] & 0x1f) << 6);
    } else if(followers == 2) {
        return (buf[2] & 0x3f) | ((buf[1] & 0x3f) << 6) | ((buf[0] & 0x0f) << 12);
    } else if(followers == 3) {
        return (buf[3] & 0x3f) | ((buf[2] & 0x3f) << 6) | ((buf[1] & 0x3f) << 12) | ((buf[0] & 0x7) << 18);
    }
    return 0xffffffff;
}

bool InputManager::handleEvent(const SDL_Event& ev)
{
    switch(ev.type) {
    case SDL_KEYDOWN:
        if(ev.key.keysym.sym != SDLK_UNKNOWN) {
            this->handleKeyDown(static_cast<Key::Enum>(ev.key.keysym.scancode));
        }
        return true;
    case SDL_KEYUP:
        this->handleKeyUp(static_cast<Key::Enum>(ev.key.keysym.scancode));
        return true;
    case SDL_MOUSEMOTION:
        this->handleMouseMove(ev.motion.x, ev.motion.y);
        return true;
    case SDL_MOUSEBUTTONDOWN:
        this->handleMouseButtonPress(SDLMouseToFTSMouse(ev.button.button));
        return true;
    case SDL_MOUSEBUTTONUP:
        // Note: we get this message for mouse scrolling too.
        // nonsense but so what ... the mouse button release looks
        // for invalid inputs.
        this->handleMouseButtonRelease(SDLMouseToFTSMouse(ev.button.button));
        return true;
    case SDL_MOUSEWHEEL:
        this->handleMouseScroll(SDLMouseToFTSScroll(ev.wheel));
        return true;
    case SDL_TEXTINPUT:
    {
        // Assumption: This event here delivers only one keyboard character at a time.
        // This character is utf8 coded and can be at max 4 bytes big.
        // It must be converted to utf32 so that CEGUI can handle it.
        auto unicode = utf8_32({ (uint8_t)ev.text.text[0] ,(uint8_t)ev.text.text[1],(uint8_t)ev.text.text[2],(uint8_t)ev.text.text[3] });
        FTSMSGDBG("  Text: " + String(ev.text.text) + "\n", 5);
        m_keyTab[m_LastPressedKey].utf32 = unicode;
        this->handleUTF32(unicode);
    }
        break;
    default:
        return false;
    }
}

/** Checks if a key is currently pressed or not.
 *
 * \param in_k The key you want to check.
 *
 * \return Whether the key is being held down at the moment or not.
 *
 * \author Pompei2
 */
bool InputManager::isKeyPressed(Key::Enum in_k) const
{
    if(0 <= in_k && in_k < Key::Last) {
        return m_keyTab[in_k].bPressed;
    }
    return false;
}

/** Checks if a "special" key is currently pressed or not.
 *
 * \param in_k The key you want to check.
 *
 * \return Whether the key is being held down at the moment or not.
 *
 * \author Pompei2
 */
bool InputManager::isKeyPressed(SpecialKey::Enum in_k) const
{
    switch(in_k) {
    case SpecialKey::Alt:
        return this->isKeyPressed(Key::LeftAlt) || this->isKeyPressed(Key::RightAlt);
    case SpecialKey::Control:
        return this->isKeyPressed(Key::LeftControl) || this->isKeyPressed(Key::RightControl);
    case SpecialKey::Shift:
        return this->isKeyPressed(Key::LeftShift) || this->isKeyPressed(Key::RightShift);
    case SpecialKey::Enter:
        return this->isKeyPressed(Key::Return) || this->isKeyPressed(Key::NumpadEnter);
    case SpecialKey::Any:
        return !m_pressedKeys.empty();
    default:
        return false;
    }
}

/** Checks if a mouse button is currently pressed or not.
 *
 * \param in_k The button you want to check.
 *
 * \return Wether the button is being held down at the moment or not.
 *
 * \author Pompei2
 */
bool InputManager::isMousePressed(MouseButton::Enum in_k) const
{
    /// Special for MouseButton::Any!
    if(in_k == MouseButton::Any) {
        return !m_pressedButtons.empty();
    }

    return m_pressedButtons.find(in_k) != m_pressedButtons.end();
}

/** Checks if a key is currently being repeated or pressed for the first time.
 *
 * \param in_k The key you want to check.
 *
 * \return Wether the key is being repeated at the moment or not.
 *
 * \author Pompei2
 */
bool InputManager::isKeyRepeating(Key::Enum in_k) const
{
    if(0 <= in_k && in_k < Key::Last) {
        return m_keyTab[in_k].bRepeating;
    }
    return false;
}

/** This adds a shortcut to the list of shortcuts to be managed.
 *
 * \param in_pCombo: The shortcut to manage.
 *
 * \return this.
 *
 * \author Pompei2
 */
int InputManager::add(InputCombo *in_pCombo)
{
    return m_ComboMgr->add(in_pCombo->getBase());
}

/// Little helper, for more info \see InputManager::add(InputCombo *in_pCombo)
int InputManager::add(const String &in_sName, const Key::Enum &in_k, CommandBase *in_pCmd, bool in_bOnPress)
{
    return this->add(new InputCombo(in_sName, in_k, in_pCmd, in_bOnPress));
}

/// Little helper, for more info \see InputManager::add(InputCombo *in_pCombo)
int InputManager::add(const String &in_sName, const SpecialKey::Enum &in_k, CommandBase *in_pCmd, bool in_bOnPress)
{
    return this->add(new InputCombo(in_sName, in_k, in_pCmd, in_bOnPress));
}

/// Little helper, for more info \see InputManager::add(InputCombo *in_pCombo)
int InputManager::add(const String &in_sName, const MouseButton::Enum &in_k, CommandBase *in_pCmd, bool in_bOnPress)
{
    return this->add(new InputCombo(in_sName, in_k, in_pCmd, in_bOnPress));
}

/// Little helper, for more info \see InputManager::add(InputCombo *in_pCombo)
int InputManager::add(const String &in_sName, const MouseScroll::Enum &in_k, CommandBase *in_pCmd)
{
    return this->add(new InputCombo(in_sName, in_k, in_pCmd));
}

/** This removes a shortcut from the list of shortcuts to be managed.
 *
 * \param in_sName: The name of the shortcut to remove.
 *
 * \return this.
 *
 * \author Pompei2
 */
int InputManager::delShortcut(const String &in_sName)
{
    return m_ComboMgr->rem(in_sName);
}
void InputManager::detachShortcut(const String &in_sName)
{
    return m_ComboMgr->detach(in_sName);
}

void InputManager::delAllShortcuts()
{
    m_ComboMgr->remAll();
}

/** This handles keypresses that affect the GUI.
 *
 * \param in_Key: The key that was pressed.
 *
 * \return true if the key input has been used by the GUI, false if not.
 *
 * \author Pompei2
 */
bool InputManager::handleKeyDownGUI(Key::Enum in_Key)
{
    CEGUI::Window *pW = GUI::getSingleton().getActiveWidget();

    // If we have no active widget, all other key presses get ignored.
    if(pW == nullptr)
        return false;

    bool bUsed = false;
    CEGUI::String sType = pW->getType();
    try {
        switch(in_Key) {
            case Key::ArrowDown:
                // Move a static text's scrollbar up by one page.
                if(sType.find(CEGUI::String("StaticText")) != CEGUI::String::npos) {
                    CEGUI::String sVertSBName = pW->getName() + "__auto_vscrollbar__";
                    CEGUI::Scrollbar *pC = (CEGUI::Scrollbar *)CEGUI::WindowManager::getSingleton().getWindow(sVertSBName);
                    pC->setScrollPosition(pC->getScrollPosition() + pC->getStepSize());
                    bUsed = true;
                }
                break;
            case Key::ArrowUp:
                // Move a static text's scrollbar up by one page.
                if(sType.find(CEGUI::String("StaticText")) != CEGUI::String::npos) {
                    CEGUI::String sVertSBName = pW->getName() + "__auto_vscrollbar__";
                    CEGUI::Scrollbar *pC = (CEGUI::Scrollbar *)CEGUI::WindowManager::getSingleton().getWindow(sVertSBName);
                    pC->setScrollPosition(pC->getScrollPosition() - pC->getStepSize());
                    bUsed = true;
                }
                break;
            case Key::PageUp:
                // Move a static text's and listbox's scrollbar up by one page.
                if(sType.find(CEGUI::String("StaticText")) != CEGUI::String::npos
                   || sType.find(CEGUI::String("Listbox")) != CEGUI::String::npos
                   ) {
                    CEGUI::String sVertSBName = pW->getName() + "__auto_vscrollbar__";
                    CEGUI::Scrollbar *pC = (CEGUI::Scrollbar *)CEGUI::WindowManager::getSingleton().getWindow(sVertSBName);
                    pC->setScrollPosition(pC->getScrollPosition() - pC->getPageSize());
                    bUsed = true;
                    // Move a combobx's scrollbar up by one page.
                }
                else if(sType.find(CEGUI::String("Combobox")) != CEGUI::String::npos) {
                    CEGUI::String sVertSBName = pW->getName() + "__auto_droplist____auto_vscrollbar__";
                    CEGUI::Scrollbar *pC = (CEGUI::Scrollbar *)CEGUI::WindowManager::getSingleton().getWindow(sVertSBName);
                    pC->setScrollPosition(pC->getScrollPosition() - pC->getPageSize());
                    bUsed = true;
                }
                break;
            case Key::PageDown:
                // Move a static text's and listbox's scrollbar down by one page.
                if(sType.find(CEGUI::String("StaticText")) != CEGUI::String::npos
                   || sType.find(CEGUI::String("Listbox")) != CEGUI::String::npos
                   ) {
                    CEGUI::String sVertSBName = pW->getName() + "__auto_vscrollbar__";
                    CEGUI::Scrollbar *pC = (CEGUI::Scrollbar *)CEGUI::WindowManager::getSingleton().getWindow(sVertSBName);
                    pC->setScrollPosition(pC->getScrollPosition() + pC->getPageSize());
                    bUsed = true;
                    // Move a combobx's scrollbar down by one page.
                }
                else if(sType.find(CEGUI::String("Combobox")) != CEGUI::String::npos) {
                    CEGUI::String sVertSBName = pW->getName() + "__auto_droplist____auto_vscrollbar__";
                    CEGUI::Scrollbar *pC = (CEGUI::Scrollbar *)CEGUI::WindowManager::getSingleton().getWindow(sVertSBName);
                    pC->setScrollPosition(pC->getScrollPosition() + pC->getPageSize());
                    bUsed = true;
                }
                break;
            default:
                break;
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return bUsed;
}

bool InputManager::InputInfo::checkForRepeat(double in_currTime)
{
    // First repetition.
    if(this->dLastTrigger == 0.0)
        this->dLastTrigger = in_currTime;

    double dToComp = this->bRepeating ? this->dRepeatInterval : this->dRepeatDelay;

    if(in_currTime - this->dLastTrigger >= dToComp) {
        // Mark me as being repeated.
        this->bRepeating = true;
        return true;
    }

    return false;
}

/** This method shall be called once every game tick. It checks for key
 *  repetitions (if a key is held down quite some time, it gets repeated)
 *  and executes them, if needed, calling InputManager::handleKeyDown.
 *
 * \author Pompei2
 */
bool InputManager::update(const Clock& in_c)
{
    // Check every currently pressed key.
    for(std::set<Key::Enum>::iterator i = m_pressedKeys.begin() ; i != m_pressedKeys.end() ; ++i) {
        if(m_keyTab[*i].checkForRepeat(in_c.getCurrentTime())) {
            // And fake a keydown for the repetition.
            this->handleKeyDown(*i);
            this->handleUTF32(m_keyTab[*i].utf32);
        }
    }

    return true;
}

/** The callback function called if a key was pressed and the result of that
 *  press is a unicode (UTF-16) character. This only handles the character, it
 *  does nothing with raw keys or so.
 *
 * \param in_Key: The key that was pressed.
 *
 * \author Pompei2
 */
void InputManager::handleUTF32(uint32_t in_iCharcode)
{
    if(in_iCharcode == 0)
        return;

    FTSMSGDBG("  Key UTF32: " + String::nr(in_iCharcode, 0, ' ', std::ios::hex) + "\n", 5);

    if(CEGUI::System::getSingletonPtr()) {
        CEGUI::System::getSingletonPtr()->injectChar(in_iCharcode);
        return;
    }
}

bool InputManager::needForwardToCEGUI(Key::Enum k)
{
    // CEGUI eats up our enter, tab and keypad enter keys if an edit box has the
    // focus. This is to validate the edit box. We do not want this, as enter
    // will likely be the keyboard shortcut to validate the whole dialog.
    // So if an edit box is selected and the input would be one of these,
    // we just don't send them down to CEGUI.
    // Exception for multi line edit boxes, the need the enter and tab key.
    if(k == Key::Return || k == Key::Tab || k == Key::NumpadEnter) {
        CEGUI::Window *pW = GUI::getSingleton().getActiveWidget();
        if(pW) {
            String t = pW->getType();

            bool bIsEditbox = t.lower().contains("editbox");
            bool bIsMulti = t.lower().contains("multiline");

            // Now we still need to check if its active child might be such
            // a monster. This is for example the ase in spinners.
            if(pW->getActiveChild()) {
                String t = pW->getActiveChild()->getType();
                bIsEditbox |= t.lower().contains("editbox");
                bIsMulti |= t.lower().contains("multiline");
            }

            // Single-line editboxes get no enter and no tab,
            if(bIsEditbox && !bIsMulti) {
                return false;
            }
            // Multilines get the enter but no tab.
            if(bIsEditbox && bIsMulti && (k == Key::Tab)) {
                return false;
            }
        }
    }
    return true;
}

/** The callback function called if a key was pressed.
 *
 * \param in_Key: The key that was pressed.
 *
 * \author Pompei2
 */
void InputManager::handleKeyDown(Key::Enum in_Key)
{
    CEGUI::EventArgs ea;
    bool bProcessed = false;
    Key::Enum k = in_Key;

    if(k >= Key::Last) {
        FTSMSG("\n?\n", MsgType::Raw);
        return;
    }

    // If the key is not yet pressed, this is the first time it is being pressed,
    // thus we need to start the key repetition timer.
    // But not for unknown keys! That fucks everything up, for example the ^ key!
    if(!m_keyTab[k].bPressed) {
        // Adjust our keystates.
        m_keyTab[k].bPressed = true;
        m_keyTab[k].bRepeating = false;
        m_pressedKeys.insert(k);
        m_LastPressedKey = k;
    }

    // Mark last trigger as being *now*, will be updated in the next update run.
    m_keyTab[k].dLastTrigger = 0.0;

#ifdef DEBUG_INPUT_MGR
    FTSMSGDBG("Key pressed:  "+String(getFTSKeyName(k))+" ("+String::nr(k)+", "+String::b(m_keyTab[k].bRepeating)+")\n", 5);
    String sPressedKeys = "  Currently Pressed keys = [ ";
    for(std::set<Key::Enum>::iterator i = m_pressedKeys.begin() ; i != m_pressedKeys.end() ; ++i) {
        sPressedKeys += getFTSKeyName(*i) + String(",");
    }
    sPressedKeys += "]\n";
    FTSMSGDBG(sPressedKeys, 5);
#endif

    if(needForwardToCEGUI(k)) {
        // to tell CEGUI that a key was pressed, we inject the scan code.
        if(CEGUI::System::getSingletonPtr() != nullptr) {
            bProcessed = CEGUI::System::getSingletonPtr()->injectKeyDown(FTSKeyToCEGUIKey(k));
        }
    }

    if( RunlevelManager::getSingletonPtr() != nullptr ) {
        if(RunlevelManager::getSingleton().getCurrRunlevel()->getName() != "Game") {
            bProcessed = this->handleKeyDownGUI(k) || bProcessed;
        }
    }
    // If the key has been used somehow by the GUI, we just quit now.
    if(bProcessed)
        return;

    // If some key combo was executed, we stop handling the keys.
    if(m_ComboMgr->handle(k)) {
        return;
    }

    return;
}

/** HandleKeyUp The callback function called if a key was released.
 *
 * \param in_Key: The key that was released.
 *
 * \author Pompei2
 */
void InputManager::handleKeyUp(Key::Enum in_Key)
{
    Key::Enum k = in_Key;

    if(k >= Key::Last)
        return;

    // Adjust our keystates.
    m_keyTab[k].bPressed = false;
    m_keyTab[k].bRepeating = false;
    m_keyTab[k].utf32 = 0;
    m_pressedKeys.erase(k);

#ifdef DEBUG_INPUT_MGR
    FTSMSGDBG("Key released: "+String(getFTSKeyName(k))+" ("+String::nr(k)+")\n", 5);
    String sPressedKeys = "  Currently Pressed keys = [ ";
    for(std::set<Key::Enum>::iterator i = m_pressedKeys.begin() ; i != m_pressedKeys.end() ; ++i) {
        sPressedKeys += getFTSKeyName(*i) + String(",");
    }
    sPressedKeys += "]\n";
    FTSMSGDBG(sPressedKeys, 5);
#endif

    CEGUI::System *pSys = CEGUI::System::getSingletonPtr();

    // And now check if the release is used by any hotkey.
    if(m_ComboMgr->handle(in_Key))
        return;

    // If the key is used by the GUI, we won't use it anymore.
    if(pSys && pSys->injectKeyUp(FTSKeyToCEGUIKey(k)))
        return;

    return;
}

/** The callback function called if the mouse moved without a button pressed.
 *
 * \param in_iX The x position of the mouse.
 * \param in_iY The y position of the mouse.
 *
 * \note If a driver needs another type of callback function, you need to write
 *        the function yourself, but the function can call this function and
 *        give her the right params.
 * \author Pompei2
 */
void InputManager::handleMouseMove(uint16_t in_iX, uint16_t in_iY)
{
    m_uiCursorX = in_iX;
    m_uiCursorY = in_iY;

    Runlevel* pRlv = RunlevelManager::getSingleton().getCurrRunlevel();
    PCursor pCurrCursor = pRlv->getActiveCursor();
    if(pCurrCursor) {
        pCurrCursor->iX = m_uiCursorX;
        pCurrCursor->iY = m_uiCursorY;
    }
    pRlv->onMouseMoved(in_iX, in_iY);

    if(CEGUI::System::getSingletonPtr()) {
        CEGUI::System::getSingletonPtr()->injectMousePosition((float)m_uiCursorX, (float)m_uiCursorY);
    }

    return;
}

void InputManager::handleMouseScroll(MouseScroll::Enum in_direction)
{
    // If some key combos were executed, we stop handling the keys.
    if(m_ComboMgr->handle(in_direction)) {
        return;
    }

#ifdef DEBUG_INPUT_MGR
    switch(in_direction) {
    case MouseScroll::Up:    FTSMSGDBG("Scrolling the mouse up", 5); break;
    case MouseScroll::Down:  FTSMSGDBG("Scrolling the mouse down", 5); break;
    case MouseScroll::Left:  FTSMSGDBG("Scrolling the mouse left", 5); break;
    case MouseScroll::Right: FTSMSGDBG("Scrolling the mouse right", 5); break;
    default: break;
    }
#endif

    CEGUI::System *pSys = CEGUI::System::getSingletonPtr();

    // No CEGUI existing yet.
    if(pSys == nullptr)
        return ;

    switch(in_direction) {
    case MouseScroll::Up:
        pSys->injectMouseWheelChange(+1);
        break;
    case MouseScroll::Down:
        pSys->injectMouseWheelChange(-1);
        break;
    default: break;
    }
}

/** This function sends the according notification to the CEGUI system.
 *
 * \param in_button The button that is being pressed or released.
 * \param in_state  The state of the button.
 * \param in_iX     The x position of the mouse.
 * \param in_iY     The y position of the mouse.
 *
 * \return true if the CEGUI system has used it, false if it has not.
 *
 * \author Pompei2
 */
bool InputManager::injectToCEGUI(MouseButton::Enum in_button, bool in_bPress)
{
    CEGUI::System *pSys = CEGUI::System::getSingletonPtr();

    // No CEGUI existing yet.
    if(pSys == nullptr)
        return false;

    bool bUsed = false;
    switch (in_button) {
    case MouseButton::Left:
        if(in_bPress) {
            bUsed = pSys->injectMouseButtonDown(CEGUI::LeftButton);
        } else {
            bUsed = pSys->injectMouseButtonUp(CEGUI::LeftButton);
        }
        break;
    case MouseButton::Right:
        if(in_bPress) {
            bUsed = pSys->injectMouseButtonDown(CEGUI::RightButton);
        } else {
            bUsed = pSys->injectMouseButtonUp(CEGUI::RightButton);
        }
        break;
    case MouseButton::Middle:
        if(in_bPress) {
            bUsed = pSys->injectMouseButtonDown(CEGUI::MiddleButton);
        } else {
            bUsed = pSys->injectMouseButtonUp(CEGUI::MiddleButton);
        }
        break;
    default:
        break;
    }

    // If a popup menu is currently opened, maybe we must close it.
    try {
        if(in_button == MouseButton::Left && GUI::getSingleton().getCurrentPopupMenu() != nullptr) {
            if(!GUI::getSingleton().getCurrentPopupMenu()->getPixelRect()
                    .isPointInRect(CEGUI::Point((float)m_uiCursorX, (float)m_uiCursorY))) {
                GUI::getSingleton().closeCurrentPopupMenu();
            }
        }
    } catch(...) {}

    // If the input has not been used, it means the user clicked outside of a
    // CEGUI element (clicked in no element) or at the window background.
    // This act deselects the currently active widget.
    if(!bUsed && GUI::getSingletonPtr() && in_bPress == true) {
        GUI::getSingleton().setActiveWidget(nullptr);
    }

    return bUsed;
}

/** The callback function called if a mouse button is pressed.
 *
 * \param in_iButton The button that is being pressed.
 *
 * \author Pompei2
 */
void InputManager::handleMouseButtonPress(MouseButton::Enum in_button)
{
    int idx = in_button-MouseButton::Left;
    if(idx < 0 || idx >= MouseButton::NoButton-MouseButton::Left)
        return;

    m_pressedButtons.insert(in_button);

#ifdef DEBUG_INPUT_MGR
    FTSMSGDBG("Mouse Button pressed:  "+String(getFTSButtonName(in_button))+" ("+String::nr(idx)+", "+String::b(m_keyTab[idx].bRepeating)+","+String::nr(m_uiCursorX)+","+String::nr(m_uiCursorY)+")\n", 5);
    String sPressedButtons = "  Currently Pressed buttons = [ ";
    for(std::set<MouseButton::Enum>::iterator i = m_pressedButtons.begin() ; i != m_pressedButtons.end() ; ++i) {
        sPressedButtons += getFTSButtonName(*i) + String(",");
    }
    sPressedButtons += "]\n";
    FTSMSGDBG(sPressedButtons, 5);
#endif

    PCursor pCurrCursor = RunlevelManager::getSingleton().getCurrRunlevel()->getActiveCursor();

    if(pCurrCursor != nullptr) {
        // Restart the cursor animation (if one).
        for(int i = 0; i < FTS_CURSOR_IMAGES; i++) {
            if(pCurrCursor->bAnimated[i])
                pCurrCursor->pAnim[i]->rewind();
        }

        // If the user clicks with the mouse, it skips the current demo and
        // displays the next one. TODO

        // set the current cursor state for the animation.
        pCurrCursor->pbState[in_button - SpecialKey::NoSpecial + 1] = true;
    }

    // If some key combos were executed, we stop handling the keys.
    if(m_ComboMgr->handle(in_button)) {
        return;
    }

    // If the GUI used the input, no further work is needed.
    if(this->injectToCEGUI(in_button, true))
        return;

    return;
}

/** The callback function called if a mouse button is pressed.
 *
 * \param in_iButton The button that is being pressed.
 *
 * \author Pompei2
 */
void InputManager::handleMouseButtonRelease(MouseButton::Enum in_button)
{
    int idx = in_button-MouseButton::Left;
    if(idx < 0 || idx >= MouseButton::NoButton-MouseButton::Left)
        return;

    // Adjust our keystates.
    m_pressedButtons.erase(in_button);

#ifdef DEBUG_INPUT_MGR
    FTSMSGDBG("Mouse Button released:  "+String(getFTSButtonName(in_button))+" ("+String::nr(idx)+","+String::nr(m_uiCursorX)+","+String::nr(m_uiCursorY)+")\n", 5);
    String sPressedButtons = "  Currently Pressed buttons = [ ";
    for(std::set<MouseButton::Enum>::iterator i = m_pressedButtons.begin() ; i != m_pressedButtons.end() ; ++i) {
        sPressedButtons += getFTSButtonName(*i) + String(",");
    }
    sPressedButtons += "]\n";
    FTSMSGDBG(sPressedButtons, 5);
#endif

    PCursor pCurrCursor = RunlevelManager::getSingleton().getCurrRunlevel()->getActiveCursor();

    // set the current cursor state for the animation.
    if(pCurrCursor != nullptr) {
        pCurrCursor->pbState[in_button - SpecialKey::NoSpecial + 1] = false;
    }

    // And now check if the release is used by any hotkey.
    if(m_ComboMgr->handle(in_button))
        return;

    // If the GUI used the input, no further work is needed.
    if(this->injectToCEGUI(in_button, false))
        return;

    return;
}

/** Sets a new cursor pos ("Warps" the cursor).
 *
 * \param in_iX The new cursor's x pos.
 * \param in_iY The new cursor's y pos.
 *
 * \note Generates a mousemove event.
 * \author Pompei2
 */
void InputManager::simulateMouseMove(uint16_t in_iX, uint16_t in_iY)
{
    SDL_WarpMouseGlobal(in_iX, in_iY);
}

void InputManager::simulateMouseClick(FTS::MouseButton::Enum in_button)
{
    this->handleMouseButtonPress(in_button);
    this->handleMouseButtonRelease(in_button);
}

void InputManager::simulateMouseClick(FTS::MouseButton::Enum in_button, uint16_t in_X, uint16_t in_Y)
{
    uint16_t oldX = this->getMouseX();
    uint16_t oldY = this->getMouseY();
    this->simulateMouseMove(in_X, in_Y);
    this->simulateMouseClick(in_button);
    this->simulateMouseMove(oldX, oldY);
}

void InputManager::simulateKeyPress(FTS::Key::Enum in_key)
{
    this->simulateKeyDown(in_key);
    this->simulateKeyUp(in_key);
}

void InputManager::simulateKeyDown(FTS::Key::Enum in_key)
{
    this->handleKeyDown(in_key);
}

void InputManager::simulateKeyUp(FTS::Key::Enum in_key)
{
    this->handleKeyUp(in_key);
}

/** Registers the default keyboard shortcuts that are used in the menu-runlevels
 *  that means all shortcuts that handle GUI stuff as well as the shortcuts to
 *  move the camera within the little 3d demo.
 *
 * \param in_bWithCamera Also register the camera shortcuts ?
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int InputManager::registerDefaultMenuShortcuts(bool in_bWithCamera)
{
    // Register some default keyboard shortcuts.
    InputCombo *pCtrlModif = NULL;

    if(in_bWithCamera) {
        // Create the up key shortcut.
        InputCombo *up = new InputCombo("cam/up", Key::ArrowUp, new CameraCmd(CameraCmd::moveUp, 0.5f));
        // Add left/right ctrl to the up key.
        pCtrlModif = new InputCombo("cam/up/rot", SpecialKey::Control, new CameraCmd(CameraCmd::rotateGlobalX, deg2rad*1.0f));
        up->addModifier(pCtrlModif);
        // Add left/right shift to the up key.
        up->addModifier(new InputCombo("cam/up/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::moveUp, 0.05f)));
        // Add left/right shift to the left/right ctrl + up key combo.
        pCtrlModif->addModifier(new InputCombo("cam/up/rot/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::rotateX, deg2rad*0.05f)));
        this->add(up);    // Add the whole set to the manager.

        // Create the up key shortcut.
        InputCombo *down = new InputCombo("cam/down", Key::ArrowDown, new CameraCmd(CameraCmd::moveUp, -0.5f));
        pCtrlModif = new InputCombo("cam/down/rot", SpecialKey::Control, new CameraCmd(CameraCmd::rotateGlobalX, deg2rad*-1.0f));
        down->addModifier(pCtrlModif);
        down->addModifier(new InputCombo("cam/down/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::moveUp, -0.05f)));
        pCtrlModif->addModifier(new InputCombo("cam/down/rot/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::rotateX, deg2rad*-0.05f)));
        this->add(down);

        // Create the left key shortcut set.
        InputCombo *left = new InputCombo("cam/left", Key::ArrowLeft, new CameraCmd(CameraCmd::moveRight, -0.5f));
        pCtrlModif = new InputCombo("cam/left/rot", SpecialKey::Control, new CameraCmd(CameraCmd::rotateGlobalY, deg2rad*1.0f));
        left->addModifier(pCtrlModif);
        left->addModifier(new InputCombo("cam/left/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::moveRight, -0.05f)));
        pCtrlModif->addModifier(new InputCombo("cam/left/rot/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::rotateY, deg2rad*0.05f)));
        this->add(left);

        // Create the right key shortcut set.
        InputCombo *right = new InputCombo("cam/right", Key::ArrowRight, new CameraCmd(CameraCmd::moveRight, 0.5f));
        pCtrlModif = new InputCombo("cam/right/rot", SpecialKey::Control, new CameraCmd(CameraCmd::rotateGlobalY, deg2rad*-1.0f));
        right->addModifier(pCtrlModif);
        right->addModifier(new InputCombo("cam/right/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::moveRight, 0.05f)));
        pCtrlModif->addModifier(new InputCombo("cam/right/rot/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::rotateY, deg2rad*-0.05f)));
        this->add(right);

        // Create the page up key shortcut set.
        InputCombo *pgup = new InputCombo("cam/pgup", Key::PageUp, new CameraCmd(CameraCmd::moveFront, 0.5f));
        pCtrlModif = new InputCombo("cam/pgup/rot", SpecialKey::Control, new CameraCmd(CameraCmd::rotateGlobalZ, deg2rad*1.0f));
        pgup->addModifier(pCtrlModif);
        pgup->addModifier(new InputCombo("cam/pgup/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::moveFront, 0.05f)));
        pCtrlModif->addModifier(new InputCombo("cam/pgup/rot/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::rotateZ, deg2rad*0.05f)));
        this->add(pgup);

        // Create the page down key shortcut set.
        InputCombo *pgdown = new InputCombo("cam/pgdown", Key::PageDown, new CameraCmd(CameraCmd::moveFront, -0.5f));
        pCtrlModif = new InputCombo("cam/pgdown/rot", SpecialKey::Control, new CameraCmd(CameraCmd::rotateGlobalZ, deg2rad*-1.0f));
        pgdown->addModifier(pCtrlModif);
        pgdown->addModifier(new InputCombo("cam/pgdown/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::moveFront, -0.05f)));
        pCtrlModif->addModifier(new InputCombo("cam/pgdown/rot/slow", SpecialKey::Shift, new CameraCmd(CameraCmd::rotateZ, deg2rad*-0.05f)));
        this->add(pgdown);

        // Create the 1 keyboard shortcut.
        this->add("cam/home", Key::One, new CameraPosCmd(Vector(0.0f,0.0f,10.0f)));
        // Create the 2 keyboard shortcut.
        this->add("cam/lookatzero", Key::Two, new CameraLookAtCmd(Vector(0.0f,0.0f,0.0f)));
    }

    // In contrast to that code:
//     if(InputManager::getManager()->isKeyPressed(GLFW_KEY_UP)) {
//         if(InputManager::getManager()->isKeyPressed(GLFW_KEY_LCTRL) ||
//            InputManager::getManager()->isKeyPressed(GLFW_KEY_RCTRL)) {
//             if(InputManager::getManager()->isKeyPressed(GLFW_KEY_LSHIFT) ||
//                InputManager::getManager()->isKeyPressed(GLFW_KEY_RSHIFT)) {
//                 globals->pCam->...
//             } else {
//                 globals->pCam->...
//             }
//         } else if(InputManager::getManager()->isKeyPressed(GLFW_KEY_LSHIFT) ||
//                   InputManager::getManager()->isKeyPressed(GLFW_KEY_RSHIFT)) {
//             globals->pCam->...
//         } else {
//             globals->pCam->...
//         }
//     }

    this->add("polygonmode", Key::F11, new NextPolyModeCmd);
#if defined(DEBUG) && defined(WINDOOF) // In VS Debugger F12 results in a break.
    this->add("nextGuiInfo", Key::F8, new NextGUIInfoCmd);
#else
    this->add("nextGuiInfo", Key::F12, new NextGUIInfoCmd);
#endif

    // Now GUI-keys to handle widgets.
    this->add("pushActiveButton", Key::Space, new ClickButtonCmd);
    this->add("toggleActiveCheckbox", Key::Space, new ToggleCheckboxCmd);
    this->add("selectActiveRadiobtn", Key::Space, new SelectRadiobuttonCmd);
    this->add("comboboxDown", Key::ArrowDown, new ComboboxNextPrevCmd(true));
    this->add("comboboxUp",   Key::ArrowUp,   new ComboboxNextPrevCmd(false));
    this->add("listboxDown", Key::ArrowDown, new ListboxNextPrevCmd(true));
    this->add("listboxUp",   Key::ArrowUp,   new ListboxNextPrevCmd(false));
    this->add("SpinnerDown",    Key::ArrowDown, new SpinnerCmd(false));
    this->add("SpinnerUp",      Key::ArrowUp,   new SpinnerCmd(true));
    this->add("SpinnerMinus",   Key::Minus,     new SpinnerCmd(false));
    this->add("SpinnerPlus",    Key::Add,       new SpinnerCmd(true));
    this->add("ScrollbarVUp",      Key::ArrowUp,       new ScrollbarCmd(false, false, ScrollbarCmd::VertOnly));
    this->add("ScrollbarVDown",    Key::ArrowDown,     new ScrollbarCmd(true,  false, ScrollbarCmd::VertOnly));
    this->add("ScrollbarHDown",    Key::ArrowDown,     new ScrollbarCmd(false, false, ScrollbarCmd::HorizOnly));
    this->add("ScrollbarHUp",      Key::ArrowUp,       new ScrollbarCmd(true,  false, ScrollbarCmd::HorizOnly));
    this->add("ScrollbarHRight",   Key::ArrowRight,    new ScrollbarCmd(true,  false, ScrollbarCmd::HorizOnly));
    this->add("ScrollbarHLeft",    Key::ArrowLeft,     new ScrollbarCmd(false, false, ScrollbarCmd::HorizOnly));
    this->add("ScrollbarVPgUp",    Key::PageUp,   new ScrollbarCmd(false, true,  ScrollbarCmd::VertOnly));
    this->add("ScrollbarVPgDown",  Key::PageDown, new ScrollbarCmd(true,  true,  ScrollbarCmd::VertOnly));
    this->add("ScrollbarHPgUp",    Key::PageUp,   new ScrollbarCmd(true,  true,  ScrollbarCmd::HorizOnly));
    this->add("ScrollbarHPgDown",  Key::PageDown, new ScrollbarCmd(false, true,  ScrollbarCmd::HorizOnly));
    this->add("ScrollbarHPlus",    Key::Add,      new ScrollbarCmd(true,  false, ScrollbarCmd::HorizOnly));
    this->add("ScrollbarHMinus",   Key::Minus,    new ScrollbarCmd(false, false, ScrollbarCmd::HorizOnly));
    this->add("Happily tabbing", Key::Tab, new TabNavigationCmd());
    this->add("Happily Stop tabbing", MouseButton::Any, new TabNavigationStopCmd());
    this->add("Script Console", ScriptConsole::getHotKey() , new OpenScriptConsoleCmd(), false);

#ifdef DEBUG
    this->add("debug/switchnormals", Key::F2, new SwitchGlobalDrawNormalsCmd);
    this->add("debug/print_win", Key::F10, new DebugPringWindowHiearachyCmd);
#endif

    return ERR_OK;
}

/** Unregisters the default keyboard shortcuts that are used in the
 *  menu-runlevels, that are all registered by registerDefaultMenuShortcuts.
 *
 * \param in_bWithCamera Also unregister the camera shortcuts ?
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int InputManager::unregisterDefaultMenuShortcuts(bool in_bWithCamera)
{
    if(in_bWithCamera) {
        this->delShortcut("cam/up");
        this->delShortcut("cam/down");
        this->delShortcut("cam/left");
        this->delShortcut("cam/right");
        this->delShortcut("cam/pgup");
        this->delShortcut("cam/pgdown");
        this->delShortcut("cam/home");
        this->delShortcut("cam/lookatzero");
    }

    this->delShortcut("polygonmode");
    this->delShortcut("nextGuiInfo");
    this->delShortcut("pushActiveButton");
    this->delShortcut("toggleActiveCheckbox");
    this->delShortcut("selectActiveRadiobtn");
    this->delShortcut("comboboxDown");
    this->delShortcut("comboboxUp");
    this->delShortcut("listboxDown");
    this->delShortcut("listboxUp");
    this->delShortcut("SpinnerDown");
    this->delShortcut("SpinnerUp");
    this->delShortcut("SpinnerMinus");
    this->delShortcut("SpinnerPlus");
    this->delShortcut("ScrollbarVUp");
    this->delShortcut("ScrollbarVDown");
    this->delShortcut("ScrollbarHDown");
    this->delShortcut("ScrollbarHUp");
    this->delShortcut("ScrollbarHRight");
    this->delShortcut("ScrollbarHLeft");
    this->delShortcut("ScrollbarVPgUp");
    this->delShortcut("ScrollbarVPgDown");
    this->delShortcut("ScrollbarHPgUp");
    this->delShortcut("ScrollbarHPgDown");
    this->delShortcut("ScrollbarHPlus");
    this->delShortcut("ScrollbarHMinus");
    this->delShortcut("Happily tabbing");
    this->delShortcut("Happily Stop tabbing");
    this->delShortcut("Script Console");

#ifdef DEBUG
    this->delShortcut("debug/switchnormals");
    this->delShortcut("debug/print_win");
#endif

    return ERR_OK;
}

void FTS::InputManager::pushContext()
{
    m_ctxInputComboManagers.push(m_ComboMgr);
    m_ComboMgr = new InputComboManager();
}

void FTS::InputManager::popContext()
{
    if( m_ctxInputComboManagers.empty() ) {
        throw InvalidCallException("InputManager::popContext(): a pushContext is missing.");
    }
    delete m_ComboMgr;
    m_ComboMgr = m_ctxInputComboManagers.top();
    m_ctxInputComboManagers.pop();
}

/* EOF */
