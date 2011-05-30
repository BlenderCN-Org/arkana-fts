/**
 * \file confirm_dialog.h
 * \author Pompei2
 * \date February 2007
 * \brief This file defines a simple Yes/No confirmation dialog.
 **/

#ifndef D_CONFIRMDIALOG_H
#define D_CONFIRMDIALOG_H

#include "main.h"
#include <CEGUIEvent.h>

namespace CEGUI {
    class Window;
    class EventArgs;
}

#include "dLib/dString/dString.h"

namespace FTS {

class ConfirmDlg {
private:
    CEGUI::Window * m_pDlg;     ///< The dialog itself.
    String m_s18NMessage;      ///< The message, translated in the current language.
    bool m_bLoaded;             ///< Wether this dialog is loaded or not.

    CEGUI::String m_sLastActive; ///< The window that was active before this dialog being created.
    bool m_bLastActiveWasModal;  ///< Whether the last window was modal or not.

    CEGUI::Event::Subscriber m_pfnYesS; ///< The callback for the yes button.
    void (*m_pfnYes) ();                ///< The callback for the yes button.
    bool m_bYesEventSubs;               ///< Wether the yes button has a CEGUI::Event::Subscriber.

    CEGUI::Event::Subscriber m_pfnNoS; ///< The callback for the no button.
    void (*m_pfnNo) ();                ///< The callback for the no button.
    bool m_bNoEventSubs;               ///< Wether the no button has a CEGUI::Event::Subscriber.

    bool onYes(const CEGUI::EventArgs & ea);
    bool onNo(const CEGUI::EventArgs & ea);

public:
    ConfirmDlg();
    virtual ~ConfirmDlg();

    int loadI18N(const String & in_pszMessageID);
    int load(const String & in_pszMessage);
    int show();

    void registerYesHandler(CEGUI::Event::Subscriber subscriber);
    void registerYesHandler(void (*subscriber) ());
    void registerNoHandler(CEGUI::Event::Subscriber subscriber);
    void registerNoHandler(void (*subscriber) ());
};

} // namespace FTS

#endif                          /* D_CONFIRMDIALOG_H */
