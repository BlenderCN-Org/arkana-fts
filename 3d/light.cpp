/**
 * \file light.cpp
 * \author Pompei2
 * \date 28 May 2008
 * \brief This file declares everything for the global light (sun, moon, ...).
 **/
#include <cmath>

#include "3d/light.h"
using namespace FTS;

bool Light::m_bUsedGLLights[8] = {false, false, false, false, false, false, false, false};
int Light::m_nEnabledLights = 0;
int Light::m_nLights = 0;

LightSystem *LightSystem::m_pSingleton = NULL;

void LightSystem::init(void)
{
    m_pSingleton = new LightSystem();
    m_pSingleton->setupGLLighting();
}

void LightSystem::deinit(void)
{
    SAFE_DELETE(m_pSingleton);
}

LightSystem::~LightSystem(void)
{
    // Remove all still existing lights.
    while(!m_lpLights.empty()) {
        Light *p = m_lpLights.front();
        m_lpLights.pop_front();
        SAFE_DELETE(p);
    }
}

int LightSystem::add(Light *in_pLight)
{
    m_lpLights.push_back(in_pLight);

    return ERR_OK;
}

int LightSystem::rem(Light *in_pLight)
{
    // Find the light.
    std::list<Light *>::iterator i = m_lpLights.begin();
    while( i != m_lpLights.end() && *i != in_pLight) i++;

    if(i != m_lpLights.end() && *i == in_pLight)
        m_lpLights.erase(i);

    return ERR_OK;
}

int LightSystem::addDefaultLights()
{
    return this->add((new SunMoon)->setPos(Vector(0.0f, 20.0f, 10.0f)));
}

int LightSystem::renderAll(const DateTime &in_time)
{
    for(std::list<Light *>::iterator i = m_lpLights.begin() ; i != m_lpLights.end() ; i++) {
        Light *p = *i;
        p->render(in_time);
    }

    return ERR_OK;
}

int LightSystem::renderAll(int in_iSecondsInDay)
{
    for(std::list<Light *>::iterator i = m_lpLights.begin() ; i != m_lpLights.end() ; i++) {
        Light *p = *i;
        p->render(in_iSecondsInDay);
    }

    return ERR_OK;
}

int LightSystem::setupGLLighting(void)
{
    verifGL("LightSystem::setupGLLighting start");
    // Enable lighting and calculate the specular color after the texture
    // mapping is done, so the specular point doesn't disappear.
    glEnable(GL_LIGHTING);
//     glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    // Now the use of glColor3f instead of glMaterialfv .. ?
//     glEnable(GL_COLOR_MATERIAL);
//     glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Maybe use a global ambient colour ? TODO: Maybe for sunset, night etc ?
    GLfloat lmodel_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    // Highlight specular are relative to the viewer's place.
    // VERY bad option, don't know its use, just caused me trouble !!
//     glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    verifGL("LightSystem::setupGLLighting end");

    return ERR_OK;
}

/// Constructor
SunMoon::SunMoon(bool in_bEnable)
    : m_iLightID(-1),
      m_bEnabled(in_bEnable)
{
    memset(m_pfAmbientLight, 0, sizeof(m_pfAmbientLight));
    memset(m_pfDiffuseLight, 0, sizeof(m_pfDiffuseLight));
    memset(m_pfSpecularLight, 0, sizeof(m_pfSpecularLight));

    if(in_bEnable)
        this->enable();
}

/// Destructor
SunMoon::~SunMoon(void)
{
}

Light *SunMoon::enable(void)
{
    verifGL("SunMoon::enable start");
    // Look for the first "free" OpenGL light.
    int i = 0;
    while(m_bUsedGLLights[i]) i++;

    // Only continue if we have found a free spot.
    if(i>7) {
        m_iLightID = -1;
        m_bEnabled = false;
        return this;
    }

    m_iLightID = i;
    m_bUsedGLLights[m_iLightID] = true;
    glEnable(GL_LIGHT0+m_iLightID);
    m_nEnabledLights++;

    // This calls all opengl calls to tell OpenGL this light exists.
    // And what it is like.
//     this->render(12.0f);

    verifGL("SunMoon::enable end");
    return this;
}

Light *SunMoon::disable(void)
{
    verifGL("SunMoon::disable start");
    glDisable(GL_LIGHT0+m_iLightID);
    m_bUsedGLLights[m_iLightID] = false;
    m_iLightID = -1;
    m_nEnabledLights--;
    m_bEnabled = false;
    verifGL("SunMoon::disable end");
    return this;
}

Light *SunMoon::setPos(const Vector &in_pos)
{
    m_pos = in_pos;

    return this;
}

Vector SunMoon::getPos(void)
{
    return m_pos;
}

int SunMoon::render(const DateTime &in_time)
{
    // Currently, we do not care about the season in our lighting model.
    this->render(in_time.getHour() * 24*60 +
                 in_time.getMinute() * 60 +
                 in_time.getSecond());

    return ERR_OK;
}

int SunMoon::render(int in_iSecondsInDay)
{
    if(!m_bEnabled)
        return ERR_OK;

    verifGL("SunMoon::render start");
    this->recalcSelf(in_iSecondsInDay);

    float fPos[] = {m_pos.x(), m_pos.y(), m_pos.z(), 1.0f};

    glLightfv(GL_LIGHT0+m_iLightID, GL_AMBIENT, m_pfAmbientLight);
    glLightfv(GL_LIGHT0+m_iLightID, GL_DIFFUSE, m_pfDiffuseLight);
    glLightfv(GL_LIGHT0+m_iLightID, GL_SPECULAR, m_pfSpecularLight);
    glLightfv(GL_LIGHT0+m_iLightID, GL_POSITION, fPos);

    // The light model
    float fAmbient[4] = {0.7f, 0.7f, 0.7f, 1.0f};
    float fDiffuse[4] = {0.7f, 0.7f, 0.7f, 1.0f};
    float fSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float fEmission[4] = {0.3f, 0.2f, 0.2f, 0.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, fAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, fDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, fSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0f);
    glMaterialfv(GL_FRONT, GL_EMISSION, fEmission);

#ifdef DEBUG
    glPushMatrix();
    glTranslatef(fPos[0], fPos[1], fPos[2]);
    glScalef(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        // Front Face
        glNormal3f( 0.0f, 0.0f, 1.0f);// Normal Pointing Towards Viewer
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);// Point 1 (Front)
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);// Point 2 (Front)
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);// Point 3 (Front)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);// Point 4 (Front)
        // Back Face
        glNormal3f( 0.0f, 0.0f,-1.0f);// Normal Pointing Away From Viewer
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);// Point 1 (Back)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);// Point 2 (Back)
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);// Point 3 (Back)
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);// Point 4 (Back)
        // Top Face
        glNormal3f( 0.0f, 1.0f, 0.0f);// Normal Pointing Up
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);// Point 1 (Top)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);// Point 2 (Top)
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);// Point 3 (Top)
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);// Point 4 (Top)
        // Bottom Face
        glNormal3f( 0.0f,-1.0f, 0.0f);// Normal Pointing Down
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);// Point 1 (Bottom)
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);// Point 2 (Bottom)
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);// Point 3 (Bottom)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);// Point 4 (Bottom)
        // Right face
        glNormal3f( 1.0f, 0.0f, 0.0f);// Normal Pointing Right
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);// Point 1 (Right)
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);// Point 2 (Right)
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);// Point 3 (Right)
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);// Point 4 (Right)
        // Left Face
        glNormal3f(-1.0f, 0.0f, 0.0f);// Normal Pointing Left
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);// Point 1 (Left)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);// Point 2 (Left)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);// Point 3 (Left)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);// Point 4 (Left)
    glEnd();// Done Drawing Quads    glPopMatrix();
    glPopMatrix();
#endif

    verifGL("SunMoon::render end");
    return ERR_OK;
}

int SunMoon::recalcSelf(int in_iSecondsInDay)
{
    // If in_fSecondsInDay is too high (means: more then one day),
    // calculate the seconds in the current day.
    const int iMaxSecsInDay = 24 * 60 * 60;
    int iSecsInDay = in_iSecondsInDay % iMaxSecsInDay;

    // Re-calculate the position (imagine the unit circle):
    //               12h
    //              y ^
    //                |
    //                |
    //                |
    //                |          x
    // 18h -----------+-----------> 6h
    //                | -pi/4
    //                |
    //                |
    //                |
    //              24/0h
    float fSunPosInRad = ((float)iSecsInDay / (float)iMaxSecsInDay) * (float)M_PI * 2 - (float)M_PI_4;
    m_pos.x(cos(fSunPosInRad)*100.0f)
         .z(100.0f);
//         ->setZ(sin(fSunPosInRad)*100.0f);

    // We separate the day into four parts:

    if((float)(21*60*60) <= iSecsInDay || iSecsInDay < (float)(6*60*60)) {
        // Night (21h->6h) dark blueish ambient light.
        m_pfAmbientLight[0] = 0.05f;
        m_pfAmbientLight[1] = 0.05f;
        m_pfAmbientLight[2] = 0.5f;
        m_pfAmbientLight[3] = 1.0f;

        m_pfDiffuseLight[0] = 0.05f;
        m_pfDiffuseLight[1] = 0.05f;
        m_pfDiffuseLight[2] = 0.5f;
        m_pfDiffuseLight[3] = 1.0f;
    } else if((float)(6*60*60) <= iSecsInDay && iSecsInDay < (float)(9*60*60)) {
        // Morning (6h->9h) bright greenish ambient light
        m_pfAmbientLight[0] = 1.2f;
        m_pfAmbientLight[1] = 0.8f;
        m_pfAmbientLight[2] = 0.8f;
        m_pfAmbientLight[3] = 1.0f;

        m_pfDiffuseLight[0] = 1.2f;
        m_pfDiffuseLight[1] = 0.8f;
        m_pfDiffuseLight[2] = 0.8f;
        m_pfDiffuseLight[3] = 1.0f;
    } else if((float)(9*60*60) <= iSecsInDay && iSecsInDay < (float)(18*60*60)) {
        // Day (9h->18h) bright white ambient light
        m_pfAmbientLight[0] = 0.1f;
        m_pfAmbientLight[1] = 0.1f;
        m_pfAmbientLight[2] = 0.1f;
        m_pfAmbientLight[3] = 1.0f;

        m_pfDiffuseLight[0] = 0.97f;
        m_pfDiffuseLight[1] = 1.00f;
        m_pfDiffuseLight[2] = 0.62f;
        m_pfDiffuseLight[3] = 1.0f;
    } else if((float)(18*60*60) <= iSecsInDay && iSecsInDay < (float)(21*60*60)) {
        // Evening (18h->21h) reddish-orange ambient light
        m_pfAmbientLight[0] = 0.5f;
        m_pfAmbientLight[1] = 0.05f;
        m_pfAmbientLight[2] = 0.05f;
        m_pfAmbientLight[3] = 1.0f;

        m_pfDiffuseLight[0] = 0.5f;
        m_pfDiffuseLight[1] = 0.05f;
        m_pfDiffuseLight[2] = 0.05f;
        m_pfDiffuseLight[3] = 1.0f;
    }

//     m_pfAmbientLight[0] = 0.2f;
//     m_pfAmbientLight[1] = (ABS(m_pos.z()) / 50.0f) * 0.2f;
//     m_pfAmbientLight[2] = (ABS(m_pos.z()) / 50.0f) * 0.2f;
//     m_pfAmbientLight[3] = 1.0f;

//     m_pfDiffuseLight[0] = 1.0f;
//     m_pfDiffuseLight[1] = 1.0f;
//     m_pfDiffuseLight[2] = 1.0f;
// //     m_pfDiffuseLight[2] = (ABS(m_pos.z()) / 50.0f);
//     m_pfDiffuseLight[3] = 1.0f;

    m_pfSpecularLight[0] = 0.5f;
    m_pfSpecularLight[1] = 0.5f;
    m_pfSpecularLight[2] = 0.5f;
    m_pfSpecularLight[3] = 1.0f;

    return ERR_OK;
}

 /* EOF */
