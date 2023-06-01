#ifndef OGL_HELPER_H
#define OGL_HELPER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#define sizeAndPtr(arr) sizeof(arr), &arr

namespace ogl_helper {
    void createShaderStorageBuffer(GLuint* glBuffer, size_t bufferSize, void* pBufferArray);

    void shaderStorageBufferData(GLuint glBuffer, size_t bufferSize, void* pBufferArray,
				 GLuint bufferBaseSize);

    GLuint genTexture(GLuint format, GLsizei width, GLsizei height, unsigned char* data,
		    bool mipmapping, int filtering);
    
}

#endif
