#ifndef GL_MODEL_LOADER_H
#define GL_MODEL_LOADER_H

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>
#include <assimp/matrix4x4.h>
#include <assimp/mesh.h>

#include <resources/model/model_info.h>

namespace Resource
{


class GLModelLoader
{
public:
    GLModelLoader();

    ModelInfo::Model LoadModel(std::string path);

private:

    Assimp::Importer importer;

    void processNode(ModelInfo::Model* model, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform, int parentNode);
    void processMesh(ModelInfo::Model* model, aiMesh* aimesh, const aiScene* scene, aiMatrix4x4 transform);
    void buildAnimation(ModelInfo::Model* model, aiAnimation* aiAnim);
};
}

#endif
