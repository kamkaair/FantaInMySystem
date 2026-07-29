#pragma once
#include "textureLoading.h"
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

	MaterialPaths createMaterialPaths(const std::string& matName, const std::string usePaths[], SettingsMaterial& SetMat);
	void setMaterialParams(SettingsMaterial& SetMat, Material*& material);
	void replaceMaterials(Material* oldMat, Material* newMat);

	void clearUnusedTextures();

	Scene* getScene() { return m_scene; }
private:
	Scene* m_scene;
};