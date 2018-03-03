/**
 * \file dlg_online_newGame.h
 * \author Pompei2
 * \date 27 October 2007
 * \brief This file defines a dialog used to enter data to host a new game.
 **/

#ifndef D_DLGONLINENEWGAME_H
#define D_DLGONLINENEWGAME_H

#include "main.h"

#include "ui/ui.h"

namespace FTS {
    class MapInfo;

class DlgOnlineNewGame : public Dlg {
private:
    FTS::MapInfo *m_pMapInfo;        ///< The informations about the map to play.

    bool cbCancel(const CEGUI::EventArgs & in_ea);
    bool cbOk(const CEGUI::EventArgs & in_ea);

    bool cbMapTypeChanged(const CEGUI::EventArgs & in_ea);
    bool cbLoadFile(const CEGUI::EventArgs & in_ea);
    bool cbResumeFile(const CEGUI::EventArgs & in_ea);

    bool cbLoadFileChosen(const CEGUI::EventArgs & in_ea);
    bool cbResumeFileChosen(const CEGUI::EventArgs & in_ea);
public:
    DlgOnlineNewGame();
    virtual ~DlgOnlineNewGame();
};

} // namespace FTS

#endif                          /* D_DLGONLINENEWGAME_H */

 /* EOF */
