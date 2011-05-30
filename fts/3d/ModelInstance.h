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

#include <memory>
#include <vector>
#include <set>
#include <map>
#include <list>

class CalCoreModel;
class CalModel;

namespace FTS {
    class String;
    class Vector;
    class Color;
    class ModelManager;
    class HardwareModel;
    class AxisAlignedBoundingBox;
    class AffineMatrix;

class ModelInstance : protected Updateable {
public:
    virtual ~ModelInstance();

    void render(const Vector& in_pos, const Color& in_playerColor);
    void render(const AffineMatrix& in_modelMatrix, const Color& in_playerColor);

    // Handling of the skin.
    std::vector<String> getAvailableSkins() const;
    void selectSkin(const String& in_sSkinName);

    // Handling of the animations
    void playAsAction(const String& in_sAnimName, float in_fSpeed = 1.0f, float in_fFadeIn = 0.3f, float in_fFadeOut = 0.3f);
    void playAsCycle(const String& in_sAnimName, float in_fWeight = 1.0f, float in_fFadeIn = 0.3f);
    void stopAction(const String& in_sAnimName, float in_fFadeOut = 0.0f);
    void stopCycle(const String& in_sAnimName, float in_fFadeOut = 0.0f);
    void stopAll(float in_fFadeOut = 0.0f);

    void pause();
    void resume();
    void pauseAction(const String& in_sAnimName);
    void resumeAction(const String& in_sAnimName);

    void setCycleSpeed(float in_fSpeed);
    float getCycleSpeed() const;

    // Animation information
    std::vector<String> getAvailableMoves() const;
    bool isPaused() const;
    bool isActionPaused(const String& in_sAnimName) const;

    // Some informations about the model.
    uint32_t getVertexCount() const;
    uint32_t getFaceCount() const;
    AxisAlignedBoundingBox getRestAABB() const;

protected:
    friend class FTS::ModelManager;

    /// Only the ModelManager can create a ModelInstance.
    /// \param in_pCoreModel The core model to use for this model instance.
    ModelInstance(std::shared_ptr<FTS::HardwareModel> in_pHwModel);

    virtual bool update(const Clock&);

private:
    /// The Cal3d model instance.
    std::shared_ptr<CalModel> m_pModel;

    /// The HardwareModel that I am an instance of.
    std::shared_ptr<FTS::HardwareModel> m_pHwModel;

    /// All the numbers that are already used as "names" for instances of a certain hw model.
    static std::map<String, std::set<unsigned int> > m_mUsedNames;

    /// And this is the number of this instance, relative to the hw model.
    unsigned int m_uiInstanceNumber;

    /// The animation speeds of all currently playing animations that are paused.
    std::list<float> m_lPausedTimeFactors;

    /// The animation speeds of all currently playing actions that are paused.
    std::map<String, float> m_lPausedActionTimeFactors;
};

}; // namespace FTS

#endif // D_MODEL_INSTANCE_H
