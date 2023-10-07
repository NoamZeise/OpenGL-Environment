#include "resource_pool.h"

#include <graphics/logger.h>

GLResourcePool::GLResourcePool(Resource::ResourcePool pool, RenderConfig config) {
    poolID = pool;
    texLoader = new Resource::GLTextureLoader(config.mip_mapping,
					      config.texture_filter_nearest,
					      poolID);
    fontLoader = new Resource::GLFontLoader(poolID);
    modelLoader = new Resource::GLModelRender(poolID);
}

GLResourcePool::~GLResourcePool() {
    delete texLoader;
    delete fontLoader;
    delete modelLoader;
}

Resource::Model GLResourcePool::loadModel(Resource::ModelType type, std::string path, std::vector<Resource::ModelAnimation> *pGetAnimations) {
    return modelLoader->loadModel(type, path, texLoader, pGetAnimations);
}

Resource::Model GLResourcePool::loadModel(Resource::ModelType type, ModelInfo::Model &model,
					std::vector<Resource::ModelAnimation> *pGetAnimations) {
    return modelLoader->loadModel(type, model, texLoader, pGetAnimations);
}


void GLResourcePool::loadGpu() {
    LOG("loading to GPU");
    texLoader->loadToGPU();
    fontLoader->loadToGPU();
    modelLoader->loadGPU();
    LOG("finished loading to GPU");
    usingGPUResources = true;
}

void GLResourcePool::unloadStaged() {
    texLoader->clearStaged();
    fontLoader->UnloadStaged();
    modelLoader->unloadStaged();
}

void GLResourcePool::unloadGPU() {
    texLoader->clearGPU();
    fontLoader->UnloadGpu();
    modelLoader->unloadGPU();
    usingGPUResources = false;
}
