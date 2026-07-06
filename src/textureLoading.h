#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <unordered_map>

#include "models.h"
#include "texture.h"
#include "scene.h"

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
	Texture* loadTexture(const std::string& path, bool flipTexture = false);
	std::pair<std::vector<GLuint>, std::vector<Texture*>> loadTextureSet(const std::string& baseColorPath, const std::string& metallicMapPath, const std::string& roughnessMapPath, const std::string& normalMapPath);

	Material* findTexturesWithPath(const std::string path, const aiScene* scene, aiMesh* mesh);
	Texture* findTexture(GLuint textureID);

	Material* createMaterial(const MaterialPaths& materialPaths);
	Material* checkAndAddMaterial(const std::pair<std::vector<GLuint>, std::vector<Texture*>>& textureData, const std::string& materialName);
	std::vector<Material*> MaterialsPushback(const std::vector<MaterialPaths>& materialList);
	void editMaterial(Material* editableMat, const MaterialPaths& materialPaths);

	Mesh* processMesh(aiMesh* mesh, const aiScene* scene, const std::string path);
	Mesh* processMeshAutoTexture(aiMesh* mesh, const aiScene* scene, const std::string path);
	std::vector<Mesh*> processMeshes(const std::string& path, bool autoTexture = false);
	void loadMeshes(std::vector<Model*>& model, std::vector<FileModels> fileModels);

	std::vector<std::string> FileSystem(const std::string path);
	std::vector<std::pair<std::string, std::string>> FileSystemTuple(const std::string path);

	std::unordered_map<GLuint, Texture*>& getTrackedTextures() { return m_textureMap; }
	void setCurrentScene(Scene* scene) { m_scene = scene; }

private:
	void checkDuplicateTextures(std::vector<GLuint>& textureIDs, std::vector<std::pair<std::string, bool>> maps);
	void processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene, const std::string path);
	void processNodeAutoTexture(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene, const std::string path);

	unsigned int m_materialIndex = 0;
	//std::vector<Texture*> m_textures;
	std::unordered_map<GLuint, Texture*> m_textureMap;
	std::string emptyNormalPath = "/textures/EmptyNormal.png";

	Scene* m_scene;
};
