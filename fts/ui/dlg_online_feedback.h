/**
 * \file dlg_online_feedback.h
 * \author Pompei2
 * \date 01 August 2009
 * \brief This file defines a dialog that lets the user give us feedback to the master-server.
 **/

#ifndef D_DLGONLINEFEEDBACK_H
#define D_DLGONLINEFEEDBACK_H

#include "main.h"
#include "ui/ui.h"

namespace FTS {
    class OnlineMenuRlv;

class OnlineFeedbackWindow : public Dlg {
private:
    bool cbCancel(const CEGUI::EventArgs & in_ea);
    bool cbSend(const CEGUI::EventArgs & in_ea);
public:
    OnlineFeedbackWindow();
    virtual ~OnlineFeedbackWindow();
};

} // namespace FTS

#endif                          /* D_DLGONLINEFEEDBACK_H */

 /* EOF */
