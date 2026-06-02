#pragma once
#include "mesh.h"
#include "material.h"
#include "savefileStructs.h"

class Scene {
public:
	void cleanupScene() {
		for (auto mesh : m_meshes) {
			delete mesh;
		}
		m_meshes.clear();

		for (auto mat : m_materials) {
			delete mat;
		}
		m_materials.clear();
		m_lights.clear();
	}

	void constructScene(std::vector<Mesh*>& meshes, std::vector<Material*>& material, std::vector<FileLights>& lights) {
		m_meshes = meshes;
		m_materials = material;
		m_lights = lights;
	}

	~Scene() {
		cleanupScene();
	}

	std::vector<Mesh*>& getMeshes() { return m_meshes; }
	std::vector<Material*>& getMaterials() { return m_materials; }
	std::vector<FileLights>& getLights() {return m_lights;}

	

private:
	std::vector<Mesh*> m_meshes;
	std::vector<FileLights> m_lights;
	std::vector<Material*> m_materials;
};

/*
* Maybe I should use smart pointers instead of raw pointers?
* 
std::vector<std::unique_ptr<FileLights>> m_lights;

m_scene->getLights().push_back(std::make_unique<FileLights>(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f));
*/