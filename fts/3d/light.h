/**
 * \file light.h
 * \author Pompei2
 * \date 28 May 2008
 * \brief This file declares everything for the global light (sun, moon, ...).
 **/

#ifndef FTS_LIGHT_H
#define FTS_LIGHT_H

#include <list>

#include "3d/Math.h"
#include "3d/3d.h"
#include "utilities/DateTime.h"

namespace FTS {

/// This abstract class is the base class of all lights.
class Light {
public:
             Light(bool in_bEnable = true) { m_nLights++;};
    virtual ~Light(void) {};

    virtual Light *enable(void) = 0;
    virtual Light *disable(void) = 0;

    virtual Light *setPos(const Vector &in_pos) = 0;
    virtual Vector getPos(void) = 0;

    virtual int render(const DateTime &in_time) = 0;
    virtual int render(int in_iSecondsInDay) = 0;

protected:
    static bool m_bUsedGLLights[8]; ///< For every OpenGL light if it is used or not.
    static int m_nEnabledLights;    ///< How many lights are there in total.
    static int m_nLights;           ///< How many lights are there in total.
};

/// This is the manager that cares about all lights.
class LightSystem {
private:
    /// Default constructor.
    LightSystem(void) {};
    /// Copy constructor ; prohibit copy.
    LightSystem(const LightSystem &) {};

public:
    virtual ~LightSystem(void);

    int add(Light *in_pLight);
    int rem(Light *in_pLight);
    int addDefaultLights();

    int renderAll(const DateTime &in_time);
    int renderAll(int in_iSecondsInDay);

    int setupGLLighting(void);

    static void init(void);
    static void deinit(void);
    inline static LightSystem *getSys(void) {return m_pSingleton;};

protected:
    std::list<Light *>m_lpLights;
    /// The singleton object.
    static LightSystem *m_pSingleton;
};

class SunMoon : public Light {
public:
             SunMoon(bool in_bEnable = true);
    virtual ~SunMoon(void);

    Light *enable(void);
    Light *disable(void);

    Light *setPos(const Vector &in_pos);
    Vector getPos(void);

    int render(const DateTime &in_time);
    int render(int in_iSecondsInDay);

private:
    int  m_iLightID;          ///< GL_LIGHT0 + this is the GL Light ID.
    bool m_bEnabled;          ///< Is this light enabled or not ?
    Vector m_pos;             ///< Where am I ?

    // My own state variables.
    float m_pfAmbientLight[4];
    float m_pfDiffuseLight[4];
    float m_pfSpecularLight[4];

    int recalcSelf(int in_iSecondsInDay);
};

} // namespace FTS

#endif /* FTS_LIGHT_H */

 /* EOF */
