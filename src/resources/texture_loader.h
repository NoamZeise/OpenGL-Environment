#ifndef GLTEXTURE_LOADER_H
#define GLTEXTURE_LOADER_H

#include <resources/resources.h>
#include <vector>

namespace Resource
{

  class GLTextureLoader
  {
  public:
    GLTextureLoader(bool mipmapping, bool pixelated);
    ~GLTextureLoader();
    Texture LoadTexture(std::string path);
    Texture LoadTexture(unsigned char* data, int width, int height, int nrChannels);
    void Bind(Texture tex);
  private:
	
    struct LoadedTex
    {
      LoadedTex(std::string path, bool mipmapping, bool pixelated);
      LoadedTex(unsigned char* data, int width, int height, int nrChannels, bool mipmapping, bool pixelated);
      ~LoadedTex();
      void Bind();
      unsigned int ID;
      int width;
      int height;
	    
    private:
      void generateTexture(unsigned char* data, int width, int height, int nrChannels, bool mipmapping, bool pixelated);
    };

    bool mipmapping;
    bool pixelated;
    
    std::vector<LoadedTex*> textures;
  };    
}
#endif
