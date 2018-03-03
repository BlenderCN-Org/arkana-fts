/**
 * \file Renderer.h
 * \author Pompei2
 * \date 13 March 2010
 * \brief This file contains the definition of the default (OpenGL) renderer.
 **/

#ifndef FTS_RENDERER_H
#define FTS_RENDERER_H

#include "main.h"

#include "3d/camera.h"
#include "utilities/Singleton.h"

#include <list>

struct SDL_Window;

namespace FTS {
    struct Resolution;

class Renderer : public Singleton<Renderer> {
    /// A default 2D camera.
    Camera m_default2DCam;

    /// A default camera object that is used for 3D mode if no other camera is
    /// set by the runlevel.
    Camera m_default3DCam;

    /// SDL: The screen surface.
    SDL_Window *m_pScreen = nullptr;
    void * m_Context = nullptr;
    // Graphics mode creation methods.
    void createSDLWindow(const Resolution& in_res);
    static uint32_t calcSDLVideoFlags(bool in_bFullscreen);

    void init();
    void deinit();
public:
    Renderer();
    virtual ~Renderer();

    virtual void enter2DMode(const Camera& in_cam);
    virtual void enter3DMode(const Camera& in_cam);

    virtual bool changeResolution(const Resolution& in_res);
    static std::list<Resolution> getSupportedResolutions();

    inline Camera& getDefault2DCamera() {return m_default2DCam;};
    inline Camera& getDefault3DCamera() {return m_default3DCam;};
    inline SDL_Window* getWindow() { return m_pScreen; }
};

} // namespace FTS

#endif // FTS_RENDERER_H
