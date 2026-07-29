#pragma once
#include "savefileStructs.h"
#include "models.h"
#include "HDRI.h"
#include <algorithm>

class Scene {
public:
	void cleanupScene() {
		for (auto model : m_models) {
			delete model;
		}
		m_models.clear();
		m_opaqueMeshes.clear();
		m_transparentMeshes.clear();

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

	void sortTransparentMeshes() {
		for (auto& trans : m_transparentMeshes) // Find the distance of the objects
			trans.first = glm::length(m_camera->getPosition() - trans.second->getPosition());

		std::sort(m_transparentMeshes.begin(), m_transparentMeshes.end(), std::greater<std::pair<float, Mesh*>>());	// Sort the list with distances... from greater to less
	}

	void updateMeshList() {
		m_opaqueMeshes.clear();
		m_transparentMeshes.clear();

		for (Model* model : m_models)
			for (Mesh* mesh : model->getMeshes()) {
				if (mesh->getMaterial()->currentAlphaMode != Material::alphaModes::blend) {
					m_opaqueMeshes.push_back(mesh);
					continue;
				}
				m_transparentMeshes.push_back(std::make_pair(0.0f, mesh));
			}
	}

	~Scene() {
		cleanupScene();
		delete m_defaultMaterial;
	}

	std::vector<Model*>& getModels() { return m_models; }
	std::vector<Mesh*>& getOpaqueMeshes() { return m_opaqueMeshes; }
	std::vector<std::pair<float, Mesh*>>& getTransparentMeshes() { return m_transparentMeshes; }

	std::vector<Material*>& getMaterials() { return m_materials; }
	std::vector<FileLights>& getLights() { return m_lights; }

	Material* getDefaultMaterial() { return m_defaultMaterial; }
	Camera* getCamera() { return m_camera; }
	HDRI* getHDRI() { return m_HDRI; }
	HDRI* createHDRI(Shader* cubemapShader, Shader* backgroundShader, Shader* irradianceShader, Shader* prefilter, Shader* brdf) {
		return m_HDRI = new HDRI(cubemapShader, backgroundShader, irradianceShader, prefilter, brdf);
	}

	void setDefaultMaterial(Material* inDefaultMat) { m_defaultMaterial = inDefaultMat; }
	void setActiveCamera(Camera* cam) { m_camera = cam; }

private:
	std::vector<Model*> m_models;
	std::vector<Mesh*> m_opaqueMeshes;
	std::vector<std::pair<float, Mesh*>> m_transparentMeshes;

	std::vector<FileLights> m_lights;
	std::vector<Material*> m_materials;
	FileCamera* cameraData;
	Camera* m_camera;
	HDRI* m_HDRI;

	Material* m_defaultMaterial;
};

/*
* Maybe I should use smart pointers instead of raw pointers?
* 
std::vector<std::unique_ptr<FileLights>> m_lights;

m_scene->getLights().push_back(std::make_unique<FileLights>(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f));
*/