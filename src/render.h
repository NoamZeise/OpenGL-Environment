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

#include <graphics/render.h>
#include <graphics/shader_structs.h>

#include "framebuffer.h"

class GLResourcePool;

namespace Resource {
  class GLModelRender;
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

  class RenderGl : public Render {
  public:
      static bool LoadOpenGL();
      RenderGl(GLFWwindow* window, RenderConfig renderConf);
      ~RenderGl();

      Resource::Texture LoadTexture(std::string filepath);
      Resource::Texture LoadTexture(Resource::ResourcePool pool, std::string path);
      // 4 channels assumed
      Resource::Texture LoadTexture(unsigned char* data, int width, int height);
      Resource::Texture LoadTexture(Resource::ResourcePool pool, unsigned char* data,
				    int width, int height);
      Resource::Model LoadModel(Resource::ModelType type, std::string filepath,
				std::vector<Resource::ModelAnimation> *pAnimations);
      Resource::Model LoadModel(Resource::ResourcePool pool, Resource::ModelType type,
				std::string filepath,
				std::vector<Resource::ModelAnimation> *pAnimations);
      Resource::Model LoadModel(Resource::ModelType type, ModelInfo::Model& model,
				std::vector<Resource::ModelAnimation> *pAnimations);
      Resource::Model LoadModel(Resource::ResourcePool pool, Resource::ModelType type,
				ModelInfo::Model& model,
				std::vector<Resource::ModelAnimation> *pAnimations);
      Resource::Model Load3DModel(std::string filepath);
      Resource::Model Load3DModel(ModelInfo::Model &model);
      Resource::Model LoadAnimatedModel(std::string filepath,
					std::vector<Resource::ModelAnimation> *pGetAnimations);

      Resource::Font LoadFont(std::string filepath);
      Resource::Font LoadFont(Resource::ResourcePool pool, std::string filepath);

      Resource::ResourcePool CreateResourcePool();
      void DestroyResourcePool(Resource::ResourcePool pool);
      // does notging in OGL version
      void setResourcePoolInUse(Resource::ResourcePool pool, bool usePool) {}
      
      void LoadResourcesToGPU();
      void LoadResourcesToGPU(Resource::ResourcePool pool);
      // does nothing in OGL version
      void UseLoadedResources() {}

      void DrawModel(Resource::Model model, glm::mat4 modelMatrix,
		     glm::mat4 normalMat);
      void DrawModel(Resource::Model model, glm::mat4 modelMatrix,
		     glm::mat4 normalMat, glm::vec4 colour);
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
      void setLightingProps(BPLighting lighting);
      void setTargetResolution(glm::vec2 resolution);
      glm::vec2 getTargetResolution();
      void setRenderConf(RenderConfig renderConf);
      RenderConfig getRenderConf();

  private:

      Resource::Model loadModel(Resource::ModelType type, ModelInfo::Model model,
				std::vector<Resource::ModelAnimation> *pGetAnimations);
      Resource::Model loadModel(Resource::ModelType type, std::string filepath,
				std::vector<Resource::ModelAnimation> *pGetAnimations);
      void draw2DBatch(int drawCount, Resource::Texture texture,
		       glm::vec4 currentColour);
      void draw3DBatch(int drawCount, Resource::Model model, glm::vec4 colour);
      void draw3DAnim(Resource::Model model);
      void setVPshader(GLShader *shader);
      void setLightingShader(GLShader *shader);

      bool _validPool(Resource::ResourcePool pool);
      void _throwIfPoolInvaid(Resource::ResourcePool pool);
      bool _poolInUse(Resource::ResourcePool pool);

      BPLighting lighting;
      RenderConfig renderConf;
      RenderConfig prevRenderConf;

      GLFWwindow *window;
      glm::vec2 windowResolution;

      GLShader *shader3D;
      GLShader *shader3DAnim;
      GLShader *flatShader;
      GLShader *finalShader;

      bool useOffscreenFramebuffer = true;
      GlFramebuffer* offscreenFramebuffer = nullptr;
      GlFramebuffer* offscreenBlitFramebuffer = nullptr;
      int msaaSamples = 1;

      glm::mat4 finalTransform = glm::mat4(1.0f);
      
      float scale2D = 1.0f; // TODO make 2D scale work
      glm::mat4 proj2D;
      glm::mat4 view2D;

      glm::mat4 proj3D;
      glm::mat4 view3D;
      float fov;

      bool inDraw = false;

      Resource::ResourcePool defaultPool;
      std::vector<GLResourcePool*> pools;
      std::vector<int> freePools;

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
	  Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix,
		 glm::vec4 colour);
      
	  Resource::Model model;
	  glm::mat4 modelMatrix;
	  glm::mat4 normalMatrix;
	  glm::vec4 colour;
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
