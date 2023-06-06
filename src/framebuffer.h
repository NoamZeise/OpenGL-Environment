#ifndef OGL_RENDER_FRAMEBUFFER_H
#define OGL_RENDER_FRAMEBUFFER_H

#include <glad/glad.h>

class Framebuffer {
 public:
    Framebuffer(GLsizei width, GLsizei height, bool multisampling, bool depthBuffer);
    ~Framebuffer();
    GLuint id();
    GLuint texture();
 private:
    GLuint framebuffer;
    GLuint texBuff;
    GLuint colourBuff;
    GLuint depthStencilBuff;
};


#endif
