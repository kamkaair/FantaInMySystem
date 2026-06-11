#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include "material.h"
#include <unordered_map>
#include "texture.h"
#include "models.h"
#include "scene.h"
#include "savefileStructs.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

class TextureLoading : public kgfw::Object {
public:
	TextureLoading();
	~TextureLoading();

	void cleanupTextures();

	std::unordered_map<int, Material*> getMaterialMap() {
		return materialsMap;
	}

	Texture* loadTexture(const std::string& path, bool flipTexture = false);
	Material* checkAndAddMaterial(const std::pair<std::vector<GLuint>, std::vector<Texture*>>& textureData, const std::string& materialName);
	std::unordered_map<int, Material*> loadMaterials(int presetMode);
	std::vector<Material*> MaterialsPushback(const std::vector<MaterialPaths>& materialList);
	std::pair<std::vector<GLuint>, std::vector<Texture*>> loadTextureSet(const std::string& baseColorPath, const std::string& metallicMapPath, const std::string& roughnessMapPath, const std::string& normalMapPath);
	
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene, const std::string path);
	void processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene, const std::string path);
	std::vector<Mesh*> loadMeshes(const std::string& path, const std::string& meshName);
	void TextureLoading::loadAllMeshes(std::vector<Mesh*>& meshes, int presetMode);
	void loadMeshes(std::vector<Model*>& model, std::vector<FileMeshes> fileMeshes);

	std::vector<std::string> FileSystem(std::string& path);
	Texture* findTexture(GLuint textureID);

	std::vector<int> getVertices() { return vertexAmount; }
	std::vector<Texture*> getTrackedTextures() { return m_textures; }

	void setCurrentScene(Scene* scene) { m_scene = scene; }

private:
	int m_materialIndex = 0;
	std::unordered_map<int, Material*> materialsMap;
	std::vector<Texture*>		m_textures;
	std::vector<int>			vertexAmount;

	Scene* m_scene;
};
