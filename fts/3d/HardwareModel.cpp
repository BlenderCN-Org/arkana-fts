/**
 * \file HardwareModel.cpp
 * \author Pompei2
 * \date 22 June 2010
 * \brief This file contains the classes to manage hardware models. A hardware
 *        model is only the resources of a model that can be shared between
 *        various model instances.
 **/

#include "HardwareModel.h"

#include "3d/3d.h"
#include "3d/Math.h"
#include "3d/Shader.h"
#include "3d/camera.h"
#include "graphic/graphic.h"
#include "graphic/Color.h"
#include "logging/logger.h"
#include "main/runlevels.h"

#include "dLib/dString/dString.h"
#include "dLib/dArchive/dArchive.h"
#include "dLib/dConf/configuration.h"

#include "bouge/bouge.hpp"
#include "bouge/IOModules/XML/Loader.hpp"
#include "bouge/IOModules/XMLParserCommon/XMLParserModules/TinyXMLParser.hpp"

#include <vector>
#include <algorithm>

extern const char* sErrorModelMesh;
extern const char* sErrorModelMaterial;
extern const char* sErrorModelMaterialSet;

namespace FTS {

static const unsigned int BONES_PER_MESH = 20;
static const String MODEL_FILENAME_SEP = "__";

struct MaterialUserData : public bouge::UserData {
    /// We can (and should) take a reference instead of a shared pointer here
    /// because the user data belongs to the material and with a shared pointer
    /// we'd have a cyclic reference here.
    const bouge::CoreMaterial& mat;

    /// The mode to use when calling glDrawElements.
    unsigned int drawMode;

    /// The shader to be used by this material.
    Program* prog;

    std::map<String, Vector> uniforms_f;
    std::map<String, Graphic*> uniforms_tex;

    /// A VAO storing the shader, shader's vertex attrib setup and vbo setup
    /// on the graphics card.
    VertexArrayObject vao;

    MaterialUserData(const bouge::CoreMaterial& in_mat, const bouge::CoreHardwareMesh& in_mesh, const String& in_sModelName, String& out_ShadernameToDestroy)
        : mat(in_mat)
        , drawMode(GL_TRIANGLES)
    {
        // Materials may specify to draw something other than triangles.
        // This is actually more of a hack than a well designed feature, but
        // it works for drawing lines and it might work for points too.
        if(this->mat.proprety("DrawMode") == "Lines")
            this->drawMode = GL_LINES;
        else if(this->mat.proprety("DrawMode") == "Points")
            this->drawMode = GL_POINTS;

        // Load shader to be used. This is a long story :) because we want to
        // allow a material to use default shaders, specify some shader
        // distributed with arkana-fts or specify a shader distributed as a
        // file inside the model archive.

        // But note that as soon as a non-default shader is specified, the
        // material has to specify all shader compile flags "by hand".

        static const String sDefVert = "Model.vert";
        static const String sDefFrag = "Model.frag";
        static const String sDefGeom = ShaderManager::DefaultGeometryShader;
        String sVertShader = in_mat.propretyOrDefault("VertexShader", sDefVert.str());
        String sFragShader = in_mat.propretyOrDefault("FragmentShader", sDefFrag.str());
        String sGeomShader = in_mat.propretyOrDefault("GeometryShader", sDefGeom.str());

        // Also note that if some custom/embedded shader is used, we want to
        // remember it such that the corresponding program can be destroyed
        // when the model is destroyed.
        out_ShadernameToDestroy = sVertShader != sDefVert ? sVertShader : (sFragShader != sDefFrag ? sFragShader : (sGeomShader != sDefGeom ? sGeomShader : String::EMPTY));
        bool hasCustomShader = !out_ShadernameToDestroy.empty();
        ShaderCompileFlags flags;

        // When using default shaders (this should be the norm), we have to
        // determine the shader compile flags automatically:
        if(!hasCustomShader) {
            if(in_mesh.boneIndicesPerVertex() > 0) {
                flags |= ShaderCompileFlag::SkeletalAnimated;
                flags |= ShaderCompileFlag("D_MAX_BONES_PER_MESH", String::nr(BONES_PER_MESH));
            }

            if(in_mat.hasProprety("uMaterialAmbient") && in_mat.hasProprety("uMaterialDiffuse") && in_mat.hasProprety("uMaterialSpecular")) {
                flags |= ShaderCompileFlag::Lit;
            }

            if(in_mat.hasProprety("uTexture")) {
                flags |= ShaderCompileFlag::Textured;
            }
        } else {
            // But if some custom shader is used, it has to specify ALL compile flags it needs.
            std::set<String> sFlags;
            String(in_mat.proprety("ShaderCompileFlags")).split(std::inserter(sFlags, sFlags.begin()));
            for(auto flag = sFlags.begin() ; flag != sFlags.end() ; ++flag) {
                flags |= *flag;
            }

            // Additionally, we want to use an embedded shader file over a default one.
            if(ShaderManager::getSingleton().hasShader(in_sModelName + MODEL_FILENAME_SEP + sVertShader)) {
                sVertShader = in_sModelName + MODEL_FILENAME_SEP + sVertShader;
            }
            if(ShaderManager::getSingleton().hasShader(in_sModelName + MODEL_FILENAME_SEP + sFragShader)) {
                sFragShader = in_sModelName + MODEL_FILENAME_SEP + sFragShader;
            }
            if(ShaderManager::getSingleton().hasShader(in_sModelName + MODEL_FILENAME_SEP + sGeomShader)) {
                sGeomShader = in_sModelName + MODEL_FILENAME_SEP + sGeomShader;
            }
        }

        this->prog = ShaderManager::getSingleton().getOrLinkProgram(sVertShader, sFragShader, sGeomShader, flags);

        // Now, we can preprocess all the uniforms that the shader needs.
        for(auto prop = in_mat.begin() ; prop != in_mat.end() ; ++prop) {
            try {
                GLenum type = this->prog->uniform(prop.name()).type;
                switch(type) {
                case GL_FLOAT:
                case GL_FLOAT_VEC2:
                case GL_FLOAT_VEC3:
                case GL_FLOAT_VEC4:
                    uniforms_f[prop.name()] = Vector(&prop.valueAsFvec()[0]);
                    break;
                case GL_SAMPLER_2D:
                    Graphic* pGraphic = nullptr;
                    // Check if we got the texture loaded from inside the model.
                    String sInModelName = in_sModelName + MODEL_FILENAME_SEP + prop.value();
                    if(GraphicManager::getSingleton().isGraphicPresent(sInModelName)) {
                        pGraphic = GraphicManager::getSingleton().getOrLoadGraphic(sInModelName);
                    } else {
                        // If not, try to load it from Arkana-FTS.
                        pGraphic = GraphicManager::getSingleton().getOrLoadGraphic(prop.value());
                    }
                    uniforms_tex[prop.name()] = pGraphic;
                    break;
                //TODO: more types, for example matrices, ints, ...
                }
            } catch(const NotExistException&) {
                // Nevermind if this property is not an uniform.
            }
        }

        // Setup shader attributes in a VAO.
        // This will actually be done by the hardware model later on.
    }
};

} // namespace FTS

FTS::HardwareModel::HardwareModel(const FTS::String& in_sName)
    : m_pCoreModel(new bouge::CoreModel(in_sName.str()))
{
    bouge::XMLLoader loader(new bouge::TinyXMLParser());
    m_pCoreModel->mesh(loader.loadMesh(sErrorModelMesh));
    m_pCoreModel->addMaterials(loader.loadMaterial(sErrorModelMaterial));
    m_pCoreModel->addMaterialSets(loader.loadMaterialSet(sErrorModelMaterialSet));

    this->createHardwareMesh();
    for(bouge::CoreModel::material_iterator iMat = m_pCoreModel->begin_material() ; iMat != m_pCoreModel->end_material() ; ++iMat) {
        String dummy;
        MaterialUserData* mud = new MaterialUserData(**iMat, *m_pHardwareModel, in_sName, dummy);
        this->setupVAO(*mud);
        iMat->userData = bouge::UserDataPtr(mud);
    }
}

FTS::HardwareModel::HardwareModel(const FTS::String& in_sName, FTS::Archive& in_modelArch)
    : m_pCoreModel(new bouge::CoreModel(in_sName.str()))
{
    std::set<String> loadedShads;
    bouge::XMLLoader loader(new bouge::TinyXMLParser());

    try {
    // There is only one skeleton and one mesh per model file. Load them first.
    String sData = in_modelArch.getFile("mesh.bxmesh").readstr();
    m_pCoreModel->mesh(loader.loadMesh(sData.c_str(), sData.byteCount()));

    // But even the skeleton is optional in some cases.
    if(in_modelArch.hasChunk("skeleton.bxskel")) {
        String sData = in_modelArch.getFile("skeleton.bxskel").readstr();
        m_pCoreModel->skeleton(loader.loadSkeleton(sData.c_str(), sData.byteCount()));
    }
    } catch(const ArkanaException&) {
        this->unloadResources();
        throw;
    } catch(const std::exception& ex) {
        this->unloadResources();
        throw CorruptDataException("Model: " + in_sName, ex.what());
    }

    // Now we load all materials, material sets and animations we can find in the archive.
    for(auto i = in_modelArch.begin() ; i != in_modelArch.end() ; ++i) {
        FTS::FileChunk* pFchk = dynamic_cast<FTS::FileChunk*>(i->second);
        if(pFchk == NULL)
            continue;

        Path fName = i->first;

        // Just try out what kind of file it may be, depending on the extension
        // If one fails, just go to the next one. Those here aren't crucial.
        try {
        if(fName.ext().lower() == "bxmset") {
            String data = pFchk->getFile().readstr();
            m_pCoreModel->addMaterialSets(loader.loadMaterialSet(data.c_str(), data.byteCount()));
        } else if(fName.ext().lower() == "bxmat") {
            String data = pFchk->getFile().readstr();
            m_pCoreModel->addMaterials(loader.loadMaterial(data.c_str(), data.byteCount()));
        } else if(fName.ext().lower() == "bxanim") {
            String data = pFchk->getFile().readstr();
            m_pCoreModel->addAnimations(loader.loadAnimation(data.c_str(), data.byteCount()));
        } else if(fName.ext().lower() == "png") {
            // We prepend the model's name to the name of the graphic in order
            // to get model-unique graphic names.
            String sName = in_sName + MODEL_FILENAME_SEP + fName;

            /// \TODO Integrate the archive name into the path (instead of the hacky way above).
            GraphicManager::getSingleton().getOrLoadGraphic(pFchk->getFile(), sName);
            m_loadedTexs.insert(sName);
        } else if(fName.ext().lower() == "vert"
               || fName.ext().lower() == "frag"
               || fName.ext().lower() == "geom"
               || fName.ext().lower().right(3) == "inc") {
            // We prepend the model's name to the name of the shader in order
            // to get model-unique shader names.
            String sName = in_sName + MODEL_FILENAME_SEP + fName;
            String sShaderSrc = pFchk->getFile().readstr();

            /// \TODO Integrate the archive name into the path (instead of the hacky way above).
            ShaderManager::getSingleton().loadShaderCode(sName, sShaderSrc);
            loadedShads.insert(sName);
        }
        } catch(const ArkanaException& ex) {
            FTSMSG(ex.what(), MsgType::Warning);
        } catch(const std::exception& ex) {
            FTSMSG(ex.what(), MsgType::Warning);
        }
    }

    try {

    // Bones consistency checkpoint //
    //////////////////////////////////
    std::set<std::string> missingBones = m_pCoreModel->missingBones();
    if(!missingBones.empty()) {
        throw CorruptDataException("Model: " + in_sName, "Missing the following bones in the skeleton: " + bouge::to_s(missingBones.begin(), missingBones.end()));
    }

    // Matset consistency checkpoint //
    ///////////////////////////////////
    struct MatSetErrors {
        std::set<std::string> missingMats;
        std::set<std::string> missingSubmeshes;
        MatSetErrors() {};
        MatSetErrors(std::set<std::string> missingMats, std::set<std::string> missingSubmeshes) : missingMats(missingMats), missingSubmeshes(missingSubmeshes) {};
    };

    // Find all matsets which are missing some materials and remove them from the model.
    std::map<std::string, MatSetErrors> badMatSets;
    for(auto matset = m_pCoreModel->begin_materialset() ; matset != m_pCoreModel->end_materialset() ; ++matset) {
        std::set<std::string> missingMaterials = m_pCoreModel->missingMaterials(matset->name());
        std::set<std::string> missingSubmeshes = m_pCoreModel->missingMatsetSpecs(matset->name());
        if(!missingMaterials.empty() || !missingSubmeshes.empty()) {
            badMatSets[matset->name()] = MatSetErrors(missingMaterials, missingSubmeshes);
        }
    }

    // Take out all the bad material sets.
    for(auto matset = badMatSets.begin() ; matset != badMatSets.end() ; ++matset) {
        // But with a readable error message please!
        String msg = "The material set \"" + matset->first + "\"";
        if(!matset->second.missingMats.empty()) {
            msg += " uses the undefined materials \"" + join(matset->second.missingMats.begin(), matset->second.missingMats.end(), "\", \"") + "\"";
            if(!matset->second.missingSubmeshes.empty()) {
                msg += " and";
            }
        }
        if(!matset->second.missingSubmeshes.empty()) {
            msg += " is missing the assossiations for the submeshes \"" + join(matset->second.missingSubmeshes.begin(), matset->second.missingSubmeshes.end(), "\", \"") + "\"";
        }
        FTS18N("CorruptData", MsgType::Warning, in_sName, msg);

        m_pCoreModel->removeMaterialSet(matset->first);
    }

    // If there is no more material set left, we got a problem :)
    if(m_pCoreModel->materialSetCount() < 1) {
        throw CorruptDataException("Model: " + in_sName, "Needs at least one valid materialset");
    }

    // Converts the data for use on the GPU and uploads it into a VBO.
    this->createHardwareMesh();

    // Now we can load all the resources needed by all the materials.
    // We offload this task to the MaterialUserData class.
    for(bouge::CoreModel::material_iterator iMat = m_pCoreModel->begin_material() ; iMat != m_pCoreModel->end_material() ; ++iMat) {
        String sEmbeddedShaderName;
        MaterialUserData* mud = new MaterialUserData(**iMat, *m_pHardwareModel, in_sName, sEmbeddedShaderName);
        this->setupVAO(*mud);
        iMat->userData = bouge::UserDataPtr(mud);

        if(!sEmbeddedShaderName.empty())
            m_loadedProgs.insert(sEmbeddedShaderName);
    }

    // We no more need the source code of the shaders loaded by this model.
    // Unlike the textures, the compiled shaders are no more needed once the
    // program linked successfully.
    for(auto shader = loadedShads.begin() ; shader != loadedShads.end() ; ++shader) {
        ShaderManager::getSingleton().unloadShader(*shader);
    }

    } catch(...) {
        this->unloadResources();
        throw;
    }

    // Cache some informations.
    for(auto matset = m_pCoreModel->begin_materialset() ; matset != m_pCoreModel->end_materialset() ; ++matset) {
        m_skins.insert(matset->name());
    }
    for(auto anim = m_pCoreModel->begin_animation() ; anim != m_pCoreModel->end_animation() ; ++anim) {
        m_anims.insert(anim->name());
    }
}

void FTS::HardwareModel::createHardwareMesh()
{
    if(m_pCoreModel->skeleton()) {
        m_pHardwareModel = bouge::CoreHardwareMeshPtr(new bouge::CoreHardwareMesh(m_pCoreModel->mesh(), BONES_PER_MESH));
    } else {
        m_pHardwareModel = bouge::CoreHardwareMeshPtr(new bouge::CoreHardwareMesh(m_pCoreModel->mesh(), 0, 0));
    }

    // Collect some numbers we need beforehand (because we want it interleaved, else we could just append the buffers).
    std::size_t floatsPerVertex = m_pHardwareModel->coordsPerVertex();
    floatsPerVertex += m_pHardwareModel->weightsPerVertex();
    floatsPerVertex += m_pHardwareModel->boneIndicesPerVertex();
    for(auto attrib = m_pHardwareModel->attribs().begin() ; attrib != m_pHardwareModel->attribs().end() ; ++attrib) {
        floatsPerVertex += attrib->second;
    }
    std::size_t stride = floatsPerVertex * sizeof(float);

    // Here, we will compile all the vertex data into a single interleaved buffer.
    std::vector<float> data(floatsPerVertex * m_pHardwareModel->vertexCount());

    // The vertex coordinates, weights and indices are a special case, unfortunately.
    std::size_t offset = 0;
    m_pHardwareModel->writeCoords(&data[offset], stride);
    offset += m_pHardwareModel->coordsPerVertex();
    m_pHardwareModel->writeWeights(&data[offset], stride);
    offset += m_pHardwareModel->weightsPerVertex();
    m_pHardwareModel->writeBoneIndices(&data[offset], stride);
    offset += m_pHardwareModel->boneIndicesPerVertex();

    // But all other attributes can be handled homogenely
    for(auto attrib = m_pHardwareModel->attribs().begin() ; attrib != m_pHardwareModel->attribs().end() ; ++attrib) {
        m_pHardwareModel->writeAttrib(attrib->first, &data[offset], stride);
        offset += attrib->second;
    }

    // Upload that data to the graphics card
    m_vbo.reset(new VertexBufferObject(data, floatsPerVertex));
    m_pVtxIdxVBO.reset(new ElementsBufferObject(m_pHardwareModel->faceIndices(), m_pHardwareModel->indicesPerFace()));
}

void FTS::HardwareModel::setupVAO(FTS::MaterialUserData& in_ud) const
{
    // And upload everything into an OpenGL VAO for later use.
    in_ud.vao.bind();
    m_vbo->bind();

    // Setup all of the vertex attributes, again vertex coords, weights and indices are special.
    std::size_t offset = 0;
    in_ud.prog->setVertexAttribute("aVertexPosition", *m_vbo, m_pHardwareModel->coordsPerVertex(), offset);
    offset += m_pHardwareModel->coordsPerVertex();
    in_ud.prog->setVertexAttribute("aWeights", *m_vbo, m_pHardwareModel->weightsPerVertex(), offset);
    offset += m_pHardwareModel->weightsPerVertex();
    in_ud.prog->setVertexAttribute("aIndices", *m_vbo, m_pHardwareModel->boneIndicesPerVertex(), offset);
    offset += m_pHardwareModel->boneIndicesPerVertex();

    // For the rest just use attributes of the same name.
    for(auto attrib = m_pHardwareModel->attribs().begin() ; attrib != m_pHardwareModel->attribs().end() ; ++attrib) {
        in_ud.prog->setVertexAttribute(attrib->first, *m_vbo, m_pHardwareModel->attribCoordsPerVertex(attrib->first), offset);
        offset += attrib->second;
    }

    // Finally, setup the face indices "element" buffer.
    m_pVtxIdxVBO->bind();

    in_ud.vao.unbind();
    VertexBufferObject::unbind();
    ElementsBufferObject::unbind();
}

FTS::HardwareModel::~HardwareModel()
{
    this->unloadResources();
}

void FTS::HardwareModel::unloadResources() const
{
    // Throw all the textures away, we no more need them.
    for(auto s = m_loadedTexs.begin() ; s != m_loadedTexs.end() ; ++s) {
        GraphicManager::getSingleton().destroyGraphic(*s);
    }

    // Same for the model-embedded shaders.
    for(auto s = m_loadedProgs.begin() ; s != m_loadedProgs.end() ; ++s) {
        ShaderManager::getSingleton().destroyProgramsUsing(*s);
    }
}

FTS::String FTS::HardwareModel::getName() const
{
    return m_pCoreModel->name();
}

const std::set<FTS::String>& FTS::HardwareModel::skins() const
{
    return m_skins;
}

const std::set<FTS::String>& FTS::HardwareModel::anims() const
{
    return m_anims;
}

uint32_t FTS::HardwareModel::vertexCount() const
{
    return m_pHardwareModel->vertexCount();
}

uint32_t FTS::HardwareModel::faceCount() const
{
    return m_pHardwareModel->faceCount();
}

FTS::AxisAlignedBoundingBox FTS::HardwareModel::restAABB() const
{
    return m_restAABB;
}

void FTS::HardwareModel::render(const AffineMatrix& in_modelMatrix, const Color& in_playerCol, bouge::ModelInstancePtrC in_modelInst)
{
    // Preliminary gets to shorten the code.
    Camera& cam = RunlevelManager::getSingleton().getCurrRunlevel()->getActiveCamera();

    static const std::string uModelViewProjectionMatrix = "uModelViewProjectionMatrix";
    static const std::string uModelViewMatrix = "uModelViewMatrix";
    static const std::string uProjectionMatrix = "uProjectionMatrix";
    static const std::string uInvModelViewProjectionMatrix = "uInvModelViewProjectionMatrix";
    static const std::string uInvModelViewMatrix = "uInvModelViewMatrix";

    // Pre-calculate a few matrices:
    General4x4Matrix mvp = cam.getViewProjectionMatrix() * in_modelMatrix;
    AffineMatrix mv = cam.getViewMatrix() * in_modelMatrix;
    General4x4Matrix p = cam.getProjectionMatrix();

    // Now, render each submesh of the mesh one after. It may need to get split
    // for example if it has too many bones.
    for(bouge::CoreHardwareMesh::const_iterator i = m_pHardwareModel->begin() ; i != m_pHardwareModel->end() ; ++i) {
        const bouge::CoreHardwareSubMesh& submesh = *i;

        // We set the per-submesh material options.

        // Here, we can assume the material exists, as we did the
        // "integrity checks" after the loading already.
        bouge::CoreMaterialPtrC pMat = in_modelInst->materialForSubmesh(submesh.submeshName());
        MaterialUserData* pUD = static_cast<MaterialUserData*>(pMat->userData.get());

        Program* prog = pUD->prog;
        prog->bind();
        pUD->vao.bind();

        // Give the shader the matrices he needs, and their inverses.
        prog->setUniform(uModelViewProjectionMatrix, mvp);
        prog->setUniform(uModelViewMatrix, mv);
        prog->setUniform(uProjectionMatrix, p);
        prog->setUniformInverse(uInvModelViewProjectionMatrix, mvp);
        prog->setUniformInverse(uInvModelViewMatrix, mv);

        // We can now also give it the bone matrices.
        for(std::size_t i = 0 ; i < submesh.boneCount() ; ++i) {
            bouge::BoneInstancePtrC bone = in_modelInst->skeleton()->bone(submesh.boneName(i));
            prog->setUniformArrayElement("uBonesPalette", i, bone->transformMatrix());
            prog->setUniformArrayElementInverse("uBonesPaletteInvTrans", i, bone->transformMatrix(), true);
        }

        // Set all the material-registered uniforms.
        for(auto uniform = pUD->uniforms_f.begin() ; uniform != pUD->uniforms_f.end() ; ++uniform) {
            prog->setUniform(uniform->first, uniform->second);
        }

        // And select all of the textures.
        uint8_t texUnit = 0;
        for(auto uniform = pUD->uniforms_tex.begin() ; uniform != pUD->uniforms_tex.end() ; ++uniform) {
            uniform->second->select(texUnit);
            prog->setUniformSampler(uniform->first, texUnit);
            texUnit++;
        }

        glDrawElements(pUD->drawMode, submesh.faceCount() * m_pHardwareModel->indicesPerFace(), BOUGE_FACE_INDEX_TYPE_GL, (const GLvoid*)(submesh.startIndex()*sizeof(BOUGE_FACE_INDEX_TYPE)));

        pUD->vao.unbind();
        Program::unbind();
    }
    verifGL("HardwareModel::render() end");
}
