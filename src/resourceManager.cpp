#include "resourceManager.h"
#include "savefile.h"
#include <unordered_set>

ResourceManager::ResourceManager() {
	// Create a new scene
	m_scene = new Scene();
	setCurrentScene(m_scene);

	// Set the default material
	m_scene->setDefaultMaterial(createMaterial(MaterialPaths{ std::string("Checkerboard"),
		useValue<glm::vec3>(glm::vec3(1.0f, 0.0f, 1.0f)),				// Diffuse
		useTexture<float>("/textures/checkerboard.png"),				// Metallic
		useTexture<float>("/textures/checkerboard.png"),				// Roughness
		useTexture<float>("/textures/checkerboard.png", 1.0f),			// Emission
		useTexture<std::string>("/textures/checkerboardNormal.png"),	// Add the default material
		useValue<float>(1.0f) }));										// Opacity
}

void ResourceManager::fileLoad(std::string file) {
	// Deserialize the object
	SaveFile restored = SaveFile::deserialize(std::string(ASSET_DIR) + "/Saves/" + file);

	// Create materials and their textures
	std::vector<Material*> materials = MaterialsPushback(restored.getPathNames());
	m_scene->getMaterials() = materials;

	// Create models and assign materials
	std::vector<Model*> models;
	loadMeshes(models, restored.getFileMeshes()); // Preset modes from 0 - 3
	m_scene->getModels() = models;

	// Update the mesh order after loading the models and materials
	m_scene->updateMeshList();

	// Set the camera values
	loadCameraOrientation(m_scene->getCamera(), restored.getFileCamera());

	// stbi_set_flip_vertically_on_load(true);
	// Load the texture for the background texture
	Texture* backgroundImage = loadTexture(restored.getBackgroundTexPath());
	m_scene->getHDRI()->setBackgroundTexture(backgroundImage);

	// Load the HDR texture and create all the HDRI maps
	m_scene->getHDRI()->ProcessHDRI(restored.getHdriPath().c_str());

	// Set up lights and color
	m_scene->getLights() = restored.getLightData();
}

void ResourceManager::fileSave(std::string saveName) {
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

				materialPath.push_back(MaterialPaths{ mesh->getDisplayName(),
				useTexture<glm::vec3>(filePaths[0], mesh->getMaterial()->diffuseColor),
				useTexture<float>(filePaths[1], mesh->getMaterial()->metallic),
				useTexture<float>(filePaths[2], mesh->getMaterial()->roughness),
				useTexture<float>(filePaths[3], mesh->getMaterial()->emission),
				useTexture<std::string>(filePaths[4], "emptyNormal"),
				useTexture<float>(filePaths[5], mesh->getMaterial()->opacity)});
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
	std::cout << "Background tex path: " << m_scene->getHDRI()->getBackgroundTexture()->getFilePath() << std::endl;
	
	saveCameraOrientation(m_scene->getCamera(), fileCamera);

	// Create and serialize an object
	SaveFile original(m_scene->getLights(), materialPath, fileModels, fileCamera, m_scene->getHDRI()->getBackgroundTexture()->getFilePath(), m_scene->getHDRI()->getHDRI_Path());
	original.serialize(std::string(ASSET_DIR) + "/Saves/" + saveName + ".bin");
}

MaterialPaths ResourceManager::createMaterialPaths(const std::string& matName, const std::string usePaths[], SettingsMaterial& SetMat) {
	return MaterialPaths{ matName,
		useTexture<glm::vec3>(usePaths[0], SetMat.diffuseColor),
		useTexture<float>(usePaths[1], SetMat.metallic),
		useTexture<float>(usePaths[2], SetMat.roughness),
		useTexture<float>(usePaths[3], SetMat.emission),
		useTexture<std::string>(usePaths[5], "emptyNormal"),
		useTexture<float>(usePaths[4], SetMat.opacity) };
}

void ResourceManager::setMaterialParams(SettingsMaterial& SetMat, Material*& material) {
	// Set material properties and bools
	SetMat.diffuseColor = material->diffuseColor;
	SetMat.metallic = material->metallic;
	SetMat.roughness = material->roughness;
	SetMat.emission = material->emission;
	SetMat.opacity = material->opacity;

	SetMat.useDiffuseTexture = material->useDiffuseTexture;
	SetMat.useMetallicTexture = material->useMetallicTexture;
	SetMat.useRoughnessTexture = material->useRoughnessTexture;
	SetMat.useEmissionTexture = material->useEmissionTexture;
	SetMat.useOpacityTexture = material->useOpacityTexture;
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
	fileCamera.cameraFocus = camera->cameraFocus;
	fileCamera.cameraFront = camera->cameraFront;
	fileCamera.cameraPos = camera->cameraPos;

	fileCamera.radius = camera->radius;
	fileCamera.theta = camera->theta;
	fileCamera.phi = camera->phi;

	fileCamera.pitch = camera->pitch;
	fileCamera.yaw = camera->yaw;

	fileCamera.lastX = camera->lastX;
	fileCamera.lastY = camera->lastY;

	fileCamera.xPos = camera->xPos;
	fileCamera.yPos = camera->yPos;

	fileCamera.freeMovementEnabled = camera->getIsMovementFree();
}

void ResourceManager::loadCameraOrientation(Camera* camera, FileCamera& fileCamera) {
	camera->cameraPos = fileCamera.cameraPos;
	camera->cameraFront = fileCamera.cameraFront;
	camera->cameraFocus = fileCamera.cameraFocus;

	camera->radius = fileCamera.radius;
	camera->theta = fileCamera.theta;
	camera->phi = fileCamera.phi;

	camera->pitch = fileCamera.pitch;
	camera->yaw = fileCamera.yaw;

	camera->lastX = fileCamera.lastX;
	camera->lastY = fileCamera.lastY;

	camera->xPos = fileCamera.xPos;
	camera->yPos = fileCamera.yPos;

	camera->getIsMovementFree() = fileCamera.freeMovementEnabled;
}

void ResourceManager::cleanResourceManager() {
	// Clean up the whole scene
	m_scene->cleanupScene();
	cleanupTextures();
	m_scene->getHDRI()->cleanUpHDRI();
	m_scene->getHDRI()->cleanBackgroundTexture();
}