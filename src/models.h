#pragma once
#include <string>
#include <vector>
#include "mesh.h"

class Model : public GameObject {
public:
	Model(std::string path, std::vector<Mesh*> meshes) : GameObject("Model"), m_modelPath(path), m_meshes(meshes) {}
	~Model() {
		for (auto meshes : m_meshes) {
			delete meshes;
		}
	}

	void setModelPath(const std::string inPath) { m_modelPath = inPath; }
	std::string& getModelPath() { return m_modelPath; }
	std::vector<Mesh*>& getMeshes() { return m_meshes; }

private:
	std::string m_modelPath;
	std::vector<Mesh*> m_meshes;
};