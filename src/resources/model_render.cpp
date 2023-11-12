#include "model_render.h"
#include <graphics/logger.h>

GLModelRender::GLModelRender(Resource::Pool pool, InternalTexLoader *texLoader)
    : InternalModelLoader(pool, texLoader) {}

GLModelRender::GLLoadedModel::~GLLoadedModel() {
    for (unsigned int i = 0; i < meshes.size(); i++)
        delete meshes[i].vertexData;
}

template <class T_Vert>
void GLModelRender::addLoadedModel(LoadedModel<T_Vert>* modelData) {
    loadedModels[modelData->ID] =  new GLLoadedModel();
    GLLoadedModel* model = loadedModels[modelData->ID];
    for(Mesh<T_Vert> *meshData: modelData->meshes) {
	model->meshes.push_back(GLMesh());
	GLMesh* mesh = &model->meshes[model->meshes.size() - 1];
	mesh->vertexData = new GLVertexData(meshData->verticies, meshData->indicies);
	mesh->diffuseColour = meshData->diffuseColour;
	mesh->texture = meshData->texture;
    }
}

  void GLModelRender::clearGPU() {
      for(unsigned int i = 0; i < loadedModels.size(); i++)
	  delete loadedModels[i];
      loadedModels.clear();
  }

  void GLModelRender::loadGPU() {
      clearGPU();
      loadQuad();
      loadedModels.resize(currentIndex);
      for(auto &model: stage2D.models)
	  addLoadedModel(&model);
      for(auto &model: stage3D.models)
	  addLoadedModel(&model);
      for(auto &model: stageAnim3D.models)
	  addLoadedModel(&model);
      clearStaged();
  }

  void GLModelRender::DrawQuad(int count) {
      loadedModels[quad.ID]->meshes[0].vertexData->DrawInstanced(GL_TRIANGLES, count);
  }

void GLModelRender::DrawModel(Resource::Model model, uint32_t spriteColourShaderLoc) {
    draw(model, glm::vec4(1.0f), 1, spriteColourShaderLoc, 0);
  }

void GLModelRender::DrawModelInstanced(Resource::Model model, glm::vec4 colour,
				       int count, uint32_t spriteColourShaderLoc,
				       uint32_t enableTexShaderLoc) {
    draw(model, colour, count, spriteColourShaderLoc, enableTexShaderLoc);
}

void GLModelRender::draw(Resource::Model model, glm::vec4 colour, int count,
                         uint32_t colLoc, uint32_t enableTexLoc) {
    if(model.ID >= loadedModels.size()) {
	LOG_ERROR("Model Draw: ID out of range. ID: "
		  << model.ID << " - model size: "
		  << loadedModels.size());
	return;
    }

    for (auto& mesh: loadedModels[model.ID]->meshes) {
	glActiveTexture(GL_TEXTURE0);
	if(mesh.texture.ID != UINT32_MAX)
	    glBindTexture(GL_TEXTURE_2D, texLoader->getViewIndex(mesh.texture));
	glUniform4fv(colLoc, 1,
		     colour.a == 0.0f ? &mesh.diffuseColour[0] :
		     &colour[0]);
	if(enableTexLoc != 0) {
	    if(mesh.texture.ID == UINT32_MAX)
		glUniform1i(enableTexLoc, GL_FALSE);
	    else
		glUniform1i(enableTexLoc, GL_TRUE);
	}
	if(count > 1)
	    mesh.vertexData->DrawInstanced(GL_TRIANGLES, count);
	else
	    mesh.vertexData->Draw(GL_TRIANGLES);
    }
}

Resource::ModelAnimation GLModelRender::getAnimation(Resource::Model model,
                                                     std::string animation) {
    return Resource::ModelAnimation();
}

Resource::ModelAnimation GLModelRender::getAnimation(Resource::Model model, int index) {
    return Resource::ModelAnimation();
}
