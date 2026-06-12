#pragma once
#include <string>
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad

struct FileLights {
	glm::vec3 pos;
	glm::vec3 color;
	float strength;
};

template <typename T> 
struct MaterialParam {
	std::string path;
	bool useMap;
	T value;
};

struct MaterialPaths {
	std::string materialName;

	MaterialParam<glm::vec3> diffuse = { "", false, glm::vec3(0) };
	MaterialParam<float> metallic = { "", false, 0.0f };
	MaterialParam<float> roughness = { "", false, 0.0f };

	std::string normalPath;
};

struct FileMeshes {
	std::string modelName;

	glm::vec3 pos = glm::vec3(0);
	glm::vec3 scaling = glm::vec3(1);
	glm::vec3 rotation = glm::vec3(0);

	int textureID = 0;
};

struct FileModels {
	std::string modelPath;
	std::vector<FileMeshes> meshes;
};