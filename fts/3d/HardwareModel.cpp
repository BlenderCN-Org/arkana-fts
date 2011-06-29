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
#include "3d/math/Vector.h"
#include "3d/math/Quaternion.h"
#include "3d/math/Matrix.h"
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

    std::vector< std::pair<String, Vector> > uniforms_f;
    std::vector< std::pair<String, Graphic*> > uniforms_tex;

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
            std::vector<String> sFlags;
            String(in_mat.proprety("ShaderCompileFlags")).split(std::back_inserter(sFlags));
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
                    uniforms_f.push_back(std::make_pair(prop.name(), Vector(&prop.valueAsFvec()[0])));
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
                    uniforms_tex.push_back(std::make_pair(prop.name(), pGraphic));
                    break;
                //TODO: more types, for example matrices, ints, ...
                }
            } catch(const NotExistException&) {
                // Nevermind if this property is not an uniform.
            }
        }

        // Setup shader attributes in a VAO.
        // This will actually be done by the hardware model later on.

        // TODO MOAR
    }
};

void setupVAO(const FTS::HardwareModel& in_hwmodel, FTS::MaterialUserData& in_ud);

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
    std::vector<String> loadedShads;

    try {
    bouge::XMLLoader loader(new bouge::TinyXMLParser());

    // There is only one skeleton and one mesh per model file. Load them first.
    String sData = in_modelArch.getFile("mesh.bxmesh").readstr();
    m_pCoreModel->mesh(loader.loadMesh(sData.c_str(), sData.byteCount()));

    // But even the skeleton is optional in some cases.
    if(in_modelArch.hasChunk("skeleton.bxskel")) {
        String sData = in_modelArch.getFile("skeleton.bxskel").readstr();
        m_pCoreModel->skeleton(loader.loadSkeleton(sData.c_str(), sData.byteCount()));
    }

    // Now we load all materials, material sets and animations we can find in the archive.
    for(auto i = in_modelArch.begin() ; i != in_modelArch.end() ; ++i) {
        FTS::FileChunk* pFchk = dynamic_cast<FTS::FileChunk*>(i->second);
        if(pFchk == NULL)
            continue;

        Path fName = i->first;

        // Just try out what kind of file it may be, depending on the extension
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
            m_loadedTexs.push_back(sName);
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
            loadedShads.push_back(sName);
        }
    }
    } catch(const ArkanaException&) {
        this->unloadResources();
        throw;
    } catch(const std::exception& ex) {
        this->unloadResources();
        throw CorruptDataException("Model: " + in_sName, ex.what());
    }

    try {

    // First consistency checkpoint //
    //////////////////////////////////
    std::set<std::string> missingBones = m_pCoreModel->missingBones();
    if(!missingBones.empty()) {
        throw CorruptDataException("Model: " + in_sName, "Missing the following bones in the skeleton: " + bouge::to_s(missingBones.begin(), missingBones.end()));
    }

    if(m_pCoreModel->materialSetCount() < 1) {
        throw CorruptDataException("Model: " + in_sName, "Needs at least one materialset");
    }

    std::set<std::string> missingMaterials = m_pCoreModel->missingMaterials();
    if(!missingMaterials.empty()) {
        throw CorruptDataException("Model: " + in_sName, "Missing the following materials in the model: " + bouge::to_s(missingMaterials.begin(), missingMaterials.end()));
    }

    std::set<std::string> missingMatsetSpecs = m_pCoreModel->missingMatsetSpecs();
    if(!missingMatsetSpecs.empty()) {
        throw CorruptDataException("Model: " + in_sName, "Missing the material set specifications for the following submeshes: " + bouge::to_s(missingMatsetSpecs.begin(), missingMatsetSpecs.end()));
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
            m_loadedProgs.push_back(sEmbeddedShaderName);
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

    // And throw all our "user-data" away too!
/*    for(int iMat = 0 ; iMat < m_pCoreModel->getCoreMaterialCount() ; ++iMat) {
        CalCoreMaterial *pCoreMat = m_pCoreModel->getCoreMaterial(iMat);

        for(int iTex = 0 ; iTex < pCoreMat->getMapCount() ; ++iTex) {
            String* pS = reinterpret_cast<String*>(pCoreMat->getMapUserData(iTex));
            if(pS) {
                delete pS;
            }
        }

        MaterialUserData* pUD = reinterpret_cast<MaterialUserData*>(pCoreMat->getUserData());
        if(pUD) {
            delete pUD;
        }
    }*/
}

/*
int FTS::HardwareModel::getOrCreateErrorMatId(const String& in_sModelName)
{
    // If it has no material yet, either make it use an existing default material...
    std::string sDefMatName = "ErrorMaterial";
    int id = m_pCoreModel->getCoreMaterialId(sDefMatName);

    CalCoreMaterial* pDefaultCoreMat = m_pCoreModel->getCoreMaterial(id);
    if(pDefaultCoreMat)
        return id;

    // If it does not exist yet, we create it (with the error texture).
    pDefaultCoreMat = new CalCoreMaterial();
    pDefaultCoreMat->setName(sDefMatName);
    pDefaultCoreMat->setDiffuseColor(CalCoreMaterial::Color(255, 255, 255, 255));
    pDefaultCoreMat->setAmbientColor(CalCoreMaterial::Color(25, 25, 25, 255));
    pDefaultCoreMat->addMap(CalCoreMaterial::Map(GraphicManager::ErrorTextureName.str()));

    // Not do these yet, they will be done later in the loading process!
//     pDefaultCoreMat->setMapUserData(0, new String(GraphicManager::ErrorTextureName));
//     pDefaultCoreMat->setUserData(new MaterialUserData(*pDefaultCoreMat, in_sModelName));
    return m_pCoreModel->addCoreMaterial(pDefaultCoreMat);
}

void FTS::HardwareModel::loadHardware(int in_nMaxTexturesPerMesh, int in_nMaxBonesPerMesh)
{
    // Load the hardware data into the vertex buffers.

    std::vector<float> vVerts; // Vertices: x,y,z
    std::vector<float> vNorms; // Normals: x,y,z
    std::vector<float> vWeights; // Weights: a,b,c,d
    std::vector<float> vBIndices; // Bone matrix indices: a,b,c,d
    std::vector<CalIndex> vVIndices; // Vertex indices of the faces: a,b,c
    std::vector<std::vector<float>> vTexCoords(in_nMaxTexturesPerMesh, std::vector<float>()); // Texture coordinates: u,v

    // Give Cal3d all of our buffers:
    m_pHardwareModel->setVertexBuffer(&vVerts, 3);
    m_pHardwareModel->setNormalBuffer(&vNorms, 3);
    m_pHardwareModel->setWeightBuffer(&vWeights, 4);
    m_pHardwareModel->setMatrixIndexBuffer(&vBIndices, 4);
    m_pHardwareModel->setIndexBuffer(&vVIndices);

    m_pHardwareModel->setTextureCoordNum(in_nMaxTexturesPerMesh);
    for(int i = 0 ; i < in_nMaxTexturesPerMesh ; ++i) {
        m_pHardwareModel->setTextureCoordBuffer(i, &(vTexCoords[i]), 2);
    }

    /// \TODO: Determine a good max bone count per mesh. (depends on how many bones we can put into a shader, how many textures we have etc.)
    // Load all the vertex&face data into our buffers from above.
    m_pHardwareModel->loadDyn(0, 0, in_nMaxBonesPerMesh);

    // the vertex indices in vVIndices are relative to the begining of the hardware mesh,
    // we make them relative to the begining of the vertex buffer.
    for(int iMesh = 0 ; iMesh < m_pHardwareModel->getHardwareMeshCount(); ++iMesh) {
        m_pHardwareModel->selectHardwareMesh(iMesh);

        for(int iFace = 0 ; iFace < m_pHardwareModel->getFaceCount() ; ++iFace) {
            vVIndices[m_pHardwareModel->getStartIndex()+iFace*3+0] += m_pHardwareModel->getBaseVertexIndex();
            vVIndices[m_pHardwareModel->getStartIndex()+iFace*3+1] += m_pHardwareModel->getBaseVertexIndex();
            vVIndices[m_pHardwareModel->getStartIndex()+iFace*3+2] += m_pHardwareModel->getBaseVertexIndex();
        }
    }

    // Combine the indices and weights into a single value.
    for(int i = 0 ; i < m_pHardwareModel->getTotalVertexCount() ; ++i) {
        // Normalize the weights, if needed:
        float fWeightSum = vWeights[i*4+0] + vWeights[i*4+1] + vWeights[i*4+2] + vWeights[i*4+3];
        if(fWeightSum > 1.001f) {
            FTSMSG("Oops, vertex " + String::nr(i) + " has a weight sum above 1.0 (" + String::nr(fWeightSum) + ") ! That is bad, normalizing it", MsgType::WarningNoMB);
            vWeights[i*4+0] /= fWeightSum;
            vWeights[i*4+1] /= fWeightSum;
            vWeights[i*4+2] /= fWeightSum;
            vWeights[i*4+3] /= fWeightSum;
        } else if(fWeightSum < 0.0f) {
            FTSMSG("Oops, vertex " + String::nr(i) + " has a weight sum below 0.0 (" + String::nr(fWeightSum) + ") ! That is bad, changing it to 1.0", MsgType::WarningNoMB);
            vWeights[i*4+0] = 0.25f;
            vWeights[i*4+1] = 0.25f;
            vWeights[i*4+2] = 0.25f;
            vWeights[i*4+3] = 0.25f;
        }
        // *0.99 in order not to add 1.0, that would change the index.
        vBIndices[i*4+0]+=vWeights[i*4+0]*0.99f;
        vBIndices[i*4+1]+=vWeights[i*4+1]*0.99f;
        vBIndices[i*4+2]+=vWeights[i*4+2]*0.99f;
        vBIndices[i*4+3]+=vWeights[i*4+3]*0.99f;
    }

    // Now we can load up the data to the GPU.
    m_pVertexVBO.reset(new VertexBufferObject(vVerts, 3));
    m_pNormalVBO.reset(new VertexBufferObject(vNorms, 3));
    m_pMatIdxAndWeightVBO.reset(new VertexBufferObject(vBIndices, 4));
    m_pVtxIdxVBO.reset(new ElementsBufferObject(vVIndices, 3));

    m_pTexCoVBOs.clear();
    for(int i = 0 ; i < in_nMaxTexturesPerMesh ; ++i) {
        m_pTexCoVBOs.push_back(std::unique_ptr<VertexBufferObject>(new VertexBufferObject(vTexCoords[i], 2)));
    }

    // Pre-calculate the bounding boxes of the model:
    this->precomputeAABB(vVerts);
}

void FTS::HardwareModel::precomputeAABB(const std::vector<float>& in_vVertices)
{
    // Forget it...
    if(in_vVertices.size() < 3)
        return ;

    // First, let Cal3d pre-compute the BB for every single bone.
    m_pCoreModel->getCoreSkeleton()->calculateBoundingBoxes(m_pCoreModel.get());

    // Now, calculate the BB for the whole model in its undeformed position.
    m_restAABB = AxisAlignedBoundingBox(Vector(in_vVertices[0], in_vVertices[1], in_vVertices[2]));

    // This is easy: just update it with every single vertex we have.
    for(std::size_t i = 3 ; i < in_vVertices.size() ; i += 3) {
        Vector v(in_vVertices[i], in_vVertices[i+1], in_vVertices[i+2]);
        m_restAABB.update(v);
    }
}
*/
FTS::String FTS::HardwareModel::getName() const
{
    return m_pCoreModel->name();
}

std::vector<FTS::String> FTS::HardwareModel::getSkinList() const
{
    std::vector<String> ret;

    for(auto i = m_mSkins.begin() ; i != m_mSkins.end() ; ++i) {
        ret.push_back(i->first);
    }

    return ret;
}

int FTS::HardwareModel::getSkinId(const FTS::String& in_sSkinName) const
{
    auto i = m_mSkins.find(in_sSkinName);
    if(i == m_mSkins.end())
        return 0;

    return i->second;
}

std::vector<FTS::String> FTS::HardwareModel::getAnimList() const
{
    std::vector<FTS::String> ret;

//     for(int i = 0 ; i < m_pCoreModel->getNumCoreAnimations() ; ++i) {
//         ret.push_back(m_pCoreModel->getCoreAnimation(i)->getName());
//     }

    return ret;
}

uint32_t FTS::HardwareModel::getVertexCount() const
{
    return m_pHardwareModel->vertexCount();
}

uint32_t FTS::HardwareModel::getFaceCount() const
{
    return m_pHardwareModel->faceCount();
}

FTS::AxisAlignedBoundingBox FTS::HardwareModel::getRestAABB() const
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
            //bouge::BoneInstancePtrC bone = m_modelInst->skeleton()->bone(submesh.boneName(i));
            //prog->uniformMatrix4fv(uBonesPalette(i), 1, false, bone->transformMatrix().array16f());
            //prog->uniformMatrix3fv(uBonesPaletteInvTrans(i), 1, true, bone->transformMatrix().array9fInverse());

            // TODO: replace by real bone matrices of model instance
            prog->setUniformArrayElement("uBonesPalette", i, AffineMatrix());
            prog->setUniformArrayElement("uBonesPaletteInvTrans", i, AffineMatrix());
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
/*
    for(int iHardwareMesh = 0 ; iHardwareMesh < m_pHardwareModel->getHardwareMeshCount() ; iHardwareMesh++)
    {
        m_pHardwareModel->selectHardwareMesh(iHardwareMesh);

        // Get the material needed by this current hardware mesh.
        // Note: we already made sure the material exists during the loading.
        CalHardwareModel::CalHardwareMesh& hwm = m_pHardwareModel->getVectorHardwareMesh()[iHardwareMesh];
        int iMat = in_model.getMesh(hwm.meshId)->getSubmesh(hwm.submeshId)->getCoreMaterialId();
        CalCoreMaterial* pMat = m_pCoreModel->getCoreMaterial(iMat);

        // Now we make use of the shader this material wants us to use:
        MaterialUserData* pUD = reinterpret_cast<MaterialUserData*>(pMat->getUserData());
        Shader* pShad = pUD->pShader;
        pShad->bind();
        pUD->vao.bind();

        // Now, we upload the bone poses:
        for(int iBone = 0 ; iBone < m_pHardwareModel->getBoneCount() ; ++iBone) {
            CalQuaternion rotationBoneSpace = m_pHardwareModel->getRotationBoneSpace(iBone, in_model.getSkeleton());
            CalVector translationBoneSpace = m_pHardwareModel->getTranslationBoneSpace(iBone, in_model.getSkeleton());

            /// \TODO Use the quaternion form of the rotation instead of transforming it to a matrix form.
            pShad->setUniformArrayElement("uBoneRotation", iBone, AffineMatrix::rotationQuat(rotationBoneSpace));
            pShad->setUniformArrayElement("uBoneTranslation", iBone, translationBoneSpace);
        }

        // Give the shader the matrices he needs.
        pShad->setUniform("uModelViewProjectionMatrix", mvp);
        pShad->setUniform("uModelViewMatrix", mv);
        pShad->setUniform("uProjectionMatrix", p);

        // And the inverses of them.
        pShad->setUniformInverse("uInvModelViewProjectionMatrix", mvp);
        pShad->setUniformInverse("uInvModelViewMatrix", mv);

        // And some more informations about the material to use.
        pShad->setUniform("uPlayerColor", Color(in_playerCol, pUD->alphaAsPlayerCol));
        pShad->setUniform("uMaterialDiffuse", pMat->getDiffuseColor());
        pShad->setUniform("uMaterialAmbient", pMat->getAmbientColor());
        pShad->setUniform("uMaterialSpecular", Color(pMat->getSpecularColor(), pMat->getShininess()));

        // The textures, if needed. We might optimize here by selecting no texture if the
        // _shader_ doesn't want a texture.

        int nMaps = pMat->getMapCount();
        for(int iMap = 0 ; iMap < nMaps ; ++iMap) {
            String* psTexName = reinterpret_cast<String*>(pMat->getMapUserData(iMap));
            GraphicManager::getSingleton().getOrLoadGraphic(*psTexName)->select(iMap);
            pShad->setUniformSampler("uTexture", iMap);
        }

#ifdef CAL_16BIT_INDICES
        glDrawElements(pUD->drawMode, m_pHardwareModel->getFaceCount() * 3, GL_UNSIGNED_SHORT, (((CalIndex *)NULL)+ m_pHardwareModel->getStartIndex()));
#else
        glDrawElements(pUD->drawMode, m_pHardwareModel->getFaceCount() * 3, GL_UNSIGNED_INT, (((CalIndex *)NULL)+ m_pHardwareModel->getStartIndex()));
#endif
        pUD->vao.unbind();
//     Shader::unbind();
    }*/
    verifGL("HardwareModel::render() end");
}
