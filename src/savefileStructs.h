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
	std::string materialName;

	std::string colorPath;
	bool useDiffuseTexture = true;
	glm::vec3 diffuseColor = glm::vec3(0.0f);

	std::string metallicPath;
	bool useMetallicTexture = true;
	float metallic = 0.0f;

	std::string roughnessPath;
	bool useRoughnessTexture = true;
	float roughness = 0.0f;

	std::string normalPath;
};

struct FileMeshes {
	std::string modelPath;
	std::string modelName;

	glm::vec3 pos = glm::vec3(0);
	glm::vec3 scaling = glm::vec3(1);
	glm::vec3 rotation = glm::vec3(0);

	int textureID = 0;
};