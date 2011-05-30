/**
 * \file runlevels.h
 * \author Pompei2
 * \date 1 December 2008
 * \brief This file defines everything about the runlevels of the game.
 **/

#ifndef D_RUNLEVELS_H
#define D_RUNLEVELS_H

#include "main.h"
#include "main/Updateable.h"

int enterMainLoop();

namespace FTS {

struct _SCursor_;
class Packet;
class Camera;

/// Base class for any runlevel.
class Runlevel : public Updateable {
private:
    /// Protect from copying
    Runlevel(const Runlevel &) {};

    /// A default cursor.
    _SCursor_ *m_pDefCursor;

protected:
    /// Private constructor.
    Runlevel() : m_pDefCursor(NULL) {};

    void renderCEGUI();
    void renderCoordSys();
    int loadDefaultCursor();
    int unloadDefaultCursor();

public:
    /// Default destructor.
    virtual ~Runlevel() {};

    /** This method needs to be overloaded. It will be called during the loading
     *  of the runlevel.\n
     *  You may for example register all your keyboard shortcuts here.
     *
     *  \return This method should return true only if it successfully loaded
     *          the whole runlevel. If it returns false, the previous runlevel
     *          will be backed up again.
     *  \author Pompei2
     */
    virtual bool load() = 0;

    /** This method needs to be overloaded. It will be called during the cleaning
     *  of the runlevel (when quitting).\n
     *  You may for example unregister all your keyboard shortcuts here.
     *
     *  \return This method should return true if it successfully unloaded
     *          the whole runlevel. If it returns false, nothing special is done
     *          (the runlevel is still unloaded).
     *  \author Pompei2
     */
    virtual bool unload() = 0;

    /** This method needs to be overloaded. It will be called once every frame,
     *  after the 3D rendering is done and all matrices are setup to render
     *  in two dimensions now.\n
     *  Thus you should use the commands glVertex2i(x,y) to draw something, x
     *  and y being screen-space pixels (from 0 to w, 0 to h).
     *  \author Pompei2
     */
    virtual void render2D(const Clock& in_c) {};

    /** This method needs to be overloaded. It will be called once every frame,
     *  before the 2D rendering, when all matrices are still setup to render in
     *  three dimensions.\n
     *  You don't need to struggle with camera etc. as your currently active
     *  camera will already be selected and setup (rendered).
     *
     *  \author Pompei2
     */
    virtual void render3D(const Clock& in_c) {};

    /** This method may be overloaded. It will be called once every game
     *  tick, that is usually once every frame, right before the rendering is
     *  set up and done.\n
     *  You may use this method as a worker method that does things as long as
     *  the runlevel is running. Thus it should not be too bloated!
     *
     *  \author Pompei2
     */
    virtual bool update(const Clock& in_c) {return true;};

    /** This method needs to be overloaded. It has to return a unique name for
     *  the runlevel.
     *
     *  \author Pompei2
     */
    virtual FTS::String getName() = 0;

    /** This method may be overloaded. It has to return a reference to the
     *  runlevel's currently used camera. You only need to overwrite this
     *  method if the runlevel does use more then one default camera.\n
     *
     *  \return A reference to the currently active camera.
     *
     *  \author Pompei2
     */
    virtual Camera &getActiveCamera();

    /** This method may be overloaded. It has to return a reference to the
     *  runlevel's main camera. That must not be the currently active camera.
     *  For example the game's main camera is the one in bird-view, but
     *  during a cut-scene there may be a lot of cameras and one of them may be
     *  currently active.\n
     *  You only need to overwrite this method if you have more then one camera.
     *
     *  \return A reference to the runlevel's main camera.
     *
     *  \author Pompei2
     */
    virtual Camera &getMainCamera();

    /** This method may be overloaded. It has to return a pointer to the
     *  runlevel's currently used cursor. You only need to overwrite this
     *  method if the runlevel does use more then one default cursor.\n
     *
     *  \return A pointer to the currently active cursor.
     *
     *  \author Pompei2
     */
    virtual _SCursor_ *getActiveCursor() {return m_pDefCursor;};

    /** This method may be overloaded. It has to return a pointer to the
     *  runlevel's main cursor. That must not be the currently active cursor.
     *  For example the game's main cursor is the one pointer, but while the
     *  game is scrolling, there is another cursor being displayed with arrows. \n
     *  You only need to overwrite this method if you have more then one cursor.
     *
     *  \return A pointer to the runlevel's main cursor.
     *
     *  \author Pompei2
     */
    virtual _SCursor_ *getMainCursor() {return m_pDefCursor;};

    /** This method may be overloaded. It will get called everytime the mouse
     *  moves, and may do whatever it wants with this information.
     *
     * \param x The new X position of the mouse, in pixels.
     * \param y The new Y position of the mouse, in pixels.
     *
     * \author Pompei2
     */
    virtual void onMouseMoved(uint16_t x, uint16_t y) {};
};

/// Base class for all runlevels that are online. They need to be able to handle
/// any messages that comes in over the network at any time whatsoever.
class OnlineRunlevel : public virtual Runlevel {
protected:
    /// Private constructor.
    OnlineRunlevel() : Runlevel() {};

public:
    /// Default destructor.
    virtual ~OnlineRunlevel() {};

    virtual bool handleMessage(Packet &in_pack) = 0;
};

class RunlevelManager : public Singleton<RunlevelManager> {
    friend int ::enterMainLoop();
protected:
    /// The currently active runlevel.
    Runlevel *m_pCurrRunlevel;

    /// This is the runlevel we need to enter right before the next game tick.
    Runlevel *m_pRunlevelToEnter;

    Runlevel *realEnterRunlevel();

public:
    RunlevelManager();
    virtual ~RunlevelManager();

    void prepareRunlevelEntrance(Runlevel *in_pRlv);

    /// \returns the runlevel that is currently active.
    /// \note Calling this right after a call to prepareRunlevelEntrance(X)
    ///       will NOT return X, as the runlevel will not yet have changed.
    inline Runlevel *getCurrRunlevel() {return m_pCurrRunlevel;};
};

} // namespace FTS

#endif /* D_RUNLEVELS_H */

 /* EOF */
