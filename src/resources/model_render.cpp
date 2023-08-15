#include "model_render.h"
#include <logger.h>

namespace Resource
{
GLModelRender::GLModelRender() {
    loadQuad();
}

GLModelRender::~GLModelRender() {
    for(unsigned int i = 0; i < loadedModels.size(); i++)
	delete loadedModels[i];
}

void GLModelRender::loadQuad() {
    ModelInfo::Model quad = makeQuadModel();
    quadIndex = loadModel(ModelType::m2D, quad, nullptr, nullptr).ID;
}
  

GLModelRender::GLLoadedModel::~GLLoadedModel() {
    for (unsigned int j = 0; j < meshes.size(); j++)
        delete meshes[j].vertexData;
}

template <class T_Vert>
void GLModelRender::addLoadedModel(LoadedModel<T_Vert>* modelData, GLTextureLoader *texLoader) {
    loadedModels.push_back(new GLLoadedModel());
    GLLoadedModel* model = loadedModels.back();
    for(Mesh<T_Vert> *meshData: modelData->meshes) {
	model->meshes.push_back(GLMesh());
	GLMesh* mesh = &model->meshes[model->meshes.size() - 1];
	mesh->vertexData = new GLVertexData(meshData->verticies, meshData->indicies);
	mesh->diffuseColour = meshData->diffuseColour;
	if(meshData->texToLoad != "") {
	    mesh->texture = texLoader->LoadTexture(MODEL_TEXTURE_LOCATION + meshData->texToLoad);
	}
    }
}

  ModelInfo::Model GLModelRender::loadModelFromFile(std::string path) {
#ifndef NO_ASSIMP
      LOG("loading model: " << path);

      return modelLoader.LoadModel(path);
#else
      throw std::runtime_error("tried to load model from file, but NO_ASSIMP is defined"
" so that feature is disabled!");
#endif
  }

  template <class T_Vert>
  Model GLModelRender::loadModelInfo(ModelInfo::Model& model,
				     ModelGroup<T_Vert>& modelGroup,
				     GLTextureLoader* texLoader,
				     std::vector<ModelAnimation> *pGetAnimations) {
      Model userModel(currentIndex++);
      userModel.type = getModelType(T_Vert());
      modelGroup.loadModel(model, userModel.ID);
      addLoadedModel(modelGroup.getPreviousModel(), texLoader);
      if(pGetAnimations != nullptr)
	  for(ModelInfo::Animation anim : model.animations) {
	      loadedAnim3D.getPreviousModel()->animations
		  .push_back(ModelAnimation(model.bones, anim));
	      pGetAnimations->push_back(
		      loadedAnim3D.getPreviousModel()->
		      animations[loadedAnim3D.getPreviousModel()->animations.size() - 1]);
	  }
      modelGroup.clearData();
      return userModel;
  }

  Model GLModelRender::loadModel(ModelType type, std::string path, GLTextureLoader *texLoader,
				 std::vector<ModelAnimation> *pGetAnimations) {
      ModelInfo::Model fileModel = loadModelFromFile(path);
      return loadModel(type, fileModel, texLoader, pGetAnimations);
  }

  Model GLModelRender::loadModel(ModelType type, ModelInfo::Model& model,
				 GLTextureLoader *texLoader,
				 std::vector<ModelAnimation> *pGetAnimations) {
      switch(type) {
      case ModelType::m2D:
	  return loadModelInfo(model, loaded2D, texLoader, pGetAnimations);
      case ModelType::m3D:
	  return loadModelInfo(model, loaded3D, texLoader, pGetAnimations);
      case ModelType::m3D_Anim:
	  return loadModelInfo(model, loadedAnim3D, texLoader, pGetAnimations);
      }
      throw std::runtime_error("tried to load model with unrecognized type");
  }

  void GLModelRender::DrawQuad(int count) {
      loadedModels[quadIndex]->meshes[0].vertexData->DrawInstanced(GL_TRIANGLES, count);
  }

  void GLModelRender::DrawModel(Model model, GLTextureLoader* texLoader,
				uint32_t spriteColourShaderLoc) {
      if(model.ID >= loadedModels.size()) {
	  LOG_ERROR("model ID out of range");
	  return;
      }

      for (auto& mesh: loadedModels[model.ID]->meshes) {
	  glActiveTexture(GL_TEXTURE0);
	  texLoader->Bind(mesh.texture);
	  glUniform4fv(spriteColourShaderLoc, 1, &mesh.diffuseColour[0]);
	    
	  mesh.vertexData->Draw(GL_TRIANGLES);
      }
  }

  void GLModelRender::DrawModelInstanced(Model model, glm::vec4 colour,
					 GLTextureLoader* texLoader,
					 int count, uint32_t spriteColourShaderLoc,
					 uint32_t enableTexShaderLoc) {
      if(model.ID >= loadedModels.size()) {
	  LOG_ERROR("model ID out of range");
	  return;
      }

      for (auto& mesh: loadedModels[model.ID]->meshes) {
	  glActiveTexture(GL_TEXTURE0);
	  texLoader->Bind(mesh.texture);
	  glUniform4fv(spriteColourShaderLoc, 1,
		       colour.a == 0.0f ? &mesh.diffuseColour[0] :
		       &colour[0]);
	  if(mesh.texture.ID == 0)
	      glUniform1i(enableTexShaderLoc, GL_FALSE);
	  else
	      glUniform1i(enableTexShaderLoc, GL_TRUE);
	  mesh.vertexData->DrawInstanced(GL_TRIANGLES, count);
      }
  }
  
}
