#include "texture_loader.h"

#include <string>

#include <glad/glad.h>
#include <logger.h>
#include <resource_loader/stb_image.h>

#include "../ogl_helper.h"

namespace Resource {

  GLTextureLoader::LoadedTex::LoadedTex(std::string path, bool mipmapping, bool filterNearest) {
      LOG("loading texture: " << path);
      ID = 0;
      width = 0;
      height = 0;
      int nrChannels;
      unsigned char *data =
	  stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
      if (!data) {
	  LOG_ERROR("stb_image: failed to load texture at " << path);
	  return;
      }
      generateTexture(data, width, height, nrChannels, mipmapping, filterNearest);   
      stbi_image_free(data);
  }

GLTextureLoader::LoadedTex::LoadedTex(unsigned char *data, int width,
                                      int height, int nrChannels, bool mipmapping, bool filterNearest) {
  generateTexture(data, width, height, nrChannels, mipmapping, filterNearest);
}

void GLTextureLoader::LoadedTex::generateTexture(unsigned char *data, int width,
                                                 int height, int nrChannels,
						 bool mipmapping, bool filterNearest) {
  GLuint format = GL_RGBA;
  if (nrChannels == 1)
    format = GL_RED;
  else if (nrChannels == 3)
    format = GL_RGB;
  else if (nrChannels == 4)
    format = GL_RGBA;
  else {
      LOG_ERROR("failed to load texture, unsupported num of channels!");
    return;
  }
  
  ID = ogl_helper::genTexture(format, width, height, data, mipmapping,
			      filterNearest ? GL_NEAREST : GL_LINEAR,
			      GL_REPEAT, 1);
}

  GLTextureLoader::LoadedTex::~LoadedTex() { glDeleteTextures(1, &ID); }
  
  void GLTextureLoader::LoadedTex::Bind() { glBindTexture(GL_TEXTURE_2D, ID); }

  GLTextureLoader::GLTextureLoader(bool mipmapping, bool pixelated) {
    this->mipmapping = mipmapping;
    this->filterNearest = pixelated;
  }

GLTextureLoader::~GLTextureLoader() {
  for (unsigned int i = 0; i < textures.size(); i++)
    delete textures[i];
}

Texture GLTextureLoader::LoadTexture(std::string path) {
  textures.push_back(new LoadedTex(path, mipmapping, filterNearest));
  return Texture((unsigned int)(textures.size() - 1),
                 glm::vec2(textures.back()->width, textures.back()->height),
                 path);
}

Texture GLTextureLoader::LoadTexture(unsigned char *data, int width, int height,
                                     int nrChannels) {
  textures.push_back(new LoadedTex(data, width, height, nrChannels, mipmapping, filterNearest));
  return Texture((unsigned int)(textures.size() - 1),
                 glm::vec2(textures.back()->width, textures.back()->height),
                 "FONT");
}

  void GLTextureLoader::Bind(Texture tex) {
      if (tex.ID >= textures.size()) {
	  LOG_ERROR("texture ID out of range");
	  return;
      }
      textures[tex.ID]->Bind();
  }
}
