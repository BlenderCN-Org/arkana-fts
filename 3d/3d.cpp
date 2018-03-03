#include "3d/3d.h"
#include "logging/logger.h"
#include "dLib/dString/dString.h"

#include <SDL_video.h> // For SDL_GL_GetProcAddress.

#include <set>
#include <iterator>

using namespace FTS;

#ifdef DEBUG
/// Check the OpenGL state for errors.
/** This looks if there was an OpenGL error, if yes, prints it out.
 *
 * \param in_sMessage The function name to display.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      The OpenGL error code.
 *
 * \note In release mode, this function does nothing.
 *
 * \author Pompei2
 */
int verifGL(const String& in_sMessage)
{
    GLenum e;

    while((e = glGetError()) != GL_NO_ERROR) {
        switch(e) {
        case GL_INVALID_ENUM:
            FTS18N("GLError", MsgType::Error, "invalid enum", in_sMessage);
            return GL_INVALID_ENUM;
        case GL_INVALID_VALUE:
            FTS18N("GLError", MsgType::Error, "invalid value", in_sMessage);
            return GL_INVALID_VALUE;
        case GL_INVALID_OPERATION:
            FTS18N("GLError", MsgType::Error, "invalid operation", in_sMessage);
            return GL_INVALID_OPERATION;
        case GL_STACK_OVERFLOW:
            FTS18N("GLError", MsgType::Error, "stack overflow", in_sMessage);
            return GL_STACK_OVERFLOW;
        case GL_STACK_UNDERFLOW:
            FTS18N("GLError", MsgType::Error, "stack underflow", in_sMessage);
            return GL_STACK_UNDERFLOW;
        case GL_OUT_OF_MEMORY:
            FTS18N("GLError", MsgType::Error, "out of memory", in_sMessage);
            return GL_OUT_OF_MEMORY;
        case GL_NO_ERROR:
            return ERR_OK;
        default:
            FTS18N("GLError", MsgType::Error, "unknown error code " + String::nr(e), in_sMessage);
            return e;
        }
    }

    return ERR_OK;
}
#endif

void* FTS::glGetProcAddress(const char* fname)
{
    return SDL_GL_GetProcAddress(fname);
}

bool FTS::glHasExtension(const char* extname)
{
    static std::set<String> exts;

    verifGL("glHasExtension start");

    if(exts.empty()) {
        // Load all extensions:
        if(glGetString(GL_VERSION)[0] >= '3') {
            GLint nExts = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &nExts);
            for(GLint i = 0 ; i < nExts ; ++i) {
                exts.insert(glGetStringi(GL_EXTENSIONS, i));
            }
        } else {
            String sExts = glGetString(GL_EXTENSIONS);
            sExts.split(std::inserter(exts, exts.begin()));
        }
    }

    verifGL("glHasExtension end");
    return exts.find(extname) != exts.end();
}

/* EOF */
