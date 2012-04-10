/**
 * \file ui_menu_options.h
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file contains all about the FTS User Interface.
 **/

#ifndef FTS_UI_MENU_OPTIONS_H
#define FTS_UI_MENU_OPTIONS_H

#include "main.h"

#include <list>

namespace CEGUI {
    class Window;
    class EventArgs;
}

namespace FTS {
    class AccountsDlg;
    class AdvVideoDlg;
    class String;
    class Configuration;

class MenuOptions {
private:
    /* All the windows. */
    CEGUI::Window * m_pRoot;
    CEGUI::Window * m_pGeneral;
    CEGUI::Window * m_pVideo;
    CEGUI::Window * m_pAudio;
    CEGUI::Window * m_pNet;
    CEGUI::Window * m_pMisc;

    /* Wether the video mode (resolution, color depth, fullscreen) changed. */
    bool m_bVideoChange;

    /* Or maybe prefer an advanced graphics one ? */
    AdvVideoDlg *m_pAdvVideoDlg;

    Configuration * m_pConf;
    class Translation* m_pTrans;

    /* Functions that load the settings into the widgets. */
    int loadGeneral();
    int loadVideo();
    int loadAudio();
    int loadNet();
    int loadMisc();

    /* Functions that save the settings into the conf file. */
    int saveGeneral(bool &out_bReloadMenu);
    int saveVideo(bool &out_bReloadMenu);
    int saveAudio(bool &out_bReloadMenu);
    int saveNet(bool &out_bReloadMenu);
    int saveMisc(bool &out_bReloadMenu);

    /* The buttons on the left side. */
    bool cbOptions_btnChooser(const CEGUI::EventArgs & in_ea);

    /* The three buttons: Ok, Cancel and Accept. */
    bool cbOptions_btnOk(const CEGUI::EventArgs & in_ea);
    bool cbOptions_btnCancel(const CEGUI::EventArgs & in_ea);
    bool cbOptions_btnAccept(const CEGUI::EventArgs & in_ea);

    bool cbOptions_btnVidAdvanced(const CEGUI::EventArgs & in_ea);

    /* Some option that need to reset the video mode changed (see m_bVideoChange). */
    bool cbOptions_btnVidOption(const CEGUI::EventArgs & in_ea);

    bool cbOptions_btnNetDefault(const CEGUI::EventArgs & in_ea);
    bool cbOptions_btnNetUpdate(const CEGUI::EventArgs & in_ea);

    // When a scrollbar changed, also change the according label.
    bool cbOptions_hsScrollChanged(const CEGUI::EventArgs & in_ea);
    // When the sound enabled checkbox switched, enable or disable the volume controls.
    bool cbOptions_chkSoundEnabled(const CEGUI::EventArgs & in_ea);

    void fillMasterServerList(bool in_bSelectDefault);

public:
    MenuOptions(const std::list<String>& in_lDisables = std::list<String>());
    virtual ~MenuOptions();

    void reload(bool in_bDoLoad = true, const std::list<String>& in_lDisables = std::list<String>());
};

} // namespace FTS

#endif                          /* FTS_UI_MENUCB_H */

 /* EOF */
