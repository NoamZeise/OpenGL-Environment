#ifndef OGL_RENDER_H
#define OGL_RENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdexcept>
#include <iostream>
#include <string>
#include <atomic>
#include <vector>

#include "shader.h"
#include "resources/resources.h"
#include "resources/vertex_data.h"
#include "resources/texture_loader.h"

const int MAX_DRAWS = 50;

class Render
{
public:
  Render() {}
	Render(GLFWwindow* window, glm::vec2 target);
	~Render();
  void Resize(int width, int height);
	Resource::Texture LoadTexture(std::string filepath);
  void Begin2DDraw();
	void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset);
  void EndDraw(std::atomic<bool>& submit);

private:
	GLFWwindow* window;
	glm::vec2 targetResolution;

  Shader* basicShader;

  glm::mat4 proj;
  glm::mat4 view;

  VertexData* quad;

  Resource::TextureLoader* textureLoader;

  struct Draw
  {
    Draw() {}
    Draw(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset);
    Resource::Texture tex;
    glm::mat4 model;
    glm::vec4 colour;
    glm::vec4 texOffset;
  };

  unsigned int currentDraw = 0;
  Draw drawCalls[MAX_DRAWS];
};





#endif
