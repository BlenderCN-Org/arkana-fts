/**
 * \file file_dialog.h
 * \author Pompei2
 * \date around the end of 2006.
 * \brief This file contains the definition of a file open dialog.
 **/

#ifndef D_FILEDIALOG_H
#define D_FILEDIALOG_H

#include "main.h"

#include "dLib/dBrowse/dBrowse.h"
#include "dLib/dString/dPath.h"

#include <CEGUIEvent.h>

#include <memory>

namespace CEGUI {
    class Window;
    class EventArgs;
}

namespace FTS {

/// A directory may be interpreted as a file, i.e. make the user think it is a
/// file by displaying the file icon in front of it and making the user unable
/// to enter it, but able to select it for opening.
/// To decide whether a directory should be interpreted as a file or not, an
/// instance of this class is called with the path to the directory.
/// Note that this base-class never interprets a directory as a file and thus
/// represents the default behavior.
class InterpretDirAsFileBase {
public:
    /// \param in_dir The path to the directory to decide whether it should be
    ///               interpreted as a file or not.
    /// \return true if it should be interpreted as a file, false if not.
    virtual bool operator()(const Path& in_dir);

    typedef std::unique_ptr<InterpretDirAsFileBase> Ptr;
};

/// This specialization interprets every directory that contains a file that
/// happens to match a given pattern as a file.
class InterpretDirWithEntryAsFile : public InterpretDirAsFileBase {
public:
    /// \param in_sPattern The pattern a file in the directory needs to match.
    InterpretDirWithEntryAsFile(const String& in_sPattern);
    virtual bool operator()(const Path& in_dir);

private:
    String m_sPattern;
};

class FileDlg {
public:
    FileDlg();
    virtual ~FileDlg();

    int loadOpenDlg(const String & in_sFilter, const String & in_sRoot,
                    InterpretDirAsFileBase::Ptr in_checker = InterpretDirAsFileBase::Ptr(new InterpretDirAsFileBase()));
    int loadOpenDlg(const String & in_sFilter,
                    const String & in_sRoot,
                    const String & in_s18NTitle,
                    const String & in_s18NText,
                    const String & in_s18NOk,
                    const String & in_s18NCancel,
                    InterpretDirAsFileBase::Ptr in_checker = InterpretDirAsFileBase::Ptr(new InterpretDirAsFileBase()));
    int loadSaveDlg(const String & in_sFilter, const String & in_sRoot);
    int loadSaveDlg(const String & in_sFilter,
                    const String & in_sRoot,
                    const String & in_s18NTitle,
                    const String & in_s18NText,
                    const String & in_s18NOk,
                    const String & in_s18NCancel);

    FileDlg *registerHandler(const CEGUI::Event::Subscriber &in_subs);

private:
    CEGUI::Window * m_pDlg;     ///< The window itself.
    bool m_bLoaded;             ///< Wether I have been loaded or not.

    CEGUI::String m_sLastActive;   ///< The window that was active before this dialog being created.
    bool m_bLastActiveWasModal;    ///< Whether the last window was modal or not.

    bool m_bOpen;     ///< If this is a file open or a file save dialog.
    String m_sFilter; ///< The file filter.
    Path m_sRoot;     ///< The root directory, where all this began :)

    CEGUI::Event::Subscriber m_pfn; ///< The CEGUI-style callback
    Path m_sFile;                   ///< The path to the selected file.

    InterpretDirAsFileBase::Ptr m_dirIsFile;

    void fillLB();
    void chdir(const String & in_sAdd);

    bool onListboxDblClick(const CEGUI::EventArgs & ea);
    bool onListboxSelChang(const CEGUI::EventArgs & ea);
    bool onOk(const CEGUI::EventArgs & ea);
    bool onCancel(const CEGUI::EventArgs & ea);
    bool onOkConfirmed(const CEGUI::EventArgs & in_ea);
    bool onOkRetracted(const CEGUI::EventArgs & in_ea);
};

class FileDlgEventArgs : public CEGUI::EventArgs {
    Path m_sFileChosen;

public:
    FileDlgEventArgs(const Path &in_sFileChosen) : m_sFileChosen(in_sFileChosen) {};
    virtual ~FileDlgEventArgs() {};

    /// Get the name of the selected file.
    /** This returns the full path to the selected file.
     *  If no file has been selected or the user canceled,
     *  this returns an empty string.
     *
     * \return If the user selected a file: The full path to the file.
     * \return If the user canceled:        An empty string.
     *
     * \author Pompei2
     */
    inline Path getFile() const {return m_sFileChosen;};
};

} // namespace FTS

#endif                          /* D_FILEDIALOG_H */
