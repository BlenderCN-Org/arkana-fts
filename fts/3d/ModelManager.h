/**
 * \file ModelManager.h
 * \author Pompei2
 * \date 12 August 2010
 * \brief This file contains the ModelManager class.
 **/

#ifndef D_MODEL_MANAGER_H
#define D_MODEL_MANAGER_H

#include "dLib/dString/dString.h"

#include <map>
#include <memory>

namespace FTS {
class ModelInstance;
class HardwareModel;

class ModelManager {
public:
    ModelManager();
    virtual ~ModelManager();

    ModelInstance* createInstance(const String& in_sModelName);

    /// Adds a model. It actually loads the whole model into hardware
    /// such that it is ready to make some instances with nearly no delay.
    /// \param in_sModelName The name of the model to load.
    void addModel(const String& in_sModelName);

    /// Unloads any resources aquired by the model \a in_sName.
    /// \param in_sModelName The name of the model to remove.
    /// \note If there are still instances using the model, this is a no-op.
    void removeModel(const String &in_sModelName);

    /// Unloads any model that has been loaded, except the error model.
    void removeAllModels();

    /// The name of the error mode, that always works.
    const String ErrorModelName;

    static const unsigned int MaxBonesPerMesh = 10;

private:
    /// If a hardware model named \a in_sName exists in the system, it is
    /// returned. If not, it is first loaded from disk and then returned.
    ///
    /// \param in_sName The name of the hardware model to get or load. This is
    ///                 in fact the path from the models directory to the
    ///                 model file, without the filename extension.
    ///
    /// \return The corresponding hardware model. If it failed to load, the
    ///         error hardware model.
    std::shared_ptr<HardwareModel> getOrLoad(const String& in_sHwModelName);

    /// \return The hardware model used when there is an error loading one.
    std::shared_ptr<HardwareModel> getErrorModel();

    // Or maybe shared_ptr? THINK ABOUT IT.
    std::map<String, std::shared_ptr<HardwareModel> > m_mHardwareModels;
};

} // namespace FTS

#endif // D_MODEL_MANAGER_H
