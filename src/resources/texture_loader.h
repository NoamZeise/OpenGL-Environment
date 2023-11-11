#ifndef GLTEXTURE_LOADER_H
#define GLTEXTURE_LOADER_H

#include <glad/glad.h>
#include <resource_loader/texture_loader.h>
  
class TextureLoaderGL : public InternalTexLoader {
public:
    TextureLoaderGL(Resource::ResourcePool pool, RenderConfig conf);
    void Bind(Resource::Texture tex);
    void loadGPU() override;
    void clearGPU() override;
private:
    std::vector<GLuint> inGpu;
};    

#endif
