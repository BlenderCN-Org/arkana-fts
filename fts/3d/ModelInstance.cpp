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
#include "3d/Math.h"
#include "graphic/Color.h"
#include "logging/logger.h"
#include "main/Clock.h"

#include <bouge/bouge.hpp>

// TODO: when creating a model using this core model:
    // Start the idle animation if there is any?

std::map<FTS::String, std::set<unsigned int> > FTS::ModelInstance::m_mUsedNames;

FTS::ModelInstance::ModelInstance(std::shared_ptr<FTS::HardwareModel> in_pHwModel)
    : m_pModel(new bouge::ModelInstance(in_pHwModel->m_pCoreModel))
    , m_pHwModel(in_pHwModel)
{
    // And we need to select the default skin...
    this->selectSkin("Default");

    // If I have at least one animation, register me as an updateable.
    // Without animation, we don't need to get updated!
    if(m_pHwModel->anims().size() > 0) {
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
    // If I have at least one animation, I've been registered as an updateable.
    // Unregister me now!
    if(m_pHwModel->anims().size() > 0) {
        UpdateableManager::getSingleton().rem(m_pHwModel->getName() + String::nr(m_uiInstanceNumber));
        m_mUsedNames[m_pHwModel->getName()].erase(m_uiInstanceNumber);
    }
}

void FTS::ModelInstance::render(const FTS::Vector& in_pos, const Color& in_playerColor)
{
    m_pHwModel->render(AffineMatrix::translation(in_pos), in_playerColor, m_pModel);
}

void FTS::ModelInstance::render(const FTS::AffineMatrix& in_modelMatrix, const Color& in_playerColor)
{
    m_pHwModel->render(in_modelMatrix, in_playerColor, m_pModel);
}

const std::set<FTS::String>& FTS::ModelInstance::skins() const
{
    return m_pHwModel->skins();
}

void FTS::ModelInstance::selectSkin(const FTS::String& in_sSkinName)
{
    // getSkinId defaults to 0 if there is no skin with such a name.
    // This is good as it will then select the "default" skin instead of crash.
    m_pModel->selectMatSet(in_sSkinName.str());
}

const std::set<FTS::String>& FTS::ModelInstance::moves() const
{
    return m_pHwModel->anims();
}

bouge::CoreAnimationPtrC FTS::ModelInstance::getMove(const String& in_sName)
{
    return m_pModel->findAnimToUse(in_sName.str());
}

bouge::AnimationPtr FTS::ModelInstance::playAction(const String& in_sAnimName, float in_fSpeed, float in_fFadeIn, float in_fFadeOut)
{
    return m_pModel->playOneShot(in_sAnimName.str(), in_fSpeed, in_fFadeIn, in_fFadeOut);
}

bouge::AnimationPtr FTS::ModelInstance::playCycle(const String& in_sAnimName, float in_fSpeed, float in_fFadeIn, float in_fWeight)
{
    return m_pModel->playCycle(in_sAnimName.str(), in_fSpeed, in_fFadeIn, in_fWeight);
}

void FTS::ModelInstance::stop(const String& in_sAnimName, float in_fFadeOut)
{
    m_pModel->stop(in_sAnimName.str(), in_fFadeOut);
}

void FTS::ModelInstance::stopAll(float in_fFadeOut)
{
    m_pModel->stopAll(in_fFadeOut);
}

void FTS::ModelInstance::pause()
{
    m_pModel->pauseAll();
}

void FTS::ModelInstance::resume()
{
    m_pModel->resumeAll();
}

bool FTS::ModelInstance::paused() const
{
    return m_pModel->paused();
}

void FTS::ModelInstance::pause(const String& in_sAnimName)
{
    m_pModel->pause(in_sAnimName.str());
}

void FTS::ModelInstance::resume(const String& in_sAnimName)
{
    m_pModel->resume(in_sAnimName.str());
}

bool FTS::ModelInstance::paused(const String& in_sAnimName) const
{
    return m_pModel->paused(in_sAnimName.str());
}

void FTS::ModelInstance::speed(float in_fSpeed)
{
    m_pModel->speed(in_fSpeed);
}

float FTS::ModelInstance::speed() const
{
    return m_pModel->speed();
}

void FTS::ModelInstance::speed(const FTS::String& in_sAnimName, float in_fSpeed)
{
    m_pModel->speed(in_sAnimName.str(), in_fSpeed);
}

float FTS::ModelInstance::speed(const FTS::String& in_sAnimName) const
{
    return m_pModel->speed(in_sAnimName.str());
}

size_t FTS::ModelInstance::vertexCount() const
{
    return m_pHwModel->vertexCount();
}

size_t FTS::ModelInstance::faceCount() const
{
    return m_pHwModel->faceCount();
}

FTS::AxisAlignedBoundingBox FTS::ModelInstance::restAABB() const
{
    return m_pHwModel->restAABB();
}

bool FTS::ModelInstance::update(const Clock& in_c)
{
    if(!m_pHwModel->isStatic()) {
        m_pModel->mixer()->update(static_cast<float>(in_c.getDeltaT()));
    }
    return true;
}
