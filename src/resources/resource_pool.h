#ifndef OGL_ENV_RESOURCE_POOL_H
#define OGL_ENV_RESOURCE_POOL_H

#include "texture_loader.h"
#include "model_render.h"
#include <resource_loader/font_loader.h>

struct GLResourcePool {
    GLResourcePool(Resource::Pool pool, RenderConfig config);
    ~GLResourcePool();

    void loadGpu();
    void unloadStaged();
    void unloadGPU();

    TextureLoaderGL* texLoader;
    InternalFontLoader* fontLoader;
    GLModelRender* modelLoader;
    Resource::Pool poolID;
    bool usingGPUResources = false;
};

#endif
