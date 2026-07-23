#include "textureLoading.h"
#include <stb_image.h>
#include <kgfw/GLUtils.h>	// Include GLUtils for checkGLError
#include <iostream>
#include <filesystem>

TextureLoading::TextureLoading() : Object(__FUNCTION__) {}

void TextureLoading::cleanupTextures() {
	for (auto texMap : m_textureMap) {
		glDeleteTextures(1, &texMap.first);
		delete texMap.second;
	}

	m_textureMap.clear();
	m_materialIndex = 0;
}

TextureLoading::~TextureLoading() {
	cleanupTextures();
}

Texture* TextureLoading::loadTexture(const std::string& path, bool flipTexture) {
	// Find out, whether the texture has been already loaded... if so, then just use the existing textureID
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(flipTexture);
	GLubyte* data = stbi_load((ASSET_DIR + path).c_str(), &width, &height, &nrChannels, 0);
	stbi_set_flip_vertically_on_load(false);

	if (!data) {
		printf("TEXTURE ERROR: failed loading a texture file \"%s\"\n", (ASSET_DIR + path).c_str());
		return nullptr;
	}

	Texture* texture = new Texture(width, height, nrChannels, data);
	texture->setFilePath(path);
	stbi_image_free(data);  // Free image data after creating the texture

	return texture;
}

std::pair<std::vector<GLuint>, std::vector<Texture*>> TextureLoading::loadTextureSet(const std::string& baseColorPath, const std::string& metallicMapPath,
	const std::string& roughnessMapPath, const std::string& normalMapPath) {
	std::vector<GLuint> textureIds;
	std::vector<Texture*> textures;

	// Load each texture and check for errors
	Texture* baseColor = loadTexture(baseColorPath.c_str());
	textureIds.push_back(baseColor->getTextureId());
	textures.push_back(baseColor);

	Texture* metallicMap = loadTexture(metallicMapPath.c_str());
	textureIds.push_back(metallicMap->getTextureId());
	textures.push_back(metallicMap);

	Texture* roughnessMap = loadTexture(roughnessMapPath.c_str());
	textureIds.push_back(roughnessMap->getTextureId());
	textures.push_back(roughnessMap);

	Texture* normalMap = loadTexture(normalMapPath.c_str());
	textureIds.push_back(normalMap->getTextureId());
	textures.push_back(normalMap);

	if (!baseColor || !metallicMap || !roughnessMap || !normalMap) {
		printf("Error: Failed to load one or more textures\n");
		return {};
	}

	return { textureIds, textures };
}

Material* TextureLoading::checkAndAddMaterial(const std::pair<std::vector<GLuint>, std::vector<Texture*>>& textureData, const std::string& materialName) {
	const std::vector<GLuint>& textureIds = textureData.first;
	const std::vector<Texture*>& textures = textureData.second;

	if (!textureIds.empty()) {
		m_scene->getMaterials().push_back(new Material(textureIds, materialName, m_materialIndex));  // Add the material to the list, last three are diffuse, metallic and roughness

		// Track the textures for later cleanup
		for (Texture* tex : textures) {
			m_textureMap.insert({tex->getTextureId(), tex});
		}
		m_materialIndex++;
	}
	else {
		std::cout << "Error loading material: " << materialName << std::endl;
	}

	return m_scene->getMaterials().back();  // Return the last added material
}

void TextureLoading::checkDuplicateTextures(std::vector<GLuint>& textureIDs, const std::vector<std::string> maps) {
	// Decide to either use an image texture or use a value for the maps
	for (auto map : maps) {
		if (map.empty()) {
			textureIDs.push_back(GLuint(0));
			continue;
		}

		Texture* textureMap;
		bool newMap = true;

		// Find out, whether the texture has been already loaded... if so, then just use the existing textureID
		// TODO: look for a better way, maybe...
		for (auto tex : m_textureMap) {
			if (tex.second->getFilePath() == map) {
				textureMap = tex.second;
				newMap = false;
				break;
			}
		}

		if (newMap) {
			textureMap = loadTexture(map.c_str());
			//m_textures.push_back(textureMap);
			if (textureMap == nullptr) { // Fallback on the default material, if loading a texture fails
				textureIDs.push_back(m_scene->getDefaultMaterial()->getTextures()[0]);
				continue;
			}

			m_textureMap.insert({ textureMap->getTextureId(), textureMap });
		}
		textureIDs.push_back(textureMap->getTextureId());
		std::cout << "Is a newMap: " << newMap << " - TexID: " << textureMap->getTextureId() << " - Name: " << textureMap->getFilePath() << std::endl;
	}
}

Material* TextureLoading::createMaterial(const MaterialPaths& materialPaths) {
	// Maps added to make the texture loading process more tidy
	std::vector<GLuint> textureIDs;

	std::vector<std::string> maps = {
		materialPaths.diffuse.path.value_or(""), // TODO: I've marked the empty file paths as nullopt during the deserialization (from empty to nullopt back to empty... take a look at this again)
		materialPaths.metallic.path.value_or(""),
		materialPaths.roughness.path.value_or(""),
		materialPaths.emission.path.value_or(""),
		materialPaths.opacity.path.value_or(""),
		materialPaths.normalPath.path.value() // Normal maps always use textures
	};

	// Decide to either use an image texture or use a value for the maps
	checkDuplicateTextures(textureIDs, maps);

	// Create a new material and push_back into the scene
	Material* newMat = new Material(textureIDs, materialPaths.materialName, m_materialIndex);
	m_scene->getMaterials().push_back(newMat);
	m_materialIndex++;

	newMat->useDiffuseTexture = textureIDs[0] != 0 ? true : false;
	newMat->useMetallicTexture = textureIDs[1] != 0 ? true : false;
	newMat->useRoughnessTexture = textureIDs[2] != 0 ? true : false;
	newMat->useEmissionTexture = textureIDs[3] != 0 ? true : false;
	newMat->useOpacityTexture = textureIDs[4] != 0 ? true : false;

	newMat->diffuseColor = materialPaths.diffuse.value;
	newMat->metallic = materialPaths.metallic.value;
	newMat->roughness = materialPaths.roughness.value;
	newMat->emission = materialPaths.emission.value;
	newMat->opacity = materialPaths.opacity.value;

	return newMat;
}

void TextureLoading::editMaterial(Material* editableMat, const MaterialPaths& materialPaths) {
	// Maps added to make the texture loading process more tidy
	std::vector<GLuint> textureIDs;

	std::vector<std::string> maps = {
		materialPaths.diffuse.path.value_or(""),
		materialPaths.metallic.path.value_or(""),
		materialPaths.roughness.path.value_or(""),
		materialPaths.emission.path.value_or(""),
		materialPaths.opacity.path.value_or(""),
		materialPaths.normalPath.path.value() // Normal maps always use textures
	};

	// Decide to either use an image texture or use a value for the maps
	checkDuplicateTextures(textureIDs, maps);

	for (size_t i = 0; i < editableMat->getTextures().size(); i++) {
		editableMat->getTextures()[i] = textureIDs[i];
	}

	editableMat->useDiffuseTexture = textureIDs[0] != 0 ? true : false;
	editableMat->useMetallicTexture = textureIDs[1] != 0 ? true : false;
	editableMat->useRoughnessTexture = textureIDs[2] != 0 ? true : false;
	editableMat->useEmissionTexture = textureIDs[3] != 0 ? true : false;
	editableMat->useOpacityTexture = textureIDs[4] != 0 ? true : false;

	editableMat->diffuseColor = materialPaths.diffuse.value;
	editableMat->metallic = materialPaths.metallic.value;
	editableMat->roughness = materialPaths.roughness.value;
	editableMat->emission = materialPaths.emission.value;
	editableMat->opacity = materialPaths.opacity.value;
}

std::vector<Material*> TextureLoading::MaterialsPushback(const std::vector<MaterialPaths>& materialList) {
	std::vector<Material*> matVec;
	for (int i = 0; i < materialList.size(); i++) {
		matVec.push_back(createMaterial(materialList[i]));
	}

	return matVec;
}

Material* TextureLoading::findTexturesWithPath(const std::string path, const aiScene* scene, aiMesh* mesh) {
	// Assign the preloaded material by index
	const std::vector<std::string> types = { "_Diffuse", "_Metallic", "_Roughness", "_Emissive", "_Normal" };

	std::vector<std::pair<std::string, std::string>> allFiles = FileSystemTuple(ASSET_DIR + getFolderSearchPath());
	std::vector<std::string> filenames, cutPath;
	for (auto f : allFiles) {
		filenames.push_back(f.first);
		cutPath.push_back(f.second);
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* Mat = scene->mMaterials[mesh->mMaterialIndex];
		aiString MatName;
		MaterialPaths usables;

		std::vector<std::optional<std::string>*> maps = {
			{&usables.diffuse.path},
			{&usables.metallic.path},
			{&usables.roughness.path},
			{&usables.emission.path},
			{&usables.opacity.path},
			{&usables.normalPath.path} // Normal maps always use textures
		};

		if ((Mat->Get(AI_MATKEY_NAME, MatName) != AI_SUCCESS)) { // Assimp error catching and Get() a material name in one
			return nullptr;
		}

		// Set the loaded material name
		usables.materialName = MatName.C_Str();

		// Check, whether a material with a same name already exists. Use the existing one
		for (const auto& material : m_scene->getMaterials()) {
			if (usables.materialName == material->getName()) {
				std::cout << "Material: " << usables.materialName << " already exists" << std::endl;
				return material;
			}
		}

		// Find the corresponding textures
		for (int i = 0; i < types.size(); i++) {
			auto it = std::find(filenames.begin(), filenames.end(), std::string(MatName.C_Str() + types[i])); // find the string
			int nameIndex = std::distance(filenames.begin(), it); // get the index for the found string, parallel usage with ends

			std::cout << "Trying to find: " << std::string(MatName.C_Str() + types[i]) << " - ";

			if (it != filenames.end()) {
				std::cout << "Texture Found! Format: " << types[i] << std::endl;
				*maps[i] = getFolderSearchPath() + cutPath[nameIndex]; // includes folders and their paths (textures/coolFolder/coolTex.png)
			}
			else {
				std::cout << "ERROR: Not found! Target format: " << types[i] << std::endl;
				*maps[i] = std::nullopt;
			}
		}

		if (!usables.normalPath.path.has_value()) // Add the default normal path, if normal path is empty
			usables.normalPath.path = emptyNormalPath;

		if(usables.emission.path.has_value())
			usables.emission.value = 1.0f;

		if (!usables.opacity.path.has_value())
			usables.opacity.value = 1.0f;

		// Create a new material with the values
		return createMaterial(usables);
	}
	return nullptr;
}

std::vector<Vertex> loadVertexData(aiMesh* mesh) {
	// Load vertex data
	std::vector<Vertex> vertices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex; //temporable container for the data of each loop
		vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		if (mesh->HasTextureCoords(0)) {
			vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else {
			vertex.texCoords = glm::vec2(0.0f, 0.0f); // Set default texture coordinates
		}
		vertices.push_back(vertex);
	}
	return vertices;
}

std::vector<unsigned int> loadIndices(aiMesh* mesh) {
	// Retrieve the corresponding vertex indices
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
	return indices;
}

Mesh* TextureLoading::processMesh(aiMesh* mesh, const aiScene* scene, const std::string path, bool autoTexture) {	
	std::vector<Vertex> vertices = loadVertexData(mesh); // Load vertex data
	std::vector<unsigned int> indices = loadIndices(mesh); // Load/retrieve the indices of the vertices
	Material* meshMaterial = nullptr; // Assign the preloaded material by index

	if (autoTexture) {
		meshMaterial = findTexturesWithPath(path, scene, mesh); // Find materials
	}
	else {
		if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < m_scene->getMaterials().size())
			meshMaterial = m_scene->getMaterials()[mesh->mMaterialIndex];
	}

	// Create the Mesh object with the vertices, indices, and preloaded material
	Mesh* newMesh = new Mesh(vertices, indices);
	newMesh->setMaterial(meshMaterial);

	// Push back all the model's vertex amounts
	newMesh->getVertices() = vertices.size();
	//std::cout << newMesh->getName() << " - " << vertices.size() << " Amount of Indices: " << indices.size() << std::endl;

	return newMesh;
}

void setMeshDisplayName(Mesh* meshRef, const std::string& name) {
	if (meshRef)
		meshRef->setDisplayName(name);

	else { printf("Error: Mesh reference is null\n"); }
}

void TextureLoading::processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene, const std::string path, bool autoTexture) {
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		// the node object only contains indices to index the actual objects in the scene.
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes->push_back(processMesh(mesh, scene, path, autoTexture));	
		setMeshDisplayName(meshes->back(), node->mName.C_Str());
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(meshes, node->mChildren[i], scene, path, autoTexture);
	}
}

std::vector<Mesh*> TextureLoading::processMeshes(const std::string& path, bool autoTexture) {
	std::vector<Mesh*> meshes; // Create the container that will be returned by this function

	//read file with Assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile((ASSET_DIR + path), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes ); // Take a look at these, if they bring any trouble

	//Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf("Error loading model file \"%s\": \"%s\" ", (ASSET_DIR + path).c_str(), importer.GetErrorString());
		return meshes;
	}

	// Process Assimp's root node recursively
	processNode(&meshes, scene->mRootNode, scene, path, autoTexture);

	return meshes;
}

std::vector<std::string> TextureLoading::FileSystem(const std::string& path) {
	std::vector<std::string> filenames;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
		if (entry.path().extension() == "") // Eliminate the folders (and the other files without an extension)
			continue;

		std::filesystem::path relative = std::filesystem::relative(entry.path(), path); // Get the path relative to the given path (cut out ASSET_DIR)
		filenames.push_back(relative.generic_string());
		//filenames.push_back(entry.path().filename().string());
	}

	return filenames;
}

std::vector<std::string> TextureLoading::FileSystemFolders(const std::string& path) {
	std::vector<std::string> folders;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
		if (entry.path().extension() == "") { // Eliminate the folders (and the other files without an extension)
			std::filesystem::path relative = std::filesystem::relative(entry.path(), path); // Get the path relative to the given path (cut out ASSET_DIR)
			folders.push_back(relative.generic_string());
		}
	}

	return folders;
}

std::vector<std::pair<std::string, std::string>> TextureLoading::FileSystemTuple(const std::string& path) {
	std::vector<std::pair<std::string, std::string>> filenames;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
		std::filesystem::path relative = std::filesystem::relative(entry.path(), path); // Get the path relative to the given path (cut out ASSET_DIR)
		filenames.push_back(std::pair(entry.path().stem().string(), relative.generic_string())); // could use relative.generic_string() to force forward slashes
	}

	return filenames;
}

Texture* TextureLoading::findTexture(GLuint textureID) {
	auto it = m_textureMap.find(textureID); // m_textureMap.at(textureID) works as well
	if (it == m_textureMap.end())
		return nullptr;

	return it->second;
}

void TextureLoading::loadMeshes(std::vector<Model*>& container, std::vector<FileModels> fileModels) {
	/*for (auto mat : m_scene->getMaterials()) {
		std::cout << "Mat index: " << mat->getMaterialIndex() << " - Mat name: " << mat->getName() << std::endl;
	}*/
	for (size_t i = 0; i < fileModels.size(); i++) {
		std::vector<Mesh*> newMeshes = processMeshes(fileModels[i].modelPath);

		for (size_t j = 0; j < newMeshes.size(); j++) {
			newMeshes[j]->setPosition(fileModels[i].meshes[j].pos);
			newMeshes[j]->setRotation(fileModels[i].meshes[j].rotation);
			newMeshes[j]->setScaling(fileModels[i].meshes[j].scaling);
			newMeshes[j]->setMaterial(m_scene->getMaterials()[fileModels[i].meshes[j].textureID]);
			newMeshes[j]->setDisplayName(fileModels[i].meshes[j].modelName);
		}
		container.push_back(new Model(fileModels[i].modelPath, newMeshes));
	}

	std::cout << "Amount of main meshes in the scene: " << container.size() << std::endl;
}