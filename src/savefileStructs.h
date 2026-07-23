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
	MaterialParam<float> opacity;

	MaterialParam<std::string> normalPath;

	void serialize(std::ofstream& file) {
		Serializer::write(file, materialName);

		Serializer::write(file, diffuse.path.value_or(""));
		Serializer::write(file, diffuse.value);

		Serializer::write(file, metallic.path.value_or(""));
		Serializer::write(file, metallic.value);

		Serializer::write(file, roughness.path.value_or(""));
		Serializer::write(file, roughness.value);

		Serializer::write(file, emission.path.value_or(""));
		Serializer::write(file, emission.value);

		Serializer::write(file, opacity.path.value_or(""));
		Serializer::write(file, opacity.value);

		Serializer::write(file, normalPath.path.value_or(""));
		Serializer::write(file, normalPath.value);
	}

	void deserialize(std::ifstream& file) {
		Serializer ser;
		Serializer::read(file, materialName);

		Serializer::read(file, diffuse.path);
		Serializer::read(file, diffuse.value);
		if (diffuse.path.value().empty())
			diffuse.path = std::nullopt;

		Serializer::read(file, metallic.path);
		Serializer::read(file, metallic.value);
		if (metallic.path.value().empty())
			metallic.path = std::nullopt;

		Serializer::read(file, roughness.path);
		Serializer::read(file, roughness.value);
		if (roughness.path.value().empty())
			roughness.path = std::nullopt;

		Serializer::read(file, emission.path);
		Serializer::read(file, emission.value);
		if (emission.path.value().empty())
			emission.path = std::nullopt;

		Serializer::read(file, opacity.path);
		Serializer::read(file, opacity.value);
		if (opacity.path.value().empty())
			opacity.path = std::nullopt;

		Serializer::read(file, normalPath.path);
		Serializer::read(file, normalPath.value);
		if (normalPath.path.value().empty())
			normalPath.path = "/textures/EmptyNormal.png";
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

	void serialize(std::ofstream& file) {
		Serializer::write(file, modelPath);

		size_t sizeMesh = meshes.size();
		Serializer::write(file, sizeMesh); // file.write(reinterpret_cast<const char*>(&sizeMesh), sizeof(sizeMesh));
		for (size_t j = 0; j < meshes.size(); j++) {
			Serializer::write(file, meshes[j].modelName);

			Serializer::write(file, meshes[j].pos);
			Serializer::write(file, meshes[j].scaling);
			Serializer::write(file, meshes[j].rotation);

			Serializer::write(file, meshes[j].textureID);
		}
	}

	void deserialize(std::ifstream& file) {
		Serializer::read(file, modelPath);

		size_t sizeMesh;
		Serializer::read(file, sizeMesh);//file.read(reinterpret_cast<char*>(&sizeMesh), sizeof(sizeMesh));
		meshes.resize(sizeMesh);
		for (size_t j = 0; j < meshes.size(); j++) {
			Serializer::read(file, meshes[j].modelName);

			Serializer::read(file, meshes[j].pos);
			Serializer::read(file, meshes[j].scaling);
			Serializer::read(file, meshes[j].rotation);

			Serializer::read(file, meshes[j].textureID);
		}
	}
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

	void serialize(std::ofstream& file) {
		// Vec3s
		Serializer::write(file, cameraPos);
		Serializer::write(file, cameraFront);
		Serializer::write(file, cameraFocus);

		// Floats
		Serializer::write(file, radius);
		Serializer::write(file, theta);
		Serializer::write(file, phi);

		Serializer::write(file, pitch);
		Serializer::write(file, yaw);

		Serializer::write(file, lastX);
		Serializer::write(file, lastY);

		// Doubles
		Serializer::write(file, xPos);
		Serializer::write(file, yPos);

		// Bool
		Serializer::write(file, freeMovementEnabled);
	}

	void deserialize(std::ifstream& file) {
		// Vec3s
		Serializer::read(file, cameraPos);
		Serializer::read(file, cameraFront);
		Serializer::read(file, cameraFocus);

		// Floats
		Serializer::read(file, radius);
		Serializer::read(file, theta);
		Serializer::read(file, phi);

		Serializer::read(file, pitch);
		Serializer::read(file, yaw);

		Serializer::read(file, lastX);
		Serializer::read(file, lastY);

		// Doubles
		Serializer::read(file, xPos);
		Serializer::read(file, yPos);

		// Bool
		Serializer::read(file, freeMovementEnabled);
	}
};