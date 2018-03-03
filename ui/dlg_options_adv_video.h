#ifndef D_ADV_VIDEO_DLG_H
#define D_ADV_VIDEO_DLG_H

#include "main.h"
#include "ui/ui.h"

namespace FTS {
    class Configuration;

class AdvVideoDlg : public Dlg {
public:
    AdvVideoDlg(Configuration* pConf);
    virtual ~AdvVideoDlg();

private:
    bool cbAccept(const CEGUI::EventArgs& in_ea);
    bool cbOk(const CEGUI::EventArgs& in_ea);
    bool cbCancel(const CEGUI::EventArgs& in_ea) {return Dlg::cbCancel(in_ea);};

    bool cbTestGL(const CEGUI::EventArgs& in_ea);
    void detectMaxVBOSize(int in_mode, double &out_lastOk, double &out_last);

    bool m_bShadersChanged;
    bool cbShadersChanged(const CEGUI::EventArgs& in_ea);

    int loadData();

    Configuration* m_pConf;
};

} // namespace FTS

#endif                          /* D_ADV_VIDEO_DLG_H */

 /* EOF */
