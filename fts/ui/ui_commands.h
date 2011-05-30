/**
 * \file ui_commands.h
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file contains a lot of commands that handle with the GUI.
 **/

#ifndef FTS_UI_COMMANDS_H
#define FTS_UI_COMMANDS_H

#include "main/main.h"

#include "utilities/command.h"
#include "dLib/dString/dString.h"

namespace FTS {

/// A command class that may be used to turn around in the Polygon modes.
class NextPolyModeCmd : public CommandBase {
public:
    bool exec();
};

/// A command class that may be used to turn around in the GUI Info to be displayed.
class NextGUIInfoCmd : public CommandBase {
public:
    bool exec();
};

/// This is a checking command that checks if the active (Frame-)window has the
/// name given it in the constructor.\n
/// This command is ideal as a conditional command.
class ActiveWindowCheckCmd : public CommandBase {
protected:
    String m_sWindowName; ///< The name that the active window should have.

public:
    ActiveWindowCheckCmd(const String &in_sWindowName);
    ActiveWindowCheckCmd(const ActiveWindowCheckCmd &o);

    /// Destructor.
    virtual ~ActiveWindowCheckCmd() {};

    bool exec();
};

/// This is the baseclass for all the commands that interact with a CEGUI widget.
class CEGUIKeyCmd : public CommandBase {
protected:
    String m_sName; ///< The name of the widget to handle.

    CEGUI::Window *getWinToUse();
    CEGUIKeyCmd(const String &in_sName) : m_sName(in_sName) {};
    CEGUIKeyCmd(const CEGUIKeyCmd &o) : m_sName(o.m_sName) {};

public:
    /// Destructor.
    virtual ~CEGUIKeyCmd() {};

    virtual bool exec() = 0;
};

/// This is a command that clicks on a button upon execution.
class ClickButtonCmd : public CEGUIKeyCmd {
public:
    ClickButtonCmd(const String &in_sName = String::EMPTY);
    ClickButtonCmd(const ClickButtonCmd &o);

    /// Destructor.
    virtual ~ClickButtonCmd() {};

    bool exec();
};

/// This is a command that toggles a checkbox upon execution.
class ToggleCheckboxCmd : public CEGUIKeyCmd {
public:
    ToggleCheckboxCmd(const String &in_sName = String::EMPTY);
    ToggleCheckboxCmd(const ToggleCheckboxCmd &o);

    /// Destructor.
    virtual ~ToggleCheckboxCmd() {};

    bool exec();
};

/// This is a command that selects a radiobutton upon execution.
class SelectRadiobuttonCmd : public CEGUIKeyCmd {
public:
    SelectRadiobuttonCmd(const String &in_sName = String::EMPTY);
    SelectRadiobuttonCmd(const ToggleCheckboxCmd &o);

    /// Destructor.
    virtual ~SelectRadiobuttonCmd() {};

    bool exec();
};

/// This is a command that selects a combobox entry, either the one right before
/// or the one right after the currently selected one. It there is none right
/// before or right after (for example because the current one is the first
/// or the last), the execute method still returns true if the combobox to act
/// on exists.
class ComboboxNextPrevCmd : public CEGUIKeyCmd {
    /// True if the command should select the NEXT entry. If that's false, this
    /// command will select the PREVIOUS entry in the combobx.
    bool m_bNext;
public:
    ComboboxNextPrevCmd(bool in_bNext, const String &in_sName = String::EMPTY);
    ComboboxNextPrevCmd(const ComboboxNextPrevCmd &o);

    /// Destructor.
    virtual ~ComboboxNextPrevCmd() {};

    bool exec();
};

/// This is a command that selects a listbox entry, either the one right before
/// or the one right after the currently selected one. It there is none right
/// before or right after (for example because the current one is the first
/// or the last), the execute method still returns true if the listbox to act
/// on exists.
class ListboxNextPrevCmd : public CEGUIKeyCmd {
    /// True if the command should select the NEXT entry. If that's false, this
    /// command will select the PREVIOUS entry in the combobx.
    bool m_bNext;
public:
    ListboxNextPrevCmd(bool in_bNext, const String &in_sName = String::EMPTY);
    ListboxNextPrevCmd(const ListboxNextPrevCmd &o);

    /// Destructor.
    virtual ~ListboxNextPrevCmd() {};

    bool exec();
};

/// This is a command that will increase or decrease a spinner's value by one
/// step.
class SpinnerCmd : public CEGUIKeyCmd {
    /// True if the command should increase the spinner's value.
    /// False if it should decrease it.
    bool m_bIncr;
public:
    SpinnerCmd(bool in_bIncr, const String &in_sName = String::EMPTY);
    SpinnerCmd(const SpinnerCmd &o);

    /// Destructor.
    virtual ~SpinnerCmd() {};

    bool exec();
};

/// This is a command that will increase or decrease a scrollbar's position
/// by a step or by a page.
class ScrollbarCmd : public CEGUIKeyCmd {
public:
    typedef enum {
        HorizOnly,
        VertOnly,
        Any,
    } ScrollbarType;

private:
    /// True if the command should increase the scrollbar's value.
    /// False if it should decrease it.
    bool m_bIncr;
    /// True if the command modify the scrollbar's value by a page.
    /// False if it should modify by a step.
    bool m_bPage;

    ScrollbarType m_type;
public:
    ScrollbarCmd(bool in_bIncr, bool in_bPage, ScrollbarType in_type = Any,
                     const String &in_sName = String::EMPTY);
    ScrollbarCmd(const ScrollbarCmd &o);

    /// Destructor.
    virtual ~ScrollbarCmd() {};

    bool exec();
};

/// This is the base-command for any command that has to do with the
/// clipboard. It holds the common methods.
class ClipboardBase {
protected:
    void setClipboard(const String& in_s);
    String getClipboard();
private:
    bool initClipboardIfNeeded();
    static bool m_bClipboardReady;
    static String m_sOwnClipboard;
};

/// This is the class that holds the common points between cut and copy, namely
/// extracting the currently selected text from the GUI.
class CopyCutCmdBase : public ClipboardBase, public CommandBase {
protected:
    String extractText() const;
};

/// This command copies the currently selected text into the clipboard.
class CopyCmd : public CopyCutCmdBase {
public:
    bool exec();
};

/// This command copies the currently selected text into the clipboard and deletes it.
class CutCmd : public CopyCutCmdBase {
public:
    bool exec();
};

/// This command pastes the current clipboard text into the current GUI item.
class PasteCmd : public ClipboardBase, public CommandBase {
public:
    bool exec();
};

/// This command enters the tab navigation mode and goes to the next item.
class TabNavigationCmd : public CommandBase {
public:
    TabNavigationCmd() : m_pActiveWidget(NULL) {};
    bool exec();
private:
    // A bunch of helper methods.
    CEGUI::Window *findFirstChildRecursive(CEGUI::Window *in_pParent);
    CEGUI::Window *findNextChildRecursive(CEGUI::Window *in_pParent, CEGUI::Window *in_pStart);
    CEGUI::Window *findNextChild(CEGUI::Window *in_pParent, CEGUI::Window *in_pStart);
    CEGUI::Window *getNextTabOrderWin(CEGUI::Window *in_pFrame, CEGUI::Window *in_pCurr);
    CEGUI::Window* m_pActiveWidget;
};

/// This command leaves the tab navigation mode.
class TabNavigationStopCmd : public CommandBase {
public:
    bool exec();
};

} // namespace FTS

#endif                          /* FTS_UI_H */

 /* EOF */
