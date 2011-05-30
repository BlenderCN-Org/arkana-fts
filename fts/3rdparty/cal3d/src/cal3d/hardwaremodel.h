//****************************************************************************//
// hardwaremodel.h                                                         //
// Copyright (C) 2004 Desmecht Laurent                                        //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_HARDWAREMODEL_H
#define CAL_HARDWAREMODEL_H


#include "cal3d/global.h"
#include "cal3d/coresubmesh.h"


class CalCoreModel;
class CalSkeleton;
class CalCoreMaterial;


class CAL3D_API CalHardwareModel
{
public:
  struct CalHardwareMesh
  {
    std::vector<int> m_vectorBonesIndices;
    
    int baseVertexIndex;
    int vertexCount;
    int startIndex;
    int faceCount;
    CalCoreMaterial *pCoreMaterial;

    int meshId,submeshId;
  };

public:
  CalHardwareModel(CalCoreModel *pCoreModel);
  ~CalHardwareModel() { }
  
  void setVertexBuffer( char *pVertexBuffer, int stride); 
  void setVertexBuffer( std::vector<float> *pVertexBuffer, int stride); 
  void setIndexBuffer( CalIndex *pIndexBuffer); 
  void setIndexBuffer( std::vector<CalIndex> *pIndexBuffer); 
  void setNormalBuffer( char *pNormalBuffer, int stride); 
  void setNormalBuffer( std::vector<float> *pNormalBuffer, int stride); 
  void setWeightBuffer( char *pWeightBuffer, int stride); 
  void setWeightBuffer( std::vector<float> *pWeightBuffer, int stride); 
  void setMatrixIndexBuffer( char *pMatrixIndexBuffer, int stride); 
  void setMatrixIndexBuffer( std::vector<float> *pMatrixIndexBuffer, int stride); 
  void setTextureCoordNum(size_t textureCoordNum);
  void setTextureCoordBuffer(int mapId, char *pTextureCoordBuffer, int stride);
  void setTextureCoordBuffer(int mapId, std::vector<float> *pTextureCoordBuffer, int stride);
  void setTangentSpaceBuffer(int mapId, char *pTangentSpaceBuffer, int stride);
  void setTangentSpaceBuffer(int mapId, std::vector<float> *pTangentSpaceBuffer, int stride);
  void setCoreMeshIds(const std::vector<int>& coreMeshIds);

  bool load(int baseVertexIndex, int startIndex,size_t maxBonesPerMesh);
  bool loadDyn(int baseVertexIndex, int startIndex,size_t maxBonesPerMesh);
      
  std::vector<CalHardwareMesh> & getVectorHardwareMesh();
  const std::vector<CalHardwareMesh> & getVectorHardwareMesh() const;
  void getAmbientColor(unsigned char *pColorBuffer) const;
  void getDiffuseColor(unsigned char *pColorBuffer) const;
  void getSpecularColor(unsigned char *pColorBuffer) const;
  const CalQuaternion & getRotationBoneSpace(int boneId, const CalSkeleton *pSkeleton) const;
  const CalVector & getTranslationBoneSpace(int boneId, const CalSkeleton *pSkeleton) const;

  float getShininess() const;
  
  int getHardwareMeshCount() const;
  int getFaceCount() const;
  int getVertexCount() const;
  int getBoneCount() const;

  int getBaseVertexIndex() const;
  int getStartIndex() const;

  int getTotalFaceCount() const;
  int getTotalVertexCount() const;

  Cal::UserData getMapUserData(int mapId);
  const Cal::UserData getMapUserData(int mapId) const;
  
  bool selectHardwareMesh(size_t meshId);
  
private:
  bool canAddFace(CalHardwareMesh &hardwareMesh, CalCoreSubmesh::Face & face,std::vector<CalCoreSubmesh::Vertex>& vectorVertex, size_t maxBonesPerMesh) const;
  int  addVertex(CalHardwareMesh &hardwareMesh, int indice , CalCoreSubmesh *pCoreSubmesh, size_t maxBonesPerMesh);
  int  addVertexDyn(CalHardwareMesh &hardwareMesh, int indice , CalCoreSubmesh *pCoreSubmesh, size_t maxBonesPerMesh);
  int  addBoneIndice(CalHardwareMesh &hardwareMesh, int Indice, size_t maxBonesPerMesh);  
    

private:
  
  std::vector<CalHardwareMesh> m_vectorHardwareMesh;
  std::vector<CalIndex>        m_vectorVertexIndiceUsed;
  int                          m_selectedHardwareMesh;
  std::vector<int>             m_coreMeshIds;
  CalCoreModel                *m_pCoreModel;
  
  
  char *m_pVertexBuffer;
  std::vector<float> *m_pvVertexBuffer;
  int   m_vertexStride;
  char *m_pNormalBuffer;
  std::vector<float> *m_pvNormalBuffer;
  int   m_normalStride;
  char *m_pWeightBuffer;
  std::vector<float> *m_pvWeightBuffer;
  int   m_weightStride;
  char *m_pMatrixIndexBuffer;
  std::vector<float> *m_pvMatrixIndexBuffer;
  int   m_matrixIndexStride;
  char *m_pTextureCoordBuffer[8];
  std::vector<float> *m_pvTextureCoordBuffer[8];
  int   m_textureCoordStride[8];
  size_t m_textureCoordNum;  
  char *m_pTangentSpaceBuffer[8];
  std::vector<float> *m_pvTangentSpaceBuffer[8];
  int   m_tangentSpaceStride[8];
  
  CalIndex *m_pIndexBuffer;
  std::vector<CalIndex> *m_pvIndexBuffer;

  int m_totalVertexCount;
  int m_totalFaceCount;
};

#endif
