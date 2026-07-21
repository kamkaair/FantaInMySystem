#pragma once
#include <string>
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include "serializer.h"

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
	MaterialParam<float> emission;
	//MaterialParam<float> opacity;

	MaterialParam<std::string> normalPath;

	void serialize(std::ofstream& file, const MaterialPaths& inVec) {
		Serializer ser;
		ser.write(file, inVec.materialName);

		ser.write(file, inVec.diffuse.path.value_or(""));
		ser.write(file, inVec.diffuse.value);

		ser.write(file, inVec.metallic.path.value_or(""));
		ser.write(file, inVec.metallic.value);

		ser.write(file, inVec.roughness.path.value_or(""));
		ser.write(file, inVec.roughness.value);

		ser.write(file, inVec.emission.path.value_or(""));
		ser.write(file, inVec.emission.value);

		ser.write(file, inVec.normalPath.path.value_or(""));
		ser.write(file, inVec.normalPath.value);
	}

	void deserialize(std::ifstream& file, MaterialPaths& inVec) {
		Serializer ser;
		ser.read(file, inVec.materialName);

		ser.read(file, inVec.diffuse.path);
		ser.read(file, inVec.diffuse.value);
		if (inVec.diffuse.path.value().empty())
			inVec.diffuse.path = std::nullopt;

		ser.read(file, inVec.metallic.path);
		ser.read(file, inVec.metallic.value);
		if (inVec.metallic.path.value().empty())
			inVec.metallic.path = std::nullopt;

		ser.read(file, inVec.roughness.path);
		ser.read(file, inVec.roughness.value);
		if (inVec.roughness.path.value().empty())
			inVec.roughness.path = std::nullopt;

		ser.read(file, inVec.emission.path);
		ser.read(file, inVec.emission.value);
		if (inVec.emission.path.value().empty())
			inVec.emission.path = std::nullopt;

		ser.read(file, inVec.normalPath.path);
		ser.read(file, inVec.normalPath.value);
		if (inVec.normalPath.path.value().empty())
			inVec.normalPath.path = "/textures/EmptyNormal.png";
	}
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