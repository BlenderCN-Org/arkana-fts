#ifndef FTS_UI_MENUONLINECB_H
#define FTS_UI_MENUONLINECB_H

#include "main.h"
#include "main/runlevels.h"

namespace FTS {
    class ModelInstance;
    class ModelManager;
    class Graphic;
    class Configuration;

class LoginMenuRlv : public Runlevel {
private:
    /// The root of the CEGUI menu.
    CEGUI::Window *m_pRoot = nullptr;

    /// The menu background picture.
    FTS::Graphic *m_pgMenuBG = nullptr;

    /// The model manager.
    ModelManager* m_pModelManager = nullptr;

    /// An instance of the menu background model.
    ModelInstance* m_pMenuBGInst = nullptr;

    Configuration* m_pConf = nullptr;

    bool cbLogin(const CEGUI::EventArgs & in_ea);
    bool cbCreate(const CEGUI::EventArgs & in_ea);
    bool cbBack(const CEGUI::EventArgs & in_ea);

public:
    LoginMenuRlv();
    virtual ~LoginMenuRlv();

    bool load() override;
    bool unload() override;
    void render2D(const Clock&) override;
    void render3D(const Clock&) override;
    String getName() override;
};

} // namespace FTS

#endif                          /* FTS_UI_MENUONLINECB_H */

 /* EOF */
