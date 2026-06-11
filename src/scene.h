#pragma once
#include "mesh.h"
#include "material.h"
#include "savefileStructs.h"
#include "models.h"

class Scene {
public:
	void cleanupScene() {
		for (auto model : m_models) {
			delete model;
		}
		m_models.clear();

		for (auto mat : m_materials) {
			delete mat;
		}
		m_materials.clear();
		m_lights.clear();
	}

	void constructScene(std::vector<Model*>& meshes, std::vector<Material*>& material, std::vector<FileLights>& lights) {
		m_models = meshes;
		m_materials = material;
		m_lights = lights;
	}

	~Scene() {
		cleanupScene();
	}

	std::vector<Model*>& getModels() { return m_models; }
	std::vector<Material*>& getMaterials() { return m_materials; }
	std::vector<FileLights>& getLights() {return m_lights;}

private:
	std::vector<Model*> m_models;
	std::vector<FileLights> m_lights;
	std::vector<Material*> m_materials;
};

/*
* Maybe I should use smart pointers instead of raw pointers?
* 
std::vector<std::unique_ptr<FileLights>> m_lights;

m_scene->getLights().push_back(std::make_unique<FileLights>(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f));
*/