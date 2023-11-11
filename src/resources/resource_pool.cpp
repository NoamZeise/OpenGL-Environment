#include "resource_pool.h"

#include <graphics/logger.h>

GLResourcePool::GLResourcePool(Resource::ResourcePool pool, RenderConfig config) {
    poolID = pool;
    texLoader = new TextureLoaderGL(pool, config);
    modelLoader = new Resource::GLModelRender(poolID);
    fontLoader = new InternalFontLoader(poolID, texLoader);
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
    texLoader->loadGPU();
    fontLoader->loadGPU();
    modelLoader->loadGPU();
    LOG("finished loading to GPU");
    usingGPUResources = true;
}

void GLResourcePool::unloadStaged() {
    texLoader->clearStaged();
    fontLoader->clearStaged();
    modelLoader->unloadStaged();
}

void GLResourcePool::unloadGPU() {
    texLoader->clearGPU();
    fontLoader->clearGPU();
    modelLoader->unloadGPU();
    usingGPUResources = false;
}
