/**
 * \file Renderer.h
 * \author Pompei2
 * \date 13 March 2010
 * \brief This file contains the definition of the default (OpenGL) renderer.
 **/

#ifndef FTS_RESOLUTION_H
#define FTS_RESOLUTION_H

#include "main.h"

#include <list>

namespace FTS {
    class String;

struct Resolution {
    uint16_t w;
    uint16_t h;
    bool fs;

#ifndef D_NO_SDL
    Resolution();
#endif // D_NO_SDL
    Resolution(const String& in_s);
    Resolution(uint16_t in_w, uint16_t in_h, bool in_fs);

    String toString(bool in_bFS) const;
    float aspectRatio() const;
    Resolution bestFit(const std::list<Resolution>& in_res) const;

    /// To allow sorting a list of resolutions. The width is compared first, then
    /// the height, then windowed mode is considered smaller than fullscreen mode.
    bool operator<(const Resolution&) const;
};

} // namespace FTS

#endif // FTS_RESOLUTION_H
