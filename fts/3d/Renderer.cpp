#include "Renderer.h"

#include "3d/3d.h"

#include "3d/Resolution.h"
#include "3d/Shader.h"
#include "3d/VertexArrayObject.h"
#include "logging/logger.h"
#include "graphic/graphic.h"
#include "main/runlevels.h"
#include "main/Exception.h"
#include "ui/ui.h"
#include "3d/light.h"

#include "dLib/dConf/ArkanaDefaultSettings.h"
#include "dLib/dConf/configuration.h"
#include "dLib/dString/dString.h"

#include <iterator>
#include <SDL.h>

namespace FTS {
/// This exception is thrown when there is some hardware limit reached, making
/// the call fail, for example no more memory.
class TooOldOpenGLException : public virtual HardwareLimitException {
protected:
    TooOldOpenGLException() throw()
        : LoggableException(new I18nLoggerCmd("InvCall", MsgType::Horror, "TooOldOpenGLException() being called!"))
    {};
public:
    TooOldOpenGLException(const String& in_sCurr, const String& in_sNeeded) throw()
        : LoggableException(new I18nLoggerCmd("TooOldOpenGL", MsgType::Error, in_sCurr, in_sNeeded))
    {};
};
} //namespace FTS

using namespace FTS;

FTS::Renderer::Renderer()
{
    m_default2DCam.ortho2DProjection();

    Configuration conf ("conf.xml", ArkanaDefaultSettings());
    this->changeResolution(Resolution(conf.getInt("HRes"), conf.getInt("VRes"), conf.getBool("Fullscreen")));
}

FTS::Renderer::~Renderer()
{
}

/** (Re)create the SDL screen surface with (new) options. You gotta remake the
 *  OpenGL matrices too ; you can do that by calling the UI's resize member.
 *
 * \param in_iW The new width.
 * \param in_iH The new height.
 * \param in_iBPP The new color depth.
 * \param in_bFullscreen Fullscreen or not ?
 *
 * \throws HardwareLimitException in case the resolution is unsupported
 *
 * \author Pompei2
 */
void FTS::Renderer::createSDLWindow(const Resolution& in_res)
{
    verifGL("Renderer::createSDLWindow start");

    // Flags to pass to SDL_SetVideoMode
    uint32_t iVideoFlags = Renderer::calcSDLVideoFlags(in_res.fs);

    // Sets up OpenGL double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // And disable vsync, so we can see the exact FPS.
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);

    //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );

    // First check, to avoid screen kills.
    if(0 == SDL_VideoModeOK(in_res.w, in_res.h, 32, iVideoFlags)) {
        throw HardwareLimitException("Screen resolution " + in_res.toString(true), 0, 0);
    }

    // get a SDL surface
    m_pScreen = SDL_SetVideoMode(in_res.w, in_res.h, 32, iVideoFlags);
    if(!m_pScreen) {
        throw HardwareLimitException("Screen resolution " + in_res.toString(true), 0, 0);
    }

    String sVersion = glGetString(GL_VERSION);
    std::vector<String> versions;
    sVersion.split(std::inserter(versions, versions.begin()), ".");
    int subVersion = 0;
    if(versions.size() < 2 || versions[0].to<int>(subVersion) < 3) {
        throw TooOldOpenGLException(sVersion, "3.0");
    }

    SDL_WM_SetCaption(FTS_WINDOW_TITLE, NULL);
    verifGL("Renderer::createSDLWindow end");
}

/** Creates the video flags combination to be used by the SDL to create display.
 *
 * \param in_bFullscreen Fullscreen or not ?
 *
 * \return the video flags.
 *
 * \author Pompei2
 */
uint32_t FTS::Renderer::calcSDLVideoFlags(bool in_bFullscreen)
{
    uint32_t iVideoFlags = 0;

    // Fetch the video info, if possible
    const SDL_VideoInfo *pVideoInfo = SDL_GetVideoInfo();

    // The flags to pass to SDL_SetVideoMode
    iVideoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL
    iVideoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
    iVideoFlags |= SDL_HWPALETTE;       // Store the palette in hardware

    // Maybe we want it in fullscreen ?
    if(in_bFullscreen)
        iVideoFlags |= SDL_FULLSCREEN;

    if(pVideoInfo) {
        if(pVideoInfo->hw_available)
            iVideoFlags |= SDL_HWSURFACE;
        else
            iVideoFlags |= SDL_SWSURFACE;

        if(pVideoInfo->blit_hw)
            iVideoFlags |= SDL_HWACCEL;
    } else {
        // VideoInfo is null for example before SDL init.
        // Any modern hardware that wants to run a game must support that...
        iVideoFlags |= SDL_HWSURFACE;
        iVideoFlags |= SDL_HWACCEL;
    }

    return iVideoFlags;
}

/// General OpenGL init function
/** This sets up some default OpenGL propreties.
 *
 * \throws
 * \TODO review for what is still needed in opengl 3.2!
 *
 * \author Pompei2
 */
void FTS::Renderer::init()
{
    verifGL("Renderer::init start");

    // Enable smooth shading
    glShadeModel(GL_SMOOTH);

    // Set the "background color"
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    // Enables Depth Testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Really Nice Perspective Calculations
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Enables the using of textures.
    glEnable(GL_TEXTURE_2D);

    // Configure the textures.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    float fCol[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, fCol);

    // Enables Alpha Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // We don't want that fucking culling, that makes avery primitive
    // That is specified counter-clockwise disappear ! It drives me crazy.
//     glDisable(GL_CULL_FACE);

    verifGL("Renderer::init end");

    // Create the lighting system and then create the sun and the moon, hehehe.
    /// \TODO: Move this into the runlevel
    LightSystem::init();
    LightSystem::getSys()->addDefaultLights();
}

void FTS::Renderer::deinit()
{
    LightSystem::deinit();
}

/// This function enters into the 2D drawing mode.
/** Prepares OpenGL for 2D drawing, i.e. sets up ortho projection matrices,
 *  disables all special features like lighting, sets up a correct blending
 *  function, ...
 *
 * \todo instead of needing all this setup, later we just need to load the
 *       correct shader!
 *
 * \param in_cam The camera to use for this 2D mode.
 *
 * \author Pompei2
 **/
void FTS::Renderer::enter2DMode(const Camera& in_cam)
{
    verifGL("Renderer::enter2DMode start");

    Program::unbind();
    VertexBufferObject::unbind();
    ElementsBufferObject::unbind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    GraphicManager::getSingleton().reinitFrame();

    // This allows alpha blending of 2D textures.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    in_cam.use();

    verifGL("Renderer::enter2DMode end");
}

/// This function enters into the 3D drawing mode.
/** This function goes back into "normal" 3D rendering mode. You can
 *  use the normal openGL drawing functions while in this mode.
 *
 * \note The graphic Draw funcitons won't work while in 3D mode.\n
 *
 * \author Pompei2
 **/
void FTS::Renderer::enter3DMode(const Camera& in_cam)
{
    verifGL("Renderer::enter3DMode start");

    // Re-Enable stuff.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Deselect any texture we might have kept selected.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    LightSystem::getSys()->setupGLLighting();
    GraphicManager::getSingleton().reinitFrame();
    in_cam.use();

    verifGL("Renderer::enter3DMode end");
}

/// Completely change the resolution, color depth and fullscreen of the game.
/** This function is only guaranteed to work properly in the menu !
 *  It creates a new SDL screen surface, re-inits the OpenGL viewports,
 *  reset and reloads the CEGUI rendering system and reloads all the
 *  FTS textures and graphics. If you want to keep one of the parameters
 *  as it was before, set it to -1. on failure, it calls itself with
 *  the old values.
 *
 * \param in_iW   The new width. If -1, this is kept unchanged.
 * \param in_iH   The new height. If -1, this is kept unchanged.
 * \param in_iBPP The new color depth. If -1, this is kept unchanged.
 * \param in_iFS  The new fullscreen mode. If -1, this is kept unchanged.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 **/
bool FTS::Renderer::changeResolution(const Resolution& res)
{
    Runlevel *pRlv = RunlevelManager::getSingleton().getCurrRunlevel();

    Resolution backupResol = Resolution();

    try {
        // Unload curr runlevel
        if(pRlv)
            pRlv->unload();

        // Everybody, grab your data!
        GUI::getSingleton().preChangeResolution(res.w, res.h);
        if(GraphicManager::getSingletonPtr())
            GraphicManager::getSingleton().grabAllGraphics();

        bool bDeletedShaderManager = false;
        if(ShaderManager::getSingletonPtr()) {
            delete ShaderManager::getSingletonPtr();
            bDeletedShaderManager = true;
        }

        // Do the hard change.
        this->createSDLWindow(res);
        this->init();

        // Reset the default cameras.
        m_default2DCam.reset((float)res.w, (float)res.h);
        m_default2DCam.ortho2DProjection();
        m_default3DCam.reset((float)res.w, (float)res.h);

        GUI::getSingleton().postChangeResolution(res.w, res.h);
        if(GraphicManager::getSingletonPtr())
            GraphicManager::getSingleton().restoreAllGraphics();

        if(bDeletedShaderManager)
            new ShaderManager();

        // Reload curr runlevel
        if(pRlv && !pRlv->load())
            throw 3;

        // We were successful! Store the options.
        Configuration conf ("conf.xml", ArkanaDefaultSettings());

        conf.set("Fullscreen", res.fs);
        conf.set("HRes", res.w);
        conf.set("VRes", res.h);
        conf.save();

        return true;
    } catch(const TooOldOpenGLException&) {
        // We cannot rescue on this one...
        throw;
    } catch(...) {
        // omg !! something gone wrong !! Wtf :p backup
        changeResolution(backupResol);
        pRlv->load();
        return false;
    }
}

std::list<Resolution> FTS::Renderer::getSupportedResolutions()
{
    std::list<Resolution> ret;

    // Then, we sort em out: one thing is we don't care about BPP, so sort out
    // those that appear with different BPPs.
    std::set< std::pair<int, int> > resols;

    SDL_Rect **pModes = SDL_ListModes(NULL, Renderer::calcSDLVideoFlags(true));
    if(pModes == (SDL_Rect **) 0) {
    } else if(pModes == (SDL_Rect **) -1) { // -1 means any works.
#ifdef DEBUG
        ret.push_back(Resolution(800, 600, true));
#endif
        ret.push_back(Resolution(1024, 768, true));
        ret.push_back(Resolution(1152, 864, true));
        ret.push_back(Resolution(1280, 720, true));
        ret.push_back(Resolution(1280, 768, true));
        ret.push_back(Resolution(1280, 960, true));
        ret.push_back(Resolution(1280, 1024, true));
    } else {
        for(int i = 0; pModes[i]; i++) {
#ifdef DEBUG
            // We don't support modes below 800x600.
            if(pModes[i]->w < 800 || pModes[i]->h < 600)
                continue;
#else
            // We don't support modes below 1024x768 or 1280x720.
            if(pModes[i]->w < 1024 || pModes[i]->h < 720)
                continue;
#endif

            // Already got that one? go ahead to the next one.
            if(resols.find(std::make_pair(pModes[i]->w, pModes[i]->h)) != resols.end())
                continue;

            ret.push_back(Resolution(pModes[i]->w, pModes[i]->h, true));
            resols.insert(std::make_pair(pModes[i]->w, pModes[i]->h));
        }
    }

    return ret;
}