/**
 * \file dlg_online_acctInfo.cpp
 * \author Pompei2
 * \date !!NOW!!
 * \brief This file implements a dialog used to enter or view online account informations and create the account or modify the informations.
 **/

#include <ctime>

#include <CEGUI.h>
#include <connection.h>
#include <dsrv_constants.h>

#include "dlg_online_acctInfo.h"
#include "ui/ui.h"
#include "ui/cegui_items/simple_list_item.h"
#include "logging/ftslogger.h"
#include "game/player.h"
#include "input/input.h"


/// Default constructor
/** This is the default constructor that creates the dialog, sets
 *  up all callbacks etc.
 *
 * \param in_mode The mode this dialog should act in (new, display, edit).
 * \param in_sPlayerName If the mode is something else then new, the dialog
 *                       should get and show the infos of this player.
 * \param in_bCheckConnection Whether to check if a connection is working.
 *
 * \author Pompei2
 */
FTS::DlgOnlineAcctInfo::DlgOnlineAcctInfo(eDlgOnlineAcctInfoMode in_mode, const String &in_sPlayerName, bool in_bCheckConnection)
    : Dlg("dlg_createOnlineAcc"),
      m_mode(in_mode)
{
    if(m_pRoot == NULL) {
        delete this;
        return;
    }

    try {
        // Connect the envents to the member functions.
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(FTS::DlgOnlineAcctInfo::cbCancel));
        m_pRoot->getChild("dlg_createOnlineAcc/btnCancel")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::DlgOnlineAcctInfo::cbCancel));
        m_pRoot->getChild("dlg_createOnlineAcc/btnOk")
            ->subscribeEvent(CEGUI::PushButton::EventClicked,
                             FTS_SUBS(FTS::DlgOnlineAcctInfo::cbOk));

        // Get the comboboxes to fill with date.
        FTSGetConvertWinMacro(CEGUI::Combobox, cbYear, "dlg_createOnlineAcc/frmPersonal/BDay_Year");
        FTSGetConvertWinMacro(CEGUI::Combobox, cbMonth, "dlg_createOnlineAcc/frmPersonal/BDay_Month");
        FTSGetConvertWinMacro(CEGUI::Combobox, cbDay, "dlg_createOnlineAcc/frmPersonal/BDay_Day");

        // Fill the comboboxes with date values.
        time_t t = time(NULL);
        tm *pNow = localtime(&t);

        for(int i = pNow->tm_year - 55; i <= pNow->tm_year - 5; i++) {
            (new SimpleListItem(String::nr(1900 + i, 4, '0')))->addAsDefault(cbYear);
        }
        for(int i = 1; i < 13; i++) {
            (new SimpleListItem(String::nr(i, 2, '0')))->addAsDefault(cbMonth);
        }
        for(int i = 1; i < 32; i++) {
            (new SimpleListItem(String::nr(i, 2, '0')))->addAsDefault(cbDay);
        }

        // Check for a connection on startup if wanted so.
        if(in_bCheckConnection) {
            Player p;
            // If the try to connect fails, the message of the og_connectMaster
            // will be suppressed and the warning below will be shown. If it
            // succeeds, the warning below will be suppressed.
            DefaultLogger *pDefLog = dynamic_cast<DefaultLogger *>(Logger::getSingletonPtr());
            if(pDefLog)
                pDefLog->suppressNextDlg();
            p.og_connectMaster(3*1000);
            FTS18N("NewOnlineAcc_NoConnection", MsgType::Warning);
        }

        if(m_mode == ModeView || m_mode == ModeEdit) {
            // Set the contents of the nickname.
            CEGUI::WindowManager::getSingleton()
                .getWindow("dlg_createOnlineAcc/frmNecess/Nick")
                ->setText(in_sPlayerName);

            this->loadDetails(in_sPlayerName);
        }
    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    // Setup the GUI
    this->disableAndHideFields();

    this->addShortcut(SpecialKey::Enter, FTS_SUBS(FTS::DlgOnlineAcctInfo::cbOk));
    this->addShortcut(Key::Escape, FTS_SUBS(FTS::DlgOnlineAcctInfo::cbCancel));
}

/// Default destructor
/** This is the default destructor that deletes the dialog
 *  and maybe gives the modal state back to the parent.
 *
 * \author Pompei2
 */
FTS::DlgOnlineAcctInfo::~DlgOnlineAcctInfo()
{
}

/// Disables uneditable and hide some fields depending on my mode.
/** Depending on the mode this dialog has been opened, this method disables all
 *  CEGUI fields that should not be edited. It also hides the fields not used
 *  in the mode.
 *
 * \author Pompei2
 */
void FTS::DlgOnlineAcctInfo::disableAndHideFields()
{
    try {
        switch(m_mode) {
        case ModeView:
            disableAllChildren(m_pRoot->getChild("dlg_createOnlineAcc/frmNecess"));
            disableAllChildren(m_pRoot->getChild("dlg_createOnlineAcc/frmContact"));
            disableAllChildren(m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal"));
            m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                   ->getChild("dlg_createOnlineAcc/frmNecess/chkMail")
                   ->hide();
            break;
        case ModeEdit:
            disableAllChildren(m_pRoot->getChild("dlg_createOnlineAcc/frmNecess"));
            m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                   ->getChild("dlg_createOnlineAcc/frmNecess/Mail")
                   ->enable();
            m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                   ->getChild("dlg_createOnlineAcc/frmNecess/chkMail")
                   ->enable();
            break;
        case ModeCreate:
        default:
            break;
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Called on a click on the cancel button in the dialog.
/** Will just delete this object (and thus close the dialog).
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::DlgOnlineAcctInfo::cbCancel(const CEGUI::EventArgs &)
{
    delete this;
    return true;
}

/// Called on a click on the ok button in the dialog.
/** Will modify details of the user or create the user or close the dialog.
 *
 * \param in_ea unused
 *
 * \return true
 *
 * \author Pompei2
 */
bool FTS::DlgOnlineAcctInfo::cbOk(const CEGUI::EventArgs & in_ea)
{
    switch(m_mode) {
    case ModeCreate:
        this->create();
        return true;
    case ModeEdit:
        this->saveDetails();
        break;
    case ModeView:
    default:
        break;
    }

    this->cbCancel(in_ea);
    return true;
}

/// Called on a click on the ok button in the create account dialog.
/** This callback is called when the user clicks the ok button that
 *  is palced in the dialog to create new online accounts.
 *
 *  This basically does all the library calls to create the account
 *  on the server and displays the error (if one).
 *
 * \param in_ea CEGUI Event arguments
 *
 * \return true
 *
 * \author Pompei2
 */
int FTS::DlgOnlineAcctInfo::create()
{
    String sNick;
    String sPass1;
    String sPass2;
    String sMail;

    try {
        sNick  = m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                        ->getChild("dlg_createOnlineAcc/frmNecess/Nick")->getText();
        sPass1 = m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                        ->getChild("dlg_createOnlineAcc/frmNecess/Pass")->getText();
        sPass2 = m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                        ->getChild("dlg_createOnlineAcc/frmNecess/Pass2")->getText();
        sMail  = m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                        ->getChild("dlg_createOnlineAcc/frmNecess/Mail")->getText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    if(sPass1 != sPass2) {
        FTS18N("Ogm_create_pass", MsgType::Error);
        return -2;
    }

    // This function does all other validity-checking too.
    if(ERR_OK != g_pMeHacky->og_accountCreate(sNick, sPass1, sMail))
        return -3;

    // Login, Setup the details, logout
    if(g_pMeHacky->og_login(sNick, sPass1) == ERR_OK) {
        this->saveDetails();
        g_pMeHacky->og_logout();
    }

    try {
        CEGUI::WindowManager::getSingleton().getWindow("menu_online/Nickname")->setText(sNick);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    // If all went right, close this window.
    CEGUI::EventArgs ea;
    this->cbCancel(ea);
    return ERR_OK;
}

/// Called on a click on the ok button in the modify account dialog.
/** This callback is called when the user clicks the ok button that
 *  is palced in the dialog to edit his online account, and also when
 *  he creates an online account.
 *
 *  This basically does all the library calls to set the account details
 *  on the server and displays the error (if one).
 *
 * \param in_ea CEGUI Event arguments
 *
 * \return true
 *
 * \author Pompei2
 */
int FTS::DlgOnlineAcctInfo::saveDetails()
{
    String sMail;
    bool bHideMail = false;
    String sJabber, sContact;
    String sFName, sName, sCmt;
    String sBDYear, sBDMonth, sBDDay;
    char cSex = 2;

    bool bEmptyCheck = false;

    if(m_mode == ModeCreate)
        bEmptyCheck = true;

    try {
        sMail    = m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                          ->getChild("dlg_createOnlineAcc/frmNecess/Mail")->getText();
        sJabber  = m_pRoot->getChild("dlg_createOnlineAcc/frmContact")
                          ->getChild("dlg_createOnlineAcc/frmContact/Jabber")->getText();
        sContact = m_pRoot->getChild("dlg_createOnlineAcc/frmContact")
                          ->getChild("dlg_createOnlineAcc/frmContact/Contact")->getText();
        sFName   = m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                          ->getChild("dlg_createOnlineAcc/frmPersonal/FName")->getText();
        sName    = m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                          ->getChild("dlg_createOnlineAcc/frmPersonal/Name")->getText();
        sBDYear  = m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                          ->getChild("dlg_createOnlineAcc/frmPersonal/BDay_Year")->getText();
        sBDMonth = m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                          ->getChild("dlg_createOnlineAcc/frmPersonal/BDay_Month")->getText();
        sBDDay   = m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                          ->getChild("dlg_createOnlineAcc/frmPersonal/BDay_Day")->getText();
        sCmt     = m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                          ->getChild("dlg_createOnlineAcc/frmPersonal/Comment")->getText();

        FTSGetConvertWinMacro(CEGUI::RadioButton, pSexM, "dlg_createOnlineAcc/frmPersonal/SexM");
        FTSGetConvertWinMacro(CEGUI::RadioButton, pSexF, "dlg_createOnlineAcc/frmPersonal/SexF");
        if(pSexM->isSelected())
            cSex = 1;
        else if(pSexF->isSelected())
            cSex = 2;
        else
            cSex = 3;

        FTSGetConvertWinMacro(CEGUI::Checkbox, pHideMail, "dlg_createOnlineAcc/frmNecess/chkMail");
        bHideMail = pHideMail->isSelected();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    if(sMail.empty()) {
        FTS18N("Ogm_create_missdata", MsgType::Error);
        return -2;
    }

    if(!sMail.contains("@") || !sMail.contains(".")) {
        FTS18N("Ogm_create_mail", MsgType::Error);
        return -3;
    }

    if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_MAIL, String(sMail, 0, 64)))
        return -4;

    if(ERR_OK != g_pMeHacky->og_accountSetFlag(DSRV_PLAYER_FLAG_HIDEMAIL, bHideMail))
        return -5;

    if((bEmptyCheck && !sJabber.empty()) || !bEmptyCheck) {
        if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_JABBER, String(sJabber, 0, 64)))
            return -6;
    }

    if((bEmptyCheck && !sContact.empty() && sContact != "\n") || !bEmptyCheck) {
        if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_CONTACT, String(sContact, 0, 255)))
            return -7;
    }

    if((bEmptyCheck && !sFName.empty()) || !bEmptyCheck) {
        if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_FNAME, String(sFName, 0, 32)))
            return -8;
    }

    if((bEmptyCheck && !sName.empty()) || !bEmptyCheck) {
        if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_NAME, String(sName, 0, 32)))
            return -9;
    }

    String sBDay = sBDYear + "-" + sBDMonth + "-" + sBDDay;

    if((bEmptyCheck && !sBDay.empty()) || !bEmptyCheck) {
        if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_BDAY, String(sBDay, 0, 10)))
            return -10;
    }

    if((bEmptyCheck && !sCmt.empty() && sCmt != "\n") || !bEmptyCheck) {
        if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_CMT, String(sCmt, 0, 255)))
            return -11;
    }

    if(ERR_OK != g_pMeHacky->og_accountSet(DSRV_TBL_USR_SEX, String::nr(cSex)))
        return -12;

    return ERR_OK;
}

/// This function fills the account details dialog with data from an user.
/** This function gets all details from an user acount and fills
 *  a currently opened account details dlg with this data.
 *
 * \param in_ea CEGUI Event arguments
 *
 * \return true
 *
 * \author Pompei2
 */
int FTS::DlgOnlineAcctInfo::loadDetails(const String &in_sName)
{
    // Now, get all data.
    String sMail   = g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_MAIL, in_sName);
    String sJabber = g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_JABBER, in_sName);
    String sContact= g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_CONTACT, in_sName);
    String sFName  = g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_FNAME, in_sName);
    String sName   = g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_NAME, in_sName);
    String sBDay   = g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_BDAY, in_sName);
    String sCmt    = g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_CMT, in_sName);
    String sSex    = g_pMeHacky->og_accountGetFrom(DSRV_TBL_USR_SEX, in_sName);
    uint32_t uiFlags= g_pMeHacky->og_accountGetIntFrom(DSRV_TBL_USR_FLAGS, in_sName);

    // And now display it in the dialog.
    try {
        FTSGetConvertWinMacro(CEGUI::Checkbox, pCb, "dlg_createOnlineAcc/frmNecess/chkMail");
        pCb->setSelected(uiFlags & DSRV_PLAYER_FLAG_HIDEMAIL);

        if(!sMail.empty()) {
            // We also don't want to show the email of ourselves if we "view"
            // our own account, even though we have the rights ; we want to see
            // it just the way others would see it.
            if(m_mode == DlgOnlineAcctInfo::ModeView && ((uiFlags & DSRV_PLAYER_FLAG_HIDEMAIL) == DSRV_PLAYER_FLAG_HIDEMAIL)) {
            } else {
                m_pRoot->getChild("dlg_createOnlineAcc/frmNecess")
                       ->getChild("dlg_createOnlineAcc/frmNecess/Mail")->setText(sMail);
            }
        }
        if(!sJabber.empty())
            m_pRoot->getChild("dlg_createOnlineAcc/frmContact")
                   ->getChild("dlg_createOnlineAcc/frmContact/Jabber")->setText(sJabber);
        if(!sContact.empty())
            m_pRoot->getChild("dlg_createOnlineAcc/frmContact")
                   ->getChild("dlg_createOnlineAcc/frmContact/Contact")->setText(sContact);
        if(!sFName.empty())
            m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                   ->getChild("dlg_createOnlineAcc/frmPersonal/FName")->setText(sFName);
        if(!sName.empty())
            m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                   ->getChild("dlg_createOnlineAcc/frmPersonal/Name")->setText(sName);
        if(!sCmt.empty())
            m_pRoot->getChild("dlg_createOnlineAcc/frmPersonal")
                   ->getChild("dlg_createOnlineAcc/frmPersonal/Comment")->setText(sCmt);

        // The date ...
        SimpleListItem *sli;

        // ... Year
        if( sBDay != String::EMPTY ) {
            FTSGetConvertWinMacro(CEGUI::Combobox, cbYear, "dlg_createOnlineAcc/frmPersonal/BDay_Year");
            sli = (SimpleListItem *) cbYear->findItemWithText(String(sBDay, 0, 4), NULL);
            sli->setAsDefault(cbYear);
            // ... Month
            FTSGetConvertWinMacro(CEGUI::Combobox, cbMonth, "dlg_createOnlineAcc/frmPersonal/BDay_Month");
            sli = (SimpleListItem *) cbMonth->findItemWithText(String(&sBDay[5], 0, 2), NULL);
            sli->setAsDefault(cbMonth);
            // ... Day
            FTSGetConvertWinMacro(CEGUI::Combobox, cbDay, "dlg_createOnlineAcc/frmPersonal/BDay_Day");
            sli = (SimpleListItem *) cbDay->findItemWithText(String(&sBDay[8], 0, 2), NULL);
            sli->setAsDefault(cbDay);
        }

        // The sex.
        if( sSex != String::EMPTY ) {
            String sRBName;
            switch (sSex[0]) {
            case 1:
                sRBName = "dlg_createOnlineAcc/frmPersonal/SexM";
                break;
            case 2:
                sRBName = "dlg_createOnlineAcc/frmPersonal/SexF";
                break;
            default:
                sRBName = "dlg_createOnlineAcc/frmPersonal/SexA";
                break;
            }
            FTSGetConvertWinMacro(CEGUI::RadioButton, pRB, sRBName);
            pRB->setSelected(true);
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return -1;
    }

    return ERR_OK;
}

 /* EOF */
