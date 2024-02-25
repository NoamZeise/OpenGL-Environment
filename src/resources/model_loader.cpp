#include "model_loader.h"
#include <graphics/logger.h>

struct GLMesh : public GPUMesh {
    GLVertexData *vertexData;

    void draw(glm::vec4 colour, int instanceCount, InternalTexLoader *texLoader,
	      int colLoc, int enableTexLoc, Resource::Texture *tex) {
	glActiveTexture(GL_TEXTURE0);
	if(tex != nullptr) {
	    glBindTexture(GL_TEXTURE_2D, texLoader->getViewIndex(*tex));
	}
	else if(texture.ID != UINT32_MAX)
	    glBindTexture(GL_TEXTURE_2D, texLoader->getViewIndex(texture));
	
        if(texture.ID == UINT32_MAX && tex == nullptr) {
	    glUniform1i(enableTexLoc, GL_FALSE);
	} else {
	    glUniform1i(enableTexLoc, GL_TRUE);
	}
	
	glUniform4fv(colLoc, 1,
		     colour.a == 0.0f ? &diffuseColour[0] :
		     &colour[0]);
	
	if(instanceCount > 1)
	    vertexData->DrawInstanced(GL_TRIANGLES, instanceCount);
	else
	    vertexData->Draw(GL_TRIANGLES);
    }
};

struct GPUModelGL : public GPUModel {
  std::vector<GLMesh> meshes;
    
  template <typename T_Vert>
  GPUModelGL(LoadedModel<T_Vert> &data) : GPUModel(data) {
      meshes.resize(data.meshes.size());
      for (int i = 0; i < meshes.size(); i++) {
	  Mesh<T_Vert> *mesh = data.meshes[i];
	  meshes[i].vertexData = new GLVertexData(mesh->verticies, mesh->indices);
	  meshes[i].load(mesh);
      }
    }
    
    ~GPUModelGL() {
	for (auto &mesh : meshes)
	    delete mesh.vertexData;
    }

    void draw(glm::vec4 colour, int instanceCount, InternalTexLoader *texLoader, int colLoc,
	      int enableTexLoc, Resource::Texture *tex) {
	for (auto& mesh: meshes) {
	    mesh.draw(colour, instanceCount, texLoader, colLoc, enableTexLoc, tex);
	}
    }
};

ModelLoaderGL::ModelLoaderGL(Resource::Pool pool, InternalTexLoader *texLoader)
    : InternalModelLoader(pool, texLoader) {}

ModelLoaderGL::~ModelLoaderGL() {
    clearGPU();
}

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
	models[model.ID] = new GPUModelGL(model);
    for(auto &model: stage3D.models)
	models[model.ID] = new GPUModelGL(model);
    for(auto &model: stageAnim3D.models)
	models[model.ID] = new GPUModelGL(model);
    clearStaged();
}

  void ModelLoaderGL::DrawQuad(int count) {
      models[quad.ID]->meshes[0].vertexData->DrawInstanced(GL_TRIANGLES, count);
  }

void ModelLoaderGL::DrawModel(Resource::Model model,
			      uint32_t spriteColourShaderLoc, uint32_t enableTexShaderLoc) {
    draw(model, nullptr, glm::vec4(1.0f), 1, spriteColourShaderLoc, enableTexShaderLoc);
  }

void ModelLoaderGL::DrawModelInstanced(Resource::Model model,
				       Resource::Texture *tex, glm::vec4 colour,
				       int count, uint32_t spriteColourShaderLoc,
				       uint32_t enableTexShaderLoc) {
    draw(model, tex, colour, count, spriteColourShaderLoc, enableTexShaderLoc);
}

void ModelLoaderGL::draw(Resource::Model model, Resource::Texture *tex, glm::vec4 colour, int count,
                         uint32_t colLoc, uint32_t enableTexLoc) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in draw with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return;
    }
    models[model.ID]->draw(colour, count, texLoader, colLoc, enableTexLoc, tex);

}

Resource::ModelAnimation ModelLoaderGL::getAnimation(Resource::Model model,
                                                     std::string animation) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in getAnimation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    return models[model.ID]->getAnimation(animation);
}

Resource::ModelAnimation ModelLoaderGL::getAnimation(Resource::Model model, int index) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in getAnimation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    return models[model.ID]->getAnimation(index);
}
