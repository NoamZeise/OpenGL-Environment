#include "model_render.h"

#include <assimp/material.h>
#include <assimp/types.h>
#include <assimp/matrix4x4.h>
#include <iostream>

namespace Resource
{

    const auto IMPORT_PROPS =
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_LimitBoneWeights;


GLModelRender::GLModelRender()
{

}

GLModelRender::~GLModelRender() {
    for(unsigned int i = 0; i < loadedModels.size(); i++)
	delete loadedModels[i];
}

GLModelRender::GLLoadedModel::~GLLoadedModel() {
    for (unsigned int j = 0; j < meshes.size(); j++)
        delete meshes[j].vertexData;
}

template <class T_Vert>
void GLModelRender::addLoadedModel(LoadedModel<T_Vert> &modelData, GLTextureLoader *texLoader) {
    loadedModels.push_back(new GLLoadedModel());
    GLLoadedModel* model = loadedModels.back();
    for(Mesh<T_Vert> *meshData: modelData.meshes) {
	model->meshes.push_back(GLMesh());
	GLMesh* mesh = &model->meshes[model->meshes.size() - 1];
	mesh->vertexData = new GLVertexData(meshData->verticies, meshData->indicies);
	mesh->diffuseColour = meshData->diffuseColour;
	if(meshData->texToLoad != "") {
	    std::string loadStr = checkTextureLoaded(meshData->texToLoad,
						     this->loadedTextures,
						     &mesh->texture);
	    if(loadStr != "")
		mesh->texture = texLoader->LoadTexture(loadStr);
	}
    }
}

  Model GLModelRender::LoadModel(std::string path, GLTextureLoader* texLoader) {
#ifndef NO_ASSIMP

#ifndef NDEBUG
      std::cout << "loading model: " << path << std::endl;
#endif
      Model userModel(currentIndex);
      ModelInfo::Model fileModel = modelLoader.LoadModel(path);
      loaded3D.loadModel(fileModel, currentIndex);
      addLoadedModel(loaded3D.models[loaded3D.models.size() - 1], texLoader);
      loaded3D.clearData();
      currentIndex++;
      return userModel;
#else
      throw std::runtime_error("tried to load model but NO_ASSIMP is defined!");
#endif
  }

  void GLModelRender::DrawModel(Model model, GLTextureLoader* texLoader,
				uint32_t spriteColourShaderLoc){
      if(model.ID >= loadedModels.size()) {
	  std::cerr << "model ID out of range" << std::endl;
	  return;
      }

      for (auto& mesh: loadedModels[model.ID]->meshes) {
	  glActiveTexture(GL_TEXTURE0);
	  texLoader->Bind(mesh.texture);
	  glUniform4fv(spriteColourShaderLoc, 1, &mesh.diffuseColour[0]);
	    
	  mesh.vertexData->Draw(GL_TRIANGLES);
      }
  }

  void GLModelRender::DrawModelInstanced(Model model, GLTextureLoader* texLoader,
					 int count, uint32_t spriteColourShaderLoc,
					 uint32_t enableTexShaderLoc) {
      if(model.ID >= loadedModels.size()) {
	  std::cerr << "model ID out of range" << std::endl;
	  return;
      }

      for (auto& mesh: loadedModels[model.ID]->meshes) {
	  glActiveTexture(GL_TEXTURE0);
	  texLoader->Bind(mesh.texture);
	  glUniform4fv(spriteColourShaderLoc, 1, &mesh.diffuseColour[0]);
	  if(mesh.texture.ID == 0)
	      glUniform1i(enableTexShaderLoc, GL_FALSE);
	  else
	      glUniform1i(enableTexShaderLoc, GL_TRUE);
	  mesh.vertexData->DrawInstanced(GL_TRIANGLES, count);
      }
  }
  
}
