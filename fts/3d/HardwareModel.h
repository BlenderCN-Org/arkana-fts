/**
 * \file HardwareModel.h
 * \author Pompei2
 * \date 22 June 2010
 * \brief This file contains the classes to manage hardware models. A hardware
 *        model is only the resources of a model that can be shared between
 *        various model instances.
 **/

#ifndef D_HARDWARE_MODEL_H
#define D_HARDWARE_MODEL_H

#include "3d/math/AxisAlignedBoundingBox.h"
#include "3d/VertexArrayObject.h"
#include "utilities/NonCopyable.h"
#include "dLib/dString/dString.h"

#include "bouge/bougefwd.hpp"

#include <memory>
#include <vector>
#include <map>

namespace FTS {
    class Color;
    class Archive;
    class Vector;
    class ModelInstance;
    class ModelManager;
    class Camera;

/// \TODO: multiple core materials with different shaders!
class HardwareModel : public NonCopyable {
public:
    String getName() const;

    std::vector<String> getSkinList() const;
    int getSkinId(const String& in_sSkinName) const;

    std::vector<String> getAnimList() const;

    uint32_t getVertexCount() const;
    uint32_t getFaceCount() const;
    AxisAlignedBoundingBox getRestAABB() const;

    bool isStatic() const;

    /// Default destructor.
    virtual ~HardwareModel();

protected:
    friend class FTS::ModelInstance;
    friend class FTS::ModelManager;

    /// Constructs the error model - a simple box with only vertices.
    /// \param in_sName The name the model should have - can be arbitrary.
    HardwareModel(const String& in_sName);

    /// Constructs a model from an archive of Cal3d files and others.
    /// \param in_sName The name the model should have - can be arbitrary.
    /// \param in_modelArch The archive containing the files to load the model.
    /// \throws CorruptDataException If some file in the archive is faulty.
    HardwareModel(const String& in_sName, Archive& in_modelArch);

    /// Renders this hardware model somewhere, somehow :) Only the ModelInstance
    /// should take care of this.
    /// \param in_modelMatrix The model-matrix to use for render (pos, rot, scale)
    /// \param in_playerCol The player-color to use for this model.
    /// \param in_model The model holding information about, for example, the pose.
    void render(const AffineMatrix& in_modelMatrix, const Color& in_playerCol, bouge::ModelInstancePtrC in_model);

    void createHardwareMesh();
    void setupVAO(struct MaterialUserData& in_ud) const;

    void unloadResources() const;
//     int getOrCreateErrorMatId(const String& in_sModelName);

//     void loadHardware(int in_nMaxTexturesPerMesh, int in_nMaxBonesPerMesh);
//     void precomputeAABB(const std::vector<float>& in_vVertices);

private:
    bouge::CoreModelPtr m_pCoreModel;
    bouge::CoreHardwareMeshPtr m_pHardwareModel;

    /// VBO holding all the vertex data in the graphics card.
    std::unique_ptr<VertexBufferObject> m_vbo;

    /// VBO holding the face's vertex indices in the graphics card.
    std::unique_ptr<ElementsBufferObject> m_pVtxIdxVBO;

    /// Names of all the textures <i>loaded</i> by this model.
    std::vector<String> m_loadedTexs;

    /// Names of all the custom shader names used in a custom program <i>loaded</i> by this model.
    std::vector<String> m_loadedProgs;

    /// Maps the skin name to the Cal3d material set number.
    std::map<String, int> m_mSkins;

    /// The AABB of the rest-position.
    AxisAlignedBoundingBox m_restAABB;
};

}; // namespace FTS

#endif // D_HARDWARE_MODEL_H
