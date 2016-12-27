//OPENGL 3.2 DEMO FOR RENDERING OBJECTS LOADED FROM OBJ FILES

//includes areas for keyboard control, mouse control, resizing the window
//and draws a spinning rectangle

#define TRIANGLECOLS 1

#include <windows.h>		// Header File For Windows
#include "gl/glew.h"
#include "gl/wglew.h"
#pragma comment(lib, "glew32.lib")

#include "console.h"
#include "shaders/Shader.h"   // include shader header file, this is not part of OpenGL

#include <algorithm>

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\matrix_inverse.hpp"
#include "Images\nvImage.h"

glm::mat4 objectRotation;
glm::quat q;
double fps;
double updatetime = 0;
#define TARGET_FPS 60


GLuint textureID;


ConsoleWindow console;
Shader* dayShader;  ///shader object 
Shader* nightShader;
Shader* activeShader;
Shader* skyShader;

//MODEL LOADING
#include "3DStruct\threeDModel.h"
#include "Obj\OBJLoader.h"
#include "Octree\Octree.h"
#include "Car.h"
#include "SkyBox.h"

float amount = 0;
float temp = 0.002f;
int viewingMode = 0;
bool cDown = false;
bool lDown = false;
bool kDown = false;
Car car;
Car town;
Car townfloor;
ThreeDModel wheel;
float wheelrot = 0;
float camx = 0, camy = 0, camz = 0;
float camxrot = 0, camyrot = 0;
float ThirdCamOffset = 0;

SkyBox sky;

OBJLoader objLoader;
///END MODEL LOADING

glm::mat4 ProjectionMatrix; // matrix for the orthographic projection
glm::mat4 ModelViewMatrix;  // matrix for the modelling and viewing

//Material properties

//Light Properties
bool night = false;

bool triColEnabled = false;

//Day
float Light_Ambient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
float Light_Diffuse[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
float Light_Specular[4] = { 0.5f,0.5f,0.5f,1.0f };
float LightPos[3] = { 0.0f, 200.0f, 0.0f };

float nightLight_Diffuse[4] = { 0.8f, 0.8f, 0.6f, 1.0f };
float nightLight_Ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
float nightLight_Specular[4] = { 0.0f,0.0f,0.0f,0.0f };




//
int	mouse_x=0, mouse_y=0;
bool LeftPressed = false;
int screenWidth=600, screenHeight=600;
bool keys[256];
float spin=180;
float speed=0;

//OPENGL FUNCTION PROTOTYPES
void display();				//called in winmain to draw everything to the screen
void reshape(int width, int height);				//called when the window is resized
void init();				//called in winmain when the program starts.
void processKeys();         //called in winmain to process keyboard input
void update();				//called in winmain to update variables
void updateTransform(float xinc, float yinc, float zinc);

/*************    START OF OPENGL FUNCTIONS   ****************/

float project(glm::vec4 axes, glm::vec4 point) {
	glm::vec3 testAxes = glm::vec3(axes);
	glm::vec3 testPoint = glm::vec3(point);
	return (testAxes.x * testPoint.x + testAxes.y * testPoint.y + testAxes.z * testPoint.z)/glm::length(testAxes);
}


std::vector<glm::vec4> transform(std::vector<glm::vec4>* mat, glm::mat4 transformationMat) {
	std::vector<glm::vec4> result;
	for (int i = 0; i < mat->size(); i++) {
		result.push_back(transformationMat * mat->at(i));
	}
	return result;
}
bool sat(std::vector<glm::vec4>* boxA,
	std::vector<glm::vec4>* boxB) {
	glm::vec4 aHorizontal = boxA->at(1) - boxA->at(0);
	glm::vec4 aVertical =	boxA->at(2) - boxA->at(0);
	glm::vec4 aDeep =		boxA->at(4) - boxA->at(0);
	glm::vec4 bHorizontal = boxB->at(1) - boxB->at(0);
	glm::vec4 bVertical =	boxB->at(2) - boxB->at(0);
	glm::vec4 bDeep =		boxB->at(4) - boxB->at(0);
	std::vector<glm::vec4> axes;
	axes.push_back(aHorizontal);
	axes.push_back(aVertical);
	axes.push_back(aDeep);
	axes.push_back(bHorizontal);
	axes.push_back(bVertical);
	axes.push_back(bDeep);
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aHorizontal), glm::vec3(bHorizontal)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aHorizontal), glm::vec3(bVertical)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aHorizontal), glm::vec3(bDeep)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aVertical), glm::vec3(bHorizontal)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aVertical), glm::vec3(bVertical)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aVertical), glm::vec3(bDeep)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aDeep), glm::vec3(bHorizontal)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aDeep), glm::vec3(bVertical)), 1.0f));
	axes.push_back(glm::vec4(glm::cross(glm::vec3(aDeep), glm::vec3(bDeep)), 1.0f));



	for (int i = 0; i < axes.size(); i++) {

		float boxAMin = project(axes[i], boxA->at(0)), boxAMax = boxAMin;
		float boxBMin = project(axes[i], boxB->at(0)), boxBMax = boxBMin;

		for (int j = 1; j < 8; j++) {
			float a = project(axes[i], boxA->at(j));
			boxAMin = std::min(boxAMin, a);
			boxAMax = std::max(boxAMax, a);
		}

		for (int j = 1; j < 8; j++) {
			float b = project(axes[i], boxB->at(j));
			boxBMin = std::min(boxBMin, b);
			boxBMax = std::max(boxBMax, b);
		}

		if (boxAMax < boxBMin || boxBMax < boxAMin) {
			return false;
		}
	}




	return true;
	
}

std::vector<glm::vec4> buildCornerList(Octree* tree) {
	std::vector<glm::vec4> result;
	result.push_back(glm::vec4(tree->minX, tree->minY, tree->minZ, 1.0f));
	result.push_back(glm::vec4(tree->maxX, tree->minY, tree->minZ, 1.0f));
	result.push_back(glm::vec4(tree->minX, tree->minY, tree->maxZ, 1.0f));
	result.push_back(glm::vec4(tree->maxX, tree->minY, tree->maxZ, 1.0f));
	result.push_back(glm::vec4(tree->minX, tree->maxY, tree->minZ, 1.0f));
	result.push_back(glm::vec4(tree->maxX, tree->maxY, tree->minZ, 1.0f));
	result.push_back(glm::vec4(tree->minX, tree->maxY, tree->maxZ, 1.0f));
	result.push_back(glm::vec4(tree->maxX, tree->maxY, tree->maxZ, 1.0f));
	return result;
}

glm::vec4 vec3dtoglmvec4(Vector3d in) {
	return glm::vec4(in.x, in.y, in.z, 1.0f);
}
glm::vec3 vec3dtoglmvec3(Vector3d in) {
	return glm::vec3(in.x, in.y, in.z);
}

//Moller and Trombore Ray Triangle intersection modified to glm vectors
int rayIntersectsTriangle(glm::vec3 p, glm::vec3 d,
	glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {



	glm::vec3 e1 = v1 - v0;
	glm::vec3 e2 = v2 - v0;


	glm::vec3 h = glm::cross(d, e2);

	float a = glm::dot(e1, h);

	if (a > -0.00001 && a < 0.00001)
		return(false);

	float f = 1 / a;

	glm::vec3 s = p - v0;
	float u = f*glm::dot(s, h);

	if (u < 0.0 || u > 1.0)
		return(false);

	glm::vec3 q = glm::cross(s, e1);
	float v = f*glm::dot(d, q);


	if (v < 0.0 || u + v > 1.0)
		return(false);

	float t = f * glm::dot(e2, q);

	if (t > 0.00001 && t < 1) 
		return(true);

	else return (false);

}

bool checkCollision(ThreeDModel* carobj, Octree* carTree, ThreeDModel* building, Octree* buildTree, glm::mat4* oldcarmat, glm::mat4* newcarmat) {

	std::vector<glm::vec4> carcol = buildCornerList(carTree);
	std::vector<glm::vec4> buildcol = buildCornerList(buildTree);

	std::vector<glm::vec4> newcarCol = transform(&carcol, *newcarmat);

	if (sat(&newcarCol, &buildcol)) {
		bool carHasChildren = false;
		bool buildHasChildren = false;
		bool collision = false;
		for (int i = 0; i < 8; i++) {




			Octree * buildchild = buildTree->children[i];
			if (buildchild != NULL) {
				buildHasChildren = true;
				for (int j = 0; j < 8; j++) {
					Octree * carchild = carTree->children[j];
					
					if (carchild != NULL) {
						carHasChildren = true;
						if (checkCollision(carobj, carchild, building, buildchild, oldcarmat, newcarmat))
							collision = true;
					}

				}
			}
		}
		if (!carHasChildren && !buildHasChildren) {


			if (triColEnabled){
				for (int i = 0; i < carTree->getPrimitiveListSize(); i++) {
					for (int j = 0; j < 3; j++) {
						glm::vec4 newvecpos = *newcarmat * vec3dtoglmvec4(carobj->theVerts[carobj->theFaces[carTree->getPrimitiveList()[i]].thePoints[j]]);
						glm::vec4 oldvecpos = *oldcarmat * vec3dtoglmvec4(carobj->theVerts[carobj->theFaces[carTree->getPrimitiveList()[i]].thePoints[j]]);
						bool clean = true;
						for (int i = 0; i < buildTree->getPrimitiveListSize(); i++) {
							//glm::vec4 buildNormal = vec3dtoglmvec4(building->theFaceNormals[building->theFaces[buildTree->getPrimitiveList()[i]].theFaceNormal]);
							glm::vec3 v0 = vec3dtoglmvec3(building->theVerts[building->theFaces[buildTree->getPrimitiveList()[i]].thePoints[0]]);
							glm::vec3 v1 = vec3dtoglmvec3(building->theVerts[building->theFaces[buildTree->getPrimitiveList()[i]].thePoints[1]]);
							glm::vec3 v2 = vec3dtoglmvec3(building->theVerts[building->theFaces[buildTree->getPrimitiveList()[i]].thePoints[2]]);
							if (rayIntersectsTriangle(glm::vec3(oldvecpos), glm::vec3(newvecpos - oldvecpos), v0, v1, v2)) {
								return true;
							}

						}
					}
				}
				return false;
			} else 
				return true;

		}
		else {
			if (collision) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		//std::cout << "sat fails" << endl;
		return false;
	}


}

void display()									
{

	if (speed > 5) speed = 5;
	if (speed < -5) speed = -5;
	if (night == true) {
		activeShader = nightShader;
	}
	else {
		activeShader = dayShader;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);

	//glUseProgram(activeShader->handle());  // use the shader

	//glUseProgram(nightShader->handle());  // use the shader


	//amount = 0;
	//glUniform1f(glGetUniformLocation(dayShader->handle(), "displacement"), amount);



	glm::mat4 carModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(car.x, car.y, car.z));
	carModelMatrix = glm::rotate(carModelMatrix, car.r, glm::vec3(0, 1, 0));

	glm::vec3 carForward = glm::vec3(carModelMatrix * glm::vec4(0,0,1,0));
	glm::vec3 carRight = glm::vec3(carModelMatrix * glm::vec4(1, 0, 0, 0));
	carModelMatrix = glm::scale(carModelMatrix, glm::vec3(0.1, 0.1, 0.1));
	glm::mat4 camModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(camx, camy, camz));
	camModelMatrix = glm::rotate(camModelMatrix, car.r, glm::vec3(0, 1, 0));

	glm::vec3 camDirection = glm::vec3(camModelMatrix * glm::vec4(0, 0, 1, 0));

	glm::mat4 viewingMatrix;
	if (viewingMode == 1) {
		// 1st
		viewingMatrix = glm::lookAt(glm::vec3(car.x - carForward.x*2, car.y+1, car.z - carForward.z*2), glm::vec3(car.x - carForward.x * 5, car.y+1, car.z - 5 * carForward.z), glm::vec3(0, 1, 0));
	}
	else if (viewingMode == 0) {
		// 3rd
		viewingMatrix = glm::lookAt(car.getPos() + carForward * 10.0f - carRight * (ThirdCamOffset) + glm::vec3(0, 8, 0), glm::vec3(car.x, car.y + 3, car.z), glm::vec3(0, 1, 0));
	}
	else if (viewingMode == 2) {
		//top
		viewingMatrix = glm::lookAt(glm::vec3(car.x, car.y + 100, car.z), glm::vec3(car.x, car.y, car.z), glm::vec3(-carForward.x, 0, -carForward.z));
	}
	else if (viewingMode == 3) {
		viewingMatrix = glm::lookAt(glm::vec3(camx, camy, camz), glm::vec3(car.x, car.y, car.z), glm::vec3(0, 1, 0));
	}

	glUseProgram(skyShader->handle());
	GLuint matLocation = glGetUniformLocation(skyShader->handle(), "projection");
	glUniformMatrix4fv(matLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glm::mat4 skyview = glm::mat4(glm::mat3(viewingMatrix));
	glUniformMatrix4fv(glGetUniformLocation(skyShader->handle(), "view"), 1, GL_FALSE, &skyview[0][0]);
	sky.render();

	glUseProgram(activeShader->handle());

	GLuint skymatLocation = glGetUniformLocation(activeShader->handle(), "ProjectionMatrix");
	glUniformMatrix4fv(skymatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(activeShader->handle(), "ViewMatrix"), 1, GL_FALSE, &viewingMatrix[0][0]);



	if (night == true) {
		glm::vec3 light1 = car.getPos() - carForward * 3.0f - carRight * 2.0f;
		glm::vec3 light2 = car.getPos() - carForward * 3.0f + carRight * 2.0f;
		glm::vec3 lightFor = -carForward;
		glUniform3fv(glGetUniformLocation(activeShader->handle(), "LightPos1"), 1, &light1[0]);
		glUniform3fv(glGetUniformLocation(activeShader->handle(), "LightPos2"), 1, &light2[0]);
		glUniform3fv(glGetUniformLocation(activeShader->handle(), "LightFor"), 1, &lightFor[0]);
		glUniform4fv(glGetUniformLocation(activeShader->handle(), "light_ambient"), 1, nightLight_Ambient);
		glUniform4fv(glGetUniformLocation(activeShader->handle(), "light_diffuse"), 1, nightLight_Diffuse);
		glUniform4fv(glGetUniformLocation(activeShader->handle(), "light_specular"), 1, nightLight_Specular);
	}
	else {
		glUniform3fv(glGetUniformLocation(activeShader->handle(), "LightPos1"), 1, LightPos);
		glUniform4fv(glGetUniformLocation(activeShader->handle(), "light_ambient"), 1, Light_Ambient);
		glUniform4fv(glGetUniformLocation(activeShader->handle(), "light_diffuse"), 1, Light_Diffuse);
		glUniform4fv(glGetUniformLocation(activeShader->handle(), "light_specular"), 1, Light_Specular);
	}



	glUniform4fv(glGetUniformLocation(activeShader->handle(), "material_ambient"), 1, &town.Material_Ambient[0]);
	glUniform4fv(glGetUniformLocation(activeShader->handle(), "material_diffuse"), 1, &town.Material_Diffuse[0]);
	glUniform4fv(glGetUniformLocation(activeShader->handle(), "material_specular"), 1, &town.Material_Specular[0]);
	glUniform1f(glGetUniformLocation(activeShader->handle(), "material_shininess"), town.Material_Shininess);

	//DRAW THE MODEL
	glm::mat4 buildmodel = glm::mat4(1.0);
	ModelViewMatrix = viewingMatrix * buildmodel;
	
	glUniformMatrix4fv(glGetUniformLocation(activeShader->handle(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(buildmodel));
	glUniformMatrix3fv(glGetUniformLocation(activeShader->handle(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glLineWidth(5.0);
	for (int i = 0; i < town.buildings.size(); i++) {
		town.buildings[i]->drawElementsUsingVBO(activeShader);
		//town.buildings[i]->drawBoundingBox(myShader);
	}
	townfloor.model.drawElementsUsingVBO(activeShader);
	
	glUniform4fv(glGetUniformLocation(activeShader->handle(), "material_ambient"), 1, &car.Material_Ambient[0]);
	glUniform4fv(glGetUniformLocation(activeShader->handle(), "material_diffuse"), 1, &car.Material_Diffuse[0]);
	glUniform4fv(glGetUniformLocation(activeShader->handle(), "material_specular"), 1, &car.Material_Specular[0]);
	glUniform1f(glGetUniformLocation(activeShader->handle(), "material_shininess"), car.Material_Shininess);

	//glUseProgram(nightShader->handle());  // use the shader
	//glUniformMatrix4fv(glGetUniformLocation(nightShader->handle(), "ProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	//glUniformMatrix4fv(glGetUniformLocation(nightShader->handle(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	
	//model.drawBoundingBox(myBasicShader);
	//model.drawOctreeLeaves(myBasicShader);
	
	
	
	ModelViewMatrix = viewingMatrix * carModelMatrix;

	float newcarx = car.x + speed*carModelMatrix[2][0] * updatetime;
	//car.posY += ModelMatrix[2][1];
	float newcarz = car.z + speed*carModelMatrix[2][2] * updatetime;
	float newcarr = car.r - car.ra * speed *  0.5 * updatetime;
		 

	glm::mat4 newcarmat = glm::translate(glm::mat4(1.0), glm::vec3(newcarx, car.y, newcarz));
	newcarmat = glm::rotate(newcarmat, newcarr, glm::vec3(0, 1, 0));
	newcarmat = glm::scale(newcarmat, glm::vec3(0.1, 0.1, 0.1));

	bool collision = false;
	glm::vec4 carCenter = newcarmat * glm::vec4(car.model.theBBox.centrePoint.x, car.model.theBBox.centrePoint.y, car.model.theBBox.centrePoint.z, 1.0f);
	float radius = car.model.theBBox.getLargestExtent();
	for (int i = 0; i < town.buildings.size(); i++) {
		glm::vec4 buildCenter = glm::vec4(town.buildings[i]->theBBox.centrePoint.x, town.buildings[i]->theBBox.centrePoint.y, town.buildings[i]->theBBox.centrePoint.z, 1.0f);
		float buildRad = town.buildings[i]->theBBox.getLargestExtent();
		if ((carCenter.x - buildCenter.x) * (carCenter.x - buildCenter.x) +
			(carCenter.y - buildCenter.y) * (carCenter.y - buildCenter.y) +
			(carCenter.z - buildCenter.z) * (carCenter.z - buildCenter.z) < (radius + buildRad) * (radius + buildRad)) {
			Octree* tree = town.buildings[i]->octree;
			if (checkCollision(&car.model, car.model.octree, town.buildings[i], town.buildings[i]->octree, &carModelMatrix, &newcarmat)) {
				collision = true;

			}
		}



	}
	if (collision == false) {


		car.x = newcarx;
		car.z = newcarz;

		if (speed != 0) {
			car.r = newcarr;
		}

	}
	else {
		//std::cout << "collided" << endl;

		speed = -speed*0.3;


		//if (speed < 0.1) speed = 0;
	}


/*	glm::vec4 worldtest;
	bool collision = false;
	for (int i = 0; i < 24; i += 3) {
		glm::vec4 testvec = glm::vec4(car.cPoints[i], car.cPoints[i + 1], car.cPoints[i + 2], 1);
		worldtest = carModelMatrix * testvec;
		for (int i = 0; i < town.buildings.size(); i++) {
			Octree* oc = town.buildings[i]->octree->getRegionFromPoint(worldtest.x, worldtest.y, worldtest.z);
			if (oc != NULL) {
				std::cout << "collision";
				oc->drawBoundingBox(myShader);
				collision = true;
			}
			else {

			}
		}
	}
	if (collision == false) {

	}
*/
	if (speed != 0) {
		wheelrot += speed * updatetime;
		if (wheelrot > 360) wheelrot -= 360;
		if (wheelrot < 0) wheelrot += 360;
	}

	
	if (speed > 0) {
		if (speed - 0.05 * updatetime <= 0) 
			speed = 0;
		else
			speed -= 0.1 * updatetime;
	}
	if (speed < 0) {
		if (speed + 0.05*updatetime >= 0)
			speed = 0;
		else
			speed += 0.1 * updatetime;
	}

	if (abs(car.ra) - 0.05*updatetime < 0) car.ra = 0;
	if (car.ra > 0) car.ra -= 0.05 * updatetime;
	if (car.ra < 0) car.ra += 0.05 * updatetime;
	if (car.r > 360) car.r -= 360;
	if (car.r < 0) car.r += 360;
//	float diff = car.ra - ThirdCamOffset;
//	ThirdCamOffset += abs(diff) / diff * updatetime;

	normalMatrix = glm::inverseTranspose(glm::mat3(carModelMatrix));
	glUniformMatrix3fv(glGetUniformLocation(activeShader->handle(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	
	//Pass the uniform for the modelview matrix - in this case just "r"
	glUniformMatrix4fv(glGetUniformLocation(activeShader->handle(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	if (viewingMode != 1) {
		car.model.drawElementsUsingVBO(activeShader);

		for (int i = 0; i < 30; i += 3) {
			glm::mat4 wheelmat = glm::translate(carModelMatrix, glm::vec3(car.wheelPoints[i] + 3, car.wheelPoints[i + 1] - 25, car.wheelPoints[i + 2]));
			if (i == 9 || i == 12 || i == 24 || i == 27) {
				wheelmat = glm::rotate(wheelmat, (float)(car.ra * 20), glm::vec3(0, 1, 0));
			}
			wheelmat = glm::rotate(wheelmat, wheelrot, glm::vec3(1, 0, 0));

			ModelViewMatrix = viewingMatrix * wheelmat;
			normalMatrix = glm::inverseTranspose(glm::mat3(wheelmat));
			glUniformMatrix3fv(glGetUniformLocation(activeShader->handle(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(activeShader->handle(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
			wheel.drawElementsUsingVBO(activeShader);
		}
	}
	glFlush();
}



void reshape(int width, int height)		// Resize the OpenGL window
{
	screenWidth=width; screenHeight = height;           // to ensure the mouse coordinates match 
														// we will use these values to set the coordinate system

	glViewport(0,0,width,height);						// Reset The Current Viewport

	//Set the projection matrix
	ProjectionMatrix = glm::perspective(60.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 1.0f, 2000.0f);
}
void loadBuilding(char* name, std::vector<ThreeDModel*>* modellist);
void init()
{
	glClearColor(0.529, 0.807, 0.921,0.0);
														//glClear(GL_COLOR_BUFFER_BIT) in the display function
														//will clear the buffer to this colour
	glEnable(GL_DEPTH_TEST);
	

	dayShader = new Shader;
	//if(!myShader->load("BasicView", "glslfiles/basicTransformationsWithDisplacement.vert", "glslfiles/basicTransformationsWithDisplacement.frag"))
    if(!dayShader->load("BasicView", "glslfiles/basicTransformations.vert", "glslfiles/basicTransformations.frag"))
	{
		cout << "failed to load shader" << endl;
	}		

	nightShader = new Shader;
	if(!nightShader->load("Basic", "glslfiles/nightbasicTransformations.vert", "glslfiles/nightbasicTransformations.frag"))
	{
		cout << "failed to load shader" << endl;
	}		

	skyShader = new Shader;
	if (!skyShader->load("sky", "glslfiles/SkyboxShade.vert", "glslfiles/SkyboxShade.frag"))
	{
		cout << "failed to load shader" << endl;
	}

	sky.build(skyShader);

	glUseProgram(skyShader->handle());

	glEnable(GL_TEXTURE_CUBE_MAP);

	glUseProgram(dayShader->handle());  // use the shader

	glEnable(GL_TEXTURE_2D);

	glUseProgram(nightShader->handle());  // use the shader

	glEnable(GL_TEXTURE_2D);

	
	townfloor.Material_Ambient = glm::vec4(0.8f, 0.8f, 1.0f, 1.0f);
	townfloor.Material_Diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	townfloor.Material_Specular = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	townfloor.Material_Shininess = 50;
	cout << " loading model " << endl;
	if (objLoader.loadModel("TestModels/town_ground.obj", townfloor.model))//returns true if the model is loaded, puts the model in the model parameter
	{
		cout << " model loaded " << endl;

		//if you want to translate the object to the origin of the screen,
		//first calculate the centre of the object, then move all the vertices
		//back so that the centre is on the origin.
		townfloor.model.calcCentrePoint();
		//model.centreOnZero();


		townfloor.model.calcVertNormalsUsingOctree();  //the method will construct the octree if it hasn't already been created.


												  //turn on VBO by setting useVBO to true in threeDmodel.cpp default constructor - only permitted on 8 series cards and higher
		townfloor.model.initDrawElements();
		townfloor.model.initVBO(dayShader);
		//townfloor.model.deleteVertexFaceData();

	}
	else
	{
		cout << " model failed to load " << endl;
	}

	town.Material_Ambient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	town.Material_Diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	town.Material_Specular = glm::vec4( 0.0f,0.0f,0.0f,0.0f );
	town.Material_Shininess = 50;

	//loadBuilding("TestModels/town1.obj", &town.buildings);
	//loadBuilding("TestModels/town8.obj", &town.buildings);
	loadBuilding("TestModels/town9.obj", &town.buildings);
	loadBuilding("TestModels/town11.obj", &town.buildings);
	loadBuilding("TestModels/town12.obj", &town.buildings);
	loadBuilding("TestModels/town13.obj", &town.buildings);
	loadBuilding("TestModels/town14.obj", &town.buildings);
	loadBuilding("TestModels/town15.obj", &town.buildings);
	loadBuilding("TestModels/town16.obj", &town.buildings);
	loadBuilding("TestModels/town17.obj", &town.buildings);
	loadBuilding("TestModels/town19.obj", &town.buildings);
	loadBuilding("TestModels/town20.obj", &town.buildings);
	loadBuilding("TestModels/town21.obj", &town.buildings);
	loadBuilding("TestModels/town22.obj", &town.buildings);
	loadBuilding("TestModels/town23.obj", &town.buildings);
	loadBuilding("TestModels/town24.obj", &town.buildings);
	loadBuilding("TestModels/town25.obj", &town.buildings);
	loadBuilding("TestModels/town26.obj", &town.buildings);
	loadBuilding("TestModels/town27.obj", &town.buildings);
	loadBuilding("TestModels/town28.obj", &town.buildings);
	loadBuilding("TestModels/town29.obj", &town.buildings);
	loadBuilding("TestModels/town30.obj", &town.buildings);
	loadBuilding("TestModels/town31.obj", &town.buildings);
	loadBuilding("TestModels/town32.obj", &town.buildings);
	loadBuilding("TestModels/town040.obj", &town.buildings);
	loadBuilding("TestModels/town041.obj", &town.buildings);
	loadBuilding("TestModels/town042.obj", &town.buildings);


	car.Material_Ambient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	car.Material_Diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	car.Material_Specular = glm::vec4(0.9f, 0.9f, 0.8f, 1.0f);
	car.Material_Shininess = 50;
	car.z = 0;
	car.x = -5;
	car.y = 3;
	car.r = 0;
	if(objLoader.loadModel("TestModels/car.obj", car.model))//returns true if the model is loaded, puts the model in the model parameter
	{
		cout << " model loaded " << endl;		

		//if you want to translate the object to the origin of the screen,
		//first calculate the centre of the object, then move all the vertices
		//back so that the centre is on the origin.
		car.model.calcCentrePoint();
		car.model.centreOnZero();

	
		car.model.calcVertNormalsUsingOctree();  //the method will construct the octree if it hasn't already been created.
				

		//turn on VBO by setting useVBO to true in threeDmodel.cpp default constructor - only permitted on 8 series cards and higher
		car.model.initDrawElements();
		car.model.initVBO(dayShader);
		//car.model.deleteVertexFaceData();
		
	}
	else
	{
		cout << " model failed to load " << endl;
	}

	if (objLoader.loadModel("TestModels/wheel.obj", wheel))//returns true if the model is loaded, puts the model in the model parameter
	{
		cout << " model loaded " << endl;

		//if you want to translate the object to the origin of the screen,
		//first calculate the centre of the object, then move all the vertices
		//back so that the centre is on the origin.
		wheel.calcCentrePoint();
		wheel.centreOnZero();


		wheel.calcVertNormalsUsingOctree();  //the method will construct the octree if it hasn't already been created.


												 //turn on VBO by setting useVBO to true in threeDmodel.cpp default constructor - only permitted on 8 series cards and higher
		wheel.initDrawElements();
		wheel.initVBO(dayShader);
		//wheel.deleteVertexFaceData();

	}
	else
	{
		cout << " model failed to load " << endl;
	}



}



void loadBuilding(char* name, std::vector<ThreeDModel*>* modellist) {
	cout << " loading model " << endl;
	ThreeDModel* temp = new ThreeDModel();
	if (objLoader.loadModel(name, *temp))//returns true if the model is loaded, puts the model in the model parameter
	{
		cout << " model loaded " << endl;

		//if you want to translate the object to the origin of the screen,
		//first calculate the centre of the object, then move all the vertices
		//back so that the centre is on the origin.
		temp->calcCentrePoint();
		//model.centreOnZero();


		temp->calcVertNormalsUsingOctree();  //the method will construct the octree if it hasn't already been created.


											//turn on VBO by setting useVBO to true in threeDmodel.cpp default constructor - only permitted on 8 series cards and higher
		temp->initDrawElements();
		temp->initVBO(dayShader);
		//temp->deleteVertexFaceData();

	}
	else
	{
		cout << " model failed to load " << endl;
	}
	modellist->push_back(temp);
}

void processKeys()
{
	if (keys[VK_RIGHT])
	{
		if(car.ra > -0.7) car.ra -= 0.1 * updatetime;


	}
	if (keys[VK_LEFT])
	{

			if(car.ra < 0.7 ) car.ra += 0.1 * updatetime;
			


	}
	if (keys[VK_UP])
	{
		if (speed > -5) speed -= 0.15 * updatetime;
	}
	if (keys[VK_DOWN])
	{
		if (speed < 5) speed += 0.15 * updatetime;
	}
	if (keys['W'])
	{
		camx += updatetime;


	}
	if (keys['A'])
	{

		camz += updatetime;



	}
	if (keys['S'])
	{
		camx -= updatetime;
	}
	if (keys['D'])
	{
		camz -= updatetime;
	}
	if (keys[VK_SPACE])
	{
		camy += updatetime;
	}
	if (keys[VK_SHIFT])
	{
		camy -= updatetime;
	}
	if (keys['C']) {
		if (cDown == false) {
			viewingMode += 1;
			viewingMode %= 4;
		}


		cDown = true;
	}
	else {
		cDown = false;
	}
	if (keys['L']) {
		if (lDown == false) {
			night = !night;
		}


		lDown = true;
	}
	else {
		lDown = false;
	}

	if (keys['K']) {
		if (kDown == false) {
			triColEnabled = !triColEnabled;
			std::cout << "tri collision is " << (triColEnabled ? "on" : "off") << endl;
		}

		
		kDown = true;
	}
	else {
		kDown = false;
	}


}
void updateTransform(float xinc, float yinc, float zinc)
{/*
	glm::mat4 matrixX, matrixXY;
	
	//rotation about the local x axis
	q = glm::angleAxis(xinc, glm::vec3(objectRotation[0][0], objectRotation[0][1], objectRotation[0][2])); 
	matrixX = glm::mat4_cast(q) * objectRotation;

	//EXAMPLE FOR ACCESSING USING A 1D array
	const float *pSource = (const float*)glm::value_ptr(matrixX);
	//rotation about the local y axis
	q = glm::angleAxis(yinc, glm::vec3(pSource[4], pSource[5], pSource[6])); 
	matrixXY = glm::mat4_cast(q) * matrixX;
	
	//EXAMPLE ACCESSING WITH 2D GLM structure.
	//rotation about the local z axis
	q = glm::angleAxis(zinc, glm::vec3(matrixXY[2][0], matrixXY[2][1], matrixXY[2][2])); 
	objectRotation = glm::mat4_cast(q) * matrixXY;
	*/
}
void update()
{

}
/**************** END OPENGL FUNCTIONS *************************/

//WIN32 functions
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
void KillGLWindow();									// releases and destroys the window
bool CreateGLWindow(char* title, int width, int height); //creates the window
int WINAPI WinMain(	HINSTANCE, HINSTANCE, LPSTR, int);  // Win32 main function

//win32 global variabless
HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application


/******************* WIN32 FUNCTIONS ***************************/
int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	bool	done=false;								// Bool Variable To Exit Loop

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);


	// Create Our OpenGL Window
	if (!CreateGLWindow("OpenGL Win32 Example",screenWidth,screenHeight))
	{
		return 0;									// Quit If Window Was Not Created
	}
	LARGE_INTEGER perfValue;
	LARGE_INTEGER oldValue;
	LARGE_INTEGER perfFreq;

	while(!done)									// Loop That Runs While done=FALSE
	{
		oldValue = perfValue;
		QueryPerformanceCounter(&perfValue);
		QueryPerformanceFrequency(&perfFreq);
		fps = perfFreq.QuadPart / (perfValue.QuadPart - oldValue.QuadPart);
		updatetime = TARGET_FPS / fps;
		//std::cout << "FPS: " << fps << endl;

		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=true;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			if(keys[VK_ESCAPE])
				done = true;

			processKeys();

			display();					// Draw The Scene
			update();					// update variables
			SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
		}
	}


	// Shutdown
	KillGLWindow();									// Kill The Window
	return (int)(msg.wParam);						// Exit The Program
}

//WIN32 Processes function - useful for responding to user inputs or other events.
LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}
		break;

		case WM_SIZE:								// Resize The OpenGL Window
		{
			reshape(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
		break;

		case WM_LBUTTONDOWN:
			{
	            mouse_x = LOWORD(lParam);          
				mouse_y = screenHeight - HIWORD(lParam);
				LeftPressed = true;
			}
		break;

		case WM_LBUTTONUP:
			{
	            LeftPressed = false;
			}
		break;

		case WM_MOUSEMOVE:
			{
	            mouse_x = LOWORD(lParam);          
				mouse_y = screenHeight  - HIWORD(lParam);
			}
		break;
		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = true;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}
		break;
		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = false;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}
		break;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void KillGLWindow()								// Properly Kill The Window
{
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*/
 
bool CreateGLWindow(char* title, int width, int height)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;											// Return FALSE
	}
	
	dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
	dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		24,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 24Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	HGLRC tempContext = wglCreateContext(hDC);
	wglMakeCurrent(hDC, tempContext);

	glewInit();

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};
	
    if(wglewIsSupported("WGL_ARB_create_context") == 1)
    {
		hRC = wglCreateContextAttribsARB(hDC,0, attribs);
		wglMakeCurrent(NULL,NULL);
		wglDeleteContext(tempContext);
		wglMakeCurrent(hDC, hRC);
	}
	else
	{	//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
		hRC = tempContext;
		cout << " not possible to make context "<< endl;
	}

	//Checking GL version
	const GLubyte *GLVersionString = glGetString(GL_VERSION);

	cout << GLVersionString << endl;

	//OpenGL 3.2 way of checking the version
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

	cout << OpenGLVersion[0] << " " << OpenGLVersion[1] << endl;

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	reshape(width, height);					// Set Up Our Perspective GL Screen

	init();
	
	return true;									// Success
}



