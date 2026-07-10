#pragma once
#include "textureLoading.h"
#include "HDRI.h"

class ResourceManager : public TextureLoading {
public:
	ResourceManager();

	void fileLoad(std::string file, HDRI* hdri);
	void fileSave(std::string saveName, HDRI* hdri);
	void cleanResourceManager(HDRI* hdri);

	void saveCameraOrientation(Camera* camera, FileCamera& fileCamera);
	void loadCameraOrientation(Camera* camera, FileCamera& fileCamera);

	void replaceMaterials(Material* oldMat, Material* newMat);
	void clearUnusedTextures();

	Scene* getScene() { return m_scene; }
private:
	Scene* m_scene;
};