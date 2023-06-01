#include "render.h"

#include "ogl_helper.h"
#include "shader.h"
#include "resources/vertex_data.h"
#include "resources/texture_loader.h"
#include "resources/font_loader.h"

#include "resources/model_render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <stdexcept>

namespace glenv {

  bool GLRender::LoadOpenGL() {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      return true;
  }
  
  GLRender::GLRender(GLFWwindow *window) {
      int width, height;
      glfwGetWindowSize(window, &width, &height);
      GLRender(window, glm::vec2(width, height));
  }
    
  GLRender::GLRender(GLFWwindow *window, glm::vec2 target) {
      glfwMakeContextCurrent(window);

      this->window = window;
      this->targetResolution = target;

      if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	  throw std::runtime_error("failed to load glad");

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_CULL_FACE);


      shader3D = new GLShader("shaders/opengl/3D-lighting.vert", "shaders/opengl/blinnphong.frag");
      shader3D->Use();
      glUniform1i(shader3D->Location("image"), 0);

      shader3DAnim = new GLShader("shaders/opengl/3D-lighting-anim.vert",
				  "shaders/opengl/blinnphong.frag");
      shader3DAnim->Use();
      glUniform1i(shader3DAnim->Location("image"), 0);

      flatShader = new GLShader("shaders/opengl/flat.vert", "shaders/opengl/flat.frag");
      flatShader->Use();
      glUniform1i(flatShader->Location("image"), 0);

      view2D = glm::mat4(1.0f);
  
      ogl_helper::createShaderStorageBuffer(&model3DSSBO, sizeAndPtr(perInstance3DModel));
      ogl_helper::createShaderStorageBuffer(&normal3DSSBO, sizeAndPtr(perInstance3DNormal));
      ogl_helper::createShaderStorageBuffer(&model2DSSBO, sizeAndPtr(perInstance2DModel));  
      ogl_helper::createShaderStorageBuffer(&texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset));
      
      setupStagingResourceLoaders();
      FramebufferResize();
  }

  GLRender::~GLRender()
  {
      delete shader3D;
      delete shader3DAnim;
      delete flatShader;
      delete textureLoader;
      delete fontLoader;
      delete modelLoader;
  }

  void GLRender::setupStagingResourceLoaders() {
      stagingTextureLoader = new Resource::GLTextureLoader(
	      true, false //mipmapping, nearest image filter
							   );
      stagingFontLoader = new Resource::GLFontLoader();
      stagingModelLoader = new Resource::GLModelRender();
      stagingTextureLoader->LoadTexture("textures/error.png");
  }

  Resource::Texture GLRender::LoadTexture(std::string filepath) {
      return stagingTextureLoader->LoadTexture(filepath);
  }
  
  Resource::Model GLRender::loadModel(Resource::ModelType type, std::string filepath,
				      std::vector<Resource::ModelAnimation> *pGetAnimations) {
      return stagingModelLoader->loadModel(type, filepath, stagingTextureLoader, pGetAnimations);
  }

  Resource::Model GLRender::loadModel(Resource::ModelType type, ModelInfo::Model model,
				      std::vector<Resource::ModelAnimation> *pGetAnimations) {
      return stagingModelLoader->loadModel(type, model, stagingTextureLoader, pGetAnimations);
  }

  Resource::Model GLRender::Load3DModel(std::string filepath) {
      return loadModel(Resource::ModelType::m3D, filepath, nullptr);
  }

  Resource::Model GLRender::Load3DModel(ModelInfo::Model &model) {
      return loadModel(Resource::ModelType::m3D, model, nullptr);
  }

  Resource::Model GLRender::LoadAnimatedModel(std::string filepath,
					      std::vector<Resource::ModelAnimation> *pGetAnimations) {
      return loadModel(Resource::ModelType::m3D_Anim, filepath, pGetAnimations);
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

  GLRender::Draw2D::Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset) {
      this->tex = tex;
      this->model = model;
      this->colour = colour;
      this->texOffset = texOffset;
  }

  GLRender::Draw3D::Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix) {
      this->model = model;
      this->modelMatrix = modelMatrix;
      this->normalMatrix = normalMatrix;
  }

  GLRender::DrawAnim3D::DrawAnim3D(Resource::Model model, glm::mat4 modelMatrix,
				   glm::mat4 normalMatrix) {
      this->model = model;
      this->modelMatrix = modelMatrix;
      this->normalMatrix = normalMatrix;
  }

  void GLRender::startDraw() {
      if(!inDraw) {
	  currentDraw = 0;
	  inDraw = true;
      }
  }

  void GLRender::Begin2DDraw() {
      startDraw();
      currentDrawMode = DrawMode::d2D;
  }

  void GLRender::Begin3DDraw() {
      startDraw();
      currentDrawMode = DrawMode::d3D;
  }

  void GLRender::BeginAnim3DDraw() {
      startDraw();
      currentDrawMode = DrawMode::d3DAnim;
  }

  void GLRender::setVPlighting(GLShader *shader) {
      shader->Use();
      glUniformMatrix4fv(shader->Location("projection"), 1, GL_FALSE,
			 &proj3D[0][0]);
      glUniformMatrix4fv(shader->Location("view"), 1, GL_FALSE,
			 &view3D[0][0]);
      
      glUniform4fv(shader->Location("lighting.ambient"), 1,
		   &lighting.ambient[0]);
      glUniform4fv(shader->Location("lighting.diffuse"), 1,
		   &lighting.diffuse[0]);
      glUniform4fv(shader->Location("lighting.specular"), 1,
		   &lighting.specular[0]);
      glUniform4fv(shader->Location("lighting.direction"), 1,
		   &lighting.direction[0]);
      glUniform4fv(shader->Location("lighting.camPos"), 1,
		   &lighting.camPos[0]);
      glm::vec4 colourWhite = glm::vec4(1);
      glUniform4fv(shader->Location("spriteColour"), 1, &colourWhite[0]);
      glUniform1i(shader->Location("enableTex"), GL_TRUE);
  }

  void GLRender::setShaderForMode(DrawMode mode, unsigned int i) {
      switch(mode) {
      case DrawMode::d2D:
	  flatShader->Use();
	  glUniformMatrix4fv(flatShader->Location("projection"), 1,
			     GL_FALSE, &proj2D[0][0]);
	  glUniformMatrix4fv(flatShader->Location("view"), 1, GL_FALSE, &view2D[0][0]);
	  glUniform1i(flatShader->Location("enableTex"), GL_TRUE);
	  break;
      case DrawMode::d3D:
	  setVPlighting(shader3D);
	  break;
      case DrawMode::d3DAnim:
	  setVPlighting(shader3DAnim);
	  glUniformMatrix4fv(shader3DAnim->Location("bones"), MAX_BONES, GL_FALSE,
			     &drawCalls[i].d3DAnim.bones[0][0][0]);
	  glUniformMatrix4fv(shader3DAnim->Location("model"), 1, GL_FALSE,
			     &drawCalls[i].d3DAnim.modelMatrix[0][0]);
	  glUniformMatrix4fv(shader3DAnim->Location("normal"), 1, GL_FALSE,
			     &drawCalls[i].d3DAnim.normalMatrix[0][0]);
	  break;
      }
  }

#define DRAW_BATCH()				\
  if(drawCount > 0) {				\
      switch(currentMode) {			\
      case DrawMode::d2D:					     \
	  draw2DBatch(drawCount, currentTexture, currentColour);     \
	  break;						     \
      case DrawMode::d3D:					     \
	  draw3DBatch(drawCount, currentModel);			     \
	  break;							\
      default:								\
	  throw std::runtime_error("ogl_DRAW_BATCH unknown mode to batch draw!"); \
      }									\
  }

  void GLRender::EndDraw(std::atomic<bool>& submit) {
      inDraw = false;
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if(currentDraw == 0) {
	  glfwSwapBuffers(window);
	  submit = true;
	  return;
      }

      DrawMode currentMode;
      Resource::Texture currentTexture;
      glm::vec4 currentColour;
      Resource::Model currentModel;
      int drawCount = 0;
      
      for(unsigned int i = 0; i < currentDraw; i++) {
	  if(i == 0 || currentMode != drawCalls[i].mode || currentMode == DrawMode::d3DAnim) {
	      DRAW_BATCH();
	      drawCount = 0;
	      currentMode = drawCalls[i].mode;
	      setShaderForMode(currentMode, i);
	  }

	  switch(currentMode) {
	  case DrawMode::d2D:
	      if((currentTexture.ID != drawCalls[i].d2D.tex.ID ||
		  currentColour != drawCalls[i].d2D.colour ||
		  drawCount == MAX_2D_BATCH) && drawCount > 0
		 ) {
		  draw2DBatch(drawCount, currentTexture, currentColour);
		  drawCount = 0;
	      }
	      currentTexture = drawCalls[i].d2D.tex;
	      currentColour = drawCalls[i].d2D.colour;
	      perInstance2DModel[drawCount] = drawCalls[i].d2D.model;
	      perInstance2DTexOffset[drawCount] = drawCalls[i].d2D.texOffset;
	      drawCount++;
	      break;
	  case DrawMode::d3D:
	      if((currentModel.ID != drawCalls[i].d3D.model.ID && drawCount > 0) ||
		 drawCount == MAX_3D_BATCH) {
		  draw3DBatch(drawCount, currentModel);
		  drawCount = 0;
	      }
	      currentModel = drawCalls[i].d3D.model;
	      perInstance3DModel[drawCount] = drawCalls[i].d3D.modelMatrix;
	      perInstance3DNormal[drawCount] = drawCalls[i].d3D.normalMatrix;
	      drawCount++;
	      break;

	  case DrawMode::d3DAnim:
	      currentModel = drawCalls[i].d3DAnim.model;
	      draw3DAnim(currentModel);
	      drawCount = 0;
	      break;
	  }
      }
      DRAW_BATCH()
	  
      glfwSwapBuffers(window);
      submit = true;
  }

  void GLRender::draw2DBatch(int drawCount, Resource::Texture texture, glm::vec4 currentColour) {
      glUniform4fv(flatShader->Location("spriteColour"), 1, &currentColour[0]);

      ogl_helper::shaderStorageBufferData(model2DSSBO, sizeAndPtr(perInstance2DModel), 4);
      ogl_helper::shaderStorageBufferData(texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset), 5);
      glActiveTexture(GL_TEXTURE0);
      textureLoader->Bind(texture);
      modelLoader->DrawQuad(drawCount);
  }

  void GLRender::draw3DBatch(int drawCount, Resource::Model model) {
      ogl_helper::shaderStorageBufferData(model3DSSBO, sizeAndPtr(perInstance3DModel), 2);
      ogl_helper::shaderStorageBufferData(normal3DSSBO, sizeAndPtr(perInstance3DNormal), 3);
      modelLoader->DrawModelInstanced(model, textureLoader, drawCount, shader3D->Location("spriteColour"), shader3D->Location("enableTex"));
  }

  void GLRender::draw3DAnim(Resource::Model model) {
      modelLoader->DrawModel(model, textureLoader, shader3DAnim->Location("spriteColour"));
  }

  void GLRender::DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat) {
      if(currentDraw < MAX_DRAWS) {
	  drawCalls[currentDraw].mode = DrawMode::d3D;
	  drawCalls[currentDraw++].d3D = Draw3D(model, modelMatrix, normalMat);
      }
  }

  void GLRender::DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix,
		     glm::mat4 normalMatrix,
		     Resource::ModelAnimation *animation) {
      if(currentDraw < MAX_DRAWS) {
	  drawCalls[currentDraw].mode = DrawMode::d3DAnim;
	  drawCalls[currentDraw].d3DAnim = DrawAnim3D(model, modelMatrix, normalMatrix);
	  std::vector<glm::mat4>* bones = animation->getCurrentBones();
	  for(int i = 0; i < MAX_BONES && i < bones->size(); i++)
	      drawCalls[currentDraw].d3DAnim.bones[i] = bones->at(i);
	  currentDraw++;
      }
  }

  void GLRender::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
			  glm::vec4 colour, glm::vec4 texOffset) {
      if(currentDraw < MAX_DRAWS) {
	  drawCalls[currentDraw].mode = DrawMode::d2D;
	  drawCalls[currentDraw++].d2D = Draw2D(texture, modelMatrix, colour, texOffset);
      }
  }

  void GLRender::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour) {
      DrawQuad(texture, modelMatrix, colour, glm::vec4(0, 0, 1, 1));
  }

  void GLRender::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix) {
      DrawQuad(texture, modelMatrix, glm::vec4(1), glm::vec4(0, 0, 1, 1));
  }

  void GLRender::DrawString(Resource::Font font, std::string text, glm::vec2 position,
			    float size, float depth, glm::vec4 colour, float rotate) {
      auto draws = fontLoader->DrawString(font, text, position, size, depth, colour, rotate);

      for(const auto &draw: draws) {
	  DrawQuad(draw.tex, draw.model, draw.colour);
      }
  }

  void GLRender::DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour) {
      DrawString(font, text, position, size, depth, colour, 0);
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

  void GLRender::setLightDirection(glm::vec4 lightDir) {
      lighting.direction = lightDir;
  }
  
  void GLRender::setVsync(bool vsync) {
      this->vsync = vsync;
      FramebufferResize();
  }

  bool GLRender::getVsync() { return this->vsync; }
  
}//namespace
