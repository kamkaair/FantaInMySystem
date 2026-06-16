#include "resourceManager.h"
#include "savefile.h"

ResourceManager::ResourceManager() {
	// Create a new scene
	m_scene = new Scene();
	setCurrentScene(m_scene);
}

void ResourceManager::fileLoad(std::string file, HDRI* hdri) {
	// Deserialize the object
	SaveFile restored = SaveFile::deserialize(std::string(ASSET_DIR) + "/Saves/" + file);

	std::vector<Material*> materials = MaterialsPushback(restored.getPathNames());
	m_scene->getMaterials() = materials;

	std::vector<Model*> models;
	loadMeshes(models, restored.getFileMeshes()); // Preset modes from 0 - 3
	m_scene->getModels() = models;

	std::cout << "Background tex load: " << restored.getBackgroundTexPath() << std::endl;

	Texture* backgroundImage = loadTexture(restored.getBackgroundTexPath());
	hdri->setBackgroundTexture(backgroundImage);

	// Load the HDR texture and create all the HDRI maps
	hdri->ProcessHDRI(restored.getHdriPath().c_str());

	// Set up lights and color
	m_scene->getLights() = restored.getLightData();

	m_scene->constructScene(models, materials, restored.getLightData());

	// Just in case, if no materials were added
	if (m_scene->getMaterials().empty()) {
		checkAndAddMaterial(loadTextureSet(
			std::string("/textures/checkerboard.png"),
			std::string("/textures/checkerboard.png"),
			std::string("/textures/checkerboard.png"),
			std::string("/textures/checkerboardNormal.png")
		), "Default Material");
		std::cout << "Material empty, creating Default Material" << std::endl;
	}
}

void ResourceManager::fileSave(std::string saveName, HDRI* hdri) {
	// And all the meshes and materials
	std::unordered_map<Material*, int> checkedMap;
	std::vector<FileModels> fileModels;
	std::vector<MaterialPaths> materialPath;

	int texIndex = 0;
	for (auto model : m_scene->getModels()) {
		std::vector<FileMeshes> fileMeshes;
		for (auto mesh : model->getMeshes()) {

			auto it = checkedMap.find(mesh->getMaterial());
			if (it == checkedMap.end()) { // Check, whether the material already exists
				std::vector<Texture*> foundTexs;
				checkedMap.insert({ mesh->getMaterial(), texIndex });
				mesh->getMaterial()->getMaterialIndex() = texIndex;

				for (auto maps : mesh->getMaterial()->getTextures()) {
					foundTexs.push_back(findTexture(maps));
				}

				materialPath.push_back(MaterialPaths{ mesh->getDisplayName(),
				foundTexs[0]->getFilePath(), mesh->getMaterial()->useDiffuseTexture, mesh->getMaterial()->diffuseColor,
				foundTexs[1]->getFilePath(), mesh->getMaterial()->useMetallicTexture, mesh->getMaterial()->metallic,
				foundTexs[2]->getFilePath(), mesh->getMaterial()->useRoughnessTexture, mesh->getMaterial()->roughness,
				foundTexs[3]->getFilePath() });
				texIndex++;
			}

			fileMeshes.push_back(FileMeshes{
				mesh->getDisplayName(),
				mesh->getPosition(),
				mesh->getScaling(),
				mesh->getRotation(), mesh->getMaterial()->getMaterialIndex() });

			std::cout << "Material ID: " << mesh->getMaterial()->getMaterialIndex() << std::endl;
		}

		fileModels.push_back(FileModels{ model->getModelPath(), fileMeshes });
	}
	std::cout << "Background tex path: " << hdri->getBackgroundTexture()->getFilePath() << std::endl;

	// Create and serialize an object
	SaveFile original(m_scene->getLights(), materialPath, fileModels, hdri->getBackgroundTexture()->getFilePath(), hdri->getHDRI_Path());
	original.serialize(std::string(ASSET_DIR) + "/Saves/" + saveName + ".bin");
}

void ResourceManager::cleanResourceManager(HDRI* hdri) {
	// Clean up the whole scene
	m_scene->cleanupScene();
	cleanupTextures();
	hdri->cleanUpHDRI();
	hdri->cleanBackgroundTexture();
}