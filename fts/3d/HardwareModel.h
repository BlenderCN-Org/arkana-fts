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
#include <set>

namespace FTS {
    class Color;
    class Archive;
    class ModelInstance;
    class ModelManager;
    class Camera;

/// \TODO: multiple core materials with different shaders!
class HardwareModel : public NonCopyable {
public:
    String getName() const;

    const std::set<String>& skins() const;
    const std::set<FTS::String>& anims() const;

    uint32_t vertexCount() const;
    uint32_t faceCount() const;
    AxisAlignedBoundingBox restAABB() const;

    bool isStatic() const;

    /// Default destructor.
    virtual ~HardwareModel();

protected:
    friend class FTS::ModelInstance;
    friend class FTS::ModelManager;

    /// Constructs the error model - a simple box with only vertices.
    /// \param in_sName The name the model should have - can be arbitrary.
    HardwareModel(const String& in_sName);

    /// Constructs a model from an archive of bouge files and others.
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

private:
    bouge::CoreModelPtr m_pCoreModel;
    bouge::CoreHardwareMeshPtr m_pHardwareModel;

    /// VBO holding all the vertex data in the graphics card.
    std::unique_ptr<VertexBufferObject> m_vbo;

    /// VBO holding the face's vertex indices in the graphics card.
    std::unique_ptr<ElementsBufferObject> m_pVtxIdxVBO;

    /// Names of all the textures <i>loaded</i> by this model.
    std::set<String> m_loadedTexs;

    /// Names of all the custom shader names used in a custom program <i>loaded</i> by this model.
    std::set<String> m_loadedProgs;

    /// The AABB of the rest-position.
    AxisAlignedBoundingBox m_restAABB;

    /// Just used for convenience, such that the skins and anims methods can
    /// return cons refs and thus be used in loops right away.
    std::set<String> m_anims;

    /// Just used for convenience, such that the skins and anims methods can
    /// return cons refs and thus be used in loops right away.
    std::set<String> m_skins;

    /// We can make some big optimizations for static meshes.
    bool m_isStatic;
};

}; // namespace FTS

#endif // D_HARDWARE_MODEL_H
