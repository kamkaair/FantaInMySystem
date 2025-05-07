#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include "material.h"
#include "utils.h"
#include <unordered_map>
#include "texture.h"
#include <iostream>

class TextureLoading : public kgfw::Object {
public:
	TextureLoading();
	~TextureLoading();

	std::unordered_map<int, Material*> getMaterialMap() {
		return materialsMap;
	}

	Texture* loadTexture(const std::string& path);
	Material* checkAndAddMaterial(const std::pair<std::vector<GLuint>, std::vector<Texture*>>& textureData, const std::string& materialName);
	std::unordered_map<int, Material*> loadMaterials(int presetMode);
	std::pair<std::vector<GLuint>, std::vector<Texture*>> loadTextureSet(const std::string& baseColorPath, const std::string& metallicMapPath, const std::string& roughnessMapPath, const std::string& normalMapPath);
	
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene, const std::vector<Material*>& loadedMaterials);
	void processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene, const std::vector<Material*>& loadedMaterials);
	std::vector<Mesh*> loadMeshes(const std::string& path, const std::vector<Material*>& loadedMaterials, const std::string& meshName);
	void TextureLoading::loadAllMeshes(std::vector<Mesh*>& meshes, int presetMode);
	std::vector<std::string> FileSystem(std::string& path);

	std::vector<Material*>& getMaterials() { return m_materials; }
	std::vector<int> getVertices() { return vertexAmount; }

private:
	std::vector<Material*>		m_materials;
	std::unordered_map<int, Material*> materialsMap;
	std::vector<Texture*>		m_trackedTextures;
	std::vector<int>			vertexAmount;
};
