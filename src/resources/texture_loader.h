#ifndef GLTEXTURE_LOADER_H
#define GLTEXTURE_LOADER_H

#include <glad/glad.h>
#include <graphics/resources.h>
#include <vector>

namespace Resource
{

  struct GLStagedTex;
  struct GLGPUTex;
  
  class GLTextureLoader {
  public:
      GLTextureLoader(bool mipmapping, bool filterNearest, Resource::ResourcePool pool);
      ~GLTextureLoader();
      Texture LoadTexture(std::string path);
      Texture LoadTexture(unsigned char* data, int width, int height, int nrChannels);
      void Bind(Texture tex);

      void loadToGPU();
      void clearStaged();
      void clearGPU();
  private:
      Texture stageTex(unsigned char* data, int width, int height, int nrChannels,
		       std::string path);
      
      Resource::ResourcePool pool;
      bool mipmapping;
      bool filterNearest;

      std::vector<GLStagedTex> staged;
      std::vector<GLuint> inGpu;
  };    
}
#endif
