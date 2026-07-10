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

	loadCameraOrientation(m_scene->getCamera(), restored.getFileCamera());

	m_scene->constructScene(models, materials, restored.getLightData());
}

void ResourceManager::fileSave(std::string saveName, HDRI* hdri) {
	// And all the meshes and materials
	std::unordered_map<Material*, int> checkedMap;
	std::vector<FileModels> fileModels;
	std::vector<MaterialPaths> materialPath;
	FileCamera fileCamera;

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
	
	saveCameraOrientation(m_scene->getCamera(), fileCamera);

	// Create and serialize an object
	SaveFile original(m_scene->getLights(), materialPath, fileModels, fileCamera, hdri->getBackgroundTexture()->getFilePath(), hdri->getHDRI_Path());
	original.serialize(std::string(ASSET_DIR) + "/Saves/" + saveName + ".bin");
}

void ResourceManager::replaceMaterials(Material* oldMat, Material* newMat) {
	for (auto& model : m_scene->getModels())
		for (auto& mesh : model->getMeshes()) {
			if (mesh->getMaterial() == oldMat)
				mesh->setMaterial(newMat);
		}
}

void ResourceManager::clearUnusedTextures() { // Probably unnecessarily mega expensive, but I'll take a look at it later
	// Get the old map
	std::unordered_set<GLuint> usedTextures;
	for (const auto& mat : m_scene->getMaterials()) {
		for(const auto& tex : mat->getTextures())
			usedTextures.insert( tex );
	}

	// Default material is an exception, it shouldn't be deleted
	GLuint defaultMaterialTextures[2] = { m_scene->getDefaultMaterial()->getTextures()[0], m_scene->getDefaultMaterial()->getTextures()[3] };

	// Find and delete redundant textures, which have not been found
	std::vector<GLuint> tbd;
	for (auto it = getTrackedTextures().begin(); it != getTrackedTextures().end(); it++) { // The iterator can be ++'d
		bool notDefaultTexture = (defaultMaterialTextures[0] != it->first && defaultMaterialTextures[1] != it->first);

		//std::cout << "Checked: " << it->first;
		if (usedTextures.count(it->first) == 0 && notDefaultTexture) {
			std::cout << "Texture ID: " << it->first << " DELETED" << std::endl;
			tbd.push_back(it->first);
			delete it->second;
		}
	}

	// Erase from the list
	for (const auto& deleteTex : tbd)
		getTrackedTextures().erase(deleteTex);
}

void ResourceManager::saveCameraOrientation(Camera* camera, FileCamera& fileCamera) { // At this point might be more suitable to create a .cpp for this class
	fileCamera.cameraFocus = camera->getCameraFocus();
	fileCamera.cameraFront = camera->getCameraFront();
	fileCamera.cameraPos = camera->getCameraPos();

	fileCamera.radius = camera->getRadius();
	fileCamera.radius = camera->getRadius();
	fileCamera.theta = camera->getTheta();
	fileCamera.phi = camera->getPhi();

	fileCamera.pitch = camera->getPitch();
	fileCamera.yaw = camera->getYaw();

	fileCamera.lastX = camera->getLastX();
	fileCamera.lastY = camera->getLastY();

	fileCamera.xPos = camera->getPosX();
	fileCamera.yPos = camera->getPosY();

	fileCamera.freeMovementEnabled = camera->getIsMovementFree();
}

void ResourceManager::loadCameraOrientation(Camera* camera, FileCamera& fileCamera) {
	camera->getCameraPos() = fileCamera.cameraPos;
	camera->getCameraFront() = fileCamera.cameraFront;
	camera->getCameraFocus() = fileCamera.cameraFocus;

	camera->getRadius() = fileCamera.radius;
	camera->getTheta() = fileCamera.theta;
	camera->getPhi() = fileCamera.phi;

	camera->getPitch() = fileCamera.pitch;
	camera->getYaw() = fileCamera.yaw;

	camera->getLastX() = fileCamera.lastX;
	camera->getLastY() = fileCamera.lastY;

	camera->getPosX() = fileCamera.xPos;
	camera->getPosY() = fileCamera.yPos;

	camera->getIsMovementFree() = fileCamera.freeMovementEnabled;
}

void ResourceManager::cleanResourceManager(HDRI* hdri) {
	// Clean up the whole scene
	m_scene->cleanupScene();
	cleanupTextures();
	hdri->cleanUpHDRI();
	hdri->cleanBackgroundTexture();
}