/**
 * \file camera.cpp
 * \author Pompei2
 * \date 15 July 2006
 * \brief This file contains the camera class implementation.
 **/

#include "3d/camera.h"

#include "3d/opengl_wrapper.h"

#include "logging/logger.h"
#include "main/runlevels.h" // To get the active camera.
#include "3d/Math.h"
#include "3d/Resolution.h" // To construct a default camera.
#include "3d/3d.h" // To setup the viewport (glViewport)

#include <cmath>

using namespace FTS;

Camera& FTS::Camera::reset(float in_fNewW, float in_fNewH)
{
    if(in_fNewW < 1.0f || in_fNewH < 1.0f) {
        m_fW = (float)Resolution().w;
        m_fH = (float)Resolution().h;
    } else {
        m_fW = in_fNewW;
        m_fH = in_fNewH;
    }

    // Default camera uses this perspective projection:
    // why is the near plane so far? we need a camera that looks far away,
    // we really don't need close-up views! And see the note about learning
    // to love your Z-Buffer in the comment of that method.
    this->perspectiveProjection(45.0f, 1.0f, 1000.0f);

    return *this;
}

/// \return The position of the camera.
Vector FTS::Camera::getPos() const
{
    // The camera has position 0,0,0 in view space.
    return m_viewMatrix.inverse()*Vector(0.0f, 0.0f, 0.0f, 1.0f);
}

/// \return The camera's "front" vector, a vector that points to the front of
///         the camera, the camera's viewing direction so to say.
/// \note The returned vector has unit length.
Vector FTS::Camera::getFront() const
{
    // This vector should already be normalized, but let's just be sure:
    return Vector(-m_viewMatrix(3,1), -m_viewMatrix(3,2), -m_viewMatrix(3,3)).normalize();
}

/// \return The camera's "right" vector, a vector that points to the right of
///         the camera.
/// \note The returned vector has unit length.
Vector FTS::Camera::getRight() const
{
    // This vector should already be normalized, but let's just be sure:
    return Vector(m_viewMatrix(1,1), m_viewMatrix(1,2), m_viewMatrix(1,3)).normalize();
}

/// \return The camera's "up" vector, a vector that points straight up from the
///         camera, like your head's top.
/// \note The returned vector has unit length.
Vector FTS::Camera::getUp() const
{
    // This vector should already be normalized, but let's just be sure:
    return Vector(m_viewMatrix(2,1), m_viewMatrix(2,2), m_viewMatrix(2,3)).normalize();
}

/// Resets the camera orientation.
/** This function resets the camera's orientation like it was at the beginning,
 *  but does not move the camera's position !
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& FTS::Camera::resetOrientation()
{
    Lock l(m_Mutex);
    m_viewMatrix = AffineMatrix::translation(m_viewMatrix);
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.to_s());

    return *this;
}

/// Makes the camera look at a certain point.
/** This function makes the camera look at a certain point. If that point is the same as its
 *  current position, the camera moves back the (current) distance |camera-target|.
 *
 * \param in_vTgt The position where the camera should look at.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::lookAt(const Vector& in_vTgt)
{
    // First, transform the target into camera (eye)-space:
    Vector vTgtEyeSpace = m_viewMatrix * in_vTgt;

    // First, look what we have to rotate around the camera's Y.
    // For this, project both the front and the target vectors onto the
    // camera's XZ plane and find out their angle.
    Vector vTgtEyeXZ = Vector(vTgtEyeSpace).y(0.0f).normalize();
    Vector vCamFront(0.0f, 0.0f, -1.0f);
    float fThetaTgt = atan2f(-vTgtEyeXZ.z(), vTgtEyeXZ.x());
    float fThetaFront = atan2f(-vCamFront.z(), vCamFront.x());
    this->rotateY(fThetaTgt-fThetaFront);

    // Next, look what we have to rotate around the camera's X.
    // For this, project both the front and the target vectors onto the
    // camera's YZ plane and find out their angle.
    Vector vTgtEyeYZ = Vector(vTgtEyeSpace).x(0.0f).normalize();
    fThetaTgt = atan2f(vTgtEyeYZ.z(), vTgtEyeYZ.y());
    fThetaFront = atan2f(vCamFront.z(), vCamFront.y());
    this->rotateX(fThetaTgt-fThetaFront);

    return *this;
}

/// Places the camera at a certain point.
/** This function places the camera at a certain point in space.
 *
 * \param in_vPos The new position of the camera.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::position(const Vector& in_vPos)
{
    Lock l(m_Mutex);
    // Translate the camera to the new position. That means translate the whole
    // scene to the opposite, that's why we do curpos-newpos.
    m_viewMatrix *= AffineMatrix::translation(this->getPos() - in_vPos);
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.to_s());

    return *this;
}

/// Moves the camera to the right.
/** This moves the camera to the right. The right is relative to the
 *  camera's orientation ! To move to the left, put a negative value here.
 *
 * \param in_fAmount The amount of units to move to the right.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveRight(float in_fAmount)
{
    // Now, moving to the right is an easy thing:
    // Just add a multiple of this vector to the current position.
    return this->position(this->getPos() + in_fAmount*this->getRight());
}

/// Moves the camera to the front.
/** This moves the camera to the front. The right is relative to the
 *  camera's orientation and to be exact, it is where the camera looks at.
 *  Moving to the front is a bit like zooming in.
 *  To move back (like zoom out), put a negative value here.
 *
 * \param in_fAmount The amount of units to move to the front.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveFront(float in_fAmount)
{
    // Now, moving to the front is an easy thing:
    // Just add a multiple of this vector to the current position.
    return this->position(this->getPos() + in_fAmount*this->getFront());
}

/// Moves the camera upwards.
/** This moves the camera upwards. Upwards is the direction that is perpendicular
 *  To your right and to your front, and shows up :) It is like the top of your head.
 *  To move downwards (to your feets), put a negative value here.
 *
 * \param in_fAmount The amount of units to move up.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveUp(float in_fAmount)
{
    // Now, moving up is an easy thing:
    // Just add a multiple of this vector to the current position.
    return this->position(this->getPos() + in_fAmount*this->getUp());
}

/// Moves the camera along the global X axis.
/** This moves the camera along the GLOBAL x axis, that means the orientation
 *  of the camera doesn't matter.
 *
 * \param in_fAmount The amount of units to move.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveGlobalX(float in_fAmount)
{
    // This one is easy too, no need for a lot of explanations:
    return this->position(this->getPos() + Vector(in_fAmount, 0.0f, 0.0f));
}

/// Moves the camera along the global Y axis.
/** This moves the camera along the GLOBAL y axis, that means the orientation
 *  of the camera doesn't matter.
 *
 * \param in_fAmount The amount of units to move.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveGlobalY(float in_fAmount)
{
    // This one is easy too, no need for a lot of explanations:
    return this->position(this->getPos() + Vector(0.0f, in_fAmount, 0.0f));
}

/// Moves the camera along the global Z axis.
/** This moves the camera along the GLOBAL z axis, that means the orientation
 *  of the camera doesn't matter.
 *
 * \param in_fAmount The amount of units to move.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveGlobalZ(float in_fAmount)
{
    // This one is easy too, no need for a lot of explanations:
    return this->position(this->getPos() + Vector(0.0f, 0.0f, in_fAmount));
}

/// Moves the camera along the front, projected down to the global XY plane.
/** This moves the camera into the direction it looks, but parallel to the
 *  global XY plane. This is what happens when scrolling in-game.
 *
 * \param in_fAmount The amount of units to move.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveFrontParralelToGlobalXY(float in_fAmount)
{
    Vector vFrontXY = this->getFront().z(0.0f).normalize();
    if(nearZero(vFrontXY.len())) {
        return this->moveUp(in_fAmount);
    }
    return this->position(this->getPos() + in_fAmount*vFrontXY);
}

/// Moves the camera along the front, projected down to the global XZ plane.
/** This moves the camera into the direction it looks, but parallel to the
 *  global XZ plane.
 *
 * \param in_fAmount The amount of units to move.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveFrontParralelToGlobalXZ(float in_fAmount)
{
    Vector vFrontXZ = this->getFront().y(0.0f).normalize();
    if(nearZero(vFrontXZ.len())) {
        return this->moveUp(in_fAmount);
    }
    return this->position(this->getPos() + in_fAmount*vFrontXZ);
}

/// Moves the camera along the front, projected down to the global YZ plane.
/** This moves the camera into the direction it looks, but parallel to the
 *  global YZ plane.
 *
 * \param in_fAmount The amount of units to move.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::moveFrontParralelToGlobalYZ(float in_fAmount)
{
    Vector vFrontYZ = this->getFront().x(0.0f).normalize();
    if(nearZero(vFrontYZ.len())) {
        return this->moveUp(in_fAmount);
    }
    return this->position(this->getPos() + in_fAmount*vFrontYZ);
}

/// \todo
Camera& Camera::orbit(const Vector& position, const Vector& in_axis, float in_fRadians)
{
/*    Quaternion q = Quaternion::rotation(in_fRadians, in_axis);
    Lock l(m_Mutex);
    m_viewMatrix *= AffineMatrix::rotationQuat(q);
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.toString());
*/
    return *this;
}

/// Rotates the camera around some arbitrary axis given in \e camera \e space.
/** This rotates the camera around some arbitrary axis given in \e camera
 *  \e space coordinates. That means if you give for example the axis (1,0,0),
 *  this will \e not rotate around the global x axis, but rather the local.
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotate(const Vector& in_axis, float in_fRadians)
{
    // Rotate in the other direction because we're the camera!
    Quaternion q = Quaternion::rotation(in_axis, -in_fRadians);
    Lock l(m_Mutex);
    m_viewMatrix = AffineMatrix::rotation(q) * m_viewMatrix;
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.to_s());
    return *this;
}

/// Rotates the camera around some arbitrary axis given in \e world \e space.
/** This rotates the camera around some arbitrary axis given in \e world
 *  \e space coordinates. That means if you give for example the axis (1,0,0),
 *  this \e will rotate around the global x axis.
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotateGlobal(const Vector& in_axis, float in_fRadians)
{
    // Get the axis into eye space:
    Vector vGlobalAxis = m_viewMatrix*Vector(in_axis, 0.0f).normalize();

    // Rotate in the other direction because we're the camera!
    Quaternion q = Quaternion::rotation(vGlobalAxis, -in_fRadians);
    Lock l(m_Mutex);
    m_viewMatrix = AffineMatrix::rotation(q) * m_viewMatrix;
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.to_s());

    return *this;
}

/// Rotates the camera around ITS x axis.
/** This rotates the camera around THE CAMERA's x axis.
 *  This basically means it looks up or down. As seen from the right,
 *  a positive value rotates counterclockwise (look up) and a negative
 *  value rotates clockwise (look down).
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotateX(float in_fRadians)
{
    Lock l(m_Mutex);
    // Rotate in the other direction because we're the camera!
    m_viewMatrix = AffineMatrix::rotationX(-in_fRadians) * m_viewMatrix;
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.to_s());

    return *this;
}

/// Rotates the camera around ITS y axis.
/** This rotates the camera around THE CAMERA's y axis.
 *  This basically means it looks left or right. As seen from the top,
 *  a positive value rotates counterclockwise (look left) and a negative
 *  value rotates clockwise (look right).
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotateY(float in_fRadians)
{
    Lock l(m_Mutex);
    // Rotate in the other direction because we're the camera!
    m_viewMatrix = AffineMatrix::rotationY(-in_fRadians) * m_viewMatrix;
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.to_s());

    return *this;
}

/// Rotates the camera around ITS z axis.
/** This rotates the camera around THE CAMERA's z axis.
 *  This is the same effect as if you rotate your head to bring an ear
 *  close to the shoulder. A positive value rotates counterclockwise (left shoulder)
 *  and a negative value rotates clockwise (right shoulder).
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotateZ(float in_fRadians)
{
    Lock l(m_Mutex);
    // Rotate in the other direction because we're the camera!
    m_viewMatrix = AffineMatrix::rotationZ(-in_fRadians) * m_viewMatrix;
    FTSMSGDBG("View matrix:\n{1}", 4, m_viewMatrix.to_s());

    return *this;
}

/// Rotates the camera around the GLOBAL x axis.
/** Imagine the GLOBAL x axis beginning at the camera's position.
 *  This function makes the camera rotate around this axis.
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotateGlobalX(float in_fRadians)
{
    return this->rotateGlobal(Vector(1.0f, 0.0f, 0.0f), in_fRadians);
}

/// Rotates the camera around the GLOBAL y axis.
/** Imagine the GLOBAL y axis beginning at the camera's position.
 *  This function makes the camera rotate around this axis.
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotateGlobalY(float in_fRadians)
{
    return this->rotateGlobal(Vector(0.0f, 1.0f, 0.0f), in_fRadians);
}

/// Rotates the camera around the GLOBAL z axis.
/** Imagine the GLOBAL z axis beginning at the camera's position.
 *  This function makes the camera rotate around this axis.
 *
 * \param in_fRadians The amount of \e radians to rotate.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::rotateGlobalZ(float in_fRadians)
{
    return this->rotateGlobal(Vector(0.0f, 0.0f, 1.0f), in_fRadians);
}

/// Orbits the camera around the \e global X axis moved to some point.
/** This makes the camera orbit \a in_fRadians radians around the global X axis
 *  in the same manner as the earth orbits around the sun for example. That
 *  means that the camera's distance to the global X axis stays the same.\n
 *  In this example \a in_vPosition would be the position of the sun.
 *
 * \param in_fRadians The amount of \e radians to orbit.
 * \param in_vPosition The position around which to orbit.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::orbitGlobalX(const Vector& in_vPosition, float in_fRadians)
{
    return this->orbit(in_vPosition, Vector(1.0f, 0.0f, 0.0f), in_fRadians);
}

/// Orbits the camera around the \e global Y axis moved somewhere.
/** This makes the camera orbit \a in_fRadians radians around the global Y axis
 *  in the same manner as the earth orbits around the sun for example. That
 *  means that the camera's distance to the global Y axis stays the same.\n
 *  In this example \a in_vPosition would be the position of the sun.
 *
 * \param in_fRadians The amount of \e radians to orbit.
 * \param in_vPosition The position around which to orbit.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::orbitGlobalY(const Vector& in_vPosition, float in_fRadians)
{
    return this->orbit(in_vPosition, Vector(0.0f, 1.0f, 0.0f), in_fRadians);
}

/// Orbits the camera around the \e global Z axis.
/** This makes the camera orbit \a in_fRadians radians around the global Z axis
 *  in the same manner as the earth orbits around the sun for example. That
 *  means that the camera's distance to the global Z axis stays the same.\n
 *  In this example \a in_vPosition would be the position of the sun.
 *
 * \param in_fRadians The amount of \e radians to orbit.
 * \param in_vPosition The position around which to orbit.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::orbitGlobalZ(const Vector& in_vPosition, float in_fRadians)
{
    return this->orbit(in_vPosition, Vector(0.0f, 0.0f, 1.0f), in_fRadians);
}

/// Use a 2D orthographic projection for this matrix's projection.
/** This makes the camera use a 2D orthographic projection. This places the
 *  origin at the top left of the screen, positive X going to the right,
 *  positive Y going down. That's what we'd expect for normal 2D drawing. The
 *  camera's frustrum is a cuboid.
 *
 * \note This creates a default (ortho) 2D projection for the current viewport
 *       settings.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::ortho2DProjection()
{
    Lock l(m_Mutex);
    m_projectionMatrix = AffineMatrix::ortho2DProjection(m_fW, m_fH);

    return *this;
}

/// Use a prespective projection for this matrix's projection.
/** This makes the camera use a perspective projection, that's the most
 *  realistic looking one. It makes the camera frustrum have a trapezoidal form.
 *
 * \param in_fFoV The Field of View, in degrees.
 * \param in_fNear The straight distance from the camera to the near plane.
 * \param in_fFar The straight distance from the camera to the far plane.
 *
 * \note Learn to love your Z-Buffer: http://wiki.arkana-fts.org/doku.php?id=misc:zbuf
 * \note If \a in_fFoV is too close to 0, 90 or 180 it will be set to 45.
 * \note For the aspect ratio, it uses the current viewport settings.
 *
 * \return A reference to myself to allow chaining.
 *
 * \author Pompei2
 */
Camera& Camera::perspectiveProjection(float in_fFoV,
                                      float in_fNearPlane, float in_fFarPlane)
{
    Lock l(m_Mutex);
    m_projectionMatrix = General4x4Matrix::perspectiveProjection(in_fFoV, m_fW/m_fH, in_fNearPlane, in_fFarPlane);

    return *this;
}

/// Sets up OpenGL to view the scene through this camera.
/** Loads the camera's view and projection matrices into OpenGL. It also sets-up
 *  opengl's viewport.
 *
 * \return A reference to myself to allow chaining.
 *
 * \todo For now, this is OK. Later we'll need to upload it to shaders.
 * \author Pompei2
 */
const Camera& Camera::use() const
{
    verifGL("Camera::use start");
    Lock l(m_Mutex);
    glViewport(0, 0, (GLsizei)m_fW, (GLsizei)m_fH);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMatrix.array16f());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m_viewMatrix.array16f());

    verifGL("Camera::use end");
    return *this;
}

const AffineMatrix& Camera::getViewMatrix() const
{
    return m_viewMatrix;
}

const General4x4Matrix& Camera::getProjectionMatrix() const
{
    return m_projectionMatrix;
}

General4x4Matrix Camera::getViewProjectionMatrix() const
{
    return m_projectionMatrix * m_viewMatrix;
}

Camera *CameraCmdBase::getCam()
{
    if(m_pCam != nullptr) {
        return m_pCam;
    }

    return &RunlevelManager::getSingleton().getCurrRunlevel()->getActiveCamera();
}

bool CameraPosCmd::exec()
{
    if(this->getCam() == nullptr)
        return false;

    this->getCam()->position(m_pos);
    return true;
}

bool CameraLookAtCmd::exec()
{
    if(this->getCam() == nullptr)
        return false;

    this->getCam()->lookAt(m_tgt);
    return true;
}

bool CameraCmd::exec()
{
    if(this->getCam() == nullptr)
        return false;

    switch(m_act) {
    case D_CAMERA_ACTION::moveRight: this->getCam()->moveRight(m_fAmount); break;
    case D_CAMERA_ACTION::moveUp: this->getCam()->moveUp(m_fAmount); break;
    case D_CAMERA_ACTION::moveFront: this->getCam()->moveFront(m_fAmount); break;
    case D_CAMERA_ACTION::moveGlobalX: this->getCam()->moveGlobalX(m_fAmount); break;
    case D_CAMERA_ACTION::moveGlobalY: this->getCam()->moveGlobalY(m_fAmount); break;
    case D_CAMERA_ACTION::moveGlobalZ: this->getCam()->moveGlobalZ(m_fAmount); break;
    case D_CAMERA_ACTION::moveFrontParralelToGlobalXY: this->getCam()->moveFrontParralelToGlobalXY(m_fAmount); break;
    case D_CAMERA_ACTION::moveFrontParralelToGlobalXZ: this->getCam()->moveFrontParralelToGlobalXZ(m_fAmount); break;
    case D_CAMERA_ACTION::moveFrontParralelToGlobalYZ: this->getCam()->moveFrontParralelToGlobalYZ(m_fAmount); break;
    case D_CAMERA_ACTION::rotateX: this->getCam()->rotateX(m_fAmount); break;
    case D_CAMERA_ACTION::rotateY: this->getCam()->rotateY(m_fAmount); break;
    case D_CAMERA_ACTION::rotateZ: this->getCam()->rotateZ(m_fAmount); break;
    case D_CAMERA_ACTION::rotateGlobalX: this->getCam()->rotateGlobalX(m_fAmount); break;
    case D_CAMERA_ACTION::rotateGlobalY: this->getCam()->rotateGlobalY(m_fAmount); break;
    case D_CAMERA_ACTION::rotateGlobalZ: this->getCam()->rotateGlobalZ(m_fAmount); break;
    }
    return true;
}
