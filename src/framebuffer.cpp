#include "framebuffer.h"

#include <stdexcept>
#include <logger.h>
#include "ogl_helper.h"

GLuint genRenderbuffer(int samples, int format, GLsizei width, GLsizei height);

std::string framebufferError(int status);

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
	colourBuff = genRenderbuffer(maxSamples, GL_RGBA8, width, height);
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
	depthStencilBuff = genRenderbuffer(maxSamples, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
				  GL_RENDERBUFFER, depthStencilBuff);
    }

    int framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	throw std::runtime_error("failed to create opengl framebuffer! status: "
				 + framebufferError(framebufferStatus));
    
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

GLuint genRenderbuffer(int samples, int format, GLsizei width, GLsizei height) {
    GLuint rb;
    glGenRenderbuffers(1, &rb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    if(samples > 1)
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);
    else
	glRenderbufferStorage(GL_RENDERBUFFER, format, width, height); 
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return rb;
}

std::string framebufferError(int status) {
    switch(status) {
    case GL_FRAMEBUFFER_UNDEFINED:
	return "undefined";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	return "incomplete attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	return "missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
	return "incomplete draw buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
	return "incomplete read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED:
	return "unsupported";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
	return "multisampling incomplete";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
	return "incomplete layer targets";
    default:
	return "unknown error code";
    }
}
