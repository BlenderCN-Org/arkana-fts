/**
 * \file ModelInstance.h
 * \author Pompei2
 * \date 10 August 2010
 * \brief This file contains the classes to manage a model instance. A model
 *        instance is an instance of a hardware model, that is the model
 *        somewhere in space, with its own position, orientation and animation
 *        state. It does not have vertex, normal, texture, ... data though, as
 *        those are stored in the hardware model.
 **/

#ifndef D_MODEL_INSTANCE_H
#define D_MODEL_INSTANCE_H

#include "main.h"

#include "main/Updateable.h"

#include "3d/Math.h"

#include <bouge/bougefwd.hpp>

#include <memory>
#include <set>
#include <map>

namespace FTS {
    class String;
    class Color;
    class ModelManager;
    class HardwareModel;
    class AxisAlignedBoundingBox;

class ModelInstance : protected Updateable {
public:
    virtual ~ModelInstance();

    void render(const Vector& in_pos, const Color& in_playerColor);
    void render(const AffineMatrix& in_modelMatrix, const Color& in_playerColor);

    // Handling of the skin.
    const std::set<String>& skins() const;
    void selectSkin(const String& in_sSkinName);

    // Handling of the animations
    bouge::AnimationPtr playAction(const String& in_sAnimName, float in_fSpeed = 1.0f, float in_fFadeIn = 0.3f, float in_fFadeOut = 0.3f);
    bouge::AnimationPtr playCycle(const String& in_sAnimName, float in_fSpeed = 1.0f, float in_fFadeIn = 0.3f, float in_fWeight = 1.0f);
    void stop(const String& in_sAnimName, float in_fFadeOut = 0.0f);
    void stopAll(float in_fFadeOut = 0.0f);
    void pause(const String& in_sAnimName);
    void resume(const String& in_sAnimName);
    bool paused(const String& in_sAnimName) const;

    void pause();
    void resume();
    bool paused() const;

    void speed(float in_fSpeed);
    float speed() const;

    void speed(const String& in_sAnimName, float in_fSpeed);
    float speed(const String& in_sAnimName) const;

    // Animation information
    const std::set<String>& moves() const;
    bouge::CoreAnimationPtrC getMove(const String& in_sName);

    // Some informations about the model.
    size_t vertexCount() const;
    size_t faceCount() const;
    AxisAlignedBoundingBox restAABB() const;

protected:
    friend class FTS::ModelManager;

    /// Only the ModelManager can create a ModelInstance.
    /// \param in_pCoreModel The core model to use for this model instance.
    ModelInstance(std::shared_ptr<FTS::HardwareModel> in_pHwModel);

    virtual bool update(const Clock&);

private:
    /// The bouge model instance.
    bouge::ModelInstancePtr m_pModel;

    /// The HardwareModel that I am an instance of.
    std::shared_ptr<FTS::HardwareModel> m_pHwModel;

    /// All the numbers that are already used as "names" for instances of a certain hw model.
    static std::map<String, std::set<unsigned int> > m_mUsedNames;

    /// And this is the number of this instance, relative to the hw model.
    unsigned int m_uiInstanceNumber;
};

}; // namespace FTS

#endif // D_MODEL_INSTANCE_H
