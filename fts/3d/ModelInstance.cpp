/**
 * \file ModelInstance.cpp
 * \author Pompei2
 * \date 10 August 2010
 * \brief This file contains the classes to manage a model instance. A model
 *        instance is an instance of a hardware model, that is the model
 *        somewhere in space, with its own position, orientation and animation
 *        state. It does not have vertex, normal, texture, ... data though, as
 *        those are stored in the hardware model.
 **/

#include "ModelInstance.h"
#include "ModelManager.h"
#include "HardwareModel.h"

#include "3d/3d.h"
#include "3d/math/Matrix.h"
#include "graphic/Color.h"
#include "logging/logger.h"
#include "main/Clock.h"
#include "utilities/Math.h"

#include "cal3d/cal3d.h"

#include <limits>

// TODO: when creating a model using this core model:
    // Start the idle animation if there is any?
    // currently not working: speed of cycle.

std::map<FTS::String, std::set<unsigned int> > FTS::ModelInstance::m_mUsedNames;

FTS::ModelInstance::ModelInstance(std::shared_ptr<FTS::HardwareModel> in_pHwModel)
    : m_pModel(new CalModel(in_pHwModel->m_pCoreModel.get()))
    , m_pHwModel(in_pHwModel)
{
    // We need to give the model all the meshes...
    for(int iMesh = 0; iMesh < in_pHwModel->m_pCoreModel->getCoreMeshCount(); ++iMesh) {
        m_pModel->attachMesh(iMesh);
    }

    // We calculate every vertex position, normal, etc. in the shader, thus
    // we don't want Cal3D to calculate it internally.
    m_pModel->disableInternalData();

    // And we need to select the default skin...
    this->selectSkin("Default");

    // If I have at least one animation, register me as an updateable.
    // Without animation, we don't need to get updated!
    if(m_pHwModel->getAnimList().size() > 0) {
        // Find an unique name for it:
        // NOTE: If this gets a bottleneck, we might create an addUnnamed
        //       method to the UpdateableManager...
        std::set<unsigned int>& numbers = m_mUsedNames[m_pHwModel->getName()];
        for(unsigned int i = 0 ; i < std::numeric_limits<unsigned int>::max() ; ++i) {
            if(numbers.find(i) == numbers.end()) {
                // Yup, this number is still free!
                UpdateableManager::getSingleton().add(m_pHwModel->getName() + String::nr(i), this);
                numbers.insert(i);
                m_uiInstanceNumber = i;
                break;
            }
        }
    }
}

FTS::ModelInstance::~ModelInstance()
{
    if(m_pHwModel->getAnimList().size() > 0) {
        UpdateableManager::getSingleton().rem(m_pHwModel->getName() + String::nr(m_uiInstanceNumber));
        m_mUsedNames[m_pHwModel->getName()].erase(m_uiInstanceNumber);
    }
}

void FTS::ModelInstance::render(const FTS::Vector& in_pos, const Color& in_playerColor)
{
    m_pHwModel->render(AffineMatrix::translation(in_pos), in_playerColor, *m_pModel);
}

void FTS::ModelInstance::render(const FTS::AffineMatrix& in_modelMatrix, const Color& in_playerColor)
{
    m_pHwModel->render(in_modelMatrix, in_playerColor, *m_pModel);
}

std::vector<FTS::String> FTS::ModelInstance::getAvailableSkins() const
{
    return m_pHwModel->getSkinList();
}

void FTS::ModelInstance::selectSkin(const FTS::String& in_sSkinName)
{
    // getSkinId defaults to 0 if there is no skin with such a name.
    // This is good as it will then select the "default" skin instead of crash.
    m_pModel->setMaterialSet(m_pHwModel->getSkinId(in_sSkinName));
}

std::vector<FTS::String> FTS::ModelInstance::getAvailableMoves() const
{
    return m_pHwModel->getAnimList();
}

void FTS::ModelInstance::playAsAction(const String& in_sAnimName, float in_fSpeed, float in_fFadeIn, float in_fFadeOut)
{
    int id = m_pModel->getCoreModel()->getCoreAnimationId(in_sAnimName.str());
    if(id < 0) {
        FTS18N("TODO: PlayAsAction with invalid action warn message", MsgType::WarningNoMB);
        return;
    }

    // Play this action!
    m_pModel->getMixer()->executeAction(id, in_fFadeIn, in_fFadeOut);

    // Set the speed with which this animation shall be played.
    if(m_pModel->getMixer()->getAnimationActionList().size() > 0) {
        m_pModel->getMixer()->getAnimationActionList().front()->setTimeFactor(in_fSpeed);
    }
    m_pModel->update(0.0);
}

void FTS::ModelInstance::playAsCycle(const String& in_sAnimName, float in_fWeight, float in_fFadeIn)
{
    int id = m_pModel->getCoreModel()->getCoreAnimationId(in_sAnimName.str());
    if(id < 0) {
        FTS18N("TODO: PlayAsCycle with invalid action warn message", MsgType::WarningNoMB);
        return;
    }

    // Fade out (all) the current cycle(s) if any before starting the new one
    // in case we want the new one to be the sole cycle.
    if(FTS::nearZero(in_fWeight - 1.0f)) {
//         for(auto iCycleId = m_currentCycles.begin() ; i != m_currentCycles.end() ; ++i) {
//             m_pModel->getMixer()->clearCycle(iCycleId, in_fFadeIn);
//         }
    }

    // Now, start fading in the cycle.
    m_pModel->getMixer()->blendCycle(id, in_fWeight, in_fFadeIn);
    m_pModel->update(0.0);
}

void FTS::ModelInstance::stopAction(const String& in_sAnimName, float in_fFadeOut)
{
    int id = m_pModel->getCoreModel()->getCoreAnimationId(in_sAnimName.str());
    if(id < 0) {
        FTS18N("TODO: stopAction with invalid action warn message", MsgType::WarningNoMB);
        return;
    }

    // No fade out, stop it immediately.
    if(nearZero(in_fFadeOut)) {
        m_pModel->getMixer()->removeAction(id);
        return;
    }

    // Fake the fade-out by adjusting the time factor.
    CalAnimationAction* pAction = m_pModel->getMixer()->animationActionFromCoreAnimationId(id);
    if(pAction) {
        float fTimeLeft = std::max(pAction->getCoreAnimation()->getDuration() - pAction->getTime(), 0.001f);
        pAction->setDelayOut(fTimeLeft);
        pAction->setTimeFactor(fTimeLeft / in_fFadeOut);
        FTSMSG("Time left: {1}\nFade out: {2}\nNew time factor: {3}", MsgType::Raw, String::nr(fTimeLeft), String::nr(in_fFadeOut), String::nr(in_fFadeOut / fTimeLeft));
    }
}

void FTS::ModelInstance::stopCycle(const String& in_sAnimName, float in_fFadeOut)
{
    int id = m_pModel->getCoreModel()->getCoreAnimationId(in_sAnimName.str());
    if(id < 0) {
        FTS18N("TODO: stopCycle with invalid action warn message", MsgType::WarningNoMB);
        return;
    }

    m_pModel->getMixer()->clearCycle(id, in_fFadeOut);
}

void FTS::ModelInstance::stopAll(float in_fFadeOut)
{
    // First, stop all the cycles.
    std::vector<CalAnimation *> &cycles = m_pModel->getMixer()->getAnimationVector();
    for(std::size_t i = 0 ; i < cycles.size() ; ++i) {
        m_pModel->getMixer()->clearCycle(i, in_fFadeOut);
    }

    // Then, stop all the actions.
    for(int i = 0 ; i < m_pModel->getCoreModel()->getCoreAnimationCount() ; ++i) {
        this->stopAction(m_pModel->getCoreModel()->getCoreAnimation(i)->getName(), in_fFadeOut);
    }
}

void FTS::ModelInstance::pause()
{
    // Don't pause a paused model!
    if(this->isPaused())
        return;

    m_lPausedTimeFactors.push_back(m_pModel->getMixer()->getTimeFactor());
    m_pModel->getMixer()->setTimeFactor(0.0f);

    // Unfortunately, we have to pause the actions separately, they do not
    // consider the mixer's time factor :(
    std::list<CalAnimationAction*>& actions = m_pModel->getMixer()->getAnimationActionList();
    for(auto action = actions.begin() ; action != actions.end() ; ++action) {
        CalAnimationAction* pAction = *action;
        m_lPausedTimeFactors.push_back(pAction->getTimeFactor());
        pAction->setTimeFactor(0.0f);
    }
}

void FTS::ModelInstance::resume()
{
    if(!m_lPausedTimeFactors.empty()) {
        m_pModel->getMixer()->setTimeFactor(m_lPausedTimeFactors.front());
        m_lPausedTimeFactors.pop_front();
    } else {
        m_pModel->getMixer()->setTimeFactor(1.0f);
    }

    // Same as for the pausing...
    std::list<CalAnimationAction*>& actions = m_pModel->getMixer()->getAnimationActionList();
    for(auto action = actions.begin() ; action != actions.end() ; ++action) {
        CalAnimationAction* pAction = *action;
        if(!m_lPausedTimeFactors.empty()) {
            pAction->setTimeFactor(m_lPausedTimeFactors.front());
            m_lPausedTimeFactors.pop_front();
        } else {
            pAction->setTimeFactor(1.0f);
        }
    }

    m_lPausedTimeFactors.clear();
}

bool FTS::ModelInstance::isPaused() const
{
    return !m_lPausedTimeFactors.empty();
}

void FTS::ModelInstance::pauseAction(const String& in_sAnimName)
{
    // Don't pause a paused action!
    if(this->isActionPaused(in_sAnimName))
        return;

    // Go find that action...
    std::list<CalAnimationAction*>& actions = m_pModel->getMixer()->getAnimationActionList();
    for(auto action = actions.begin() ; action != actions.end() ; ++action) {
        CalAnimationAction* pAction = *action;
        if(pAction->getCoreAnimation()->getName() != in_sAnimName)
            continue;

        // Found it, pause it! (and keep in mind its current speed.)
        m_lPausedActionTimeFactors[in_sAnimName] = pAction->getTimeFactor();
        pAction->setTimeFactor(0.0f);
        break;
    }
}

void FTS::ModelInstance::resumeAction(const String& in_sAnimName)
{
    // Don't resume a running action!
    if(!this->isActionPaused(in_sAnimName))
        return;

    // Go find that action...
    std::list<CalAnimationAction*>& actions = m_pModel->getMixer()->getAnimationActionList();
    for(auto action = actions.begin() ; action != actions.end() ; ++action) {
        CalAnimationAction* pAction = *action;
        if(pAction->getCoreAnimation()->getName() != in_sAnimName)
            continue;

        // Found it, resume it!
        pAction->setTimeFactor(m_lPausedActionTimeFactors[in_sAnimName]);
        m_lPausedActionTimeFactors.erase(in_sAnimName);
        break;
    }
}

bool FTS::ModelInstance::isActionPaused(const String& in_sAnimName) const
{
    return m_lPausedActionTimeFactors.find(in_sAnimName) != m_lPausedActionTimeFactors.end();
}

void FTS::ModelInstance::setCycleSpeed(float in_fSpeed)
{
    // As far as I can tell, the default cal3d mixer only supports one
    // animation speed (time factor) for all cyclic animations together!
    m_pModel->getMixer()->setTimeFactor(in_fSpeed);
}

float FTS::ModelInstance::getCycleSpeed() const
{
    return m_pModel->getMixer()->getTimeFactor();
}

uint32_t FTS::ModelInstance::getVertexCount() const
{
    return m_pHwModel->getVertexCount();
}

uint32_t FTS::ModelInstance::getFaceCount() const
{
    return m_pHwModel->getFaceCount();
}

FTS::AxisAlignedBoundingBox FTS::ModelInstance::getRestAABB() const
{
    return m_pHwModel->getRestAABB();
}

bool FTS::ModelInstance::update(const Clock& in_c)
{
    m_pModel->update(static_cast<float>(in_c.getDeltaT()));
    return true;
}
