#ifndef OGL_RENDER_FRAMEBUFFER_H
#define OGL_RENDER_FRAMEBUFFER_H

#include <glad/glad.h>

class Framebuffer {
 public:
    Framebuffer(GLsizei width, GLsizei height);
    ~Framebuffer();

 private:
    GLuint framebuffer;
    GLuint texBuff;
    GLuint depthStencilBuff;
};


#endif
