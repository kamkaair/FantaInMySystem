#include "resourceManager.h"
#include "savefile.h"
#include <unordered_set>

ResourceManager::ResourceManager() {
	// Create a new scene
	m_scene = new Scene();
	setCurrentScene(m_scene);

	// Set the default material
	m_scene->setDefaultMaterial(createMaterial(MaterialPaths{ std::string("Checkerboard"), // Add the default material
		useValue<glm::vec3>(glm::vec3(1.0f, 0.0f, 1.0f)),					// Diffuse
		useTexture<float>("/textures/checkerboard.png"),					// Metallic
		useTexture<float>("/textures/checkerboard.png"),					// Roughness
		useTexture<float>("/textures/checkerboard.png", 1.0f),				// Emission
		useValue<float>(1.0f),												// Opacity
		useTexture<std::string>("/textures/checkerboardNormal.png") }));	// Normal
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

				std::string filePaths[6];
				checkedMap.insert({ mesh->getMaterial(), texIndex });
				mesh->getMaterial()->getMaterialIndex() = texIndex;

				for (int i = 0; i < mesh->getMaterial()->getTextures().size(); i++) {
					Texture* foundTexture = findTexture(mesh->getMaterial()->getTextures()[i]);
					if (foundTexture != nullptr)
						filePaths[i] = foundTexture->getFilePath();
					else
						filePaths[i] = "";

				}

				
				materialPath.push_back(createMaterialPaths(filePaths, mesh));
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
		useTexture<float>(usePaths[4], SetMat.opacity),
		useTexture<std::string>(usePaths[5], "emptyNormal") };
}

MaterialPaths ResourceManager::createMaterialPaths(const std::string usePaths[], Mesh* mesh) {
	return MaterialPaths{ mesh->getDisplayName(),
		useTexture<glm::vec3>(usePaths[0], mesh->getMaterial()->diffuseColor),
		useTexture<float>(usePaths[1], mesh->getMaterial()->metallic),
		useTexture<float>(usePaths[2], mesh->getMaterial()->roughness),
		useTexture<float>(usePaths[3], mesh->getMaterial()->emission),
		useTexture<float>(usePaths[4], mesh->getMaterial()->opacity),
		useTexture<std::string>(usePaths[5], "emptyNormal") };
}

void ResourceManager::findMaterialPaths(std::string usePaths[], SettingsMaterial& SetMat, std::vector<std::string>& m_materialFileNames, static int currentItem[]) {
	// In case, where the texture is not used ("" is handled as no texture)
	//std::string usePaths[6];
	bool useTextures[5] = { SetMat.useDiffuseTexture, SetMat.useMetallicTexture, SetMat.useRoughnessTexture, SetMat.useEmissionTexture, SetMat.useOpacityTexture };
	for (std::uint8_t i = 0; i < 5; i++) {
		usePaths[i] = useTextures[i] ? "/textures/" + m_materialFileNames[currentItem[i]] : "";
	}

	usePaths[5] = "/textures/EmptyNormal.png"; // Set the default normal map name
	if (SetMat.useNormalTexture)
		usePaths[5] = "/textures/" + m_materialFileNames[currentItem[5]];
}

void ResourceManager::findComboBoxMaterials(Material* material, const std::int8_t& loopSize, static int selectionArr[]) {
	//select = i; // Set the selected index
	for (size_t j = 0; j < loopSize; j++) { // comboBoxSelection size
		Texture* foundTex = findTexture(material->getTextures()[j]);
		if (foundTex == nullptr) // solid values are nullptrs
			continue;

		std::string texFilename = foundTex->getTextureFilename();
		for (size_t k = 0; k < m_materialFileNames.size(); k++) {
			if (m_materialFileNames[k] == texFilename) { // maybe store the material names into an unordered_map
				selectionArr[j] = k;
				break;
			}
		}
	}

	// Set material properties and bools
	setUIMaterialParams(m_settingsEditMat, material);
}

void ResourceManager::applyEditedMaterial(Material* material, static int selectionArr[]) {
	stbi_set_flip_vertically_on_load(false);

	// In case, where the texture is not used ("" is handled as no texture)
	std::string usePaths[6];
	findMaterialPaths(usePaths, getSettingsEdit(), m_materialFileNames, selectionArr);
	MaterialPaths newMaterialParams = createMaterialPaths(std::string(material->getName()), usePaths, getSettingsEdit());

	// Material to be edited and the parameters
	editMaterial(material, newMaterialParams);
	getScene()->updateMeshList();
}

void ResourceManager::setUIMaterialParams(SettingsMaterial& SetMat, Material*& material) {
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

void ResourceManager::transformOperation(glm::vec3 inValue, std::function<void(glm::vec3, Mesh*)> operationFunc) {
	for (auto models : getScene()->getModels()) {
		for (auto meshes : models->getMeshes()) {
			operationFunc(inValue, meshes);
		}
	}
}

void ResourceManager::replaceMaterials(Material* oldMat, Material* newMat) {
	for (auto& model : m_scene->getModels())
		for (auto& mesh : model->getMeshes()) {
			if (mesh->getMaterial() == oldMat)
				mesh->setMaterial(newMat);
		}
}

void ResourceManager::removeModelBySelection(int select) {
	// Remove mesh from vector and cleanup
	delete getScene()->getModels()[select];
	getScene()->getModels().erase(getScene()->getModels().begin() + select);
	getScene()->updateMeshList();
}

void ResourceManager::removeMaterialBySelection(int select) {
	std::vector<Material*>& materials = getScene()->getMaterials();
	replaceMaterials(materials[select], getScene()->getDefaultMaterial()); // Everything using the deleted material should use the default material
	
	delete materials[select]; // Delete the material ptr and rezise the vec
	materials.erase(materials.begin() + select);
	select = 0;

	clearUnusedTextures(); // Clear and update the scene
	getScene()->updateMeshList();
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