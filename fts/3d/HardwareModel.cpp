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

static const unsigned int BONES_PER_MESH = 20;

namespace FTS {

// struct MaterialUserData {
//     String sVertShaderName;
//     String sFragShaderName;
//     String sGeomShaderName;
//     VertexArrayObject vao;
//     float alphaAsPlayerCol;
//     Program * pShader;
//
//     /// The mode to use when calling glDrawElements.
//     unsigned int drawMode;
//
//     MaterialUserData(const String& in_sVertShaderName = String::EMPTY, const String& in_sFragShaderName = String::EMPTY, const String& in_sGeomShaderName = String::EMPTY)
//         : sVertShaderName(in_sVertShaderName), sFragShaderName(in_sFragShaderName), sGeomShaderName(in_sGeomShaderName), alphaAsPlayerCol(1.0f), drawMode(GL_TRIANGLES)
//     {
//         pShader = ShaderManager::getSingleton().getOrLinkProgram(sVertShaderName, sFragShaderName, sGeomShaderName);
//     };
//
//     MaterialUserData(const CalCoreMaterial& in_coreMat, const String& in_sModelName, bool in_bHasNonemptySkeleton)
//         : sVertShaderName(in_bHasNonemptySkeleton ? "__FTS__RiggedModel.vert" : "__FTS__StaticModel.vert")
//         , sFragShaderName("__FTS__TnL.frag")         /// \todo make dependend on whether there is a map or not for ex.
//         , alphaAsPlayerCol(1.0f)
//         , drawMode(GL_TRIANGLES)
//     {
//         String sAlphaAsPlayerColor = in_coreMat.getProprety("AlphaAsPlayerColor");
//         if(!sAlphaAsPlayerColor.isEmpty()) {
//             // Either, we set the player color, fully opaque or we don't use player
//             // color thus make it fully transparent.
//             // The shader makes the rest. standard c++ says true -> 1.0f, false -> 0.0f
//             // and that's exactly what we need, cool :)
//             this->alphaAsPlayerCol = static_cast<float>(sAlphaAsPlayerColor.to_Boolean());
//         }
//
//         String sDrawMode = in_coreMat.getProprety("Mode");
//         if(!sDrawMode.isEmpty()) {
//             // They want us to draw something other than GL_TRIANGLES... Do so.
//             if(sDrawMode.ieq("Lines"))
//                 this->drawMode = GL_LINES;
//         }
//
//         // Read what shaders to use. Prefer embedded ones over external references.
//         if(!in_coreMat.getProprety("VertexShader").empty()) {
//             // First, check if the shader was built into the model:
//             String sBuiltinShaderName = in_sModelName + ":" + in_coreMat.getProprety("VertexShader");
//             if(ShaderManager::getSingleton().hasShader(sBuiltinShaderName))
//                 this->sVertShaderName = sBuiltinShaderName;
//             else
//                 this->sVertShaderName = in_coreMat.getProprety("VertexShader");
//         }
//
//         // Caution: If the model contains no texture, we have to switch to a
//         //          lighting-only fragment shader.
//         if(in_coreMat.getMapCount() == 0) {
//             this->sFragShaderName = "__FTS__OnlyLighting.frag";
//         }
//
//         if(!in_coreMat.getProprety("FragmentShader").empty()) {
//             // First, check if the shader was built into the model:
//             String sBuiltinShaderName = in_sModelName + ":" + in_coreMat.getProprety("FragmentShader");
//             if(ShaderManager::getSingleton().hasShader(sBuiltinShaderName))
//                 this->sFragShaderName = sBuiltinShaderName;
//             else
//                 this->sFragShaderName = in_coreMat.getProprety("FragmentShader");
//         }
//
//         if(!in_coreMat.getProprety("GeometryShader").empty()) {
//             // First, check if the shader was built into the model:
//             String sBuiltinShaderName = in_sModelName + ":" + in_coreMat.getProprety("GeometryShader");
//             if(ShaderManager::getSingleton().hasShader(sBuiltinShaderName))
//                 this->sGeomShaderName = sBuiltinShaderName;
//             else
//                 this->sGeomShaderName = in_coreMat.getProprety("GeometryShader");
//         }
//         pShader = ShaderManager::getSingleton().getOrLinkProgram(sVertShaderName, sFragShaderName, sGeomShaderName);
//     }
// };

struct MaterialUserData : public bouge::UserData {
    /// We can (and should) take a reference instead of a shared pointer here
    /// because the user data belongs to the material and with a shared pointer
    /// we'd have a cyclic reference here.
    const bouge::CoreMaterial& mat;

    /// The mode to use when calling glDrawElements.
    unsigned int drawMode;

    /// The shader to be used by this material.
    Program* prog;

    /// A VAO storing the shader, shader's vertex attrib setup and vbo setup
    /// on the graphics card.
    VertexArrayObject vao;

    MaterialUserData(const bouge::CoreMaterial& in_mat)
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

        // Load shader to be used.
        // TODO
        this->prog = ShaderManager::getSingleton().getOrLinkProgram();

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
        MaterialUserData* mud = new MaterialUserData(**iMat);
        this->setupVAO(*mud);
        iMat->userData = bouge::UserDataPtr(mud);
    }
}

// FTS::HardwareModel::HardwareModel(const String& in_sName)
//     : m_pCoreModel(new CalCoreModel(in_sName.str()))
//     , m_pHardwareModel(new CalHardwareModel(m_pCoreModel.get()))
// {
//     CalLoader::setLoadingMode(0);
//
//     // Load the error skeleton, mesh and material.
//     m_pCoreModel->loadCoreSkeleton(reinterpret_cast<const void*>(sErrorModelSkeleton));
//     m_pCoreModel->loadCoreMesh(reinterpret_cast<const void*>(sErrorModelMesh));
//     m_pCoreModel->loadCoreMaterial(reinterpret_cast<const void*>(sErrorModelMaterial));
//
//     // I still need to create something with the skins and mat threads, no?
//     m_mSkins["Default"] = 0;
//     m_pCoreModel->createCoreMaterialThread(0);
//     m_pCoreModel->getCoreMesh(0)->getCoreSubmesh(0)->setCoreMaterialThreadId(0);
//     m_pCoreModel->setCoreMaterialId(0, 0, 0);
//
//     // Load the hardware data into the vertex buffers.
//
//     std::vector<float> vVerts;       // Vertices: x,y,z
//     std::vector<CalIndex> vVIndices; // Vertex indices of the faces: a,b,c
//
//     // Give Cal3d all of our buffers:
//     m_pHardwareModel->setVertexBuffer(&vVerts, 3);
//     m_pHardwareModel->setIndexBuffer(&vVIndices);
//     m_pHardwareModel->setTextureCoordNum(0);
//
//     m_pHardwareModel->loadDyn(0, 0, /*MAX_BONES_PER_MESH*/ 20);
//
//     // Now we can load up the data to the GPU.
//     m_pVertexVBO.reset(new VertexBufferObject(vVerts, 3));
//     m_pVtxIdxVBO.reset(new ElementsBufferObject(vVIndices, 3));
//
//     // And store the default material informations into memory:
//     MaterialUserData* pUD = new MaterialUserData();
//     m_pCoreModel->getCoreMaterial(0)->setUserData(pUD);
//
//     // And store them vertex attribute associations into memory.
//     pUD->vao.bind();
//     ShaderManager::getSingleton().getOrLinkShader()->setVertexAttribute("aVertexPosition", *m_pVertexVBO);
//     m_pVtxIdxVBO->bind();
//     pUD->vao.unbind();
// }

FTS::HardwareModel::HardwareModel(const FTS::String& in_sName, FTS::Archive& in_modelArch)
    : m_pCoreModel(new bouge::CoreModel(in_sName.str()))
{
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
/*        } else if(fName.ext().lower() == "png") {
            // We prepend the model's name to the name of the graphic in order
            // to get model-unique graphic names.
            String sName = in_sName + ":" + fName;

            /// \TODO Integrate the archive name into the path (instead of the hacky way above).
            GraphicManager::getSingleton().getOrLoadGraphic(pFchk->getFile(), sName);
            m_loadedTexs.push_back(sName);
        } else if(fName.ext().lower() == "vert"
               || fName.ext().lower() == "frag"
               || fName.ext().lower() == "geom"
               || fName.ext().lower() == "shadinc") {
            // We prepend the model's name to the name of the shader in order
            // to get model-unique shader names.
            String sName = in_sName + ":" + fName;
            String sShaderSrc = pFchk->getFile().readstr();

            /// \TODO Integrate the archive name into the path (instead of the hacky way above).
            if(ShaderManager::getSingleton().makeShader(sName, sShaderSrc))
                m_loadedShads.push_back(sName);*/
        }
    }
    } catch(const ArkanaException&) {
        throw;
    } catch(const std::exception& ex) {
        throw CorruptDataException("Model: " + in_sName, ex.what());
    }

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
        MaterialUserData* mud = new MaterialUserData(**iMat);
        this->setupVAO(*mud);
        iMat->userData = bouge::UserDataPtr(mud);
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
    for(auto attrib = m_pHardwareModel->attribs().begin() ; attrib != m_pHardwareModel->attribs().end() ; ++attrib) {
        floatsPerVertex += attrib->second;
    }
    std::size_t stride = floatsPerVertex * sizeof(float);

    // Here, we will compile all the vertex data into a single interleaved buffer.
    std::vector<float> data(floatsPerVertex * m_pHardwareModel->vertexCount());

    // The vertex coordinates are a special case, unfortunately.
    std::size_t offset = 0;
    m_pHardwareModel->writeCoords(&data[0], stride);
    offset += m_pHardwareModel->coordsPerVertex();

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

    // Setup all of the vertex attributes, again vertex coords are special.
    std::size_t offset = 0;
    in_ud.prog->setVertexAttribute("aVertexPosition", *m_vbo, m_pHardwareModel->coordsPerVertex(), 0);
    offset += m_pHardwareModel->coordsPerVertex();

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

/*
FTS::HardwareModel::HardwareModel(const String& in_sName, Archive& in_modelArch)
    : m_pCoreModel(new CalCoreModel(in_sName.str()))
    , m_pHardwareModel(new CalHardwareModel(m_pCoreModel.get()))
{
    CalLoader::setLoadingMode(0);

    // We have to load the skeleton first. It is _really_ needed.
    if(!m_pCoreModel->loadCoreSkeleton(in_modelArch.getFile("skeleton").getDataContainer().getData())) {
        throw CorruptDataException("Skeleton of model " + in_sName, CalError::getLastErrorDescription());
    }

    // Then, we load all the meshes, materials, textures and shaders there are.
    for(auto i = in_modelArch.begin() ; i != in_modelArch.end() ; ++i) {
        // Currently, we only handle files:
        FTS::FileChunk* pFchk = dynamic_cast<FTS::FileChunk*>(i->second);
        if(pFchk == NULL)
            continue;

        Path fName = i->first;

        // Just try out what kind of file it may be, depending on the extension
        if(fName.ext().lower() == "cmf" || fName.ext().lower() == "xmf") {
            int iMesh = m_pCoreModel->loadCoreMesh(pFchk->getContents().getData());
            if(iMesh == -1) {
                throw CorruptDataException("Model: " + in_sName + ",\nmesh: " + fName, CalError::getLastErrorDescription());
            }
            m_pCoreModel->getCoreMesh(iMesh)->setFilename(fName.str());
            m_pCoreModel->getCoreMesh(iMesh)->setName(fName.basename().withoutExt().str());
        } else if(fName.ext().lower() == "crf" || fName.ext().lower() == "xrf") {
            int iMat = m_pCoreModel->loadCoreMaterial(pFchk->getContents().getData());
            if(iMat == -1) {
                throw CorruptDataException("Model: " + in_sName + ",\nmaterial: " + fName, CalError::getLastErrorDescription());
            }
            m_pCoreModel->getCoreMaterial(iMat)->setFilename(fName.str());
            m_pCoreModel->getCoreMaterial(iMat)->setName(fName.basename().withoutExt().str());
            m_pCoreModel->addMaterialName(fName.basename().withoutExt().str(), iMat);
        } else if(fName.ext().lower() == "caf" || fName.ext().lower() == "xaf") {
            int iAnim = m_pCoreModel->loadCoreAnimation(pFchk->getContents().getData());
            if(iAnim == -1) {
                throw CorruptDataException("Model: " + in_sName + ",\nanimation: " + fName, CalError::getLastErrorDescription());
            }
            m_pCoreModel->getCoreAnimation(iAnim)->setFilename(fName.str());
            m_pCoreModel->getCoreAnimation(iAnim)->setName(fName.basename().withoutExt().str());
            m_pCoreModel->addAnimationName(fName.basename().withoutExt().str(), iAnim);
        } else if(fName.ext().lower() == "png") {
            // We prepend the model's name to the name of the graphic in order
            // to get model-unique graphic names.
            String sName = in_sName + ":" + fName;

            /// \TODO Integrate the archive name into the path (instead of the hacky way above).
            GraphicManager::getSingleton().getOrLoadGraphic(pFchk->getFile(), sName);
            m_loadedTexs.push_back(sName);
        } else if(fName.ext().lower() == "vert"
               || fName.ext().lower() == "frag"
               || fName.ext().lower() == "geom"
               || fName.ext().lower() == "shadinc") {
            // We prepend the model's name to the name of the shader in order
            // to get model-unique shader names.
            String sName = in_sName + ":" + fName;
            String sShaderSrc = pFchk->getFile().readstr();

            /// \TODO Integrate the archive name into the path (instead of the hacky way above).
            if(ShaderManager::getSingleton().makeShader(sName, sShaderSrc))
                m_loadedShads.push_back(sName);
        }
    }

    // We need to find out the maximum number of textures a mesh has at the same time.
    int nMaxTexturesPerMesh = 0;
    for(int iMat = 0 ; iMat < m_pCoreModel->getCoreMaterialCount() ; ++iMat) {
        CalCoreMaterial *pCoreMat = m_pCoreModel->getCoreMaterial(iMat);
        nMaxTexturesPerMesh = std::max(pCoreMat->getMapCount(), nMaxTexturesPerMesh);
    }

    // We have only one material-set: 0. The material-set is like "chainmail", leather, ...
    // We call them "skins".

    // Then there come the material threads. One or several submeshes form a
    // material thread. For example: upperarm left, upperarm right, lowerarm
    // left, lowerarm right and torso might form the material thread
    // "upper body".

    std::map<std::pair<int, int>, int> mOrigMatIds;

    // We make things a little easier by just assuming (forcing) a one-to-one
    // association between a submesh and a material thread, that means that we
    // assign a material thread to every submesh.
    for(int iCoreMesh = 0, nMaterialThreads = 0 ; iCoreMesh < m_pCoreModel->getCoreMeshCount() ; ++iCoreMesh) {
        CalCoreMesh* pCoreMesh = m_pCoreModel->getCoreMesh(iCoreMesh);
        for(int iCoreSubmesh = 0 ; iCoreSubmesh < pCoreMesh->getCoreSubmeshCount() ; ++iCoreSubmesh) {
            CalCoreSubmesh* pCoreSubMesh = pCoreMesh->getCoreSubmesh(iCoreSubmesh);

            // Though we need to keep in mind the index we loaded from the mesh file.
            // We will use that one as material id in case there is no skin file avail.
            int iMatFromFile = pCoreSubMesh->getCoreMaterialThreadId();
            mOrigMatIds[std::make_pair(iCoreMesh, iCoreSubmesh)] = iMatFromFile;

            // And now we give it a unique thread id.
            m_pCoreModel->createCoreMaterialThread(nMaterialThreads);
            pCoreMesh->getCoreSubmesh(iCoreSubmesh)->setCoreMaterialThreadId(nMaterialThreads);
            nMaterialThreads++;
        }
    }

    // This class states what entries to get from the conf file.
    // Each conf-file describes one skin, thus one material per submesh.
    // The entries are of the form: MeshName[0-x] where x is the number of
    // submeshes in the mesh called MeshName.
    class DefaultMaterialSetOpts : public DefaultOptions {
    public:
        DefaultMaterialSetOpts(CalCoreModel* in_pCoreModel) {
            for(int iCoreMesh = 0 ; iCoreMesh < in_pCoreModel->getCoreMeshCount() ; ++iCoreMesh) {
                CalCoreMesh* pCoreMesh = in_pCoreModel->getCoreMesh(iCoreMesh);
                for(int iCoreSubmesh = 0 ; iCoreSubmesh < pCoreMesh->getCoreSubmeshCount() ; ++iCoreSubmesh) {
                    add(String(pCoreMesh->getName()) + String::nr(iCoreSubmesh), 0);
                }
            }
        }
    };

    // Then, we read out all skins that are available in the archive.
    int iSkinId = -1;
    for(auto i = in_modelArch.begin() ; i != in_modelArch.end() ; ++i) {
        FTS::FileChunk* pFchk = dynamic_cast<FTS::FileChunk*>(i->second);
        if(pFchk == NULL || i->first.right(10).lower() != ".ftsmatset")
            continue;

        // Let's make a map holding the Skin name -> Skin Id associations.
        m_mSkins[i->first.mid(0, 10)] = ++iSkinId;

        // The skin states what material to use for every single
        // material-thread (that is for every single submesh).
        Configuration matset(pFchk->getFile(), DefaultMaterialSetOpts(m_pCoreModel.get()));

        // Now we assign those settings to the model.
        for(int iCoreMesh = 0, iMatThread = 0 ; iCoreMesh < m_pCoreModel->getCoreMeshCount() ; ++iCoreMesh) {
            CalCoreMesh* pCoreMesh = m_pCoreModel->getCoreMesh(iCoreMesh);
            for(int iCoreSubmesh = 0 ; iCoreSubmesh < pCoreMesh->getCoreSubmeshCount() ; ++iCoreSubmesh) {
                String sMatName = matset.get(String(pCoreMesh->getName()) + String::nr(iCoreSubmesh));
                int iMatId = m_pCoreModel->getCoreMaterialId(sMatName.str());

                // If the material doesn't exist, or such an entry for this
                // submesh doesn't exist in the conf file, take the error mat.
                if(iMatId == -1 || sMatName.isEmpty()) {
                    iMatId = this->getOrCreateErrorMatId(in_sName);
                }

                m_pCoreModel->setCoreMaterialId(iMatThread, iSkinId, iMatId);
                iMatThread++;
            }
        }
    }

    // If there are no skins, we need to create a default one!
    if(m_mSkins.empty()) {
        m_mSkins["Default"] = 0;

        for(int iCoreMesh = 0, iMatThread = 0 ; iCoreMesh < m_pCoreModel->getCoreMeshCount() ; ++iCoreMesh) {
            CalCoreMesh* pCoreMesh = m_pCoreModel->getCoreMesh(iCoreMesh);
            for(int iCoreSubmesh = 0 ; iCoreSubmesh < pCoreMesh->getCoreSubmeshCount() ; ++iCoreSubmesh) {
                // We take this from the number defined in the submesh in the mesh file.
                int iMatId = mOrigMatIds[std::make_pair(iCoreMesh, iCoreSubmesh)];

                // But check if it exists or not.
                if(m_pCoreModel->getCoreMaterial(iMatId)) {
                    m_pCoreModel->setCoreMaterialId(iMatThread, 0, iMatId);
                } else {
                    m_pCoreModel->setCoreMaterialId(iMatThread, 0, this->getOrCreateErrorMatId(in_sName));
                }
                iMatThread++;
            }
        }
    }

    this->loadHardware(nMaxTexturesPerMesh, 20);

    // And now load all the textures and shaders used in the materials.

    for(int iMat = 0 ; iMat < m_pCoreModel->getCoreMaterialCount() ; ++iMat) {
        CalCoreMaterial *pCoreMat = m_pCoreModel->getCoreMaterial(iMat);

        for(int iTex = 0 ; iTex < pCoreMat->getMapCount() ; ++iTex) {
            // We prepend the model's name to the name of the graphic in order
            // to get model-unique graphic names.
            String sTexName = pCoreMat->getMapFilename(iTex);
            String sFullName = in_sName + ":" + sTexName;

            // Check if the file has already been loaded from within the archive?
            if(std::find(m_loadedTexs.begin(), m_loadedTexs.end(), sFullName) != m_loadedTexs.end()) {
                pCoreMat->setMapUserData(iTex, new String(sFullName));
            } else {
                // The file isn't located in the archive? Look if a file with
                // the same name exists within our data folder:
                Graphic* pGraph = GraphicManager::getSingleton().getOrLoadGraphic(sTexName);
                if(pGraph != GraphicManager::getSingleton().getErrorTexture()) {
                    pCoreMat->setMapUserData(iTex, new String(sTexName));
                } else {
                    // Or else use the error file.
                    pCoreMat->setMapUserData(iTex, new String(GraphicManager::ErrorTextureName));
                }
            }
        }

        bool bIsModelNonStatic = m_pCoreModel->getCoreSkeleton()->getNumCoreBones() > 0;
        MaterialUserData* pUD = new MaterialUserData(*pCoreMat, in_sName, bIsModelNonStatic);
        pCoreMat->setUserData(pUD);

        // The shaders must already have been loaded: either it is a
        // builtin FTS shader or it is embedded in the model archive,
        // thus has been loaded already.

        /// \TODO: What is when there was an error in one of the above shaders? Is the message shown? Is the default shader used?
        Shader* pShad = ShaderManager::getSingleton().getOrLinkShader(pUD->sVertShaderName, pUD->sFragShaderName, pUD->sGeomShaderName);

        // And store the vertex attribute associations into memory.
        pUD->vao.bind();
        pShad->setVertexAttribute("aVertexPosition", *m_pVertexVBO);
        pShad->setVertexAttribute("aVertexNormal", *m_pNormalVBO);
        pShad->setVertexAttribute("aBoneIndicesAndWeights", *m_pMatIdxAndWeightVBO);

        // Just send out all the texture coordinates, even if this material uses
        // less textures, the shader might want the coordinates.
        for(int i = 0 ; i < nMaxTexturesPerMesh ; ++i) {
            pShad->setVertexAttribute("aVertexTexCo" + String::nr(i), *m_pTexCoVBOs[i]);
        }

        // Here, we check if the shader expects more texture coordinate attributes
        // then we send him. If so, print out a warning:
        if(pShad->hasVertexAttribute("aVertexTexCo" + String::nr(nMaxTexturesPerMesh))) {
            FTS18N("Model_Shader_TooManyTexCo", MsgType::Warning, in_sName, pCoreMat->getName(), pUD->sVertShaderName, String::nr(nMaxTexturesPerMesh));
        }

        // The same goes for the uniforms:
        if(pShad->hasUniform("uTexture" + String::nr(pCoreMat->getMapCount()))) {
            FTS18N("Model_Shader_TooManyTexUniforms", MsgType::Warning, in_sName, pCoreMat->getName(), pUD->sFragShaderName, String::nr(pCoreMat->getMapCount()));
        }

        m_pVtxIdxVBO->bind();
        pUD->vao.unbind();
    }

    /// \TODO: Model animations

    /// \TODO: Model attach points
}
*/
FTS::HardwareModel::~HardwareModel()
{
    // Throw all the textures away..
    for(auto s = m_loadedTexs.begin() ; s != m_loadedTexs.end() ; ++s) {
        GraphicManager::getSingleton().destroyGraphic(*s);
    }

    // And the shaders away..
    for(auto s = m_loadedShads.begin() ; s != m_loadedShads.end() ; ++s) {
        ShaderManager::getSingleton().destroyShader(*s);
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
    static const std::string uAmbient = "uAmbient";
    static const std::string ambient = "ambient";
    static const std::string uDiffuse = "uDiffuse";
    static const std::string diffuse = "diffuse";
    static const std::string uSpecular = "uSpecular";
    static const std::string specular = "specular";
    static const std::string uShininess = "uShininess";
    static const std::string shininess = "shininess";
    static const std::string uDiffTex = "uTexture";

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

        // What about qNormalMatrix??

        if(pMat->hasProprety(ambient)) {
            prog->setUniform(uAmbient, Vector(&pMat->propretyAsFvec(ambient)[0]));
        }

        if(pMat->hasProprety(diffuse)) {
            prog->setUniform(uDiffuse, Vector(&pMat->propretyAsFvec(diffuse)[0]));
        }

        if(pMat->hasProprety(specular)) {
            prog->setUniform(uSpecular, Vector(&pMat->propretyAsFvec(specular)[0]));
        }

        if(pMat->hasProprety(shininess)) {
            prog->setUniform(uShininess, pMat->propretyAsFvec(shininess)[0]);
        }

        if(pMat->userData) {
//             static_cast<TextureUserData*>(pMat->userData.get())->tex->selectTexture(0);
//             pShad->setUniformSampler(uDiffTex, 0);
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
