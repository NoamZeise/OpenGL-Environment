#include "render.h"

#include "ogl_helper.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <stdexcept>
#include <config.h>
#include <iostream>

namespace glenv {
GLRender::GLRender(GLFWwindow *window, glm::vec2 target)
{
  glfwMakeContextCurrent(window);

  this->window = window;
  this->targetResolution = target;

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::runtime_error("failed to load glad");

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);


  blinnPhongShader = new GLShader("shaders/opengl/3D-lighting.vert", "shaders/opengl/blinnphong.frag");
  blinnPhongShader->Use();

  glUniform1i(blinnPhongShader->Location("image"), 0);

  flatShader = new GLShader("shaders/opengl/flat.vert", "shaders/opengl/flat.frag");
  flatShader->Use();
  glUniform1i(flatShader->Location("image"), 0);

  view2D = glm::mat4(1.0f);

  std::vector<GLVertex2D> quadVerts = {
    GLVertex2D(0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
    GLVertex2D(1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
    GLVertex2D(0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
    GLVertex2D(0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
    GLVertex2D(1.0f, 1.0f, 0.0f, 1.0f, 1.0f),
    GLVertex2D(1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
  };
  std::vector<unsigned int> quadInds =  {0, 1, 2, 3, 4, 5};
  quad = new GLVertexData(quadVerts, quadInds);

  ogl_helper::createShaderStorageBuffer(&model3DSSBO, sizeAndPtr(perInstance3DModel));
  ogl_helper::createShaderStorageBuffer(&normal3DSSBO, sizeAndPtr(perInstance3DNormal));
  ogl_helper::createShaderStorageBuffer(&model2DSSBO, sizeAndPtr(perInstance2DModel));  
  ogl_helper::createShaderStorageBuffer(&texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset));
  setupStagingResourceLoaders();
  FramebufferResize();
}

GLRender::~GLRender()
{
  delete quad;
  delete blinnPhongShader;
  delete flatShader;
  delete textureLoader;
  delete fontLoader;
  delete modelLoader;
}

void GLRender::setupStagingResourceLoaders() {
  stagingTextureLoader = new Resource::GLTextureLoader();
  stagingFontLoader = new Resource::GLFontLoader();
  stagingModelLoader = new Resource::GLModelRender();
  stagingTextureLoader->LoadTexture("textures/error.png");
}

Resource::Texture GLRender::LoadTexture(std::string filepath) {
  return stagingTextureLoader->LoadTexture(filepath);
}

Resource::Model GLRender::LoadModel(std::string filepath) {
  return stagingModelLoader->LoadModel(filepath, stagingTextureLoader);
}

Resource::Font GLRender::LoadFont(std::string filepath) {
  return stagingFontLoader->LoadFont(filepath, stagingTextureLoader);
}

void GLRender::LoadResourcesToGPU() {
  // Does nothing in OGL version, but needs to match vulkan functions.
}

void GLRender::UseLoadedResources() {
  delete textureLoader;
  delete fontLoader;
  delete modelLoader;

  textureLoader = stagingTextureLoader;
  fontLoader = stagingFontLoader;
  modelLoader = stagingModelLoader;
  setupStagingResourceLoaders();
}

GLRender::Draw2D::Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset)
{
  this->tex = tex;
  this->model = model;
  this->colour = colour;
  this->texOffset = texOffset;
}

GLRender::Draw3D::Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix)
{
  this->model = model;
  this->modelMatrix = modelMatrix;
  this->normalMatrix = normalMatrix;
}

void GLRender::Begin2DDraw()
{
  current2DDraw = 0;
}

void GLRender::Begin3DDraw()
{
  current3DDraw = 0;
}

void GLRender::EndDraw(std::atomic<bool>& submit)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//Draw 2D

  flatShader->Use();

  glUniformMatrix4fv(flatShader->Location("projection"), 1, GL_FALSE, &proj2D[0][0]);
  glUniformMatrix4fv(flatShader->Location("view"), 1, GL_FALSE, &view2D[0][0]);
  glUniform1i(flatShader->Location("enableTex"), GL_TRUE);

  Resource::Texture currentTexture;
  glm::vec4 currentColour;

  int drawCount = 0;
  for(unsigned int i = 0; i < current2DDraw; i++)
  {
    if((currentTexture.ID != draw2DCalls[i].tex.ID || currentColour != draw2DCalls[i].colour) && drawCount > 0)
    {
      draw2DBatch(drawCount, currentTexture, currentColour);
      drawCount = 0;
    }
    currentTexture = draw2DCalls[i].tex;
    currentColour = draw2DCalls[i].colour;
    perInstance2DModel[drawCount] = draw2DCalls[i].model;
    perInstance2DTexOffset[drawCount] = draw2DCalls[i].texOffset;

    drawCount++;
  }
  if(drawCount != 0)
  {
    draw2DBatch(drawCount, currentTexture, currentColour);
  }

//Draw 3D

  blinnPhongShader->Use();
  glUniformMatrix4fv(blinnPhongShader->Location("projection"), 1, GL_FALSE, &proj3D[0][0]);
  glUniformMatrix4fv(blinnPhongShader->Location("view"), 1, GL_FALSE, &view3D[0][0]);
  
  glUniform4fv(blinnPhongShader->Location("lighting.ambient"), 1, &lighting.ambient[0]);
  glUniform4fv(blinnPhongShader->Location("lighting.diffuse"), 1, &lighting.diffuse[0]);
  glUniform4fv(blinnPhongShader->Location("lighting.specular"), 1, &lighting.specular[0]);
  glUniform4fv(blinnPhongShader->Location("lighting.direction"), 1, &lighting.direction[0]);
  glUniform4fv(blinnPhongShader->Location("lighting.camPos"), 1, &lighting.camPos[0]);
  glm::vec4 colourWhite = glm::vec4(1);
  glUniform4fv(blinnPhongShader->Location("spriteColour"), 1, &colourWhite[0]);
  glUniform1i(blinnPhongShader->Location("enableTex"), GL_TRUE);

  Resource::Model currentModel;
  drawCount = 0;
  for(unsigned int i = 0; i < current3DDraw; i++)
  {
    if((currentModel.ID != draw3DCalls[i].model.ID && drawCount > 0) || drawCount >= MAX_3D_DRAWS)
    {
      draw3DBatch(drawCount, currentModel);
      drawCount = 0;
    }
    currentModel = draw3DCalls[i].model;
    perInstance3DModel[drawCount] = draw3DCalls[i].modelMatrix;
    perInstance3DNormal[drawCount] = draw3DCalls[i].normalMatrix;
    drawCount++;
  }
  if(drawCount != 0)
  {
    draw3DBatch(drawCount, currentModel);
  }

  glfwSwapBuffers(window);
  submit = true;
}

void GLRender::draw2DBatch(int drawCount, Resource::Texture texture, glm::vec4 currentColour)
{
  glUniform4fv(flatShader->Location("spriteColour"), 1, &currentColour[0]);

  ogl_helper::shaderStorageBufferData(model2DSSBO, sizeAndPtr(perInstance2DModel), 4);
  ogl_helper::shaderStorageBufferData(texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset), 5);
  glActiveTexture(GL_TEXTURE0);
  textureLoader->Bind(texture);
  quad->DrawInstanced(GL_TRIANGLES, drawCount);
}

void GLRender::draw3DBatch(int drawCount, Resource::Model model)
{
    ogl_helper::shaderStorageBufferData(model3DSSBO, sizeAndPtr(perInstance3DModel), 2);
    ogl_helper::shaderStorageBufferData(normal3DSSBO, sizeAndPtr(perInstance3DNormal), 3);
    modelLoader->DrawModelInstanced(model, textureLoader, drawCount, blinnPhongShader->Location("spriteColour"), blinnPhongShader->Location("enableTex"));
}

void GLRender::DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat)
{
  if(current3DDraw < MAX_3D_DRAWS)
  {
    draw3DCalls[current3DDraw++] = Draw3D(model, modelMatrix, normalMat);
  }
}

void GLRender::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset)
{
  if(current2DDraw < MAX_2D_DRAWS)
  {
    draw2DCalls[current2DDraw++] = Draw2D(texture, modelMatrix, colour, texOffset);
  }
}

void GLRender::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour)
{
  DrawQuad(texture, modelMatrix, colour, glm::vec4(0, 0, 1, 1));
}

void GLRender::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix)
{
  DrawQuad(texture, modelMatrix, glm::vec4(1), glm::vec4(0, 0, 1, 1));
}

void GLRender::DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate)
{
  auto draws = fontLoader->DrawString(font, text, position, size, depth, colour, rotate);

  for(const auto &draw: draws)
  {
    DrawQuad(draw.tex, draw.model, draw.colour);
  }
}

float GLRender::MeasureString(Resource::Font font,std::string text, float size) {
  return fontLoader->MeasureString(font, text, size);
}

void GLRender::FramebufferResize()
{
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  glViewport(0, 0, width, height);

  float deviceRatio = (float)width /
                  (float)height;
  float virtualRatio = targetResolution.x / targetResolution.y;
  float xCorrection = width / targetResolution.x;
  float yCorrection = height / targetResolution.y;

  float correction;
  if (virtualRatio < deviceRatio) {
    correction = yCorrection;
  } else {
    correction = xCorrection;
  }
  proj2D = glm::ortho(
      0.0f, (float)width*scale2D / correction, (float)height*scale2D / correction, 0.0f, -10.0f, 10.0f);

  set3DViewMatrixAndFov(view3D, fov, lighting.camPos);
}

  void GLRender::set3DViewMatrixAndFov(glm::mat4 view, float fov, glm::vec4 camPos)
  {
  this->fov = fov;
  view3D = view;
  proj3D = glm::perspective(glm::radians(fov),
                            (targetResolution.x / targetResolution.y), 0.1f, 500.0f);
  lighting.camPos = camPos;
  }

  void GLRender::set2DViewMatrixAndScale(glm::mat4 view, float scale)
  {
    view2D = view;
    scale2D = scale;
  }

  void GLRender::setForceTargetRes(bool force) {
    if(forceTargetResolution != force) {
      forceTargetResolution = force;
      FramebufferResize();
    }
  }
  
  bool GLRender::isTargetResForced() { return forceTargetResolution; }

  void GLRender::setTargetResolution(glm::vec2 resolution) {
    targetResolution = resolution;
    forceTargetResolution = true;
    FramebufferResize();
  }

  glm::vec2 GLRender::getTargetResolution() { return targetResolution; }
  
  void GLRender::setVsync(bool vsync) {
    this->vsync = vsync;
    FramebufferResize();
  }
  
}//namespace
