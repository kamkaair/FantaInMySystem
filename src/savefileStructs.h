#pragma once
#include <string>
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad

#include <optional>

struct FileLights {
	glm::vec3 pos;
	glm::vec3 color;
	float strength;
};

template <typename T>
struct MaterialParam {
	T value{}; // Self initializing
	std::optional<std::string> path; // std::optional seems neat, just use std::nullopt when the value is not needed!
};

// MaterialPaths helpers functions/templates
template<typename T>
MaterialParam<T> useValue(T value) { // Only value, no path included
	return MaterialParam<T>({ value, std::nullopt });
}
template<typename T>
MaterialParam<T> useTexture(std::string path = "", T defaultValue = {}) { // Includes a default value as a backup, but primarily uses a texture
	return MaterialParam<T>({ defaultValue, path });
}

template<typename T>
MaterialParam<T> materialParam(const std::string& var) {
	return useTexture<T>(var);
}

template<typename T>
MaterialParam<T> materialParam(const T& var) {
	return useValue<T>(var);
}

struct MaterialPaths {
	std::string materialName;

	MaterialParam<glm::vec3> diffuse;
	MaterialParam<float> metallic;
	MaterialParam<float> roughness;
	//MaterialParam<float> opacity;
	//MaterialParam<glm::vec3> emission;

	MaterialParam<std::string> normalPath;
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