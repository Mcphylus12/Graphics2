#pragma once
#include "3dStruct\threeDModel.h"
#include "glm\glm.hpp"

class Car
{

public:
	double wheelPoints[30] = { 
		-15.016, 8.286, 50.27, 
		-15.016, 8.286, 7.1,
		-15.016, 8.286, -9.608,
		-15.016, 8.286, -47.511, //steering 9
		-15.016, 8.286, -30.804, //steering 12

		10.059, 8.286, 50.27,
		10.059, 8.286, 7.1,
		10.059, 8.286, -9.608,
		10.059, 8.286, -47.511,   //steering 24
		10.059, 8.286, -30.804 }; //steering 27
	ThreeDModel model;
	std::vector<ThreeDModel*> buildings;
	glm::vec4 Material_Ambient, Material_Diffuse, Material_Specular;
	float x, y, z, r, ra;
	float Material_Shininess;


	glm::vec3 getPos() const {
		return glm::vec3(x, y, z);
	}



};

