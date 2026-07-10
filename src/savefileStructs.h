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

struct FileCamera {
	glm::vec3 cameraPos = { 0.0f, 0.5f, 1.0f };
	glm::vec3 cameraFront = { 0.0f, 0.0f, -1.0f };
	glm::vec3 cameraFocus = { 0.0f, 0.0f, 0.0f };

	float radius = 10.0f, theta = 0.0f, phi = 3.14159265359f / 4.0f;
	float pitch = 0.0f, yaw = -90.0;
	float lastX = 800.0f / 2.0, lastY = 600.0 / 2.0;
	double xPos = 0.0f, yPos = 0.0f;

	bool freeMovementEnabled = false;
};