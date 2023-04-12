#ifndef GLVERTEX_DATA_H
#define GLVERTEX_DATA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vertex_types.h>

#include <vector>

class GLVertexData
{
public:
    GLVertexData() {}
    GLVertexData(std::vector<Vertex2D> &vertices, std::vector<unsigned int> &indicies);
    GLVertexData(std::vector<Vertex3D> &vertices, std::vector<unsigned int> &indicies);
    ~GLVertexData();
    
    void Draw(unsigned int mode);
    void DrawInstanced(unsigned int mode, int count);
    void Draw(unsigned int mode, unsigned int verticies);

private:
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint size;
};



#endif
