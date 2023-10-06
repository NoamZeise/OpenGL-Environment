#ifndef OGL_ENV_RESOURCE_POOL_H
#define OGL_ENV_RESOURCE_POOL_H

#include "texture_loader.h"
#include "model_render.h"
#include "font_loader.h"
#include <graphics/render_config.h>

struct GLResourcePool {
    GLResourcePool(uint32_t ID, RenderConfig config);
    ~GLResourcePool();

    void unloadStaged();
    void unloadGPU();

    Resource::GLTextureLoader* texLoader;
    Resource::GLFontLoader* fontLoader;
    Resource::GLModelRender* modelLoader;
    Resource::ResourcePool poolID;
    bool usingResources;
};

#endif
