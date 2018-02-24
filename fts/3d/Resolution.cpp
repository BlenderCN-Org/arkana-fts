#include "Resolution.h"

#include "3d/Math.h"
#include "3d/Renderer.h"

#include "dLib/dString/dString.h"

#ifndef D_NO_SDL
#  include <SDL.h>
#endif // D_NO_SDL

#include <iterator>
#include <vector>
#include <cmath>

using namespace FTS;

FTS::Resolution::Resolution(const String& in_s)
    : w(0)
    , h(0)
    , fs(true)
{
    // Find the "x" that separates w and h.
    std::vector<String> vWH;
    in_s.split(std::inserter(vWH, vWH.begin()), "x");

    // read out the width and height.
    if(vWH.size() > 0)
        vWH[0].to(this->w);
    if(vWH.size() > 1)
        vWH[1].to(this->h);

    // check if in fullscreen.
    String s = String::nr(w) + "x" + String::nr(h);
    String rest = in_s.mid(s.len(), 0);
    if(rest == ", windowed")
        this->fs = false;
}

#ifndef D_NO_SDL
FTS::Resolution::Resolution()
{
    if(Renderer::getSingletonPtr() == nullptr) {
        // If called before the singleton is created (before the fts video init) return.
        return;
    }

    // If the SDL window is not initialized yet, return.
    auto window = Renderer::getSingleton().getWindow();
    if(window == nullptr) {
        return;
    }

    // Get the current window resolution.
    // Not taking the one from the config so that it even works before we have
    // a window. Useful when there is no config yet.
    SDL_DisplayMode info;
    if(SDL_GetWindowDisplayMode(window, &info) == 0) {
        w = info.w;
        h = info.h;
    } else {
        w = h = 1;
    }

    auto flags = SDL_GetWindowFlags(window);
    if(flags & SDL_WINDOW_FULLSCREEN) {
        fs = true;
    } else {
        fs = false;
    }
}
#endif // D_NO_SDL

FTS::Resolution::Resolution(uint16_t in_w, uint16_t in_h, bool in_fs)
    : w(in_w)
    , h(in_h)
    , fs(in_fs)
{
}

String FTS::Resolution::toString(bool in_bFS) const
{
    String s = String::nr(this->w) + "x" + String::nr(this->h);

    if(in_bFS) {
        if(!this->fs)
            s += ", windowed";
        else
            s += ", fullscreen";
    }

    return s;
}

float Resolution::aspectRatio() const
{
    if(h == 0)
        return 1.0f;

    return static_cast<float>(w)/static_cast<float>(h);
}

Resolution Resolution::bestFit(const std::list<Resolution>& in_res) const
{
    // First, get a list of those with matching aspect ratios.
    std::list<Resolution> goodAr;
    float ar = this->aspectRatio();

    Resolution bestInexactAr = Resolution(100, 1, false); // Aspect ratio of 100.
    for(auto iRes = in_res.begin() ; iRes != in_res.end() ; ++iRes){
        if(nearZero(ar - iRes->aspectRatio())) {
            // Found a perfect fit? Add it to the found list.
            goodAr.push_back(*iRes);
        } else if(std::abs(ar - iRes->aspectRatio()) < std::abs(ar - bestInexactAr.aspectRatio())) {
            // Found a better approx than the current best? use that!
            bestInexactAr = *iRes;
        }
    }

    // No exact aspect ratio fit? just use the best approximation...
    if(goodAr.empty()) return bestInexactAr;

    // But if we had a fitting aspect ratio, now choose the most fitting resolution.
    // We always "round up" that is prefer too big rather than too small.
    // Also, we need only to consider the width, as the aspect ratios match.
    Resolution bestApprox = Resolution(10000, 10000, false); // huge.
    for(auto iRes = goodAr.begin() ; iRes != goodAr.end() ; ++iRes){
        if(iRes->w > this->w && iRes->w < bestApprox.w) {
            bestApprox = *iRes;
        }
    }

    return bestApprox.w != 10000 ? bestApprox : goodAr.front();
}

bool Resolution::operator<(const Resolution& o) const
{
    if(this->w < o.w)
        return true;

    if(this->w > o.w)
        return false;

    // Same width, look at height.
    if(this->h < o.h)
        return true;

    if(this->h > o.h)
        return false;

    // Same width and height, look at fullscreen.
    if(!this->fs && o.fs)
        return true;

    return false;
}
