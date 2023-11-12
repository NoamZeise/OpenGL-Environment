#include "texture_loader.h"

#include <graphics/logger.h>
#include "../ogl_helper.h"

TextureLoaderGL::TextureLoaderGL(Resource::Pool pool, RenderConfig conf)
    : InternalTexLoader(pool, conf) {}

void TextureLoaderGL::loadGPU() {
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

void TextureLoaderGL::clearGPU() {
    glDeleteTextures(inGpu.size(), inGpu.data());
    inGpu.clear(); 
}
  
unsigned int TextureLoaderGL::getViewIndex(Resource::Texture tex) {
    if (tex.ID >= inGpu.size()) {
	LOG_ERROR("in pool: " << tex.pool.ID <<
		  " texture ID out of range: " << tex.ID
		  << " max: " << inGpu.size());
	return 0;
    }
    return inGpu[tex.ID];
}
