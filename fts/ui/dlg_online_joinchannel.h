/**
 * \file dlg_online_joinchannel.h
 * \author Pompei2
 * \date 11 October 2007
 * \brief This file defines a dialog that lets the user choose a channel to join.
 **/

#ifndef D_DLGONLINEJOINCHANNEL_H
#define D_DLGONLINEJOINCHANNEL_H

#include "main.h"
#include "ui/ui.h"

namespace FTS {
    class OnlineMenuRlv;

class OnlineJoinChannelWindow : public Dlg {
private:
    bool cbCancel(const CEGUI::EventArgs & in_ea);
    bool cbJoin(const CEGUI::EventArgs & in_ea);
    bool cbChanSel(const CEGUI::EventArgs & in_ea);

    OnlineMenuRlv &m_rlv; /// Pointer to the current runlevel.
public:
    OnlineJoinChannelWindow(OnlineMenuRlv &in_rlv);
    virtual ~OnlineJoinChannelWindow();
};

} // namespace FTS

#endif                          /* D_DLGONLINEJOINCHANNEL_H */

 /* EOF */
