
#include "texture_loader.h"

#include <iostream>
#include <string>

#include <glad/glad.h>

#include <resource_loader/stb_image.h>

#include "../ogl_helper.h"

namespace Resource
{

  GLTextureLoader::LoadedTex::LoadedTex(std::string path, bool mipmapping, bool pixelated) {
#ifndef NDEBUG
      std::cout << "loading texture: " << path << std::endl;
#endif
  ID = 0;
  width = 0;
  height = 0;
  int nrChannels;
  unsigned char *data =
      stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
  if (!data) {
      std::cerr << "stb_image: failed to load texture at " << path << std::endl;
      return;
  }
  
  generateTexture(data, width, height, nrChannels, mipmapping, pixelated);

  stbi_image_free(data);
}

GLTextureLoader::LoadedTex::LoadedTex(unsigned char *data, int width,
                                      int height, int nrChannels, bool mipmapping, bool pixelated) {
  generateTexture(data, width, height, nrChannels, mipmapping, pixelated);
}

void GLTextureLoader::LoadedTex::generateTexture(unsigned char *data, int width,
                                                 int height, int nrChannels,
						 bool mipmapping, bool pixelated) {
  GLuint format = GL_RGBA;
  if (nrChannels == 1)
    format = GL_RED;
  else if (nrChannels == 3)
    format = GL_RGB;
  else if (nrChannels == 4)
    format = GL_RGBA;
  else {
    std::cerr << "failed to load texture, unsupported num of channels!"
              << std::endl;
    return;
  }
  
  ID = ogl_helper::genTexture(format, width, height, data, mipmapping,
			      pixelated ? GL_NEAREST : GL_LINEAR);
}

  GLTextureLoader::LoadedTex::~LoadedTex() { glDeleteTextures(1, &ID); }
  
  void GLTextureLoader::LoadedTex::Bind() { glBindTexture(GL_TEXTURE_2D, ID); }

  GLTextureLoader::GLTextureLoader(bool mipmapping, bool pixelated) {
    this->mipmapping = mipmapping;
    this->pixelated = pixelated;
  }

GLTextureLoader::~GLTextureLoader() {
  for (unsigned int i = 0; i < textures.size(); i++)
    delete textures[i];
}

Texture GLTextureLoader::LoadTexture(std::string path) {
  textures.push_back(new LoadedTex(path, mipmapping, pixelated));
  return Texture((unsigned int)(textures.size() - 1),
                 glm::vec2(textures.back()->width, textures.back()->height),
                 path);
}

Texture GLTextureLoader::LoadTexture(unsigned char *data, int width, int height,
                                     int nrChannels) {
  textures.push_back(new LoadedTex(data, width, height, nrChannels, mipmapping, pixelated));
  return Texture((unsigned int)(textures.size() - 1),
                 glm::vec2(textures.back()->width, textures.back()->height),
                 "FONT");
}

void GLTextureLoader::Bind(Texture tex) {
  if (tex.ID >= textures.size()) {
    std::cerr << "texture ID out of range" << std::endl;
    return;
  }
  textures[tex.ID]->Bind();
}
}
