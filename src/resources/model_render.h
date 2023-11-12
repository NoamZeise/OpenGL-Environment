#ifndef GL_MODEL_RENDER_H
#define GL_MODEL_RENDER_H

#include "vertex_data.h"
#include <resource_loader/texture_loader.h>
#include <resource_loader/model_loader.h>
#include <resource_loader/vertex_model.h>

class GLModelRender : public InternalModelLoader {
public:
    GLModelRender(Resource::Pool pool, InternalTexLoader *texLoader);
    void loadGPU() override;
    void clearGPU() override;
    void DrawQuad(int count);
    void DrawModel(Resource::Model model, uint32_t spriteColourShaderLoc);
    void DrawModelInstanced(Resource::Model model, glm::vec4 colour, int count,
			    uint32_t spriteColourShaderLoc,
			    uint32_t enableTexShaderLoc);
    Resource::ModelAnimation getAnimation(Resource::Model model, std::string animation) override;
    Resource::ModelAnimation getAnimation(Resource::Model model, int index) override;

private:
    struct GLMesh {
	GLVertexData *vertexData;
	Resource::Texture texture;
	glm::vec4 diffuseColour;
    };
    struct GLLoadedModel {
	GLLoadedModel(){}
	~GLLoadedModel();
	std::vector<GLMesh> meshes;
	std::string directory;
    };
    template <class T_Vert>
    void addLoadedModel(LoadedModel<T_Vert>* model);
    std::vector<GLLoadedModel *> loadedModels;
    void draw(Resource::Model model, glm::vec4 colour, int count,
	      uint32_t colLoc, uint32_t enableTexLoc);
};

#endif
