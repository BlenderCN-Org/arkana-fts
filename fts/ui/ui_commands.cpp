#include "ui/ui_commands.h"
#include "ui/ui.h"
#include "input/input.h"
#include "main/runlevels.h"
#include "logging/logger.h"

#include "3rdparty/scrap.h"

#include <CEGUI.h>

using namespace FTS;

bool NextPolyModeCmd::exec()
{
    if(GUI::getSingletonPtr()) {
        GUI::getSingleton().set(GUI::getSingleton().next(GUI::getSingleton().getPolyMode()));
        return true;
    }
    return false;
}

bool NextGUIInfoCmd::exec()
{
    if(GUI::getSingletonPtr()) {
        GUI::getSingleton().set(GUI::getSingleton().next(GUI::getSingleton().getGUIInfo()));
        return true;
    }
    return false;
}

/** Builds a ActiveWindowCheckCmd that, upon execution, checks if the
 *  active (at execution time) (CEGUI Frame-)window has the name that is
 *  given in the constructor.
 *
 * \param in_sWindowName The name to check the active window for.
 *
 * \author Pompei2
 **/
ActiveWindowCheckCmd::ActiveWindowCheckCmd(const String &in_sWindowName)
    : m_sWindowName(in_sWindowName)
{
}

/// Copy constructor.
ActiveWindowCheckCmd::ActiveWindowCheckCmd(const ActiveWindowCheckCmd &o)
    : m_sWindowName(o.m_sWindowName)
{
}

/** Checks if the currently active (CEGUI Frame-)window has the name that is
 *  specified by the \a m_sWindowName member.
 *
 * \return true if the currently active window has that name, false else.
 *
 * \author Pompei2
 **/
bool ActiveWindowCheckCmd::exec()
{
    if(!GUI::getSingletonPtr())
        return false;

    CEGUI::Window *pWin = GUI::getSingleton().getActiveWin();
    if(!pWin)
        return false;

    return m_sWindowName == pWin->getName().c_str();
}

/// \return the window that should be used for the command, or NULL if none should be used.
CEGUI::Window *CEGUIKeyCmd::getWinToUse()
{
    CEGUI::Window *pWin = NULL;

    // If the button name is empty, get the active button.
    if(m_sName == String::EMPTY) {
        pWin = GUI::getSingleton().getActiveWidget();
    // If it isn't empty, get that button.
    } else {
        if(CEGUI::WindowManager::getSingleton().isWindowPresent(m_sName)) {
            pWin = CEGUI::WindowManager::getSingleton().getWindow(m_sName);
        }
    }

    return pWin;
}

/** Builds a ClickButtonCmd that, upon execution, clicks on a button.
 *
 * \param in_sName The name of the button to click on.
 *
 * \note If the name of the button is an empty string (the default), the
 *       button that is active upon execution will be clicked.
 *
 * \author Pompei2
 **/
ClickButtonCmd::ClickButtonCmd(const String &in_sName)
    : CEGUIKeyCmd(in_sName)
{
}

/// Copy constructor.
ClickButtonCmd::ClickButtonCmd(const ClickButtonCmd &o)
    : CEGUIKeyCmd(o)
{
}

bool ClickButtonCmd::exec()
{
    // Convert it to a button to see if it really is one.
    CEGUI::PushButton *pBtn = dynamic_cast<CEGUI::PushButton *>(this->getWinToUse());
    if(pBtn == NULL)
        return false;

    try {
        CEGUI::Rect r = pBtn->getPixelRect();

        // Click on the button.
        CEGUI::System::getSingleton().injectMousePosition(r.d_left+1.f,r.d_top+1.f);
        CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);
        CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::LeftButton);

        // Move the cursor back.
        PCursor pCurrCursor = RunlevelManager::getSingleton().getCurrRunlevel()
                                                            ->getActiveCursor();
        if(pCurrCursor) {
            CEGUI::System::getSingleton().injectMousePosition((float)pCurrCursor->iX,
                                                              (float)pCurrCursor->iY);
        }
    } catch(CEGUI::Exception &) { }

    return true;
}

/** Builds a ToggleCheckboxCmd that, upon execution, toggles a checkbox.
 *
 * \param in_sName The name of the checkbox to toggle.
 *
 * \note If the name of the checkbox is an empty string (the default), the
 *       checkbox that is active upon execution will be clicked.
 *
 * \author Pompei2
 **/
ToggleCheckboxCmd::ToggleCheckboxCmd(const String &in_sName)
    : CEGUIKeyCmd(in_sName)
{
}

/// Copy constructor.
ToggleCheckboxCmd::ToggleCheckboxCmd(const ToggleCheckboxCmd &o)
    : CEGUIKeyCmd(o)
{
}

bool ToggleCheckboxCmd::exec()
{
    // Convert it to a button to see if it really is one.
    CEGUI::Checkbox *pCB = dynamic_cast<CEGUI::Checkbox *>(this->getWinToUse());
    if(pCB == NULL)
        return false;

    // Toggle the checkbox.
    try {
        pCB->setSelected(!pCB->isSelected());
    } catch(CEGUI::Exception &) { }

    return true;
}

/** Builds a SelectRadiobuttonCmd that, upon execution, selects a radiobutton.
 *
 * \param in_sName The name of the radiobutton to select.
 *
 * \note If the name of the radiobutton is an empty string (the default), the
 *       radiobutton that is active upon execution will be selected.
 *
 * \author Pompei2
 **/
SelectRadiobuttonCmd::SelectRadiobuttonCmd(const String &in_sName)
    : CEGUIKeyCmd(in_sName)
{
}

/// Copy constructor.
SelectRadiobuttonCmd::SelectRadiobuttonCmd(const ToggleCheckboxCmd &o)
    : CEGUIKeyCmd(o)
{
}

bool SelectRadiobuttonCmd::exec()
{
    // Convert it to a button to see if it really is one.
    CEGUI::RadioButton *pRB = dynamic_cast<CEGUI::RadioButton *>(this->getWinToUse());
    if(pRB == NULL)
        return false;

    // Toggle the checkbox.
    try {
        pRB->setSelected(true);
    } catch(CEGUI::Exception &) { }

    return true;
}

/** Builds a ComboboxNextPrevCmd that, upon execution, changes what item is
 *  currently selected in a combobox.
 *
 * \param in_sName The name of the combobox to act on.
 * \param in_bNext If that is true, this command will select the entry AFTER the
 *                 current one, else it will select the entry BEFORE the current
 *                 one.
 *
 * \note If the name of the combobox is an empty string (the default), the
 *       combobox that is active upon execution will be selected.
 *
 * \author Pompei2
 **/
ComboboxNextPrevCmd::ComboboxNextPrevCmd(bool in_bNext, const String &in_sName)
    : CEGUIKeyCmd(in_sName)
    , m_bNext(in_bNext)
{
}

/// Copy constructor.
ComboboxNextPrevCmd::ComboboxNextPrevCmd(const ComboboxNextPrevCmd &o)
    : CEGUIKeyCmd(o)
    , m_bNext(o.m_bNext)
{
}

bool ComboboxNextPrevCmd::exec()
{
    // Convert it to a button to see if it really is one.
    CEGUI::Combobox *pCB = dynamic_cast<CEGUI::Combobox *>(this->getWinToUse());
    if(pCB == NULL)
        return false;

    size_t iCurrIdx = (size_t)-1;

    // Select the next/prev only if it exists.
    try {
        // Work around as the getSelectedItem does not seem to always work...
        iCurrIdx = pCB->getItemIndex(pCB->findItemWithText(pCB->getText(), NULL));
//         size_t iCurrIdx = pCB->getItemIndex(pCB->getSelectedItem());
        CEGUI::ListboxItem *pLI = NULL;
        if(m_bNext) {
            pLI = pCB->getListboxItemFromIndex(iCurrIdx+1);
        } else {
            pLI = pCB->getListboxItemFromIndex(iCurrIdx-1);
        }
        pCB->setItemSelectState(pLI, true);
        pCB->setItemSelectState(iCurrIdx, false);
        pCB->setText(pLI->getText());
        pLI->setSelected(true);
        pCB->getDropList()->ensureItemIsVisible(pLI);
    } catch(CEGUI::Exception &) { }

    // If nothing is selected, select the first item.
    if(iCurrIdx == (size_t)-1) {
        try {
            pCB->setItemSelectState((size_t)0, true);
        } catch(CEGUI::Exception &) { }
    }

    // Even if the next does not exist, the user tried to interact
    // with the GUI by doing this, so the input got eaten up.
    return true;
}

/** Builds a ListboxNextPrevCmd that, upon execution, changes what item is
 *  currently selected in a listbox.
 *
 * \param in_sName The name of the listbox to act on.
 * \param in_bNext If that is true, this command will select the entry AFTER the
 *                 current one, else it will select the entry BEFORE the current
 *                 one.
 *
 * \note If the name of the listbox is an empty string (the default), the
 *       listbox that is active upon execution will be selected.
 *
 * \author Pompei2
 **/
ListboxNextPrevCmd::ListboxNextPrevCmd(bool in_bNext, const String &in_sName)
    : CEGUIKeyCmd(in_sName)
    , m_bNext(in_bNext)
{
}

/// Copy constructor.
ListboxNextPrevCmd::ListboxNextPrevCmd(const ListboxNextPrevCmd &o)
    : CEGUIKeyCmd(o)
    , m_bNext(o.m_bNext)
{
}

bool ListboxNextPrevCmd::exec()
{
    // Convert it to a button to see if it really is one.
    CEGUI::Listbox *pLB = dynamic_cast<CEGUI::Listbox *>(this->getWinToUse());
    if(pLB == NULL)
        return false;

    size_t iCurrIdx = (size_t)-1;

    // Select the next/prev only if it exists.
    try {
        iCurrIdx = pLB->getItemIndex(pLB->getFirstSelectedItem());
        CEGUI::ListboxItem *pLI = NULL;
        if(m_bNext) {
            pLI = pLB->getListboxItemFromIndex(iCurrIdx+1);
        } else {
            pLI = pLB->getListboxItemFromIndex(iCurrIdx-1);
        }
        pLB->setItemSelectState(pLI, true);
        pLB->setItemSelectState(iCurrIdx, false);
        pLI->setSelected(true);
        pLB->ensureItemIsVisible(pLI);
    } catch(CEGUI::Exception &) { }

    // If nothing is selected, select the first item.
    if(iCurrIdx == (size_t)-1) {
        try {
            pLB->setItemSelectState((size_t)0, true);
        } catch(CEGUI::Exception &) { }
    }

    // Even if the next does not exist, the user tried to interact
    // with the GUI by doing this, so the input got eaten up.
    return true;
}

/** Builds a SpinnerCmd that, upon execution, will increase or decrease
 *  a spinner's value.
 *
 * \param in_sName The name of the spinner to act on.
 * \param in_bIncr If that is true, this command will increase the spinner's
 *                 value, if it's false, it will decrease the spinner's value.
 *
 * \note If the name of the spinner is an empty string (the default), the
 *       spinner that is active upon execution will be selected.
 *
 * \author Pompei2
 **/
SpinnerCmd::SpinnerCmd(bool in_bIncr, const String &in_sName)
    : CEGUIKeyCmd(in_sName)
    , m_bIncr(in_bIncr)
{
}

/// Copy constructor.
SpinnerCmd::SpinnerCmd(const SpinnerCmd &o)
    : CEGUIKeyCmd(o)
    , m_bIncr(o.m_bIncr)
{
}

bool SpinnerCmd::exec()
{
    // Convert it to a button to see if it really is one.
    CEGUI::Spinner *pSp = dynamic_cast<CEGUI::Spinner *>(this->getWinToUse());
    if(pSp == NULL)
        return false;

    pSp->setCurrentValue(pSp->getCurrentValue() + pSp->getStepSize());
    return true;
}

/** Builds a ScrollbarCmd that, upon execution, will increase or
 *  a scrollbar's value.
 *
 * \param in_sName The name of the scrollbar to act on.
 * \param in_bIncr If that is true, this command will increase the scrollbar's
 *                 value, if it's false, it will decrease the scrollbar's value.
 * \param in_bPage If that is true, this command will modify the scrollbar's value
 *                 by one pagesize, else it will by one stepsize.
 *
 * \note If the name of the scrollbar is an empty string (the default), the
 *       scrollbar that is active upon execution will be selected.
 *
 * \author Pompei2
 **/
ScrollbarCmd::ScrollbarCmd(bool in_bIncr, bool in_bPage,
                                   ScrollbarCmd::ScrollbarType in_type,
                                   const String &in_sName)
    : CEGUIKeyCmd(in_sName)
    , m_bIncr(in_bIncr)
    , m_bPage(in_bPage)
    , m_type(in_type)
{
}

/// Copy constructor.
ScrollbarCmd::ScrollbarCmd(const ScrollbarCmd &o)
    : CEGUIKeyCmd(o)
    , m_bIncr(o.m_bIncr)
    , m_bPage(o.m_bPage)
    , m_type(o.m_type)
{
}

bool ScrollbarCmd::exec()
{
    // Convert it to a button to see if it really is one.
    CEGUI::Scrollbar *pSb = dynamic_cast<CEGUI::Scrollbar *>(this->getWinToUse());
    if(pSb == NULL)
        return false;

    switch(m_type) {
    case ScrollbarCmd::HorizOnly:
    {
        CEGUI::String sType = pSb->getType();
        if(sType.find("Horiz") == CEGUI::String::npos)
            return false;
        break;
    }
    case ScrollbarCmd::VertOnly:
    {
        CEGUI::String sType = pSb->getType();
        if(sType.find("Vert") == CEGUI::String::npos)
            return false;
        break;
    }
    default: break;
    }

    float fModif = 0.0f;

    if(m_bPage) {
        fModif = pSb->getPageSize();
    } else {
        fModif = pSb->getStepSize();
    }

    if(!m_bIncr) {
        fModif = -fModif;
    }

    pSb->setScrollPosition(pSb->getScrollPosition() + fModif);
    return true;
}


/// Navigates trough all the widgets.
/** This function navigates trough all widgets, using the ID as tab order.
 *
 * \return the window that got activated (selected).
 *
 * \author Pompei2
 */
bool FTS::TabNavigationCmd::exec()
{
    // Get the currently active frame window.
    CEGUI::Window *pActiveFW = GUI::getSingleton().getActiveWin();
    if(pActiveFW == NULL)
        return NULL;

    // find the next tab order window and activate it.
    CEGUI::Window *pNext = this->getNextTabOrderWin(pActiveFW, GUI::getSingleton().getActiveWidget());
    if(pNext) {
        GUI::getSingleton().setActiveWidget(pNext);
        GUI::getSingleton().tabbing(true);
        return true;
    }

    return false;
}

/// Finds the next (in tab order) widget in this window.
/** This function goes trough all visible and enabled childs of
 *  the window and compares their ID with the one of the currently
 *  active child. It returns the next one.\n
 *  If currently none of its childs has the focus, it returns the
 *  first one with the lowest ID that is nonzero.
 *
 * \param in_pFrame The Window whose next child to find.
 * \param in_pCurr The currently active window, can be NULL to find first tab window.
 *
 * \return The next window (widget) in the tab order list or, if none,
 *         the first one. Might also return NULL if no child with an ID != 0 was found.
 *
 * \author Pompei2
 */
CEGUI::Window *FTS::TabNavigationCmd::getNextTabOrderWin(CEGUI::Window *in_pFrame, CEGUI::Window *in_pCurr)
{
    if(in_pFrame == NULL)
        return NULL;

    // Nothing to do here.
    if(in_pFrame->getChildCount() < 2)
        return NULL;

    // If currently none is active, return the first one.
    if(in_pCurr == NULL)
        return this->findFirstChildRecursive(in_pFrame);

    CEGUI::Window *pChild = NULL;
    uint32_t uiID = static_cast<uint32_t>(-1); // maximum.

    // If the currently active widget is a direct child, we can just travel.
    if(in_pFrame->isChild(in_pCurr)) {
        // Just find the next one to have.
        CEGUI::Window *pNext = this->findNextChildRecursive(in_pFrame, in_pCurr);

        // We got at the end, restart.
        if(pNext == NULL) {
            return this->findFirstChildRecursive(in_pFrame);
        }

        // If it is a container, find the first in it.
        if(FTS::GUI::isContainer(pNext)) {
            pNext = this->findFirstChildRecursive(pNext);
        }

        return pNext;
    }

    // Else, we need to walk trough the frame window's childs to find the one
    // that contains the currently active one.
    for(size_t i = 0 ; i < in_pFrame->getChildCount() ; i++) {
        pChild = in_pFrame->getChildAtIdx(i);

        // Skip invisible, disabled and auto windows.
        if(!pChild->isVisible() ||
            pChild->isDisabled() ||
            pChild->getName().find(CEGUI::Window::AutoWidgetNameSuffix) != CEGUI::String::npos)
            continue;

        uiID = pChild->getID( );

        // Also skip widgets with ID 0 (means no ID has been chosen)
        if(uiID == 0)
            continue;

        // If it contains the currently active widget, we have luck.
        if(FTS::GUI::isChildRecursive(pChild, in_pCurr)) {
            // Just find the next to have.
            CEGUI::Window *pNext = this->findNextChildRecursive(pChild, in_pCurr);

            // If it was the last one, go out of here, find the next one in the frame window.
            if(pNext == NULL) {
                CEGUI::Window *pStart = pChild;
                do {
                    pNext = this->findNextChildRecursive(in_pFrame, pStart);

                    // This means it was the last at all.
                    if(pNext == NULL) {
                        // Then restart at the beginning.
                        return this->findFirstChildRecursive(in_pFrame);
                    }

                    // If it is a container, find the first in it.
                    if(FTS::GUI::isContainer(pNext)) {
                        pStart = pNext;
                        pNext = this->findFirstChildRecursive(pNext);

                        if(pNext != NULL)
                            return pNext;

                        // If it has no children, continue to the next widget.
                        continue;
                    } else
                        return pNext;
                } while(true);
            }

            return pNext;
        }

        // Reset it to say we didn't find anything yet.
        pChild = NULL;
    }

    return NULL;
}

/// Finds the first (in tab order) widget in this window and all its childs.
/** This function goes trough all visible and enabled childs of
 *  the window and also their childs, to find the first widget in tab order
 *  and returns it. It may return null if there is no such window.
 *
 * \param in_pParent The Window where to begin the search.
 *
 * \return The first window (widget) in the tab order list.
 *         Might also return NULL if no child with an ID != 0 was found.
 *
 * \author Pompei2
 */
CEGUI::Window *FTS::TabNavigationCmd::findFirstChildRecursive(CEGUI::Window *in_pParent)
{
    CEGUI::Window *pChild = NULL;
    CEGUI::Window *pLowest = NULL;
    unsigned int uiLowestID = static_cast<uint32_t>(-1);
    unsigned int uiID = static_cast<uint32_t>(-1);
    unsigned int uiMinRequired = 0;

find:
    for(size_t i = 0 ; i < in_pParent->getChildCount() ; i++) {
        pChild = in_pParent->getChildAtIdx(i);

        // Skip invisible, disabled and auto windows.
        if(!pChild->isVisible() ||
            pChild->isDisabled() ||
            pChild->getName().find(CEGUI::Window::AutoWidgetNameSuffix) != CEGUI::String::npos)
            continue;

        uiID = pChild->getID( );

        // Also skip widgets with ID 0 (means no ID has been chosen)
        if(uiID == 0 || uiID <= uiMinRequired)
            continue;

        if(uiID < uiLowestID) {
            uiLowestID = uiID;
            pLowest = pChild;
        }
    }

    // If it is a container, find the first one in it.
    if(pLowest && FTS::GUI::isContainer(pLowest)) {
        uiMinRequired = pLowest->getID();
        pLowest = this->findFirstChildRecursive(pLowest);

        // If the first was a container, but had no good childs, find the next one here.
        if(pLowest == NULL) {
            uiLowestID = static_cast<uint32_t>(-1);
            // uiMinRequired was adapted above, so we will find the next if we restart.
            goto find;
        }
    }

    return pLowest;
}

/// Finds the next (in tab order) widget in this window.
/** This function goes trough all visible and enabled childs of
 *  the window, to find the next widget in tab order and returns it.
 *  It may return null if it reaches the end.
 *
 * \param in_pParent The window where to begin the search.
 * \param in_pStart The widget that is currently active.
 *
 * \return The next window (widget) in the tab order list.
 *         Might also return NULL if no child with an ID != 0 was found.
 *
 * \author Pompei2
 */
CEGUI::Window *FTS::TabNavigationCmd::findNextChild(CEGUI::Window *in_pParent, CEGUI::Window *in_pStart)
{
    CEGUI::Window *pChild = NULL;
    CEGUI::Window *pNext = NULL;
    unsigned int uiMaxID = static_cast<uint32_t>(-1);
    unsigned int uiID = static_cast<uint32_t>(-1);

    for(size_t i = 0 ; i < in_pParent->getChildCount() ; i++) {
        pChild = in_pParent->getChildAtIdx(i);

        // Skip invisible, disabled and auto windows.
        if(!pChild->isVisible() ||
            pChild->isDisabled() ||
            pChild->getName().find(CEGUI::Window::AutoWidgetNameSuffix) != CEGUI::String::npos)
            continue;

        uiID = pChild->getID( );

        // Also skip widgets with ID 0 (means no ID has been chosen)
        if(uiID == 0)
            continue;

        if(uiID > in_pStart->getID() && uiID < uiMaxID) {
            uiMaxID = uiID;
            pNext = pChild;
        }
    }

    // If it is a container, find the first one in it.
    if(pNext && FTS::GUI::isContainer(pNext)) {
        pNext = this->findFirstChildRecursive(pNext);
    }

    return pNext;
}

/// Finds the next (in tab order) widget in this window, recursing into child windows.
/** This function goes trough all visible and enabled childs of
 *  the window and also their childs, to find the next widget in
 *  tab order and returns it.
 *  It may return null if it reaches the end of all.
 *
 * \param in_pParent The window where to begin the search.
 * \param in_pStart The widget that is currently active.
 *
 * \return The next window (widget) in the tab order list.
 *         Might also return NULL if no child with an ID != 0 was found.
 *
 * \author Pompei2
 */
CEGUI::Window *FTS::TabNavigationCmd::findNextChildRecursive(CEGUI::Window *in_pParent, CEGUI::Window *in_pStart)
{
    CEGUI::Window *pChild = NULL;
    CEGUI::Window *pStart = in_pStart;
    CEGUI::Window *pParent = in_pStart;

    do {
        pStart = pParent;
        pParent = pParent->getParent();

        // If we reached the topmost parent, stop here.
        if(pParent == in_pParent)
            return this->findNextChild(in_pParent, pStart);

        // Else, find my next brother.
        pChild = this->findNextChild(pParent, pStart);
        if(pChild != NULL)
            return pChild;

        // If we reach here, it means we finished this container and have to go to
        // the next parent child pair.
    } while(true);

    return NULL;
}

bool FTS::TabNavigationStopCmd::exec()
{
    GUI::getSingleton().tabbing(false);
    return false; // We pass-through, we don't want to hinder clicks.
}


/// Copies the currently selected text.
/** This function copies the currently selected text (maybe more to come ?)
 *  into the clipboard. It is system-independent !
 *
 * \return true if something got copied, false if not.
 *
 * \author Pompei2
 */
FTS::String FTS::CopyCutCmdBase::extractText() const
{
    CEGUI::Window *activeWin = GUI::getSingleton().getActiveWidget();

    if(activeWin == NULL)
        return String::EMPTY;

    // Cancel, if the active widget is not an editable.
    if(activeWin->getType() != "ArkanaLook/Editbox" &&
       activeWin->getType() != "ArkanaLook/MultiLineEditbox")
        return String::EMPTY;

    // Get the currently selected text from the widget.

    // First, for normal editboxes:
    if(activeWin->getType() == "ArkanaLook/Editbox") {
        CEGUI::Editbox *e = (CEGUI::Editbox *)activeWin;

        // No need to copy an empty selection.
        if(e->getSelectionLength() == 0)
            return String::EMPTY;

        return CEGUI::String(e->getText(), e->getSelectionStartIndex(), e->getSelectionLength());
    // Then, for normal MultiLineEditbox:
    } else if(activeWin->getType() == "ArkanaLook/MultiLineEditbox") {
        CEGUI::MultiLineEditbox *e = (CEGUI::MultiLineEditbox *)activeWin;

        // No need to copy an empty selection.
        if(e->getSelectionLength() == 0)
            return String::EMPTY;

        return CEGUI::String(e->getText(), e->getSelectionStartIndex(), e->getSelectionLength());
    }

    return String::EMPTY;
}

// Some statics for the clipboard laying around here...
String FTS::ClipboardBase::m_sOwnClipboard;
bool FTS::ClipboardBase::m_bClipboardReady = false;

bool FTS::ClipboardBase::initClipboardIfNeeded()
{
    if(!m_bClipboardReady || lost_scrap()) {
        if(init_scrap() < 0) {
            // No message box in the warning, we might over-repeat ourselves.
            FTS18N("SDL_Clipboard", MsgType::WarningNoMB, SDL_GetError());
            return false;
        }
        m_bClipboardReady = true;
    }

    return true;
}

void FTS::ClipboardBase::setClipboard(const String& in_s)
{
    if(this->initClipboardIfNeeded()) {
        put_scrap(T('T','E','X','T'), in_s.len(), in_s.c_str());
    }

    m_sOwnClipboard = in_s;
}

String FTS::ClipboardBase::getClipboard()
{
    if(!this->initClipboardIfNeeded()) {
        return m_sOwnClipboard;
    }

    char *pszScrap = NULL;
    int iScraplen = 0;
    get_scrap(T('T','E','X','T'), &iScraplen, &pszScrap);

    // If there is nothing in the clipboard, it may be the system's didn't
    // work. Use our own one then.
    if(pszScrap && pszScrap[0] != 0) {
        String sText(pszScrap, 0, iScraplen);
        free(pszScrap);
        return sText;
    } else {
        return m_sOwnClipboard;
    }
}

bool FTS::CopyCmd::exec()
{
    String s = this->extractText();
    if(s.isEmpty())
        return false;

    this->setClipboard(s);
    return true;
}

bool FTS::CutCmd::exec()
{
    String s = this->extractText();
    if(s.isEmpty())
        return false;

    this->setClipboard(s);
    CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Backspace);
    return true;
}

/// Copies the currently copied text into the current widget.
/** This function pastes the content of the clipboard into the currently selected
 *  object. Currently this works only for texts.
 *
 * \return true if something got pasted, false if not.
 *
 * \author Pompei2
 */
bool FTS::PasteCmd::exec()
{
    CEGUI::Window *activeWin = GUI::getSingleton().getActiveWidget();
    String sNewline = " ";

    if(activeWin == NULL)
        return false;

    // Cancel, if the active widget is not an editable.
    if(activeWin->getType() == "ArkanaLook/Editbox")
        sNewline = " ";
    else if(activeWin->getType() == "ArkanaLook/MultiLineEditbox")
        sNewline = "\n";
    else
        return false;

    String sNewText = this->getClipboard();
    if(sNewText.isEmpty())
        return false;

    // Convert mac newlines to unix newlines or spaces, whatever is needed.
    sNewText.replaceStr("\r\n", "\n");
    sNewText.replaceStr("\r", sNewline);
    sNewText.replaceStr("\n", sNewline);

    // Build an CEGUI string based on this, to keep the accents and stuff.
    //CEGUI::String s((const CEGUI::utf8 *)pszScrap);
    CEGUI::String s(sNewText);

    // And now inject each charachter, so it get placed into the editbox.
    for(size_t i = 0 ; i < s.size() ; i++) {
        if(s[i] == '\n') {
            CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Return);
        } else
            CEGUI::System::getSingleton().injectChar(s[i]);
    }

    return true;
}

 /* EOF */
