#pragma once
#include <string>
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad

struct FileLights {
	glm::vec3 pos;
	glm::vec3 color;
	float strength;
};

struct MaterialPaths {
	std::string colorPath;
	std::string metallicPath;
	std::string roughnessPath;
	std::string normalPath;
};