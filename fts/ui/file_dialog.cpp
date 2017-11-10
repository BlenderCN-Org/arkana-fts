/**
 * \file file_dialog.cpp
 * \author Pompei2
 * \date around the end of 2006.
 * \brief This file contains the implementation of a file open dialog.
 **/

#include <CEGUI.h>

#include "ui/file_dialog.h"
#include "ui/ui.h"
#include "ui/ui_commands.h"
#include "ui/cegui_items/imaged_list_item.h"
#include "ui/confirm_dialog.h"
#include "logging/logger.h"
#include "utilities/utilities.h"
#include "dLib/dBrowse/dBrowse.h"
#include "dLib/dFile/dFile.h"
#include "dLib/dString/dTranslation.h"
#include "input/input.h"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

using namespace FTS;

bool FTS::InterpretDirAsFileBase::operator()(const Path& in_dir)
{
    // The default one says that a directory is a directory, forever ;)
    return false;
}

FTS::InterpretDirWithEntryAsFile::InterpretDirWithEntryAsFile(const String& in_sPattern)
    : m_sPattern(in_sPattern)
{
}

bool FTS::InterpretDirWithEntryAsFile::operator()(const Path& in_dir)
{
    auto files = dBrowse(in_dir, m_sPattern);
    return !files.empty();
}

/** Constructor for the file dialog item. */
FTS::FileDlg::FileDlg()
{
    CEGUI::Window *pLastActive = GUI::getSingleton().getActiveWin();
    m_sLastActive = pLastActive ? pLastActive->getName() : "None";
    m_bLastActiveWasModal = pLastActive ? pLastActive->getModalState() : false;

    m_bLoaded = false;
    m_bOpen = true;
    m_pDlg = NULL;
}

/** Destructor for the file dialog item. */
FTS::FileDlg::~FileDlg(void)
{
    // Now that the dialog gets closed, we can remove both shortcuts from the system.
    InputManager::getSingleton().delShortcut(String(m_pDlg->getName()) + "1");
    InputManager::getSingleton().delShortcut(String(m_pDlg->getName()) + "2");

    try {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_pDlg);
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

/// loads the file open dialog.
/** Loads all data necessary to display a file open dialog.
 *
 * \param in_sFilter The filter for the files (like "*.map" or so.)
 * \param in_sRoot   The root directory from where to choose.
 * \param in_checker This one is called for every directory encountered and if
 *                   it returns true, the directory will be interpreted as a
 *                   file, i.e. it cannot be entered but it can be chosen for opening.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note Even if you choose a root directory, the user can still go above it.
 *
 * \author Pompei2
 */
int FTS::FileDlg::loadOpenDlg(const String & in_sFilter, const String & in_sRoot,
                              InterpretDirAsFileBase::Ptr in_checker)
{
    return this->loadOpenDlg(in_sFilter, in_sRoot, "File_OpenTitle",
                             "File_OpenTxt", "File_Open",
                             "General_Cancel", std::move(in_checker));
}

/// loads the file open dialog.
/** Loads all data necessary to display a file open dialog.
 *
 * \param in_sFilter The filter for the files (like "*.map" or so.)
 * \param in_sRoot   The root directory from where to choose.
 * \param in_s18NTitle  If you want your own string in the title.
 * \param in_s18NText   If you want your own string in the description.
 * \param in_s18NOk     If you want your own string on the OK button.
 * \param in_s18NCancel If you want your own string on the Cancel button.
 * \param in_checker This one is called for every directory encountered and if
 *                   it returns true, the directory will be interpreted as a
 *                   file, i.e. it cannot be entered but it can be chosen for opening.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note Even if you choose a root directory, the user can still go above it.
 *       The personal strings get translated (from the ui file).
 *
 * \author Pompei2
 */
int FTS::FileDlg::loadOpenDlg(const String & in_sFilter, const String & in_sRoot,
                              const String & in_s18NTitle,
                              const String & in_s18NText,
                              const String & in_s18NOk,
                              const String & in_s18NCancel,
                              InterpretDirAsFileBase::Ptr in_checker)
{
    if(m_bLoaded) {
        FTS18N("InvParam", MsgType::Horror, "FTS::FileDlg::loadOpenDlg already loaded");
        return -1;
    }

    if(!in_sFilter || !in_sRoot || !in_s18NTitle || !in_s18NText
       || !in_s18NOk || !in_s18NCancel) {
        FTS18N("InvParam", MsgType::Horror, "FTS::FileDlg::loadOpenDlg");
        return -1;
    }

    // Print out debugging stuff.
    FTSMSGDBG("  FileDialog for opening opened", 2);

    m_bOpen = true;
    m_sFilter = in_sFilter;

    if(!FTS_IS_DIR_SEPARATOR(in_sRoot.getCharAt(in_sRoot.len() - 1)))
        m_sRoot = in_sRoot + FTS_DIR_SEPARATOR;
    else
        m_sRoot = in_sRoot;

    // Load all language settings.
    Translation trans("ui");
    String s18NTitle = trans.get(in_s18NTitle);
    String s18NText = trans.get(in_s18NText);
    String s18NOk = trans.get(in_s18NOk);
    String s18NCancel = trans.get(in_s18NCancel);

    // Load the dialog and set it's items texts to the just loaded ones.
    try {
        m_pDlg = GUI::getSingleton().loadLayout("file_dlg", true, true, true);
        m_pDlg->setText(s18NTitle);
        m_pDlg->getChild("file_dlg/TxtEdit")->setText(s18NText);
        m_pDlg->getChild("file_dlg/Ok")->setText(s18NOk);
        m_pDlg->getChild("file_dlg/Cancel")->setText(s18NCancel);
        m_pDlg->getChild("file_dlg/FName")->setEnabled(false);

        CEGUI::Listbox * lb = (CEGUI::Listbox *) m_pDlg->getChild("file_dlg/FList");
        lb->subscribeEvent(CEGUI::Listbox::EventMouseDoubleClick,
                           FTS_SUBS(FTS::FileDlg::onListboxDblClick));
        lb->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
                           FTS_SUBS(FTS::FileDlg::onListboxSelChang));

        CEGUI::PushButton * pb = (CEGUI::PushButton *) m_pDlg->getChild("file_dlg/Ok");
        pb->subscribeEvent(CEGUI::PushButton::EventClicked,
                           FTS_SUBS(FTS::FileDlg::onOk));

        pb = (CEGUI::PushButton *) m_pDlg->getChild("file_dlg/Cancel");
        pb->subscribeEvent(CEGUI::PushButton::EventClicked,
                           FTS_SUBS(FTS::FileDlg::onCancel));
        m_pDlg->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                               FTS_SUBS(FTS::FileDlg::onCancel));

        m_bLoaded = true;
    }
    catch(CEGUI::UnknownObjectException & ex) {
        FTS18N("CEGUI_Init", MsgType::Error, ex.getMessage());
        return -1;
    }
    catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return -1;
    }

    // Add a keyboard shortcut to close this messagebox.
    InputManager *pMgr = InputManager::getSingletonPtr();
    ActiveWindowCheckCmd *pCond = NULL;
    CallbackCommand *pCbCmd = NULL;

    // One for the return key.
    pCond = new ActiveWindowCheckCmd(m_pDlg->getName());
    pCbCmd = new CallbackCommand(FTS_SUBS(FTS::FileDlg::onOk));
    pMgr->add(String(m_pDlg->getName()) + "1", SpecialKey::Enter,
              new ConditionalCommand(pCond, pCbCmd));

    // And one for the escape key.
    pCond = new ActiveWindowCheckCmd(m_pDlg->getName());
    pCbCmd = new CallbackCommand(FTS_SUBS(FTS::FileDlg::onCancel));
    pMgr->add(String(m_pDlg->getName()) + "2", Key::Escape,
              new ConditionalCommand(pCond, pCbCmd));

    // Fill the files/directories listbox.
    m_dirIsFile = std::move(in_checker);
    fillLB();
    return ERR_OK;
}

/// loads the file save dialog.
/** Loads all data necessary to display a file save dialog.
 *
 * \param in_sFilter The filter for the files (like "*.map" or so.)
 * \param in_sRoot   The root directory from where to choose.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note Even if you choose a root directory, the user can still go above it.
 *
 * \author Pompei2
 */
int FTS::FileDlg::loadSaveDlg(const String & in_sFilter,
                          const String & in_sRoot)
{
    return this->loadSaveDlg(in_sFilter, in_sRoot, "File_SaveTitle",
                             "File_SaveTxt", "File_Save",
                             "General_Cancel");
}

/// loads the file save dialog.
/** Loads all data necessary to display a file save dialog.
 *
 * \param in_sFilter The filter for the files (like "*.map" or so.)
 * \param in_sRoot   The root directory from where to choose.
 * \param in_s18NTitle  If you want your own string in the title.
 * \param in_s18NText   If you want your own string in the description.
 * \param in_s18NOk     If you want your own string on the OK button.
 * \param in_s18NCancel If you want your own string on the Cancel button.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note Even if you choose a root directory, the user can still go above it.
 *       The personal strings get translated (from the ui.conf file).
 *
 * \author Pompei2
 */
int FTS::FileDlg::loadSaveDlg(const String & in_sFilter,
                          const String & in_sRoot,
                          const String & in_s18NTitle,
                          const String & in_s18NText,
                          const String & in_s18NOk,
                          const String & in_s18NCancel)
{
    if(m_bLoaded) {
        FTS18N("InvParam", MsgType::Horror, "FTS::FileDlg::loadSaveDlg already loaded");
        return -1;
    }

    if(!in_sFilter || !in_sRoot || !in_s18NTitle || !in_s18NText
       || !in_s18NOk || !in_s18NCancel) {
        FTS18N("InvParam", MsgType::Horror, "FTS::FileDlg::loadSaveDlg");
        return -1;
    }

    // Print out debugging stuff.
    FTSMSGDBG("  FileDialog for saving opened", 2);

    m_bOpen = true;
    m_sFilter = in_sFilter;

    if(!FTS_IS_DIR_SEPARATOR(in_sRoot.getCharAt(in_sRoot.len() - 1)))
        m_sRoot = in_sRoot + FTS_DIR_SEPARATOR;
    else
        m_sRoot = in_sRoot;

    // Load all language settings.
    Translation trans("ui");
    String s18NTitle = trans.get(in_s18NTitle);
    String s18NText = trans.get(in_s18NText);
    String s18NOk = trans.get(in_s18NOk);
    String s18NCancel = trans.get(in_s18NCancel);

    // Load the dialog and set it's items texts to the just loaded ones.
    try {
        m_pDlg = GUI::getSingleton().loadLayout("file_dlg", false, false, true);
        m_pDlg->setVisible(false);
        m_pDlg->setText(s18NTitle);
        m_pDlg->getChild("file_dlg/TxtEdit")->setText(s18NText);
        m_pDlg->getChild("file_dlg/Ok")->setText(s18NOk);
        m_pDlg->getChild("file_dlg/Cancel")->setText(s18NCancel);

        CEGUI::Listbox * lb =
            (CEGUI::Listbox *) m_pDlg->getChild("file_dlg/FList");
        lb->subscribeEvent(CEGUI::Listbox::EventMouseDoubleClick,
                           FTS_SUBS(FTS::FileDlg::onListboxDblClick));
        lb->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
                           FTS_SUBS(FTS::FileDlg::onListboxSelChang));

        CEGUI::PushButton * pb =
            (CEGUI::PushButton *) m_pDlg->getChild("file_dlg/Ok");
        pb->subscribeEvent(CEGUI::PushButton::EventClicked,
                           FTS_SUBS(FTS::FileDlg::onOk));

        pb = (CEGUI::PushButton *) m_pDlg->getChild("file_dlg/Cancel");
        pb->subscribeEvent(CEGUI::PushButton::EventClicked,
                           FTS_SUBS(FTS::FileDlg::onCancel));
        m_pDlg->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                               FTS_SUBS(FTS::FileDlg::onCancel));

        m_bLoaded = true;
    }
    catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return -1;
    }

    // Add a keyboard shortcut to close this messagebox.
    InputManager *pMgr = InputManager::getSingletonPtr();
    ActiveWindowCheckCmd *pCond = NULL;
    CallbackCommand *pCbCmd = NULL;

    // One for the return key.
    pCond = new ActiveWindowCheckCmd(m_pDlg->getName());
    pCbCmd = new CallbackCommand(FTS_SUBS(FTS::FileDlg::onOk));
    pMgr->add(String(m_pDlg->getName()) + "1", SpecialKey::Enter,
              new ConditionalCommand(pCond, pCbCmd));

    // And one for the escape key.
    pCond = new ActiveWindowCheckCmd(m_pDlg->getName());
    pCbCmd = new CallbackCommand(FTS_SUBS(FTS::FileDlg::onCancel));
    pMgr->add(String(m_pDlg->getName()) + "2", Key::Escape,
              new ConditionalCommand(pCond, pCbCmd));

    // Fill the files/directories listbox.
    fillLB();
    return ERR_OK;
}

bool compare_dirfile_list(ImagedListItem *first, ImagedListItem *second)
{
    if(first->getText() == "..")
        return true;

    if(second->getText() == "..")
        return false;

    bool bFirstIsDir = reinterpret_cast<unsigned long>(first->getUserData()) == 1UL;
    bool bSecondIsDir = reinterpret_cast<unsigned long>(second->getUserData()) == 1UL;
    if(bFirstIsDir && !bSecondIsDir)
        return true;
    if(!bFirstIsDir && bSecondIsDir)
        return false;

    // Need a lexical comparison:
    return first->getText().compare(second->getText()) < 0;
}

/// Private function to fill the listbox.
/** Reads the content of the current directory and fills the listbox with it.
 *
 * \author Pompei2
 */
void FTS::FileDlg::fillLB(void)
{
    // If we aren't able to open the directory, go back one.
    auto state = fs::status(m_sRoot.c_str());
    if((state.type() != fs::file_type::regular) && (state.type() != fs::file_type::directory)) {
        this->chdir("..");
        return;
    }

    try {
        // Empty the list.
        CEGUI::Listbox * pLB = (CEGUI::Listbox *) m_pDlg->getChild("file_dlg/FList");
        pLB->resetList();

        std::list<ImagedListItem *> listOfFilesAndDirs;

        // Get all files and put them in a sorted list.
        ImagedListItem *pLTI = new ImagedListItem("..", "FTSUI", "BackIcon", (void *)1UL);
        listOfFilesAndDirs.push_back(pLTI);
        for(auto& p : fs::directory_iterator(m_sRoot.c_str())) {
            if(fs::is_directory(p.path())) {
                pLTI = new ImagedListItem(p.path().stem().string(), "FTSUI", "DirIcon", (void *)1UL);
            } else {
                FTS::String sPath = p.path().filename().generic_string();
                // Only take files that correspond to the pattern, but all directories.
                if(sPath.matchesPattern(m_sFilter)) {
                    pLTI = new ImagedListItem(p.path().stem().string(), "FTSUI", "FileIcon", (void *)0UL);
                }
            }
            listOfFilesAndDirs.push_back(pLTI);
        }
        // Sort the list now.
        listOfFilesAndDirs.sort(compare_dirfile_list);

        // And insert the sorted items into the listbox.
        for(std::list<ImagedListItem *>::iterator i = listOfFilesAndDirs.begin(); i != listOfFilesAndDirs.end(); ++i) {
            pLB->addItem(*i);
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return ;
    }

}

/// Private function to change into a directory.
/** Changes into a directory and updates the listbox.
 *
 * \param in_sAdd The directory to add to the current one.
 *
 * \return If successful: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note If ".." is the selected directory, this will really
 *       go one directory up, not just add a "..".
 *
 * \author Pompei2
 */
void FTS::FileDlg::chdir(const String & in_sAdd)
{
    if(in_sAdd.empty())
        return;

    // Maybe we just need to go back one directory.
    if(in_sAdd == "..") {
        m_sRoot = m_sRoot.cdUp();
        fillLB();
    } else {
        m_sRoot = m_sRoot.appendWithSeparator(in_sAdd);
        fillLB();
    }
}

/** Callback when the user clicks one time on the listbox and chooses an item. */
bool FTS::FileDlg::onListboxSelChang(const CEGUI::EventArgs & in_ea)
{
    try {
        CEGUI::Listbox * pLB = (CEGUI::Listbox *) m_pDlg->getChild("file_dlg/FList");
        ImagedListItem *pLBI = (ImagedListItem *) pLB->getFirstSelectedItem();

        if(NULL == pLBI)
            return true;

        // If the user double-clicks on a directory, open it.
        m_pDlg->getChild("file_dlg/FName")->setText(pLBI->getText());
    }
    catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/** Callback when the user doubleclicks on the listbox. */
bool FTS::FileDlg::onListboxDblClick(const CEGUI::EventArgs & in_ea)
{
    try {
        CEGUI::Listbox * pLB = (CEGUI::Listbox *) m_pDlg->getChild("file_dlg/FList");
        ImagedListItem *pLBI = (ImagedListItem *) pLB->getFirstSelectedItem();

        if(NULL == pLBI)
            return true;

        // If the user double-clicks on a directory, open it.
        if(pLBI->getUserData() == (void *)1) {
            this->chdir(pLBI->getText());
            // If it wasn't a directory he double-clicked on, do as if he pressed the ok button.
        } else {
            onOk(in_ea);
        }
    }
    catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/** Callback for the OK button. DOESN'T close itself ! just hides itself. */
bool FTS::FileDlg::onOk(const CEGUI::EventArgs & in_ea)
{
    try {
        CEGUI::Listbox * pLB = (CEGUI::Listbox *) m_pDlg->getChild("file_dlg/FList");
        ImagedListItem *pLBI = (ImagedListItem *) pLB->getFirstSelectedItem();

        if(pLBI == NULL)
            return true;

        // If a directory is selected and the user clicks Ok, open the dir.
        if(pLBI && (pLBI->getUserData() == (void *)1)) {
            this->chdir(pLBI->getText());
            return true;
        }

        // Here the user clicked the Open button while a file is selected.
        m_pDlg->hide();
        m_sFile = m_sRoot.appendWithSeparator(m_pDlg->getChild("file_dlg/FName")->getText().c_str());

        // In case of a save, ask if the user wants to overwrite.
        if(FileUtils::fileExists(m_sFilter) && !m_bOpen) {
            ConfirmDlg cfd;
            cfd.loadI18N("File_Save_Confirm");
            cfd.registerYesHandler(FTS_SUBS(FTS::FileDlg::onOkConfirmed));
            cfd.registerNoHandler(FTS_SUBS(FTS::FileDlg::onOkRetracted));
            cfd.show();
        } else {
            this->onOkConfirmed(in_ea);
        }

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/** Callback for the Cancel button. Deletes itself. */
bool FTS::FileDlg::onCancel(const CEGUI::EventArgs & in_ea)
{
    // Here the user clicked the Cancel button.
    m_sFile = String::EMPTY;

    // Close and call the callback.
    FileDlgEventArgs ea(m_sFile);
    CEGUI::Event::Subscriber s = m_pfn;

    delete this;
    s(ea);

    return true;
}

/** This does really do the ok step, meaning the user confirmed and all. */
bool FTS::FileDlg::onOkConfirmed(const CEGUI::EventArgs & in_ea)
{
    try {
        // Print out debugging stuff.
        FTSMSGDBG("  FileDialog Chose file "+m_sFile, 2);

        // Close and call the callback.
        FileDlgEventArgs ea(m_sFile);
        CEGUI::Event::Subscriber s = m_pfn;
        delete this;
        s(ea);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/** This happens if the user clicked ok, has been asked if he is sure, and the clicked no. */
bool FTS::FileDlg::onOkRetracted(const CEGUI::EventArgs & in_ea)
{
    try {
        m_pDlg->show();
        m_sFile = String::EMPTY;
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI_Init", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

/// register a callback when the dialog is closed.
/** This registers a CEGUI-style callback function for the
 *  case that the dialog has been closed.
 *
 * \param in_subs The CEGUI-style callback.
 *
 * \note The callback will get a FileDlgEventArgs object as argument.
 *
 * \author Pompei2
 */
FileDlg *FTS::FileDlg::registerHandler(const CEGUI::Event::Subscriber &in_subs)
{
    m_pfn = in_subs;
    return this;
}
