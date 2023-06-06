#include "framebuffer.h"

#include <stdexcept>
#include <logger.h>
#include "ogl_helper.h"

GLuint genDepthStencilRenderBuffer(GLsizei width, GLsizei height, int samples) {
    GLuint depth;
    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    if(samples > 1)
	glRenderbufferStorageMultisample(
		GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
    else
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return depth;
}
  
Framebuffer::Framebuffer(GLsizei width, GLsizei height, bool multisampling, bool depthBuffer) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    int maxSamples = 1;
    if(multisampling) {
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	if(maxSamples == 1)
	    multisampling = false;
    }

    if(multisampling) {
	glGenRenderbuffers(1, &colourBuff);
	glBindRenderbuffer(GL_RENDERBUFFER, colourBuff);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, maxSamples, GL_RGBA8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, colourBuff);
    } else {
	texBuff = ogl_helper::genTexture(GL_RGB, width, height, 0, false,
					 GL_NEAREST, GL_CLAMP_TO_BORDER, maxSamples);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       multisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
			       texBuff, 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
    }

    
    if(depthBuffer) {
	depthStencilBuff = genDepthStencilRenderBuffer(width, height, maxSamples);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
				  GL_RENDERBUFFER, depthStencilBuff);
    }

    int framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
	//check which it matches
	std::cout << "framebuffer error: ";
	switch(framebufferStatus) {
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	    std::cout << "incomplete attachment";
	    break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	    std::cout << "missing attachment";
	    break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
	    std::cout << "unsupported";
	    break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
	    std::cout << "multisampling incomplete";
	    break;
	}
	std::cout << std::endl;
	throw std::runtime_error("failed to create opengl framebuffer!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LOG("created framebuffer, width: " << width << "  height:" << height);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &depthStencilBuff);
    glDeleteTextures(1, &texBuff);
}

GLuint Framebuffer::id() {
    return framebuffer;
}

GLuint Framebuffer::texture() {
    return texBuff;
}
