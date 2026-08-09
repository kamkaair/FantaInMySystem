#pragma once
#include "textureLoading.h"
#include <functional>
#include "HDRI.h"

struct SettingsMaterial {
	glm::vec3 diffuseColor = glm::vec3(1.0f);  // Default white color
	float roughness = 0.5f;                    // Default roughness
	float metallic = 0.0f;                     // Default metallic value
	float emission = 0.0f;
	float opacity = 1.0f;

	bool useDiffuseTexture = true;				// Whether to use a texture or a value
	bool useMetallicTexture = true;
	bool useRoughnessTexture = true;
	bool useEmissionTexture = false;
	bool useNormalTexture = false;
	bool useOpacityTexture = false;
};

class ResourceManager : public TextureLoading {
public:
	ResourceManager();

	void fileLoad(std::string file);
	void fileSave(std::string saveName);
	void cleanResourceManager();

	void saveCameraOrientation(Camera* camera, FileCamera& fileCamera);
	void loadCameraOrientation(Camera* camera, FileCamera& fileCamera);

	void findMaterialPaths(std::string usePaths[], SettingsMaterial& SetMat, std::vector<std::string>& m_materialFileNames, static int currentItem[]);
	void findComboBoxMaterials(Material* material, const std::int8_t& loopSize, static int selectionArr[]);

	MaterialPaths createMaterialPaths(const std::string& matName, const std::string usePaths[], SettingsMaterial& SetMat);
	MaterialPaths createMaterialPaths(const std::string usePaths[], Mesh* mesh);
	void setUIMaterialParams(SettingsMaterial& SetMat, Material*& material);
	void replaceMaterials(Material* oldMat, Material* newMat);

	void removeMaterialBySelection(static int select);
	void removeModelBySelection(static int select);
	void applyEditedMaterial(Material* material, static int selectionArr[]);
	void transformOperation(glm::vec3 inValue, void (*operationFunc)(const glm::vec3&, GameObject*));

	void clearUnusedTextures();

	void updateAllFiles() { // TODO: probably for resource manager
		updateFiles(meshFileNames, "models/", [this](const std::string& path) { return FileSystem(path); });
		updateFiles(m_materialFileNames, "textures/", [this](const std::string& path) { return FileSystem(path); });
		updateFiles(m_saveFiles, "Saves/", [this](const std::string& path) { return FileSystem(path); });
		updateFiles(hdrFileNames, "HDRI/", [this](const std::string& path) { return FileSystem(path); });
		updateFiles(m_folderNames, "textures/", [this](const std::string& path) { return FileSystemFolders(path); });
	}

	void updateFiles(std::vector<std::string>& fileNames, std::string location, std::function< std::vector<std::string>(const std::string path) > func) {
		if (!fileNames.empty())
			fileNames.clear();

		//fileNames = m_resoManager->FileSystem((std::string(ASSET_DIR) + "/" + location));
		fileNames = func(std::string(ASSET_DIR) + "/" + location);
	}

	Scene* getScene() { return m_scene; }
	SettingsMaterial& getSettingsCreate() { return m_settingsCreateMat; }
	SettingsMaterial& getSettingsEdit() { return m_settingsEditMat; }
	std::vector<std::string> m_saveFiles, meshFileNames, hdrFileNames, m_materialFileNames, m_folderNames;
private:
	Scene* m_scene;
	SettingsMaterial m_settingsCreateMat, m_settingsEditMat;
};