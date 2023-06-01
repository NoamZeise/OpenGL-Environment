#include "framebuffer.h"

#include <stdexcept>
#include <logger.h>
#include "ogl_helper.h"

GLuint genDepthStencilRenderBuffer(GLsizei width, GLsizei height) {
    GLuint depth;
    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return depth;
}
  
Framebuffer::Framebuffer(GLsizei width, GLsizei height) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    texBuff = ogl_helper::genTexture(GL_RGB, width, height, 0, false, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texBuff, 0);
    depthStencilBuff = genDepthStencilRenderBuffer(width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			      GL_RENDERBUFFER, depthStencilBuff);

    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	throw std::runtime_error("failed to create opengl framebuffer!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LOG("Created framebuffer");
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &depthStencilBuff);
    glDeleteTextures(1, &texBuff);
}
