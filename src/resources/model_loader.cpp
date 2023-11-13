#include "model_loader.h"
#include <graphics/logger.h>

struct GLMesh : public GPUMesh {
  GLVertexData *vertexData;
};

struct GPUModelGL : public GPUModel {
  std::vector<GLMesh> meshes;
  template <typename T_Vert>
  GPUModelGL(LoadedModel<T_Vert> data) {
      T_Vert vert = T_Vert();
      meshes.resize(data->meshes.size());
      for (int i = 0; i < data->meshes.size(); i++) {
	  Mesh<T_Vert> *mesh = data->meshes[i];
	  meshes[i].vertexData = new GLVertexData(mesh->verticies, mesh->indices);
	  meshes[i].load(mesh);
      }
      setAnimations(data);
    }   
    ~GPUModelGL() {
	for (auto &mesh : meshes)
	    delete mesh.vertexData;
    }       
};

ModelLoaderGL::ModelLoaderGL(Resource::Pool pool, InternalTexLoader *texLoader)
    : InternalModelLoader(pool, texLoader) {}

void ModelLoaderGL::clearGPU() {
    for (GPUModelGL *model : models)
	delete model;
    models.clear();      
}

void ModelLoaderGL::loadGPU() {
    clearGPU();
    loadQuad();
    models.resize(currentIndex);
    for(auto &model: stage2D.models)
	models.push_back(new GPUModelGL(model));
    for(auto &model: stage3D.models)
	models.push_back(new GPUModelGL(model));
    for(auto &model: stageAnim3D.models)
	models.push_back(new GPUModelGL(model));
    clearStaged();
}

  void ModelLoaderGL::DrawQuad(int count) {
      models[quad.ID]->meshes[0].vertexData->DrawInstanced(GL_TRIANGLES, count);
  }

void ModelLoaderGL::DrawModel(Resource::Model model, uint32_t spriteColourShaderLoc) {
    draw(model, glm::vec4(1.0f), 1, spriteColourShaderLoc, 0);
  }

void ModelLoaderGL::DrawModelInstanced(Resource::Model model, glm::vec4 colour,
				       int count, uint32_t spriteColourShaderLoc,
				       uint32_t enableTexShaderLoc) {
    draw(model, colour, count, spriteColourShaderLoc, enableTexShaderLoc);
}

void ModelLoaderGL::draw(Resource::Model model, glm::vec4 colour, int count,
                         uint32_t colLoc, uint32_t enableTexLoc) {
    if(model.ID >= models.size()) {
	LOG_ERROR("Model Draw: ID out of range. ID: "
		  << model.ID << " - model size: "
		  << models.size());
	return;
    }

    for (auto& mesh: models[model.ID]->meshes) {
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

Resource::ModelAnimation ModelLoaderGL::getAnimation(Resource::Model model,
                                                     std::string animation) {
    if (model.ID >= models.size()) {
        LOG_ERROR("Requested animation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    if (models[model.ID]->animationMap.find(animation) == models[model.ID]->animationMap.end()) {
        LOG_ERROR("No animation called " << animation << " could be found in the"
		  " animation map for model with id" << model.ID);
	return Resource::ModelAnimation();
    }        
    return getAnimation(model, models[model.ID]->animationMap[animation]);       
}

Resource::ModelAnimation ModelLoaderGL::getAnimation(Resource::Model model, int index) {
    if (model.ID >= models.size()) {
        LOG_ERROR("Requested animation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    if (index >= models[model.ID]->animations.size()) {
        LOG_ERROR("Model animation index was out of range. "
                  "model id: "
                  << model.ID << " index: " << index
                  << " - size: " << models[model.ID]->animations.size());
	return Resource::ModelAnimation();
    }        
    return models[model.ID]->animations[index];
}
