#include "SkyBox.h"
#include "gl\glew.h"
#include "gl\wglew.h"
#include "shaders\Shader.h"
#include "Images\nvImage.h";


int SkyBox::build(Shader* myShader)
{

	// First simple object
	verts[0] = -dim;   verts[1] = -dim;  verts[2] = -dim;
	verts[3] = -dim;   verts[4] = dim;  verts[5] = -dim;
	verts[6] = dim;   verts[7] = dim;  verts[8] = -dim;
	verts[9] = dim;   verts[10] = -dim;  verts[11] = -dim;

	verts[12] = -dim;   verts[13] = -dim;  verts[14] = dim;
	verts[15] = -dim;   verts[16] = dim;  verts[17] = dim;
	verts[18] = dim;   verts[19] = dim;  verts[20] = dim;
	verts[21] = dim;   verts[22] = -dim;  verts[23] = dim;


	tris[0] = 0; tris[1] = 1; tris[2] = 2;
	tris[3] = 0; tris[4] = 2; tris[5] = 3;
	tris[6] = 4; tris[7] = 6; tris[8] = 5;
	tris[9] = 4; tris[10] = 7; tris[11] = 6;
	tris[12] = 1; tris[13] = 5; tris[14] = 6;
	tris[15] = 1; tris[16] = 6; tris[17] = 2;
	tris[18] = 0; tris[19] = 7; tris[20] = 4;
	tris[21] = 0; tris[22] = 3; tris[23] = 7;
	tris[24] = 0; tris[25] = 5; tris[26] = 1;
	tris[27] = 0; tris[28] = 4; tris[29] = 5;
	tris[30] = 3; tris[31] = 2; tris[32] = 7;
	tris[33] = 2; tris[34] = 6; tris[35] = 7;

	// VAO allocation
	glGenVertexArrays(1, &VAO);

	// First VAO setup
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//initialises data storage of vertex buffer object
	glBufferData(GL_ARRAY_BUFFER, numOfVerts * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
	GLint vertexLocation = glGetAttribLocation(myShader->handle(), "in_Position");
	glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vertexLocation);





	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfTris * 3 * sizeof(unsigned int), tris, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	nv::Image img1;
	img1.loadImageFromFile("TestModels/textures/skybox/xpos.png");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, img1.getInternalFormat(), img1.getWidth(), img1.getHeight(), 0, img1.getFormat(), img1.getType(), img1.getLevel(0));

	nv::Image img2;
	img2.loadImageFromFile("TestModels/textures/skybox/xneg.png");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, img2.getInternalFormat(), img2.getWidth(), img2.getHeight(), 0, img2.getFormat(), img2.getType(), img2.getLevel(0));

	nv::Image img3;
	img3.loadImageFromFile("TestModels/textures/skybox/ypos.png");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, img3.getInternalFormat(), img3.getWidth(), img3.getHeight(), 0, img3.getFormat(), img3.getType(), img3.getLevel(0));

	nv::Image img4;
	img4.loadImageFromFile("TestModels/textures/skybox/yneg.png");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, img4.getInternalFormat(), img4.getWidth(), img4.getHeight(), 0, img4.getFormat(), img4.getType(), img4.getLevel(0));

	nv::Image img5;
	img5.loadImageFromFile("TestModels/textures/skybox/zpos.png");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, img5.getInternalFormat(), img5.getWidth(), img5.getHeight(), 0, img5.getFormat(), img5.getType(), img5.getLevel(0));

	nv::Image img6;
	img6.loadImageFromFile("TestModels/textures/skybox/zzneg.png");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, img6.getInternalFormat(), img6.getWidth(), img6.getHeight(), 0, img6.getFormat(), img6.getType(), img6.getLevel(0));

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return 0;

}

void SkyBox::render()
{

	
	//draw objects
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
	glBindVertexArray(VAO);		// select VAO

									//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDepthMask(GL_FALSE);
	glDrawElements(GL_TRIANGLES, numOfTris * 3, GL_UNSIGNED_INT, 0);
	glDepthMask(GL_TRUE);

	// Done

	glBindVertexArray(0); //unbind the vertex array object
	//

}


SkyBox::~SkyBox()
{
}
