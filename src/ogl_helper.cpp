#include "ogl_helper.h"

namespace ogl_helper {
  
void createShaderStorageBuffer(GLuint* glBuffer, size_t bufferSize, void* pBufferArray) {
    glGenBuffers(1, glBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *glBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, pBufferArray, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

 void shaderStorageBufferData(GLuint glBuffer, size_t bufferSize, void* pBufferArray,
			      GLuint bufferBaseSize){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, pBufferArray, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferBaseSize, glBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  GLuint genTexture(GLuint format, GLsizei width, GLsizei height, unsigned char* data,
		  bool mipmapping, int filtering) {
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

      if(mipmapping)
	  glGenerateMipmap(GL_TEXTURE_2D);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

      glBindTexture(GL_TEXTURE_2D, 0);
      return texture;
  }
}
