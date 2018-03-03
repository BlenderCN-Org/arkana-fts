#ifndef FTS_UI_MENUCB_H
#define FTS_UI_MENUCB_H

#include "main.h"
#include "main/runlevels.h"

namespace CEGUI {
    class Window;
    class EventArgs;
}

namespace FTS {
    class ModelInstance;
    class ModelManager;
    class Graphic;
    void setVersionInfo();

class MainMenuRlv : public Runlevel {
public:
    MainMenuRlv();
    virtual ~MainMenuRlv();

    bool load() override;
    bool unload() override;
    void render2D(const Clock&) override;
    void render3D(const Clock&) override;
    String getName() override;

    bool loadCEGUI();
    bool unloadCEGUI();
private:
    /// The root of the CEGUI menu.
    CEGUI::Window *m_pRoot = nullptr;

    /// The model manager.
    ModelManager* m_pModelManager = nullptr;

    /// The menu background picture.
    Graphic *m_pgMenuBG = nullptr;

    /// The current screen width.
    int m_iScreenWidth = 0;

    /// The current screen height.
    int m_iScreenHeigth = 0;

    /// An instance of the menu background model.
    ModelInstance* m_pMenuBGInst = nullptr;

    void loadSettingsFromConf();

    // Callbacks for the main menu buttons.
    bool cbNewGameOpened(const CEGUI::EventArgs& in_ea);
    bool cbNewGame(const CEGUI::EventArgs& in_ea);
    bool cbOnlineGame(const CEGUI::EventArgs& in_ea);
    bool cbOptions(const CEGUI::EventArgs& in_ea);
    bool cbMdlViewer(const CEGUI::EventArgs& in_ea);
    bool cbUpdate(const CEGUI::EventArgs& in_ea);
    bool cbUpdateCancel(const CEGUI::EventArgs& in_ea);
    bool cbUpdateDoIt(const CEGUI::EventArgs& in_ea);
    bool cbInfo(const CEGUI::EventArgs& in_ea);
    bool cbQuit(const CEGUI::EventArgs& in_ea);
};

}

#endif                          /* FTS_UI_MENUCB_H */

 /* EOF */
