#include "texture_loader.h"

#include <string>

#include <glad/glad.h>
#include <graphics/logger.h>
#include <resource_loader/stb_image.h>

#include "../ogl_helper.h"

namespace Resource {

  const std::string NON_PATH = "NULL";
  
  struct GLStagedTex {
      unsigned char* data;
      int width;
      int height;
      int nrChannels;
      std::string path = NON_PATH;
  };

  Texture GLTextureLoader::stageTex(unsigned char* data, int width, int height, int nrChannels,
				    std::string path) {
      LOG("pool " << pool.ID << " loaded texture " << path << " at ID: " << staged.size());
      GLStagedTex tex;
      tex.data = data;
      tex.width = width;
      tex.height = height;
      tex.nrChannels = nrChannels;
      tex.path = path;
      staged.push_back(tex);
      return Texture(staged.size() - 1, glm::vec2(width, height), pool);
  }

  void GLTextureLoader::loadToGPU() {
      clearGPU();
      inGpu.resize(staged.size());
      for(int i = 0; i < staged.size(); i++) {
	  GLuint format = GL_RGBA;
	  switch(staged[i].nrChannels) {
	  case 1:
	      format = GL_RED;
	      break;
	  case 2:
	      format = GL_RG;
	      break;
	  case 3:
	      format = GL_RGB;
	      break;
	  case 4:
	      format = GL_RGBA;
	      break;
	  default:
	      throw std::runtime_error("Unsupported no. of channels");
	  }  
	  inGpu[i] = ogl_helper::genTexture(format,
					    staged[i].width,
					    staged[i].height,
					    staged[i].data,
					    mipmapping,
					    filterNearest ? GL_NEAREST : GL_LINEAR,
					    GL_REPEAT, 1);
      }
      clearStaged();
  }

  void GLTextureLoader::clearStaged() {
      for(auto& s: staged) {
	  if(s.path == NON_PATH)
	      delete s.data;
	  else
	      stbi_image_free(s.data);
      }
      staged.clear();
  }

  void GLTextureLoader::clearGPU() {
      glDeleteTextures(inGpu.size(), inGpu.data());
      inGpu.clear(); 
  }
  
  GLTextureLoader::GLTextureLoader(bool mipmapping, bool pixelated, Resource::ResourcePool pool) {
    this->mipmapping = mipmapping;
    this->filterNearest = pixelated;
    this->pool = pool;
  }

GLTextureLoader::~GLTextureLoader() {
  clearStaged();
  clearGPU();
}
  
Texture GLTextureLoader::LoadTexture(std::string path) {
    for(int i = 0; i < staged.size(); i++)
	if(staged[i].path == path)
	    return Texture(i, glm::vec2(staged[i].width, staged[i].height), pool);
    int width, height, nrChannels;
    const int DESIRED_CHANNELS = 4;
    unsigned char* data  = stbi_load(path.c_str(), &width, &height, &nrChannels, DESIRED_CHANNELS);
    if(!data) {
	LOG_ERROR("Failed to load texture - path: " << path);
	throw std::runtime_error("failed to load texture - path: " + path);
    }
    if(DESIRED_CHANNELS != 0)
	nrChannels = DESIRED_CHANNELS;
    return stageTex(data, width, height, nrChannels, path);
}

Texture GLTextureLoader::LoadTexture(unsigned char *data, int width, int height,
                                     int nrChannels) {
    return stageTex(data, width, height, nrChannels, NON_PATH);
}

  void GLTextureLoader::Bind(Texture tex) {
      if (tex.ID >= inGpu.size()) {
	  LOG_ERROR("in pool: " << tex.pool.ID << " texture ID out of range: " << tex.ID << " max: " << inGpu.size());
	  return;
      }
      glBindTexture(GL_TEXTURE_2D, inGpu[tex.ID]);
  }
}
