#include "resourceManager.h"
#include "savefile.h"
#include <unordered_set>

ResourceManager::ResourceManager() {
	// Create a new scene
	m_scene = new Scene();
	setCurrentScene(m_scene);

	// Set the default material
	m_scene->setDefaultMaterial(createMaterial(MaterialPaths{ std::string("Checkerboard"),
		std::string("/textures/checkerboard.png"), true, glm::vec3(0),	// Diffuse
		std::string("/textures/checkerboard.png"), true, 0.0f,			// Metallic
		std::string("/textures/checkerboard.png"), true, 0.0f,			// Roughness
		std::string("/textures/checkerboardNormal.png") })); // Add the default material
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
				std::vector<std::string> filePaths;
				checkedMap.insert({ mesh->getMaterial(), texIndex });
				mesh->getMaterial()->getMaterialIndex() = texIndex;

				for (auto maps : mesh->getMaterial()->getTextures()) {
					Texture* foundTexture = findTexture(maps);
					if (foundTexture != nullptr)
						filePaths.push_back(foundTexture->getFilePath());
					else
						filePaths.push_back("");
					
				}
				std::cout << filePaths[0] << std::endl;
				std::cout << mesh->getMaterial()->useDiffuseTexture << std::endl;
				std::cout << glm::to_string(mesh->getMaterial()->diffuseColor) << std::endl;

				materialPath.push_back(MaterialPaths{ mesh->getDisplayName(),
				filePaths[0], mesh->getMaterial()->useDiffuseTexture, mesh->getMaterial()->diffuseColor,
				filePaths[1], mesh->getMaterial()->useMetallicTexture, mesh->getMaterial()->metallic,
				filePaths[2], mesh->getMaterial()->useRoughnessTexture, mesh->getMaterial()->roughness,
				filePaths[3] });
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

void ResourceManager::replaceMaterials(Material* oldMat, Material* newMat) {
	for (auto& model : m_scene->getModels())
		for (auto& mesh : model->getMeshes()) {
			if (mesh->getMaterial() == oldMat)
				mesh->setMaterial(newMat);
		}
}

void ResourceManager::checkInvalidTextures() { // Probably unnecessarily mega expensive, but I'll take a look at it later
	// Get the old map
	std::unordered_map<GLuint, bool> checkMap;
	for (auto map : getTrackedTextures()) {
		checkMap.insert({ map.first, false });
	}

	// Find redundant textures
	for (auto mat : m_scene->getMaterials()) {
		for (auto texture : mat->getTextures()) {
			auto it = checkMap.find(texture);
			std::cout << "textureID: " << texture << std::endl;
			if (it != checkMap.end()) {
				it->second = true;
			}
		}
	}
	for (auto map : checkMap) {
		std::cout << "Redundant textureID: " << map.first << " - Condition: " << map.second << std::endl;
	}

	// Delete redundant textures, which have not been found
	/*for (auto map : checkMap) {
		if (map.second != false) {
			glDeleteTextures(1, &map.first);
			delete texture.second;
		}
	}*/

	std::vector<std::pair<const GLuint, Texture*>> tbd;
	for (auto& texture : getTrackedTextures()) {
		auto it = checkMap.find(texture.first);
		if (!it->second) {
			tbd.push_back(texture);
			glDeleteTextures(1, &texture.first);
			delete texture.second;
		}		
	}

	for (auto deleteTex : tbd) 
		getTrackedTextures().erase(deleteTex.first);
}

void ResourceManager::cleanResourceManager(HDRI* hdri) {
	// Clean up the whole scene
	m_scene->cleanupScene();
	cleanupTextures();
	hdri->cleanUpHDRI();
	hdri->cleanBackgroundTexture();
}