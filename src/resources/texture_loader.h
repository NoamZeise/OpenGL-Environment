#ifndef GLTEXTURE_LOADER_H
#define GLTEXTURE_LOADER_H

#include <graphics/resources.h>
#include <vector>

namespace Resource
{

  struct GLStagedTex;
  
  class GLTextureLoader {
  public:
      GLTextureLoader(bool mipmapping, bool filterNearest);
      ~GLTextureLoader();
      Texture LoadTexture(std::string path);
      Texture LoadTexture(unsigned char* data, int width, int height, int nrChannels);
      void Bind(Texture tex);

      void loadToGPU();
      void clearStaged();
  private:
	
      struct LoadedTex {
	  LoadedTex(std::string path, bool mipmapping, bool pixelated);
	  LoadedTex(unsigned char* data, int width, int height,
		    int nrChannels, bool mipmapping, bool pixelated);
	  ~LoadedTex();
	  void Bind();
	  Texture getTexture(uint32_t ID);
	  unsigned int glID;
	  int width;
	  int height;
	  std::string path;
	    
      private:
	  void generateTexture(unsigned char* data, int width, int height,
			       int nrChannels, bool mipmapping, bool pixelated);
      };

      Texture stageTex(unsigned char* data, int width, int height, int nrChannels,
		       std::string path);

      bool mipmapping;
      bool filterNearest;

      std::vector<GLStagedTex> staged;
      std::vector<LoadedTex*> textures;
  };    
}
#endif
