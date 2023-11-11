#ifndef OGL_ENV_RESOURCE_POOL_H
#define OGL_ENV_RESOURCE_POOL_H

#include "texture_loader.h"
#include "model_render.h"
#include "font_loader.h"
#include <graphics/render_config.h>

struct GLResourcePool {
    GLResourcePool(Resource::ResourcePool pool, RenderConfig config);
    ~GLResourcePool();

    Resource::Model loadModel(Resource::ModelType type, std::string path,
			      std::vector<Resource::ModelAnimation> *pGetAnimations);

    Resource::Model loadModel(Resource::ModelType type, ModelInfo::Model &model,
			      std::vector<Resource::ModelAnimation> *pGetAnimations);

    void loadGpu();
    void unloadStaged();
    void unloadGPU();

    TextureLoaderGL* texLoader;
    Resource::GLFontLoader* fontLoader;
    Resource::GLModelRender* modelLoader;
    Resource::ResourcePool poolID;
    bool usingGPUResources = false;
};

#endif
