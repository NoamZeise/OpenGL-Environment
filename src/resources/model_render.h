#ifndef GL_MODEL_RENDER_H
#define GL_MODEL_RENDER_H

#ifndef NO_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include "vertex_data.h"
#include "texture_loader.h"

#include <resource_loader/model_loader.h>
#include <resource_loader/vertex_model.h>
#include <graphics/resources.h>

namespace Resource {

  class GLModelRender {
  public:
      GLModelRender();
      ~GLModelRender();
      Model loadModel(ModelType type, std::string path, GLTextureLoader *texLoader,
		      std::vector<ModelAnimation> *pGetAnimations);
      Model loadModel(ModelType type, ModelInfo::Model& model, GLTextureLoader *texLoader,
		      std::vector<ModelAnimation> *pGetAnimations);
      Model Load3DModel(std::string path, GLTextureLoader *texLoader);
      Model Load3DModel(ModelInfo::Model& model, GLTextureLoader *texLoader);
      void DrawQuad(int count);
      void DrawModel(Model model, GLTextureLoader *texLoader,
		     uint32_t spriteColourShaderLoc);
      void DrawModelInstanced(Model model, GLTextureLoader *texLoader, int count,
			      uint32_t spriteColourShaderLoc,
			      uint32_t enableTexShaderLoc);

  private:
      struct GLMesh {
	  GLVertexData *vertexData;
	  Texture texture;
	  glm::vec4 diffuseColour;
      };
      struct GLLoadedModel {
	  GLLoadedModel(){}
	  ~GLLoadedModel();
	  std::vector<GLMesh> meshes;
	  std::string directory;
      };

      void loadQuad();
      
      
      template <class T_Vert>
      Model loadModelInfo(ModelInfo::Model& model, ModelGroup<T_Vert>& modelGroup,
			  GLTextureLoader* texLoader, std::vector<ModelAnimation> *pGetAnimations);
      ModelInfo::Model loadModelFromFile(std::string path);
      template <class T_Vert>
      void addLoadedModel(LoadedModel<T_Vert>* model, GLTextureLoader *texLoader);
    
      ModelLoader modelLoader;
      std::vector<GLLoadedModel *> loadedModels;
      std::vector<Texture> loadedTextures;

      uint32_t quadIndex;

      uint32_t currentIndex = 0;
      ModelGroup<Vertex2D> loaded2D;
      ModelGroup<Vertex3D> loaded3D;
      ModelGroup<VertexAnim3D> loadedAnim3D;
  };

} // namespace Resource

#endif
