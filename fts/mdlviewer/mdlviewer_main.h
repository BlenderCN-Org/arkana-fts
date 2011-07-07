/**
 * \file mdlviewer_main.h
 * \author Pompei2
 * \date 02 July 2007
 * \brief This file contains all the main FTS Model Viewer parts.
 **/

#ifndef FTS_MDLVIEWER_MAIN_H
#define FTS_MDLVIEWER_MAIN_H

#include "main.h"

#include "main/runlevels.h"
#include "graphic/Color.h"

#include "dLib/dString/dString.h"

#include <list>
#include <memory>

namespace CEGUI {
    class Window;
}

namespace FTS {
    class ModelInstance;
    class ModelManager;
    class DecorativeMO;

class ModelViewerRlv : public Runlevel {
public:
    ModelViewerRlv();
    virtual ~ModelViewerRlv();

    virtual bool load();
    virtual bool unload();
    virtual void render2D(const Clock& in_c);
    virtual void render3D(const Clock& in_c);
    virtual String getName();

    virtual void onMouseMoved(uint16_t x, uint16_t y);

private:
    // Functions that load the GUI parts widgets.
    int loadGUI();
    int unloadGUI();
    int setupGUI();

    int setupMovesCombobox();
    int setupSkinsCombobox();
    int setupStatusbar();
    int setupModelInstances();
    void destroyModelInstances();

    // Two buttons: Back, Load.
    bool cbBack(const CEGUI::EventArgs & in_ea);
    bool cbLoad(const CEGUI::EventArgs & in_ea);
    bool cbLoadDone(const CEGUI::EventArgs & in_ea);

    // The three buttons: play, pause, stop.
    bool cbPlayAction(const CEGUI::EventArgs & in_ea);
    bool cbPlayCycle(const CEGUI::EventArgs & in_ea);
    bool cbPlayPause(const CEGUI::EventArgs & in_ea);
    bool cbStop(const CEGUI::EventArgs & in_ea);

    bool cbSkinChanged(const CEGUI::EventArgs& in_ea);
    bool cbPlayerColChanged(const CEGUI::EventArgs& in_ea);
    bool cbShowAABBChanged(const CEGUI::EventArgs& in_ea);

    // When a scrollbar changed, also change the according label.
    bool cbhsScrollChanged(const CEGUI::EventArgs & in_ea);

    String getSelectedMoveName() const;
    float getSelectedMoveSpeed() const;
    float getSelectedMovePrio() const;

    String getSelectedSkinName() const;

    CEGUI::Window *m_pRoot;       ///< A pointer to this dialog's window.

    ModelManager* m_pModelManager;
    String m_sModelName;

    /// The instances of this model and their position (For massive rendering).
    std::list< std::shared_ptr<DecorativeMO> > m_modelInsts;

    ModelInstance* m_pCoordSys;
    ModelInstance* m_pAABB;

    Color m_playerColor;

    bool m_bShowAABB;
};

} // namespace FTS

#endif                          /* FTS_MDLVIEWER_MAIN_H */

 /* EOF */

