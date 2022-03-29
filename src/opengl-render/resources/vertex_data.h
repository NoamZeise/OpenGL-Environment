#ifndef VERTEX_DATA_H
#define VERTEX_DATA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

struct Vertex
{
	Vertex(float x, float y, float z, float tX, float tY)
	{
		this->position = glm::vec3(x, y, z);
		this->texCoords = glm::vec2(tX, tY);
	}
	Vertex(glm::vec3 pos, glm::vec2 texCoords)
	{
		this->position = pos;
		this->texCoords = texCoords;
	}
	glm::vec3 position;
	glm::vec2 texCoords;
};

class VertexData
{
public:
	VertexData() {}
	VertexData(std::vector<Vertex> &vertices, std::vector<unsigned int> &indicies);
	~VertexData();

	void Draw(unsigned int mode);
	void Draw(unsigned int mode, unsigned int verticies);

private:
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int size;
};



#endif
