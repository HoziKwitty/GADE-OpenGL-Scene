#pragma once

#include <GL\glew.h>

class Mesh
{
public:
	Mesh();

	void createMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void createMeshFromHeightmap(std::vector<float> vertices, std::vector<unsigned int> indices);
	void renderMesh();
	void renderMeshFromHeightmap(int numStrips, int numVertsPerStrip);
	void clearMesh();

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};
