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

#include <memory>
#include <vector>
#include <map>

class CalModel;
class CalCoreModel;
class CalHardwareModel;

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
    void render(const AffineMatrix& in_modelMatrix, const Color& in_playerCol, const CalModel& in_model);

    int getOrCreateErrorMatId(const String& in_sModelName);

    void loadHardware(int in_nMaxTexturesPerMesh, int in_nMaxBonesPerMesh);
    void precomputeAABB(const std::vector<float>& in_vVertices);

private:
    /// The Cal3d core model.
    std::unique_ptr<CalCoreModel> m_pCoreModel;
    /// The Cal3d hardware representation of the model.
    std::unique_ptr<CalHardwareModel> m_pHardwareModel;

    /// VBO holding the vertex positions in the graphics card.
    std::unique_ptr<VertexBufferObject> m_pVertexVBO;
    /// VBO holding the vertex normals in the graphics card.
    std::unique_ptr<VertexBufferObject> m_pNormalVBO;
    /// VBO holding the texture coordinates in the graphics card.
    std::vector< std::unique_ptr<VertexBufferObject> > m_pTexCoVBOs;
    /// VBO holding the bone weights (fraction part) and indices (natural part) in the graphics card.
    std::unique_ptr<VertexBufferObject> m_pMatIdxAndWeightVBO;
    /// VBO holding the face's vertex indices in the graphics card.
    std::unique_ptr<ElementsBufferObject> m_pVtxIdxVBO;

    /// Vertex Array Object, storing which VBO belongs to which vertex attribute.
    VertexArrayObject m_vao;

    /// Names of all the textures <i>loaded</i> by this model.
    std::vector<String> m_loadedTexs;

    /// Names of all the shaders <i>loaded</i> by this model.
    std::vector<String> m_loadedShads;

    /// Maps the skin name to the Cal3d material set number.
    std::map<String, int> m_mSkins;

    /// The AABB of the rest-position.
    AxisAlignedBoundingBox m_restAABB;
};

}; // namespace FTS

#endif // D_HARDWARE_MODEL_H
