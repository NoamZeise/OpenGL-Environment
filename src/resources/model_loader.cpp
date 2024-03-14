#include "model_loader.h"
#include <graphics/logger.h>

struct GLMesh : public GPUMesh {
    GLVertexData *vertexData;
    void draw(glm::vec4 colour, int instanceCount, InternalTexLoader *texLoader,
	      int colLoc, int enableTexLoc);
};

struct GPUModelGL : public GPUModel {
    std::vector<GLMesh> meshes;    
    template <typename T_Vert>
    GPUModelGL(LoadedModel<T_Vert> &data);    
    ~GPUModelGL();
    void draw(glm::vec4 colour, int instanceCount, InternalTexLoader *texLoader, int colLoc,
	      int enableTexLoc);
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

void ModelLoaderGL::DrawModel(Resource::Model model, uint32_t spriteColourShaderLoc, uint32_t enableTexShaderLoc) {
    draw(model, 1, spriteColourShaderLoc, enableTexShaderLoc);
  }

void ModelLoaderGL::DrawModelInstanced(Resource::Model model,
				       int count, uint32_t spriteColourShaderLoc,
				       uint32_t enableTexShaderLoc) {
    draw(model, count, spriteColourShaderLoc, enableTexShaderLoc);
}

void ModelLoaderGL::draw(Resource::Model model, int count,
                         uint32_t colLoc, uint32_t enableTexLoc) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in draw with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return;
    }
    models[model.ID]->draw(model.colour, count, texLoader, colLoc, enableTexLoc);

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


///  ---  GL MESH  ---

void GLMesh::draw(glm::vec4 colour, int instanceCount, InternalTexLoader *texLoader,
		  int colLoc, int enableTexLoc) {
    glActiveTexture(GL_TEXTURE0);
    if(texture.ID != Resource::NULL_TEX_ID) {
	glUniform1i(enableTexLoc, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D, texLoader->getViewIndex(texture));
    } else {
	glUniform1i(enableTexLoc, GL_FALSE);
    }
    glUniform4fv(colLoc, 1,
		 colour.a == 0.0f ? &diffuseColour[0] :
		 &colour[0]);
    
    if(instanceCount > 1)
	vertexData->DrawInstanced(GL_TRIANGLES, instanceCount);
    else
	vertexData->Draw(GL_TRIANGLES);
}

/// --- GPU Model ---


template<typename T_Vert>
GPUModelGL::GPUModelGL(LoadedModel<T_Vert> &data) : GPUModel(data) {
    meshes.resize(data.meshes.size());
    for (int i = 0; i < meshes.size(); i++) {
	Mesh<T_Vert> *mesh = data.meshes[i];
	meshes[i].vertexData = new GLVertexData(mesh->verticies, mesh->indices);
	meshes[i].load(mesh);
    }
}

GPUModelGL::~GPUModelGL() {
    for (auto &mesh : meshes)
	delete mesh.vertexData;
}

void GPUModelGL::draw(glm::vec4 colour, int instanceCount, InternalTexLoader *texLoader, int colLoc,
		      int enableTexLoc) {
    for (auto& mesh: meshes) {
	mesh.draw(colour, instanceCount, texLoader, colLoc, enableTexLoc);
    }
}
