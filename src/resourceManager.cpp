#include "resourceManager.h"
#include "savefile.h"

void ResourceManager::fileLoad(std::string file, Scene* scene, HDRI* hdri) {
	// Deserialize the object
	SaveFile restored = SaveFile::deserialize(std::string(ASSET_DIR) + "/Saves/" + file);

	std::vector<Material*> materials = MaterialsPushback(restored.getPathNames());
	scene->getMaterials() = materials;

	std::vector<Model*> models;
	loadMeshes(models, restored.getFileMeshes()); // Preset modes from 0 - 3
	scene->getModels() = models;

	std::cout << "Background tex load: " << restored.getBackgroundTexPath() << std::endl;

	Texture* backgroundImage = loadTexture(restored.getBackgroundTexPath());
	hdri->setBackgroundTexture(backgroundImage);

	// Load the HDR texture and create all the HDRI maps
	hdri->ProcessHDRI(restored.getHdriPath().c_str());

	// Set up lights and color
	scene->getLights() = restored.getLightData();

	scene->constructScene(models, materials, restored.getLightData());

	// Just in case, if no materials were added
	if (scene->getMaterials().empty()) {
		checkAndAddMaterial(loadTextureSet(
			std::string("/textures/checkerboard.png"),
			std::string("/textures/checkerboard.png"),
			std::string("/textures/checkerboard.png"),
			std::string("/textures/checkerboardNormal.png")
		), "Default Material");
		std::cout << "Material empty, creating Default Material" << std::endl;
	}
}

void ResourceManager::fileSave(std::string saveName, Scene* scene, HDRI* hdri) {
	// And all the meshes and materials
	std::unordered_map<Material*, int> checkedMap;
	std::vector<FileModels> fileModels;
	std::vector<MaterialPaths> materialPath;

	int texIndex = 0;
	for (auto model : scene->getModels()) {
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
	SaveFile original(scene->getLights(), materialPath, fileModels, hdri->getBackgroundTexture()->getFilePath(), hdri->getHDRI_Path());
	original.serialize(std::string(ASSET_DIR) + "/Saves/" + saveName + ".bin");
}

void ResourceManager::cleanResourceManager(Scene* scene, HDRI* hdri) {
	// Clean up the whole scene
	scene->cleanupScene();
	cleanupTextures();
	hdri->cleanUpHDRI();
	hdri->cleanBackgroundTexture();
}