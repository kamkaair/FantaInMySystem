#pragma once
#include "textureLoading.h"
#include "HDRI.h"

class ResourceManager : public TextureLoading {
public:
	void fileLoad(std::string file, Scene* scene, HDRI* hdri);
	void fileSave(std::string saveName, Scene* scene, HDRI* hdri);
	void cleanResourceManager(Scene* scene, HDRI* hdri);

private:
};