/**
 * \file dlg_online_acctInfo.h
 * \author Pompei2
 * \date 11 October 2007
 * \brief This file defines a dialog used to enter or view online account informations and create the account or modify the informations.
 **/

#ifndef D_DLGONLINEACCTINFO_H
#define D_DLGONLINEACCTINFO_H

#include "main.h"
#include "ui/ui.h"

namespace FTS {

class DlgOnlineAcctInfo : public Dlg {
public:
    typedef enum {
        ModeCreate,
        ModeEdit,
        ModeView
    } eDlgOnlineAcctInfoMode;
private:
    eDlgOnlineAcctInfoMode m_mode; ///< The mode of this dialog.

    bool cbCancel(const CEGUI::EventArgs & in_ea);
    bool cbOk(const CEGUI::EventArgs & in_ea);

    int loadDetails(const String &in_sName);
    int saveDetails();
    int create();

    void disableAndHideFields();

public:
    DlgOnlineAcctInfo(eDlgOnlineAcctInfoMode in_mode, const String &in_sPlayerName = String::EMPTY, bool in_bCheckConnection = false);
    virtual ~DlgOnlineAcctInfo();
};

} // namespace FTS

#endif                          /* D_DLGONLINEACCTINFO_H */

 /* EOF */
