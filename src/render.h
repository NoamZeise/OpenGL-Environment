#ifndef OGL_RENDER_H
#define OGL_RENDER_H

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <atomic>
#include <vector>
#include <iostream>

#include <graphics/resources.h>
#include <graphics/render_config.h>

#include "framebuffer.h"

namespace Resource {
    class GLTextureLoader;
    class GLModelRender;
    class GLFontLoader;

} // namespace Resource

class GLVertexData;

namespace glenv {
  class GLShader;
  const int MAX_DRAWS = 10000;

  //match in shaders
  const int MAX_2D_BATCH = 10000;
  const int MAX_3D_BATCH = 10000;
  const int MAX_3D_ANIM_BATCH = 1;

  const int MAX_BONES = 50;

  class GLRender {
  public:
      static bool LoadOpenGL();
      GLRender(GLFWwindow* window);
      GLRender(GLFWwindow* window, glm::vec2 target);
      ~GLRender();

      Resource::Texture LoadTexture(std::string filepath);
      Resource::Model Load3DModel(std::string filepath);
      Resource::Model Load3DModel(ModelInfo::Model &model);
      Resource::Model LoadAnimatedModel(std::string filepath,
					std::vector<Resource::ModelAnimation> *pGetAnimations);

      Resource::Font LoadFont(std::string filepath);
  
      void LoadResourcesToGPU();
      void UseLoadedResources();

      void Begin2DDraw();
      void Begin3DDraw();
      void BeginAnim3DDraw();
      void DrawModel(Resource::Model model, glm::mat4 modelMatrix,
		     glm::mat4 normalMat);
      void DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix,
			 glm::mat4 normalMatrix,
			 Resource::ModelAnimation *animation);
      void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
		    glm::vec4 colour, glm::vec4 texOffset);
      void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
		    glm::vec4 colour);
      void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix);
      void DrawString(Resource::Font font, std::string text, glm::vec2 position,
		      float size, float depth, glm::vec4 colour, float rotate);
      void DrawString(Resource::Font font, std::string text, glm::vec2 position,
		      float size, float depth, glm::vec4 colour);
      float MeasureString(Resource::Font font, std::string text, float size);
      void EndDraw(std::atomic<bool> &submit);

      void FramebufferResize();

      void set3DViewMatrixAndFov(glm::mat4 view, float fov, glm::vec4 camPos);
      void set2DViewMatrixAndScale(glm::mat4 view, float scale);
      void setLightDirection(glm::vec4 lightDir);
      void setForceTargetRes(bool force);
      bool isTargetResForced();
      void setTargetResolution(glm::vec2 resolution);
      glm::vec2 getTargetResolution();
      void setVsync(bool vsync);
      bool getVsync();

  private:
      void setupStagingResourceLoaders();
      Resource::Model loadModel(Resource::ModelType type, ModelInfo::Model model,
				std::vector<Resource::ModelAnimation> *pGetAnimations);
      Resource::Model loadModel(Resource::ModelType type, std::string filepath,
				std::vector<Resource::ModelAnimation> *pGetAnimations);
      void startDraw();
      void draw2DBatch(int drawCount, Resource::Texture texture,
		       glm::vec4 currentColour);
      void draw3DBatch(int drawCount, Resource::Model model);
      void draw3DAnim(Resource::Model model);

      void setVPlighting(GLShader *shader);

      struct LightingParameters {
	  LightingParameters() {
	      ambient = glm::vec4(1.0f, 1.0f, 1.0f, 0.35f);
	      diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
	      specular = glm::vec4(1.0f, 1.0f, 1.0f, 8.0f);
	      direction = glm::vec4(0.3f, -0.3f, -0.5f, 0.0f);
	      camPos = glm::vec4(0.0f);
	  }

	  alignas(16) glm::vec4 ambient;
	  alignas(16) glm::vec4 diffuse;
	  alignas(16) glm::vec4 specular;
	  alignas(16) glm::vec4 direction;
	  alignas(16) glm::vec4 camPos;
      };

      LightingParameters lighting;
      RenderConfig renderConf;

      GLFWwindow *window;
      glm::vec2 windowResolution;
      glm::vec2 targetResolution;

      GLShader *shader3D;
      GLShader *shader3DAnim;
      GLShader *flatShader;
      GLShader *finalShader;

      bool useOffscreenFramebuffer = true;
      Framebuffer* offscreenFramebuffer = nullptr;
      Framebuffer* offscreenBlitFramebuffer = nullptr;
      int msaaSamples = 1;

      glm::mat4 finalTransform = glm::mat4(1.0f);
      
      float scale2D = 1.0f; // TODO make 2D scale work
      glm::mat4 proj2D;
      glm::mat4 view2D;

      glm::mat4 proj3D;
      glm::mat4 view3D;
      float fov;

      bool inDraw = false;
    
      Resource::GLTextureLoader *stagingTextureLoader;
      Resource::GLFontLoader *stagingFontLoader;
      Resource::GLModelRender *stagingModelLoader;
      Resource::GLTextureLoader *textureLoader = nullptr;
      Resource::GLFontLoader *fontLoader = nullptr;
      Resource::GLModelRender *modelLoader = nullptr;

      enum class DrawMode {
	  d2D,
	  d3D,
	  d3DAnim,
      };

      void setShaderForMode(DrawMode mode, unsigned int i);

      struct Draw2D {
	  Draw2D() {}
	  Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset);
	  Resource::Texture tex;
	  glm::mat4 model;
	  glm::vec4 colour;
	  glm::vec4 texOffset;
      };
      struct Draw3D {
	  Draw3D() {}
	  Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix);
      
	  Resource::Model model;
	  glm::mat4 modelMatrix;
	  glm::mat4 normalMatrix;
      };
      struct DrawAnim3D {
	  DrawAnim3D() {}
	  DrawAnim3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix);
	  Resource::Model model;
	  glm::mat4 modelMatrix;
	  glm::mat4 normalMatrix;
	  glm::mat4 bones[MAX_BONES];
      };

      struct DrawCall {
	  DrawMode mode;
	  Draw2D d2D;
	  Draw3D d3D;
	  DrawAnim3D d3DAnim;
      };

      

      DrawMode currentDrawMode = DrawMode::d2D;
      unsigned int currentDraw = 0;
      DrawCall drawCalls[MAX_DRAWS];

      glm::mat4 perInstance2DModel[MAX_2D_BATCH];
      glm::vec4 perInstance2DTexOffset[MAX_2D_BATCH];
      GLuint model2DSSBO;
      GLuint texOffset2DSSBO;

      glm::mat4 perInstance3DModel[MAX_3D_BATCH];
      glm::mat4 perInstance3DNormal[MAX_3D_BATCH];
      GLuint model3DSSBO;
      GLuint normal3DSSBO;
  };

} // namespace glenv

#endif
