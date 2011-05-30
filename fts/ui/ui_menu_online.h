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
    CEGUI::Window *m_pRoot;

    /// The menu background picture.
    FTS::Graphic *m_pgMenuBG;

    /// The model manager.
    ModelManager* m_pModelManager;

    /// An instance of the menu background model.
    ModelInstance* m_pMenuBGInst;

    Configuration* m_pConf;

    bool cbLogin(const CEGUI::EventArgs & in_ea);
    bool cbCreate(const CEGUI::EventArgs & in_ea);
    bool cbBack(const CEGUI::EventArgs & in_ea);

public:
    LoginMenuRlv();
    virtual ~LoginMenuRlv();

    virtual bool load();
    virtual bool unload();
    virtual void render2D(const Clock&);
    virtual void render3D(const Clock&);
    virtual String getName();
};

} // namespace FTS

#endif                          /* FTS_UI_MENUONLINECB_H */

 /* EOF */
