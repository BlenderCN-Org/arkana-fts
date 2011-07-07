/**
 * \file camera.h
 * \author Pompei2
 * \date 15 July 2007
 * \brief This file contains the camera class definition.
 **/

#ifndef FTS_CAMERA_H
#define FTS_CAMERA_H

#include "main.h"

#include "3d/Math.h"
#include "utilities/threading.h"
#include "utilities/command.h"

namespace FTS {

namespace CameraDrawMode {
    enum Enum {
        Dont,
        Axes,
    };
}

/// This class represents a camera in Arkana-FTS.
/// One important thing to note is that all functions taking angles as parameters
/// do take radians. Also one should note that positive angles mean positive
/// rotation in the sense of a right-handed coordinate system.
class Camera {
public:
    Camera(float in_fViewportW = -1.0f, float in_fViewportH = -1.0f,
           const CameraDrawMode::Enum& in_mode = CameraDrawMode::Dont);
    virtual ~Camera();

    Vector getPos() const;
    Vector getFront() const;
    Vector getRight() const;
    Vector getUp() const;

    Camera& resetOrientation();
    Camera& lookAt(const Vector & in_vTgt);
    Camera& position(const Vector & in_vPos);

    Camera& moveRight(float in_fAmount);
    Camera& moveUp(float in_fAmount);
    Camera& moveFront(float in_fAmount);

    Camera& moveGlobalX(float in_fAmount);
    Camera& moveGlobalY(float in_fAmount);
    Camera& moveGlobalZ(float in_fAmount);

    Camera& moveFrontParralelToGlobalXY(float in_fAmount);
    Camera& moveFrontParralelToGlobalXZ(float in_fAmount);
    Camera& moveFrontParralelToGlobalYZ(float in_fAmount);

    /// \todo
    Camera& orbit(const Vector& position, const Vector& axis, float in_fRadians);
    Camera& rotate(const Vector& axis, float in_fRadians);
    Camera& rotateGlobal(const Vector& axis, float in_fRadians);

    Camera& rotateX(float in_fRadians);
    Camera& rotateY(float in_fRadians);
    Camera& rotateZ(float in_fRadians);

    Camera& rotateGlobalX(float in_fRadians);
    Camera& rotateGlobalY(float in_fRadians);
    Camera& rotateGlobalZ(float in_fRadians);

    /// \todo
    Camera& orbitGlobalX(const Vector& position, float in_fRadians);
    /// \todo
    Camera& orbitGlobalY(const Vector& position, float in_fRadians);
    /// \todo
    Camera& orbitGlobalZ(const Vector& position, float in_fRadians);

    Camera& ortho2DProjection();
    Camera& perspectiveProjection(float in_fFoV,
                                  float in_fNearPlane = 2.5f, float in_fFarPlane = 1000.0f);

    Camera& reset(float in_fNewW, float in_fNewH);

    const Camera& use() const;

    const AffineMatrix& getViewMatrix() const;
    const General4x4Matrix& getProjectionMatrix() const;
    General4x4Matrix getViewProjectionMatrix() const;

    /// \todo inherit from world object and render!
//     Camera& render() const;

private:
    /// This object contains the View transformation matrix and its 3x3 inverse.
    /// For example if your camera resides at the point x,y,z, this matrix in
    /// fact moves all objects by -x,-y,-z in order to have the effect of a
    /// camera. The same holds for rotations: it rotates "the other way around".
    AffineMatrix m_viewMatrix;

    /// This matrix holds the projection mode of the camera.
    General4x4Matrix m_projectionMatrix;

    /// The mutex to protect myself.
    mutable Mutex m_Mutex;

    CameraDrawMode::Enum m_mode;

    /// The width of this camera's viewport.
    float m_fW;
    /// The height of this camera's viewport.
    float m_fH;
};

class CameraCmdBase : public CommandBase {
private:
    Camera *m_pCam;
protected:
    CameraCmdBase(Camera *in_pCam) : m_pCam(in_pCam) {};
public:
    virtual ~CameraCmdBase() {};
    virtual bool exec() = 0;
    Camera *getCam();
};

class CameraPosCmd : public CameraCmdBase {
private:
    Vector m_pos;
public:
    CameraPosCmd(const Vector &in_pos)
        : CameraCmdBase(NULL),
          m_pos(in_pos) {};
    CameraPosCmd(Camera *in_pCam, const Vector &in_pos)
        : CameraCmdBase(in_pCam),
          m_pos(in_pos) {};
    virtual ~CameraPosCmd() {};
    bool exec();
};

class CameraLookAtCmd : public CameraCmdBase {
private:
    Vector m_tgt;
public:
    CameraLookAtCmd(const Vector &in_tgt)
        : CameraCmdBase(NULL),
          m_tgt(in_tgt) {};
    CameraLookAtCmd(Camera *in_pCam, const Vector &in_tgt)
        : CameraCmdBase(in_pCam),
          m_tgt(in_tgt) {};
    virtual ~CameraLookAtCmd() {};
    bool exec();
};

// Some command classes to control the camera.
class CameraCmd : public CameraCmdBase {
public:
    typedef enum {
        moveRight = 1,
        moveUp,
        moveFront,
        moveGlobalX,
        moveGlobalY,
        moveGlobalZ,
        moveFrontParralelToGlobalXY,
        moveFrontParralelToGlobalXZ,
        moveFrontParralelToGlobalYZ,
        rotateX,
        rotateY,
        rotateZ,
        rotateGlobalX,
        rotateGlobalY,
        rotateGlobalZ,
    } D_CAMERA_ACTION;

    CameraCmd(D_CAMERA_ACTION in_act, float in_fAmount)
        : CameraCmdBase(NULL),
          m_fAmount(in_fAmount),
          m_act(in_act) {};
    CameraCmd(Camera *in_pCam, D_CAMERA_ACTION in_act, float in_fAmount)
        : CameraCmdBase(in_pCam),
          m_fAmount(in_fAmount),
          m_act(in_act) {};
    virtual ~CameraCmd() {};
    bool exec();

protected:
    float m_fAmount;
    D_CAMERA_ACTION m_act;
};

} // namespace FTS

#endif /* FTS_CAMERA_H */

 /* EOF */