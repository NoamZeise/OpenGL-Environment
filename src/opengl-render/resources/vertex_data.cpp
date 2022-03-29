#include "vertex_data.h"

VertexData::VertexData(std::vector<Vertex> &vertices, std::vector<unsigned int> &indicies)
{
	this->size = indicies.size();
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned int), indicies.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
}

VertexData::~VertexData()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
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
