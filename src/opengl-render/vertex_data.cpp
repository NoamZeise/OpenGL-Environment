#include "vertex_data.h"


VertexData::VertexData(float* verticies, int verticesSize, unsigned int* indicies, int indicesSize)
{
	this->size = indicesSize;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(float), verticies, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(unsigned int), indicies, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

VertexData::~VertexData()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void VertexData::Draw(unsigned int mode)
{
	glBindVertexArray(VAO);
	glDrawElements(mode, size, GL_UNSIGNED_INT, 0);
}

void VertexData::Draw(unsigned int mode, unsigned int verticies)
{
	if (verticies > size)
		verticies = size;
	glBindVertexArray(VAO);
	glDrawElements(mode, verticies, GL_UNSIGNED_INT, 0);
}
