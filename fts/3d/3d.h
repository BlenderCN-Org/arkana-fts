#ifndef D_3D_H
#define D_3D_H

#include "main.h"
#include "opengl_wrapper.h"

namespace FTS {
    class String;
}

/// Verify the error state of openGL, and if there is an error,
/// it displays an error message.
/// \param msg The error message to display in case of error.
/// \return ERR_OK if there was no error, else the opengl error code.
/// \author Pompei2
#ifdef DEBUG
#include "dLib/dString/dString.h"
int verifGL(const FTS::String& in_sMessage);
#else                         // Personally I think in release mode no opengl error should happen !
// int verifGL(const String &in_sMessage);
#  define verifGL( a ) (void)0;
#endif

namespace FTS {
    void* glGetProcAddress(const char* fname);
    bool glHasExtension(const char* extname);
}; // namespace FTS

#endif                          /* D_3D_H */

/* EOF */
