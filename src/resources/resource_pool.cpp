#include "resource_pool.h"

GLResourcePool::GLResourcePool(uint32_t ID, RenderConfig config) {
    poolID = Resource::ResourcePool(ID);
    texLoader = new Resource::GLTextureLoader(config.mip_mapping,
					      config.texture_filter_nearest,
					      poolID);
    fontLoader = new Resource::GLFontLoader();
    modelLoader = new Resource::GLModelRender();
}

GLResourcePool::~GLResourcePool() {
    delete texLoader;
    delete fontLoader;
    delete modelLoader;
}
