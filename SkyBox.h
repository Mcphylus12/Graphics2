#pragma once
#include "shaders\Shader.h"
class SkyBox
{
	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
	GLuint texID;

	float dim = 10.0f;
	float numOfVerts = 8;
	float numOfTris = 12;

	float verts[8 * 3];
	unsigned int tris[12 * 3];
	

public:
	int build(Shader* myShader);
	void render();
	~SkyBox();
};

