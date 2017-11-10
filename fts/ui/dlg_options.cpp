#include <CEGUI.h>
#include "connection.h" // To get the master server list

#include "ui/dlg_options.h"

#include "3d/Renderer.h"
#include "3d/Resolution.h"
#include "input/input.h"
#include "logging/ftslogger.h"
#include "scripting/DaoVm.h" // To load the menu sounds.
#include "sound/fts_Snd.h"
#include "ui/cegui_items/simple_list_item.h"
#include "ui/dlg_options_adv_video.h"
#include "ui/ui.h"
#include "ui/ui_commands.h"
#include "ui/ui_menu.h"
#include "utilities/utilities.h"

#include "dLib/dBrowse/dBrowse.h"
#include "dLib/dConf/configuration.h"
#include "dLib/dString/dTranslation.h"

using namespace FTS;

FTS::MenuOptions::MenuOptions(const std::list<String>& in_lDisables)
    : m_pRoot(NULL)
    , m_pConf(nullptr)
    , m_pTrans(nullptr)
{
    this->reload(true, in_lDisables);
}

FTS::MenuOptions::~MenuOptions()
{
    delete m_pConf;
    delete m_pTrans;
}

void FTS::MenuOptions::reload(bool in_bDoLoad, const std::list<String>& in_lDisables)
{
    if(m_pConf != nullptr) {
        delete m_pConf;
    }
    m_pConf = new Configuration("conf.xml", ArkanaDefaultSettings());
    if( m_pTrans != nullptr ) {
        delete m_pTrans;
    }
    m_pTrans = new Translation("ui");
    
    // Now that the dialog gets closed, we can remove both shortcuts from the system.
    if(m_pRoot) {
        InputManager::getSingleton().delShortcut(String(m_pRoot->getName()) + "1");
        InputManager::getSingleton().delShortcut(String(m_pRoot->getName()) + "2");
    }

    // The plate that should be active at the moment.
    String sCurrentlyActive = "btnGeneral";

    // First, unload.
    try {
        // If the dialog is still opened, close it.
        if(CEGUI::WindowManager::getSingleton().isWindowPresent("dlg_options/advVideo")) {
            SAFE_DELETE(m_pAdvVideoDlg);
        }
        if(CEGUI::WindowManager::getSingleton().isWindowPresent("dlg_options")) {
            // Determine what plate is currently active.
            if(CEGUI::WindowManager::getSingleton().
               getWindow("dlg_options/btnGeneral")->isVisible()) {
                sCurrentlyActive = "btnGeneral";
            } else if(CEGUI::WindowManager::getSingleton().
                    getWindow("dlg_options/btnVideo")->isVisible()) {
                sCurrentlyActive = "btnVideo";
            } else if(CEGUI::WindowManager::getSingleton().
                    getWindow("dlg_options/btnAudio")->isVisible()) {
                sCurrentlyActive = "btnAudio";
            } else if(CEGUI::WindowManager::getSingleton().
                    getWindow("dlg_options/btnNetwork")->isVisible()) {
                sCurrentlyActive = "btnNetwork";
            } else {
                sCurrentlyActive = "btnMisc";
            }
        }

        // And close this window.
        if(CEGUI::WindowManager::getSingleton().isWindowPresent("dlg_options")) {
            CEGUI::WindowManager::getSingleton().destroyWindow("dlg_options");
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    if(!in_bDoLoad)
        return ;

    m_bVideoChange = false;

    // Then, load.
    try {
        m_pRoot = GUI::getSingleton().loadLayout("dlg_options", true, true, true);

        if(!m_pRoot)
            return;

        // Set the callbacks.
        m_pRoot->getChild("left_btns_bg")
            ->getChild("btnGeneral")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnChooser));
        m_pRoot->getChild("left_btns_bg")
            ->getChild("btnVideo")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnChooser));
        m_pRoot->getChild("left_btns_bg")
            ->getChild("btnAudio")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnChooser));
        m_pRoot->getChild("left_btns_bg")
            ->getChild("btnNetwork")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnChooser));
        m_pRoot->getChild("left_btns_bg")
            ->getChild("btnMisc")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnChooser));
        /* The Ok, Cancel and Accept buttons. */
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnCancel));
        m_pRoot->getChild("dlg_options/buttons_frame")
            ->getChild("dlg_options/buttons_frame/btnOk")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnOk));
        m_pRoot->getChild("dlg_options/buttons_frame")
            ->getChild("dlg_options/buttons_frame/btnCancel")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnCancel));
        m_pRoot->getChild("dlg_options/buttons_frame")
            ->getChild("dlg_options/buttons_frame/btnAccept")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                FTS_SUBS(FTS::MenuOptions::cbOptions_btnAccept));

        loadGeneral();
        loadVideo();
        loadAudio();
        loadNet();
        loadMisc();

        // Choose the category to activate.
        m_pRoot->getChild("left_btns_bg")->getChild(sCurrentlyActive)->setEnabled(false);
        CEGUI::WindowManager::getSingleton().getWindow("dlg_options/" + sCurrentlyActive)->setVisible(true);

        // Disable all windows we want to have disabled.
        for(std::list<String>::const_iterator i = in_lDisables.begin() ; i != in_lDisables.end() ; ++i) {
            try {
                CEGUI::WindowManager::getSingleton().getWindow(*i)->disable();
            } catch(CEGUI::Exception & e) {
                FTS18N("CEGUI", MsgType::Error, e.getMessage());
            }
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    // Add a keyboard shortcut to close this options box.
    InputManager *pMgr = InputManager::getSingletonPtr();
    ActiveWindowCheckCmd *pCond = NULL;
    CallbackCommand *pCbCmd = NULL;

    // One for the return key.
    pCond = new ActiveWindowCheckCmd(m_pRoot->getName());
    pCbCmd = new CallbackCommand(CEGUI::Event::Subscriber(&FTS::MenuOptions::cbOptions_btnOk, this));
    pMgr->add(String(m_pRoot->getName()) + "1", SpecialKey::Enter,
              new ConditionalCommand(pCond, pCbCmd));

    // And one for the escape key.
    pCond = new ActiveWindowCheckCmd(m_pRoot->getName());
    pCbCmd = new CallbackCommand(CEGUI::Event::Subscriber(&FTS::MenuOptions::cbOptions_btnCancel, this));
    pMgr->add(String(m_pRoot->getName()) + "2", Key::Escape,
              new ConditionalCommand(pCond, pCbCmd));
}

bool FTS::MenuOptions::cbOptions_btnChooser(const CEGUI::EventArgs & in_ea)
{
    CEGUI::WindowEventArgs * wea = (CEGUI::WindowEventArgs *) (&in_ea);

    try {
        // Select the current button.
        wea->window->getParent()
            ->getChild("btnGeneral")
            ->setEnabled(wea->window->getName().compare("btnGeneral") != 0);
        wea->window->getParent()
            ->getChild("btnVideo")
            ->setEnabled(wea->window->getName().compare("btnVideo") != 0);
        wea->window->getParent()
            ->getChild("btnAudio")
            ->setEnabled(wea->window->getName().compare("btnAudio") != 0);
        wea->window->getParent()
            ->getChild("btnNetwork")
            ->setEnabled(wea->window->getName().compare("btnNetwork") != 0);
        wea->window->getParent()
            ->getChild("btnMisc")
            ->setEnabled(wea->window->getName().compare("btnMisc") != 0);

        // Set them to be all invisible, exept the current one.
        CEGUI::WindowManager::getSingleton().
            getWindow("dlg_options/btnGeneral")->setVisible(false);
        CEGUI::WindowManager::getSingleton().
            getWindow("dlg_options/btnVideo")->setVisible(false);
        CEGUI::WindowManager::getSingleton().
            getWindow("dlg_options/btnAudio")->setVisible(false);
        CEGUI::WindowManager::getSingleton().
            getWindow("dlg_options/btnNetwork")->setVisible(false);
        CEGUI::WindowManager::getSingleton().
            getWindow("dlg_options/btnMisc")->setVisible(false);
        CEGUI::WindowManager::getSingleton().
            getWindow("dlg_options/" + String(wea->window->getName()))->
            setVisible(true);

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return true;
    }

    return true;
}

bool FTS::MenuOptions::cbOptions_btnOk(const CEGUI::EventArgs & in_ea)
{
    cbOptions_btnAccept(in_ea);
    cbOptions_btnCancel(in_ea);
    return true;
}

bool FTS::MenuOptions::cbOptions_btnCancel(const CEGUI::EventArgs & in_ea)
{
    this->reload(false);
    delete this;

    return true;
}

bool FTS::MenuOptions::cbOptions_btnAccept(const CEGUI::EventArgs & in_ea)
{
    bool bReloadMenu = false;
    saveGeneral(bReloadMenu);
    saveVideo(bReloadMenu);
    saveAudio(bReloadMenu);
    saveNet(bReloadMenu);
    saveMisc(bReloadMenu);

    // If anyone decided that we need to reload the menu, do this now!
    if(bReloadMenu) {
        // Reload the menu's GUI.
        RunlevelManager::getSingleton().getCurrRunlevel()->unload();
        RunlevelManager::getSingleton().getCurrRunlevel()->load();

        // Then reload ourselves.
        this->reload();
    }

    return true;
}

bool FTS::MenuOptions::cbOptions_btnVidOption(const CEGUI::EventArgs & in_ea)
{
    m_bVideoChange = true;
    return true;
}

int FTS::MenuOptions::loadGeneral()
{
    SimpleListItem *sli = nullptr;
    String sLang;

    // Load the layout.
    m_pGeneral = GUI::getSingleton().loadLayout("dlg_options_btnGeneral", false);
    if(m_pGeneral == NULL)
        return -1;

    try {
        // Add it to the right place.
        m_pRoot->getChild("dlg_options/options_bg")->addChildWindow(m_pGeneral);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -2;
    }

    try {
        // Fill the listbox for the language choice.
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnGeneral/cbLang");
        std::vector<String> fileNames = dBrowse(Path::datadir("Languages"));
        for(auto&& sLang : fileNames) {
            sli = new SimpleListItem(sLang);
            cb->addItem(sli);
        }

        String sLanguage = m_pConf->get("Language");
        sli = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sLanguage, NULL));
        if(sli)
            sli->setAsDefault(cb);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Set the spinner for the debug level choice.
        FTSGetConvertWinMacro(CEGUI::Spinner, sp, "dlg_options/btnGeneral/spGDLL");
        sp->setCurrentValue((float)m_pConf->getInt("DebugLevel"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Set the horiz scrollbar for the mouse scroll speed.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnGeneral/hsMouseScroll");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("MouseScrollSpeed"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Set the horiz scrollbar for the kbd scroll speed.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnGeneral/hsKbdScroll");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("KbdScrollSpeed"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return ERR_OK;
}

int FTS::MenuOptions::loadVideo()
{
    // Load the layout.
    m_pVideo = GUI::getSingleton().loadLayout("dlg_options_btnVideo", false);
    if(m_pVideo == NULL)
        return -1;

    try {
        // Add it to the right place.
        m_pRoot->getChild("dlg_options/options_bg")->addChildWindow(m_pVideo);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    try {
        // This fills the resolutions listbox with values.
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnVideo/cbResol");

        std::list<Resolution> modes = Renderer::getSupportedResolutions();

        // wtf, I want python for i in modes .....
        for(std::list<Resolution>::iterator i = modes.begin() ; i != modes.end() ; ++i) {
            cb->addItem(new SimpleListItem(i->toString(false)));
        }

        // Add the current one to the list if it isn't in!
        String sRes = Resolution().toString(false);
        SimpleListItem *lti = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sRes, NULL));
        if(!lti) {
            lti = new SimpleListItem(sRes);
            cb->addItem(lti);
        }
        lti->setAsDefault(cb);

        cb->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_btnVidOption));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Set the fullscreen checkbox to the correct state.
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnVideo/chkFullscr");
        ck->setSelected(m_pConf->getBool("Fullscreen"));

        ck->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_btnVidOption));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Fill the Texture filter listbox.
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnVideo/cbFilt");

        String sTexFilter1 = m_pTrans->get("Opts_Vid_Tex_VLow");
        String sTexFilter2 = m_pTrans->get("Opts_Vid_Tex_Low");
        String sTexFilter3 = m_pTrans->get("Opts_Vid_Tex_Mid");
        SimpleListItem *lti = new SimpleListItem(sTexFilter1);
        lti->setUserData((void *)((size_t)1));
        cb->addItem(lti);
        lti = new SimpleListItem(sTexFilter2);
        lti->setUserData((void *)((size_t)2));
        cb->addItem(lti);
        lti = new SimpleListItem(sTexFilter3);
        lti->setUserData((void *)((size_t)3));
        cb->addItem(lti);

        switch (m_pConf->getInt("TextureFilter")) {
        case 1:
            lti = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sTexFilter1, NULL));
            break;
        case 3:
            lti = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sTexFilter3, NULL));
            break;
        case 2:
        default:
            lti = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sTexFilter2, NULL));
            break;
        }
        if(!lti) {
            lti = new SimpleListItem(sTexFilter2);
            cb->addItem(lti);
        }
        lti->setAsDefault(cb);

        cb->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_btnVidOption));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Fill the model details listbox.
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnVideo/cbModDet");

        String sModDet1 = m_pTrans->get("Opts_Vid_Mod_Low");
        String sModDet2 = m_pTrans->get("Opts_Vid_Mod_Mid");
        String sModDet3 = m_pTrans->get("Opts_Vid_Mod_Hig");
        SimpleListItem *lti = new SimpleListItem(sModDet1);
        lti->setUserData((void *)1);
        cb->addItem(lti);
        lti = new SimpleListItem(sModDet2);
        lti->setUserData((void *)2);
        cb->addItem(lti);
        lti = new SimpleListItem(sModDet3);
        lti->setUserData((void *)3);
        cb->addItem(lti);

        switch (m_pConf->getInt("ModelDetails")) {
        case 1:
            lti = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sModDet1, NULL));
            break;
        case 3:
            lti = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sModDet3, NULL));
            break;
        case 2:
        default:
            lti = dynamic_cast<SimpleListItem *>(cb->findItemWithText(sModDet2, NULL));
            break;
        }
        if(!lti) {
            lti = new SimpleListItem(sModDet2);
            cb->addItem(lti);
        }
        lti->setAsDefault(cb);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Enable the advanced button.
        m_pVideo->getChild("dlg_options/btnVideo/Advanced")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::MenuOptions::cbOptions_btnVidAdvanced));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return ERR_OK;
}

int FTS::MenuOptions::loadAudio()
{
    // Load the layout.
    m_pAudio = GUI::getSingleton().loadLayout("dlg_options_btnAudio", false);
    if(m_pAudio == NULL)
        return -1;

    try {
        // Add it to the right place.
        m_pRoot->getChild("dlg_options/options_bg")->addChildWindow(m_pAudio);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -2;
    }

    try {
        CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();

        // Display a warning if no snd sys has been compiled in
        if(pWM->isWindowPresent("dlg_options/btnAudio/lblMessage")
#if D_SND_SYS != D_FTS_NoSound
            // or if there were problems during the init of the sound system.
           && ISndSys::getSingleton().getType() == "None"
           && m_pConf->getBool("SoundEnabled")
#endif
          ) {
            Translation trans("messages");
            pWM->getWindow("dlg_options/btnAudio/lblMessage")->setText(trans.get("SND_NoSys"));
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Adjust the sound enabled checkbox.
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnAudio/chkSnd");
        ck->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_chkSoundEnabled));
        // This is a little trick to be SURE to toggle the event.
        bool bEnabled = m_pConf->getBool("SoundEnabled");
        ck->setSelected(!bEnabled);
        ck->setSelected(bEnabled);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeMusic.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsMusicVol");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("SoundVolumeMusic"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXUnitReaction.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsUnitReacVol");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("SoundVolumeSFXUnitReaction"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXAction.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsActionVol");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("SoundVolumeSFXAction"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXEnvironment.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsEnvironmentVol");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("SoundVolumeSFXEnvironment"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXAttention.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsAttentionVol");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("SoundVolumeSFXAttention"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXMagic.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsMagicVol");
        sb->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,
                           FTS_SUBS(FTS::MenuOptions::cbOptions_hsScrollChanged));
        sb->setScrollPosition((float)m_pConf->getInt("SoundVolumeSFXMagic"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return ERR_OK;
}

int FTS::MenuOptions::loadNet()
{
    // Load the layout.
    m_pNet = GUI::getSingleton().loadLayout("dlg_options_btnNetwork", false);
    if(m_pNet == NULL)
        return -1;

    try{
        // Add it to the right place.
        m_pRoot->getChild("dlg_options/options_bg")->addChildWindow(m_pNet);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -2;
    }

    try{
        // Set the currently used port.
        FTSGetConvertWinMacro(CEGUI::Spinner, sp, "dlg_options/btnNetwork/spPort");
        sp->setCurrentValue((float)m_pConf->getInt("MasterServerPort"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try{
        // Set the currently used master server address.
        this->fillMasterServerList(false);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try{
        // Set the currently used default channel name.
        m_pNet->getChild("dlg_options/btnNetwork/edChannel")
              ->setText(m_pConf->get("DefaultChannel"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Adjust the Clear Chatbox checkbox.
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnNetwork/chkClearChat");
        ck->setSelected(m_pConf->getBool("ClearChatbox"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the callback for the update button.
        m_pNet->getChild("dlg_options/btnNetwork/btnUpdateMS")
              ->subscribeEvent(CEGUI::PushButton::EventClicked,
                               FTS_SUBS(FTS::MenuOptions::cbOptions_btnNetUpdate));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try{
        // Setup the callback for the reset defaults button.
        m_pNet->getChild("dlg_options/btnNetwork/btnDefault")
                ->subscribeEvent(CEGUI::PushButton::EventClicked,
                                 FTS_SUBS(FTS::MenuOptions::cbOptions_btnNetDefault));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return ERR_OK;
}

int FTS::MenuOptions::loadMisc()
{
    // Load the layout.
    m_pMisc = GUI::getSingleton().loadLayout("dlg_options_btnMisc", false);
    if(m_pMisc == NULL)
        return -1;

    try{
        // Add it to the right place.
        m_pRoot->getChild("dlg_options/options_bg")->addChildWindow(m_pMisc);

        // Set the cursor warping checkbox.
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnMisc/chkMouseWarp");
        ck->setSelected(m_pConf->getBool("MenuMouseWarp"));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return ERR_OK;
}

bool FTS::MenuOptions::cbOptions_btnVidAdvanced(const CEGUI::EventArgs & in_ea)
{
    m_pAdvVideoDlg = new AdvVideoDlg(m_pConf);

    return true;
}

int FTS::MenuOptions::saveGeneral(bool &out_bReloadMenu)
{
    int iGDLL = 0;
    out_bReloadMenu = false;

    try {
        // Language
        String sLng(m_pGeneral->getChild("dlg_options/btnGeneral/cbLang")->getText());
        if(sLng != m_pConf->get("Language")) {
            m_pConf->set("Language", sLng);
            out_bReloadMenu = true;
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    try {
        // Debug level
        FTSGetConvertWinMacro(CEGUI::Spinner, sp, "dlg_options/btnGeneral/spGDLL");
        iGDLL = (int)sp->getCurrentValue();
        m_pConf->set("DebugLevel", iGDLL);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    try {
        // Mouse scrolling speed
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnGeneral/hsMouseScroll");
        int iMouseScrollSpeed = (int)sb->getScrollPosition();
        m_pConf->set("MouseScrollSpeed", iMouseScrollSpeed);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    try {
        // Keyboard scrolling speed
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnGeneral/hsKbdScroll");
        int iKbdScrollSpeed = (int)sb->getScrollPosition();
        m_pConf->set("KbdScrollSpeed", iKbdScrollSpeed);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    // Apply some options directly.
    DefaultLogger *pDefLog = dynamic_cast<DefaultLogger *>(Logger::getSingletonPtr());
    if(pDefLog)
        pDefLog->setGDLL(iGDLL);
    m_pConf->save();
    return ERR_OK;
}

int FTS::MenuOptions::saveVideo(bool &out_bReloadMenu)
{
    // The values.
    Resolution res;
    int mod = 2;

    try {
        String sTmp(m_pVideo->getChild("dlg_options/btnVideo/cbResol")->getText());
        res = Resolution(sTmp);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnVideo/chkFullscr");
        res.fs = ck->isSelected();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnVideo/cbFilt");
        int tex = (size_t)cb->findItemWithText(cb->getText(), NULL)->getUserData();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnVideo/cbModDet");
        mod = (size_t)cb->findItemWithText(cb->getText(), NULL)->getUserData();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    m_pConf->set("ModelDetails", mod);
    m_pConf->save();

    if(m_bVideoChange) {
        // This saves the settings to the conf file if it works and keeps the
        // old ones else.
        Renderer::getSingleton().changeResolution(res);
        m_bVideoChange = false;
        out_bReloadMenu = true;
        // Reload the configuration, since it is changed by the Renderer.
        delete m_pConf;
        m_pConf = new Configuration("conf.xml", ArkanaDefaultSettings());
    }

    return ERR_OK;
}

int FTS::MenuOptions::saveAudio(bool &out_bReloadMenu)
{
    // All volume settings are saved first so that when enabling sound,
    // It already has the correct volume.

    try {
        // Setup the scrollbars for the SoundVolumeMusic.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsMusicVol");
        int iVol = (int)sb->getScrollPosition();
        m_pConf->set("SoundVolumeMusic", iVol);
        ISndSys::getSingleton().setVolume((float)iVol / 100.0f, SndGroup::Music);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXUnitReaction.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsUnitReacVol");
        int iVol = (int)sb->getScrollPosition();
        m_pConf->set("SoundVolumeSFXUnitReaction", iVol);
        ISndSys::getSingleton().setVolume((float)iVol / 100.0f, SndGroup::UnitReaction);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXAction.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsActionVol");
        int iVol = (int)sb->getScrollPosition();
        m_pConf->set("SoundVolumeSFXAction", iVol);
        ISndSys::getSingleton().setVolume((float)iVol / 100.0f, SndGroup::UnitAction);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXEnvironment.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsEnvironmentVol");
        int iVol = (int)sb->getScrollPosition();
        m_pConf->set("SoundVolumeSFXEnvironment", iVol);
        ISndSys::getSingleton().setVolume((float)iVol / 100.0f, SndGroup::Environment);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXAttention.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsAttentionVol");
        int iVol = (int)sb->getScrollPosition();
        m_pConf->set("SoundVolumeSFXAttention", iVol);
        ISndSys::getSingleton().setVolume((float)iVol / 100.0f, SndGroup::Attention);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Setup the scrollbars for the SoundVolumeSFXMagic.
        FTSGetConvertWinMacro(CEGUI::Scrollbar, sb, "dlg_options/btnAudio/hsMagicVol");
        int iVol = (int)sb->getScrollPosition();
        m_pConf->set("SoundVolumeSFXMagic", iVol);
        ISndSys::getSingleton().setVolume((float)iVol / 100.0f, SndGroup::Magic);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnAudio/chkSnd");
        // Sound was enabled and now wants to be disabled:
        if(!ck->isSelected() && ISndSys::getSingleton().getType() != "None") {
            m_pConf->set("SoundEnabled", false);
            m_pConf->save();
            FTS18NDBG("SoundDrvU", 1);
            ISndSys::createSoundSys();
            DaoVm::getSingleton().execute(Path("loadMenuSounds.dao"));
            Logger::getSingletonPtr()->doneConsoleMessage();
        }
        // Sound was disabled and now wants to be enabled:
        else if(ck->isSelected() && ISndSys::getSingleton().getType() == "None") {
            m_pConf->set("SoundEnabled", true);
            m_pConf->save();
            FTS18NDBG("SoundDrvL", 1);
            ISndSys::createSoundSys();
            // Check if the sound sys creation failed.
            if(ISndSys::getSingleton().getType() == "None") {
                ck->setSelected(false);
            }
            DaoVm::getSingleton().execute(Path("loadMenuSounds.dao"));
            Logger::getSingletonPtr()->doneConsoleMessage();
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
    m_pConf->save();
    return ERR_OK;
}

int FTS::MenuOptions::saveNet(bool &out_bReloadMenu)
{
    try {
        // Save the port:
        FTSGetConvertWinMacro(CEGUI::Spinner, sp, "dlg_options/btnNetwork/spPort");
        int iPort = (int)sp->getCurrentValue();
        m_pConf->set("MasterServerPort", iPort);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Save the master server address:
        FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnNetwork/cbMaster");
        ServerListItem *pLI = dynamic_cast<ServerListItem *>(cb->getSelectedItem());
        if(pLI && pLI->getText() == cb->getText().c_str()) {
            // If it is a master-server chosen from the combobox.
            String sMasterServer = pLI->getAddr();
            m_pConf->set("MasterServerName", sMasterServer);
        } else {
            // Or it is a master-server typed-in by the user.
            String sMasterServer = cb->getText();
            m_pConf->set("MasterServerName", sMasterServer);
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Save the default channel:
        String sDefaultChannel = m_pNet->getChild("dlg_options/btnNetwork/edChannel")->getText();
        m_pConf->set("DefaultChannel", sDefaultChannel);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        // Save the Clear Chatbox option
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnNetwork/chkClearChat");
        m_pConf->set("ClearChatbox", ck->isSelected());
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    m_pConf->save();
    return ERR_OK;
}

int FTS::MenuOptions::saveMisc(bool &out_bReloadMenu)
{
    try {
        // Get the options from the dialog.
        FTSGetConvertWinMacro(CEGUI::Checkbox, ck, "dlg_options/btnMisc/chkMouseWarp");
        bool bWarpCursor = ck->isSelected();
        m_pConf->set("MenuMouseWarp", bWarpCursor);
        m_pConf->save();

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    /* Directly apply those options. */

    return ERR_OK;
}

bool FTS::MenuOptions::cbOptions_btnNetDefault(const CEGUI::EventArgs & in_ea)
{
    try {
        // Update the master servers list file.
        downloadHTTPFile("arkana-fts.sourceforge.net", "/masterservers", "masterservers", 1000);
        this->fillMasterServerList(true);

        // Set the currently used port.
        FTSGetConvertWinMacro(CEGUI::Spinner, sp, "dlg_options/btnNetwork/spPort");
        sp->setCurrentValue((float)D_DEFAULT_SERVER_PORT);

        // Set the currently used default channel.
        m_pNet->getChild("dlg_options/btnNetwork/edChannel")
              ->setText("Talk To Survive (main channel)");

    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

bool FTS::MenuOptions::cbOptions_btnNetUpdate(const CEGUI::EventArgs & in_ea)
{
    try {
        // Update the master servers list file.
        downloadHTTPFile("arkana-fts.sourceforge.net", "/masterservers", "masterservers", 1000);
        this->fillMasterServerList(false);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

bool FTS::MenuOptions::cbOptions_hsScrollChanged(const CEGUI::EventArgs & in_ea)
{
    const CEGUI::WindowEventArgs *wea = dynamic_cast<const CEGUI::WindowEventArgs *>(&in_ea);
    if(!wea)
        return true;

    CEGUI::Scrollbar *sb = dynamic_cast<CEGUI::Scrollbar *>(wea->window);
    if(!sb)
        return true;

    CEGUI::String sName;
    try {
        // Find the corresponding label and set its value.
        sName = wea->window->getName();
        sb->getParent()->getChild(sName + "Val")->setText(String::nr((int)sb->getScrollPosition()));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    // Also, for the volume scrollbars, we make this take effect immediately:
    if(sName == "dlg_options/btnAudio/hsMusicVol") {
        ISndSys::getSingleton().setVolume(sb->getScrollPosition() / 100.0f, SndGroup::Music);
    } else if(sName == "dlg_options/btnAudio/hsUnitReacVol") {
        ISndSys::getSingleton().setVolume(sb->getScrollPosition() / 100.0f, SndGroup::UnitReaction);
    } else if(sName == "dlg_options/btnAudio/hsActionVol") {
        ISndSys::getSingleton().setVolume(sb->getScrollPosition() / 100.0f, SndGroup::UnitAction);
    } else if(sName == "dlg_options/btnAudio/hsEnvironmentVol") {
        ISndSys::getSingleton().setVolume(sb->getScrollPosition() / 100.0f, SndGroup::Environment);
    } else if(sName == "dlg_options/btnAudio/hsAttentionVol") {
        ISndSys::getSingleton().setVolume(sb->getScrollPosition() / 100.0f, SndGroup::Attention);
    } else if(sName == "dlg_options/btnAudio/hsMagicVol") {
        ISndSys::getSingleton().setVolume(sb->getScrollPosition() / 100.0f, SndGroup::Magic);
    }

    return true;
}

bool FTS::MenuOptions::cbOptions_chkSoundEnabled(const CEGUI::EventArgs & in_ea)
{
    const CEGUI::WindowEventArgs *wea = dynamic_cast<const CEGUI::WindowEventArgs *>(&in_ea);
    if(!wea)
        return true;

    CEGUI::Checkbox *ck = dynamic_cast<CEGUI::Checkbox *>(wea->window);
    if(!ck)
        return true;

    try {
        bool bEnable = ck->isSelected();
        m_pAudio->getChild("dlg_options/btnAudio/hsMusicVol"      )->setEnabled(bEnable);
        m_pAudio->getChild("dlg_options/btnAudio/hsUnitReacVol"   )->setEnabled(bEnable);
        m_pAudio->getChild("dlg_options/btnAudio/hsActionVol"     )->setEnabled(bEnable);
        m_pAudio->getChild("dlg_options/btnAudio/hsEnvironmentVol")->setEnabled(bEnable);
        m_pAudio->getChild("dlg_options/btnAudio/hsAttentionVol"  )->setEnabled(bEnable);
        m_pAudio->getChild("dlg_options/btnAudio/hsMagicVol"      )->setEnabled(bEnable);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return true;
}

void FTS::MenuOptions::fillMasterServerList(bool in_bSelectDefault)
{
    std::ifstream fMasterServers("masterservers");

    FTSGetConvertWinMacro(CEGUI::Combobox, cb, "dlg_options/btnNetwork/cbMaster");
    cb->resetList();

    String sCurrMasterServer = D_DEFAULT_SERVER_NAME;

    if(!in_bSelectDefault) {
        sCurrMasterServer = m_pConf->get("MasterServerName");
    }

    // Strip out the "http://" thing if present.
    if(sCurrMasterServer.left(7) == "http://")
        sCurrMasterServer = sCurrMasterServer.right(sCurrMasterServer.len()-7);

    bool bIsIn = false;
    if(fMasterServers.good()) {
        while(fMasterServers) {
            std::string sServer;
            std::string sDesc;
            fMasterServers >> sServer;

            // But if there is, read the description:
            if(fMasterServers.peek() != '\n' && fMasterServers.peek() != '\r') {
                // Skip all whitespace.
                while(isspace(fMasterServers.peek())) {
                    fMasterServers.get();
                }
                std::getline(fMasterServers, sDesc);
            }

            // Strip out the "http://" thing if present.
            String sServerNoHTTP = sServer;
            if(sServerNoHTTP.left(7) == "http://")
                sServerNoHTTP = sServerNoHTTP.right(sServerNoHTTP.len()-7);

            if(sServerNoHTTP.empty())
                continue;

            // Add that one, either as the selected one or as an alternative.
            if(sServerNoHTTP == sCurrMasterServer) {
                bIsIn = true;
                (new ServerListItem(sServerNoHTTP, sDesc))->addAsDefault(cb);
            } else {
                cb->addItem(new ServerListItem(sServerNoHTTP, sDesc));
            }
        }
    } else {
        // No master servers file found, take this as a default.
        if(sCurrMasterServer != D_DEFAULT_SERVER_NAME) {
            cb->addItem(new ServerListItem(D_DEFAULT_SERVER_NAME, D_DEFAULT_SERVER_DESC));
        }
    }

    // Add the current user's master server if it ain't yet in the list.
    if(!bIsIn) {
        (new ServerListItem(sCurrMasterServer, String::EMPTY))->addAsDefault(cb);
    }
}
