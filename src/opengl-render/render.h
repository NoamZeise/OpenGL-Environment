#ifndef OGL_RENDER_H
#define OGL_RENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>
#include <atomic>

#include "resources.h"
#include "shader.h"
#include "vertex_data.h"

const int MAX_DRAWS = 50;

class Render
{
public:
  Render() {}
	Render(GLFWwindow* window, glm::vec2 target);
	~Render();
  void Resize(int width, int height);
	void restartResourceLoad();
	Resource::Texture LoadTexture(std::string filepath);
	void endResourceLoad();
  void Begin2DDraw();
	void DrawQuad(const Resource::Texture& texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset);
  void EndDraw(std::atomic<bool>& submit);

private:
	GLFWwindow* window;
	glm::vec2 targetResolution;

  Shader* basicShader;

  glm::mat4 proj;
  glm::mat4 view;

  VertexData* quad;

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
