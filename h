diff --git a/src/render.cpp b/src/render.cpp
index a8a08a3..c65da08 100644
--- a/src/render.cpp
+++ b/src/render.cpp
@@ -1,5 +1,12 @@
 #include "render.h"
 
+#include "ogl_helper.h"
+#include <glm/gtc/matrix_transform.hpp>
+#include <glm/gtc/matrix_inverse.hpp>
+#include <stdexcept>
+#include <config.h>
+#include <iostream>
+
 namespace glenv {
 GLRender::GLRender(GLFWwindow *window, glm::vec2 target)
 {
@@ -39,28 +46,11 @@ GLRender::GLRender(GLFWwindow *window, glm::vec2 target)
   std::vector<unsigned int> quadInds =  {0, 1, 2, 3, 4, 5};
   quad = new GLVertexData(quadVerts, quadInds);
 
-  glGenBuffers(1, &model3DSSBO);
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, model3DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance3DModel), &perInstance3DModel, GL_DYNAMIC_DRAW);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
-  glGenBuffers(1, &normal3DSSBO);
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal3DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance3DNormal), &perInstance3DNormal, GL_DYNAMIC_DRAW);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
-  glGenBuffers(1, &model2DSSBO);
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, model2DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance2DModel), &perInstance2DModel, GL_DYNAMIC_DRAW);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
-  glGenBuffers(1, &texOffset2DSSBO);
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, texOffset2DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance2DTexOffset), &perInstance2DTexOffset, GL_DYNAMIC_DRAW);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
+  ogl_helper::createShaderStorageBuffer(&model3DSSBO, sizeAndPtr(perInstance3DModel));
+  ogl_helper::createShaderStorageBuffer(&normal3DSSBO, sizeAndPtr(perInstance3DNormal));
+  ogl_helper::createShaderStorageBuffer(&model2DSSBO, sizeAndPtr(perInstance2DModel));  
+  ogl_helper::createShaderStorageBuffer(&texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset));
   setupStagingResourceLoaders();
-
   FramebufferResize();
 }
 
@@ -123,7 +113,6 @@ GLRender::Draw3D::Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4
   this->normalMatrix = normalMatrix;
 }
 
-
 void GLRender::Begin2DDraw()
 {
   current2DDraw = 0;
@@ -212,16 +201,8 @@ void GLRender::draw2DBatch(int drawCount, Resource::Texture texture, glm::vec4 c
 {
   glUniform4fv(flatShader->Location("spriteColour"), 1, &currentColour[0]);
 
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, model2DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance2DModel), perInstance2DModel, GL_DYNAMIC_DRAW);
-  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, model2DSSBO);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, texOffset2DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance2DTexOffset), perInstance2DTexOffset, GL_DYNAMIC_DRAW);
-  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, texOffset2DSSBO);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
+  ogl_helper::shaderStorageBufferData(model2DSSBO, sizeAndPtr(perInstance2DModel), 4);
+  ogl_helper::shaderStorageBufferData(texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset), 5);
   glActiveTexture(GL_TEXTURE0);
   textureLoader->Bind(texture);
   quad->DrawInstanced(GL_TRIANGLES, drawCount);
@@ -229,17 +210,9 @@ void GLRender::draw2DBatch(int drawCount, Resource::Texture texture, glm::vec4 c
 
 void GLRender::draw3DBatch(int drawCount, Resource::Model model)
 {
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, model3DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance3DModel), perInstance3DModel, GL_DYNAMIC_DRAW);
-  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, model3DSSBO);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
-  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal3DSSBO);
-  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(perInstance3DNormal), perInstance3DNormal, GL_DYNAMIC_DRAW);
-  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, normal3DSSBO);
-  glBindBuffer( GL_SHADER_STORAGE_BUFFER,0 );
-
-  modelLoader->DrawModelInstanced(model, textureLoader, drawCount, blinnPhongShader->Location("spriteColour"), blinnPhongShader->Location("enableTex"));
+    ogl_helper::shaderStorageBufferData(model3DSSBO, sizeAndPtr(perInstance3DModel), 2);
+    ogl_helper::shaderStorageBufferData(normal3DSSBO, sizeAndPtr(perInstance3DNormal), 3);
+    modelLoader->DrawModelInstanced(model, textureLoader, drawCount, blinnPhongShader->Location("spriteColour"), blinnPhongShader->Location("enableTex"));
 }
 
 void GLRender::DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat)
@@ -327,6 +300,7 @@ void GLRender::FramebufferResize()
       FramebufferResize();
     }
   }
+  
   bool GLRender::isTargetResForced() { return forceTargetResolution; }
 
   void GLRender::setTargetResolution(glm::vec2 resolution) {
@@ -334,12 +308,12 @@ void GLRender::FramebufferResize()
     forceTargetResolution = true;
     FramebufferResize();
   }
-  glm::vec2 GLRender::getTargetResolution() {
-    return targetResolution;
-  }
+
+  glm::vec2 GLRender::getTargetResolution() { return targetResolution; }
+  
   void GLRender::setVsync(bool vsync) {
     this->vsync = vsync;
     FramebufferResize();
   }
-
+  
 }//namespace
diff --git a/src/render.h b/src/render.h
index afaaf5e..de00c96 100644
--- a/src/render.h
+++ b/src/render.h
@@ -3,19 +3,13 @@
 
 #include <glad/glad.h>
 #include <GLFW/glfw3.h>
-
 #include <glm/glm.hpp>
-#include <glm/gtc/matrix_transform.hpp>
-#include <glm/gtc/matrix_inverse.hpp>
 
-#include <stdexcept>
-#include <iostream>
 #include <string>
 #include <atomic>
 #include <vector>
 
 #include "shader.h"
-#include <config.h>
 #include <resources/resources.h>
 #include "resources/vertex_data.h"
 #include "resources/texture_loader.h"
diff --git a/src/resources/model_loader.cpp b/src/resources/model_loader.cpp
index a691ed0..3892e71 100644
--- a/src/resources/model_loader.cpp
+++ b/src/resources/model_loader.cpp
@@ -170,6 +170,10 @@ void GLModelLoader::loadMaterials(Mesh* mesh, aiMaterial* material, GLTextureLoa
 		aiString aistring;
 		material->GetTexture(aiTextureType_DIFFUSE, i, &aistring);
 		std::string texLocation = aistring.C_Str();
+		for(int i = 0; i < texLocation.size(); i++) {
+		    if(texLocation[i] == '\\')
+			texLocation[i] = '/';
+		}
 		texLocation = "textures/" + texLocation;
 
 		bool skip = false;
diff --git a/src/resources/texture_loader.cpp b/src/resources/texture_loader.cpp
index 1fa8ecc..df6e5d1 100644
--- a/src/resources/texture_loader.cpp
+++ b/src/resources/texture_loader.cpp
@@ -22,10 +22,10 @@ GLTextureLoader::LoadedTex::LoadedTex(std::string path) {
   unsigned char *data =
       stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
   if (!data) {
-    std::cerr << "stb_image: failed to load texture at " << path << std::endl;
-    return;
+      std::cerr << "stb_image: failed to load texture at " << path << std::endl;
+      return;
   }
-
+  
   generateTexture(data, width, height, nrChannels);
 
   stbi_image_free(data);
