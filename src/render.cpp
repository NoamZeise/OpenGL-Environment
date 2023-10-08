#include "render.h"

#include "ogl_helper.h"
#include "shader.h"
#include "resources/vertex_data.h"
#include "resources/resource_pool.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/logger.h>
#include <graphics/glm_helper.h>
#include <stdexcept>


glm::vec2 getTargetRes(RenderConfig renderConf, glm::vec2 winRes);

namespace glenv {

  bool GLRender::LoadOpenGL() {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      return true;
  }
    
  GLRender::GLRender(GLFWwindow *window, RenderConfig renderConf) {
      glfwMakeContextCurrent(window);

      this->window = window;
      this->renderConf = renderConf;
      this->prevRenderConf = renderConf;
      
      if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	  throw std::runtime_error("failed to load glad");
      LOG("glad loaded");

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

      finalShader = new GLShader("shaders/opengl/final.vert", "shaders/opengl/final.frag");
      finalShader->Use();
      glUniformMatrix4fv(finalShader->Location("screenTransform"),
			 1, GL_FALSE, &finalTransform[0][0]);

      LOG("shaders loaded");
      
      view2D = glm::mat4(1.0f);
  
      ogl_helper::createShaderStorageBuffer(&model3DSSBO, sizeAndPtr(perInstance3DModel));
      ogl_helper::createShaderStorageBuffer(&normal3DSSBO, sizeAndPtr(perInstance3DNormal));
      ogl_helper::createShaderStorageBuffer(&model2DSSBO, sizeAndPtr(perInstance2DModel));  
      ogl_helper::createShaderStorageBuffer(&texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset));
      LOG("shader buffers created");
      
      defaultPool = CreateResourcePool();
      FramebufferResize();
      setLightingProps(BPLighting());
      LOG("renderer initialized");
  }

  GLRender::~GLRender() {
      if(useOffscreenFramebuffer) {
	  delete offscreenFramebuffer;
	  if(offscreenBlitFramebuffer != nullptr)
	      delete offscreenBlitFramebuffer;
      }
      delete shader3D;
      delete shader3DAnim;
      delete flatShader;
      delete finalShader;
      for(auto& pool : pools)
	  delete pool;
  }

  Resource::ResourcePool GLRender::CreateResourcePool() {
      int index = pools.size();
      if(freePools.empty()) {
	  pools.push_back(nullptr);
      } else {
	  index = freePools.back();
	  freePools.pop_back();
      }
      pools[index] = new GLResourcePool(Resource::ResourcePool(index), renderConf);
      return pools[index]->poolID;
  }
  void GLRender::DestroyResourcePool(Resource::ResourcePool pool) {
      for(int i = 0; i < pools.size(); i++) {
	  if(pools[i]->poolID.ID == pool.ID) {
	      delete pools[i];
	      pools[i] = nullptr;
	      freePools.push_back(i);
	  }
      }
  }
  
  Resource::Texture GLRender::LoadTexture(Resource::ResourcePool pool, std::string path) {
      _throwIfPoolInvaid(pool);
      return pools[pool.ID]->texLoader->LoadTexture(path);
  }

  Resource::Texture GLRender::LoadTexture(std::string filepath) {
      return LoadTexture(defaultPool, filepath);
  }

  Resource::Texture GLRender::LoadTexture(Resource::ResourcePool pool, unsigned char* data,
					  int width, int height) {
      _throwIfPoolInvaid(pool);
      return pools[pool.ID]->texLoader->LoadTexture(data, width, height, 4);
  }
  
  Resource::Texture GLRender::LoadTexture(unsigned char* data, int width, int height) {
      return LoadTexture(defaultPool, data, width, height);
  }

  Resource::Model GLRender::LoadModel(Resource::ModelType type, std::string filepath,
				      std::vector<Resource::ModelAnimation> *pAnimations) {
      return LoadModel(defaultPool, type, filepath, pAnimations);
  }
  
  Resource::Model GLRender::LoadModel(Resource::ResourcePool pool, Resource::ModelType type,
				      std::string filepath,
				      std::vector<Resource::ModelAnimation> *pAnimations) {
      _throwIfPoolInvaid(pool);
      return pools[pool.ID]->loadModel(type, filepath, pAnimations);
  }
  
  Resource::Model GLRender::LoadModel(Resource::ModelType type, ModelInfo::Model& model,
			    std::vector<Resource::ModelAnimation> *pAnimations) {
      return LoadModel(defaultPool, type, model, pAnimations);
  }
  
  Resource::Model GLRender::LoadModel(Resource::ResourcePool pool, Resource::ModelType type,
				      ModelInfo::Model& model,
				      std::vector<Resource::ModelAnimation> *pAnimations) {
      _throwIfPoolInvaid(pool);
      return pools[pool.ID]->loadModel(type, model, pAnimations);
  }
  
  Resource::Model GLRender::loadModel(Resource::ModelType type, std::string filepath,
				      std::vector<Resource::ModelAnimation> *pGetAnimations) {
      return pools[defaultPool.ID]->modelLoader->loadModel(type, filepath, pools[defaultPool.ID]->texLoader, pGetAnimations);
  }

  Resource::Model GLRender::loadModel(Resource::ModelType type, ModelInfo::Model model,
				      std::vector<Resource::ModelAnimation> *pGetAnimations) {
      return pools[defaultPool.ID]->modelLoader->loadModel(type, model, pools[defaultPool.ID]->texLoader, pGetAnimations);
  }

  Resource::Model GLRender::Load3DModel(std::string filepath) {
      return loadModel(Resource::ModelType::m3D, filepath, nullptr);
  }

  Resource::Model GLRender::Load3DModel(ModelInfo::Model &model) {
      return loadModel(Resource::ModelType::m3D, model, nullptr);
  }

  Resource::Model GLRender::LoadAnimatedModel(
	  std::string filepath, std::vector<Resource::ModelAnimation> *pGetAnimations) {
      return loadModel(Resource::ModelType::m3D_Anim, filepath, pGetAnimations);
  }

  Resource::Font GLRender::LoadFont(std::string filepath) {
      return LoadFont(defaultPool, filepath);
  }

  Resource::Font GLRender::LoadFont(Resource::ResourcePool pool, std::string filepath) {
      _throwIfPoolInvaid(pool);
      return pools[pool.ID]->fontLoader->LoadFont(filepath, pools[pool.ID]->texLoader);
  }

  void GLRender::LoadResourcesToGPU(Resource::ResourcePool pool) {
      _throwIfPoolInvaid(pool);
      pools[pool.ID]->loadGpu();
  }
	   
  void GLRender::LoadResourcesToGPU() {
      LoadResourcesToGPU(defaultPool);
  }

  GLRender::Draw2D::Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset) {
      this->tex = tex;
      this->model = model;
      this->colour = colour;
      this->texOffset = texOffset;
  }

  GLRender::Draw3D::Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix,
			   glm::vec4 colour) {
      this->model = model;
      this->modelMatrix = modelMatrix;
      this->normalMatrix = normalMatrix;
      this->colour = colour;
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

  void GLRender::setVPshader(GLShader *shader) {
      shader->Use();
      glUniformMatrix4fv(shader->Location("projection"), 1, GL_FALSE,
			 &proj3D[0][0]);
      glUniformMatrix4fv(shader->Location("view"), 1, GL_FALSE,
			 &view3D[0][0]);
      glUniform4fv(shader->Location("lighting.camPos"), 1,
		   &lighting.camPos[0]);
      glm::vec4 colourWhite = glm::vec4(1);
      glUniform4fv(shader->Location("spriteColour"), 1, &colourWhite[0]);
      glUniform1i(shader->Location("enableTex"), GL_TRUE);
  }

  void GLRender::setLightingShader(GLShader *shader) {
      shader->Use();
      glUniform4fv(shader->Location("lighting.ambient"), 1,
		   &lighting.ambient[0]);
      glUniform4fv(shader->Location("lighting.diffuse"), 1,
		   &lighting.diffuse[0]);
      glUniform4fv(shader->Location("lighting.specular"), 1,
		   &lighting.specular[0]);
      glUniform4fv(shader->Location("lighting.direction"), 1,
		   &lighting.direction[0]);
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
	  setVPshader(shader3D);
	  break;
      case DrawMode::d3DAnim:
	  setVPshader(shader3DAnim);
	  glUniformMatrix4fv(shader3DAnim->Location("bones"), MAX_BONES, GL_FALSE,
			     &drawCalls[i].d3DAnim.bones[0][0][0]);
	  glUniformMatrix4fv(shader3DAnim->Location("model"), 1, GL_FALSE,
			     &drawCalls[i].d3DAnim.modelMatrix[0][0]);
	  glUniformMatrix4fv(shader3DAnim->Location("normal"), 1, GL_FALSE,
			     &drawCalls[i].d3DAnim.normalMatrix[0][0]);
	  break;
      }
  }

#define DRAW_BATCH()							\
  if(drawCount > 0) {							\
      switch(currentMode) {						\
      case DrawMode::d2D:						\
	  draw2DBatch(drawCount, currentTexture, currentColour);	\
	  break;							\
      case DrawMode::d3D:						\
	  draw3DBatch(drawCount, currentModel, currentModelColour);			\
	  break;							\
      default:								\
	  throw std::runtime_error("ogl_DRAW_BATCH unknown mode to batch draw!"); \
      }									\
  }

  void GLRender::EndDraw(std::atomic<bool>& submit) {
      inDraw = false;
      glm::vec2 targetResolution = getTargetRes(renderConf, windowResolution);
      if(useOffscreenFramebuffer) {
	  glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer->id());
	  glEnable(GL_DEPTH_TEST);
	  glViewport(0, 0, (GLsizei)targetResolution.x, (GLsizei)targetResolution.y);
      }
      glClearColor(renderConf.clear_colour[0],
		   renderConf.clear_colour[1],
		   renderConf.clear_colour[2], 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      if(currentDraw == 0) {
	  glfwSwapBuffers(window);
	  submit = true;
	  return;
      }

      DrawMode currentMode;
      Resource::Texture currentTexture;
      glm::vec4 currentColour;
      glm::vec4 currentModelColour = glm::vec4(0.0f);
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
	      if((currentTexture.pool.ID != drawCalls[i].d2D.tex.pool.ID ||
		  currentTexture.ID != drawCalls[i].d2D.tex.ID ||
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
	      if(((currentModel.pool.ID != drawCalls[i].d3D.model.pool.ID ||
		   currentModel.ID != drawCalls[i].d3D.model.ID ||
		   currentModelColour != drawCalls[i].d3D.colour) && drawCount > 0) ||
		 drawCount == MAX_3D_BATCH) {
		  draw3DBatch(drawCount, currentModel, currentModelColour);
		  drawCount = 0;
	      }
	      currentModel = drawCalls[i].d3D.model;
	      currentModelColour = drawCalls[i].d3D.colour;
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
      
      DRAW_BATCH();

      if(useOffscreenFramebuffer) {
	  if(renderConf.multisampling) {
	      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, offscreenBlitFramebuffer->id());
	      glBindFramebuffer(GL_READ_FRAMEBUFFER, offscreenFramebuffer->id());
	      glDrawBuffer(GL_BACK);
	      glBlitFramebuffer(0, 0,
				(GLsizei)targetResolution.x,
				(GLsizei)targetResolution.y,
				0, 0,
				(GLsizei)targetResolution.x,
				(GLsizei)targetResolution.y,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
	  }
	  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	  glClearColor(renderConf.scaled_border_colour[0],
		       renderConf.scaled_border_colour[1],
		       renderConf.scaled_border_colour[2], 1.0f);
	  glClear(GL_COLOR_BUFFER_BIT);
	  glViewport(0, 0, (GLsizei)windowResolution.x, (GLsizei)windowResolution.y);
	  
	  finalShader->Use();
	  glDisable(GL_DEPTH_TEST);
	  glBindTexture(GL_TEXTURE_2D, renderConf.multisampling ?
			offscreenBlitFramebuffer->textureId(0) :
			offscreenFramebuffer->textureId(0));
	  glDrawArrays(GL_TRIANGLES, 0, 3);
      }
      
      glfwSwapBuffers(window);
      submit = true;
  }

  void GLRender::draw2DBatch(int drawCount, Resource::Texture texture, glm::vec4 currentColour) {
      if(!_poolInUse(texture.pool)) {
	  LOG_ERROR("Tried Drawing with pool that is not in use");
	  return;
      }
      glUniform4fv(flatShader->Location("spriteColour"), 1, &currentColour[0]);

      ogl_helper::shaderStorageBufferData(model2DSSBO, sizeAndPtr(perInstance2DModel), 4);
      ogl_helper::shaderStorageBufferData(texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset), 5);
      glActiveTexture(GL_TEXTURE0);
      pools[texture.pool.ID]->texLoader->Bind(texture);
      pools[texture.pool.ID]->modelLoader->DrawQuad(drawCount);
  }

  void GLRender::draw3DBatch(int drawCount, Resource::Model model, glm::vec4 colour) {
      if(!_poolInUse(model.pool)) {
		LOG_ERROR("Tried Drawing with pool that is not in use");
	  return;
      }
      ogl_helper::shaderStorageBufferData(model3DSSBO, sizeAndPtr(perInstance3DModel), 2);
      ogl_helper::shaderStorageBufferData(normal3DSSBO, sizeAndPtr(perInstance3DNormal), 3);
      pools[model.pool.ID]->modelLoader->DrawModelInstanced(
	      model, colour, pools[model.pool.ID]->texLoader, drawCount,
	      shader3D->Location("spriteColour"), shader3D->Location("enableTex"));
  }

  bool GLRender::_validPool(Resource::ResourcePool pool) {
      if(pool.ID > pools.size() || pools[pool.ID] == nullptr) {
	  LOG_ERROR("Passed Pool does not exist."
		    " It has either been destroyed or was never created.");
	  return false;
      }
      return true;
  }

  bool GLRender::_poolInUse(Resource::ResourcePool pool) {
    return _validPool(pool) && pools[pool.ID]->usingGPUResources;
  }

  void GLRender::_throwIfPoolInvaid(Resource::ResourcePool pool) {
      if(!_validPool(pool))
	  throw std::runtime_error("Tried to load resource "
				   "with a pool that does not exist");
  }

  void GLRender::draw3DAnim(Resource::Model model) {
      if(!_poolInUse(model.pool)) {
	  LOG_ERROR("tried to draw string with pool that is not currently in use!");
	  return;
      }
      pools[model.pool.ID]->modelLoader->DrawModel(model, pools[model.pool.ID]->texLoader, shader3DAnim->Location("spriteColour"));
  }

  void GLRender::DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat) {
      DrawModel(model, modelMatrix, normalMat, glm::vec4(0.0f));
  }

  void GLRender::DrawModel(Resource::Model model, glm::mat4 modelMatrix,
	    glm::mat4 normalMat, glm::vec4 colour) {
      if(currentDraw < MAX_DRAWS) {
	  drawCalls[currentDraw].mode = DrawMode::d3D;
	  drawCalls[currentDraw++].d3D = Draw3D(model, modelMatrix, normalMat, colour);
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
      if(!_poolInUse(font.pool)) {
	  LOG_ERROR("tried to draw string with pool that is not currently in use!");
	  return;
      }
      auto draws = pools[font.pool.ID]->fontLoader->DrawString(font, text, position, size, depth, colour, rotate);
      for(const auto &draw: draws) 
	  DrawQuad(draw.tex, draw.model, draw.colour, draw.texOffset);
  }

  void GLRender::DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour) {
      DrawString(font, text, position, size, depth, colour, 0);
  }

  float GLRender::MeasureString(Resource::Font font, std::string text, float size) {
      _throwIfPoolInvaid(font.pool);
      return pools[font.pool.ID]->fontLoader->MeasureString(font, text, size);
  }

  void GLRender::FramebufferResize() {
      glfwSwapInterval(renderConf.vsync ? 1 : 0);
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      windowResolution = glm::vec2((float)width, (float)height);
      glViewport(0, 0, width, height);
      LOG("resizing framebuffer, window width: " << width << "  height:" << height);
      glm::vec2 targetResolution = getTargetRes(renderConf, windowResolution);

      if(renderConf.multisampling) 
	  glEnable(GL_MULTISAMPLE);
      else
	  glDisable(GL_MULTISAMPLE);

      if(useOffscreenFramebuffer) {
	  if(renderConf.multisampling)
	      glGetIntegerv(GL_MAX_SAMPLES, &msaaSamples);
	  else
	      msaaSamples = 1;
	  if(offscreenFramebuffer != nullptr)
	      delete offscreenFramebuffer;
	  offscreenFramebuffer = new GlFramebuffer(
		  (GLsizei)targetResolution.x, (GLsizei)targetResolution.y, msaaSamples, {
		      GlFramebuffer::Attachment(
			      GlFramebuffer::Attachment::Position::color0,
			      renderConf.multisampling ?
			      GlFramebuffer::AttachmentType::renderbuffer :
			      GlFramebuffer::AttachmentType::texture2D,
			      GL_RGB),
		       GlFramebuffer::Attachment(
		            GlFramebuffer::Attachment::Position::depthStencil,
		            GlFramebuffer::AttachmentType::renderbuffer,
		            GL_DEPTH24_STENCIL8),
		  });
	  if(renderConf.multisampling) {
	      offscreenBlitFramebuffer = new GlFramebuffer(
		      (GLsizei)targetResolution.x, (GLsizei)targetResolution.y, 1, {
			  GlFramebuffer::Attachment(
				  GlFramebuffer::Attachment::Position::color0,
				  GlFramebuffer::AttachmentType::texture2D,
				  GL_RGB)});
	  }
	  finalShader->Use();
	  finalTransform = glmhelper::calcFinalOffset(targetResolution, windowResolution);
	  glUniformMatrix4fv(finalShader->Location("screenTransform"),
			     1, GL_FALSE, &finalTransform[0][0]);
      }
      
      proj2D = glm::ortho(
	      0.0f,
	      (float)targetResolution.x * scale2D,
	      (float)targetResolution.y * scale2D,
	      0.0f,
	      renderConf.depth_range_2D[0],
	      renderConf.depth_range_2D[1]);

      set3DViewMatrixAndFov(view3D, fov, lighting.camPos);
      prevRenderConf = renderConf;
  }

  void GLRender::set3DViewMatrixAndFov(glm::mat4 view, float fov, glm::vec4 camPos) {
      this->fov = fov;
      view3D = view;

      float ratio =
	  renderConf.target_resolution[0] == 0.0 ||
	  renderConf.target_resolution[1] == 0.0 ?
	  windowResolution.x / windowResolution.y :
	  renderConf.target_resolution[0] / renderConf.target_resolution[1];
      
      proj3D = glm::perspective(glm::radians(fov), ratio,
				renderConf.depth_range_3D[0],
				renderConf.depth_range_3D[1]);
      lighting.camPos = camPos;
  }

  void GLRender::set2DViewMatrixAndScale(glm::mat4 view, float scale) {
      view2D = view;
      scale2D = scale;
  }

  void GLRender::setRenderConf(RenderConfig renderConf) {
      this->renderConf = renderConf;
      FramebufferResize();
  }
  
  RenderConfig GLRender::getRenderConf() {
      return renderConf;
  }

  void GLRender::setTargetResolution(glm::vec2 resolution) {
      if(renderConf.target_resolution[0] == resolution.x &&
	 renderConf.target_resolution[1] == resolution.y)
	  return;
      renderConf.target_resolution[0] = resolution.x;
      renderConf.target_resolution[1] = resolution.y;
      FramebufferResize();
  }

  glm::vec2 GLRender::getTargetResolution() {
      return glm::vec2(renderConf.target_resolution[0],
		       renderConf.target_resolution[1]);
  }

  void GLRender::setLightingProps(BPLighting lighting) {
      this->lighting = lighting;
      setLightingShader(shader3D);
      setLightingShader(shader3DAnim);
  }
  
  
}//namespace

  glm::vec2 getTargetRes(RenderConfig renderConf, glm::vec2 winRes) {
      glm::vec2 targetResolution(renderConf.target_resolution[0],
				 renderConf.target_resolution[1]);
      if(targetResolution.x == 0.0 || targetResolution.y == 0.0)
	  targetResolution = winRes;
      return targetResolution;
  }
