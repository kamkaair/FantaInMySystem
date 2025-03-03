#include "textureLoading.h"
#include <stb_image.h>
#include <kgfw/GLUtils.h>	// Include GLUtils for checkGLError
#include <iostream>

TextureLoading::TextureLoading() : Object(__FUNCTION__) {}

TextureLoading::~TextureLoading() {
	for (Texture* tex : m_trackedTextures) {
		delete tex;
	}
	m_trackedTextures.clear();

	// Clean up all the materials
	for (Material* mat : m_materials) {
		delete mat;
	}
	m_materials.clear();
}

Texture* TextureLoading::loadTexture(const std::string& path) {
	int width, height, nrChannels;
	GLubyte* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data) {
		Texture* texture = new Texture(width, height, nrChannels, data);
		stbi_image_free(data);  // Free image data after creating the texture
		return texture;
	}
	else {
		printf("Error loading texture file \"%s\"\n", path.c_str());
		return nullptr;
	}
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
		m_materials.push_back(new Material(textureIds, materialName));  // Add the material to the list, last three are diffuse, metallic and roughness

		// Track the textures for later cleanup
		for (Texture* tex : textures) {
			m_trackedTextures.push_back(tex);
		}

		std::cout << "Loaded material: " << materialName << std::endl;
	}
	else {
		std::cout << "Error loading material: " << materialName << std::endl;
	}

	return m_materials.back();  // Return the last added material
}

std::unordered_map<int, Material*> TextureLoading::loadMaterials() {
	//std::unordered_map<int, Material*> materialsMap;

	// mat 0
	materialsMap[0] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/checkerboard.png"),
		ASSET_DIR + std::string("/textures/checkerboard.png"),
		ASSET_DIR + std::string("/textures/checkerboard.png"),
		ASSET_DIR + std::string("/textures/checkerboard.png")
	), "TestMaterial");

	//mat 1
	materialsMap[1] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Blade1_BaseColor.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Blade1_Metallic.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Blade1_Roughness.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Blade1_Normal.png")
	), "Blade");

	//mat 2
	materialsMap[2] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Grip_BaseColor.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Grip_Metallic.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Grip_Roughness.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Grip_Normal.png")
	), "Grip");

	//mat 3
	materialsMap[3] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_GripDetail_BaseColor.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_GripDetail_Metallic.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_GripDetail_Roughness.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_GripDetail_Normal.png")
	), "Ornaments");

	// mat 4
	materialsMap[4] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_BaseColor.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_Metallic.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_Roughness.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_Normal.png")
	), "Holder");

	// mat 5
	materialsMap[5] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Screws_BaseColor.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Screws_Metallic.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Screws_Roughness.png"),
		ASSET_DIR + std::string("/textures/OrnamentKnife/GoofyAhhKnife_Screws_Normal.png")
	), "Screws");

	// mat 6
	materialsMap[6] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/Barrel_BaseColor.png"),
		ASSET_DIR + std::string("/textures/Barrel_Metallic.png"),
		ASSET_DIR + std::string("/textures/Barrel_Roughness.png"),
		ASSET_DIR + std::string("/textures/Barrel_Normal.png")
	), "Barrel");

	// mat 7
	materialsMap[7] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/MP18Low_Metallic_BaseColor.png"),
		ASSET_DIR + std::string("/textures/MP18Low_Metallic_Metallic.png"),
		ASSET_DIR + std::string("/textures/MP18Low_Metallic_Roughness.png"),
		ASSET_DIR + std::string("/textures/MP18Low_Metallic_Normal.png")
	), "MP18");

	return materialsMap;
}

Mesh* TextureLoading::processMesh(aiMesh* mesh, const aiScene* scene, const std::vector<Material*>& loadedMaterials) {
	//TODO 1: add data containers for vertices and indices
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	// Load vertex data
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex; //temporable container for the data of each loop
		//TODO 2: load data from the Assimp mesh to our containers
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

	// Retrieve the corresponding vertex indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Assign the preloaded material by index
	Material* meshMaterial = nullptr;
	if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < loadedMaterials.size()) {
		meshMaterial = loadedMaterials[mesh->mMaterialIndex];
	}

	// Create the Mesh object with the vertices, indices, and preloaded material
	Mesh* newMesh = new Mesh(vertices, indices);
	newMesh->setMaterial(meshMaterial);

	// Push back all the model's vertex amounts
	vertexAmount.push_back(vertices.size());
	//std::cout << newMesh->getName() << " - " << vertices.size() << " Amount of Indices: " << indices.size() << std::endl;

	return newMesh;
}

void setName(Mesh* meshRef, const std::string& name) {
	if (meshRef) {
		meshRef->setDisplayName(name);
		std::cout << name << std::endl;
	}
	else { printf("Error: Mesh reference is null\n"); }
}

void setNameBack(Mesh* meshRef, const std::string& name) {
	if (meshRef) {
		meshRef->setBackgroundName(name);
		std::cout << name << std::endl;
	}
	else { printf("Error: Mesh reference is null\n"); }
}

void TextureLoading::processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene, const std::vector<Material*>& loadedMaterials) {
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		// the node object only contains indices to index the actual objects in the scene.
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes->push_back(processMesh(mesh, scene, loadedMaterials)); // Pass scene here

		setNameBack(meshes->back(), node->mName.C_Str());

		// Use (*var)[i] to deference the pointer!!
		//setName((*meshes)[i], node->mName.C_Str());
		//std::cout << node->mName.C_Str() << std::endl;
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(meshes, node->mChildren[i], scene, loadedMaterials);
	}
}

std::vector<Mesh*> TextureLoading::loadMeshes(const std::string& path, const std::vector<Material*>& loadedMaterials, const std::string& meshName) {
	std::vector<Mesh*> meshes; // Create the container that will be returned by this function

	//read file with Assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	//Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf("Error loading model file \"%s\": \"%s\" ", path.c_str(), importer.GetErrorString());
		return meshes;
	}

	// Process Assimp's root node recursively
	processNode(&meshes, scene->mRootNode, scene, loadedMaterials);

	// Set the name for each mesh
	int i = 0;
	//std::string lastly = meshName + i;
	for (auto& mesh : meshes) {
		setName(mesh, meshName + " " + std::to_string(i));
		i++;
	}

	return meshes;
}

std::vector<std::string> TextureLoading::FileSystem(std::string& path)
{
	std::vector<std::string> filenames;
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		//std::cout << entry.path() << std::endl;
		filenames.push_back(entry.path().filename().string());
	}

	return filenames;

}

// Added & to pass a reference, silly dinky me...
void TextureLoading::loadAllMeshes(std::vector<Mesh*>& meshes) {
	auto OrnamentKnifeMesh = loadMeshes((std::string(ASSET_DIR) + "/models/OrnamentKnife/1.0OrnamentKnife.obj"), m_materials, "OrnamentKnife");
	for (size_t i = 0; i < OrnamentKnifeMesh.size(); ++i) {
		meshes.push_back(OrnamentKnifeMesh[i]);
		// Set unique transformations for each object (grip, blade, ornaments)
		switch (i) {
		case 0: // Blade
			meshes.back()->setMaterial(m_materials[1]);
			meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			break;
		case 1: // Screws - not the actual texture, uses the blade's texture
			meshes.back()->setMaterial(m_materials[0]);
			meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			break;
		case 2: // Ornaments
			meshes.back()->setMaterial(m_materials[3]);
			meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			break;
		case 3: // Grip
			meshes.back()->setMaterial(m_materials[2]);
			meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			break;
		case 4: // Holder
			meshes.back()->setMaterial(m_materials[4]);
			meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			break;
		default:
			std::cout << "LoadMeshes Index out of reach" << std::endl;
		}
	}

	auto BarrelMesh = loadMeshes((std::string(ASSET_DIR) + "/models/barrel.obj"), m_materials, "Barrel");
	for (size_t i = 0; i < BarrelMesh.size(); ++i) {
		meshes.push_back(BarrelMesh[i]);

		if (i == 0) {  // Barrel
			meshes.back()->setScaling(glm::vec3(1.0f));
			meshes.back()->setMaterial(m_materials[6]);
			meshes.back()->setPosition(glm::vec3(0.0f, 2.0f, 0.0f));
		}
	}

	auto MP18Mesh = loadMeshes((std::string(ASSET_DIR) + "/models/MP18Low.obj"), m_materials, "MP18");
	for (size_t i = 0; i < MP18Mesh.size(); ++i) {
		meshes.push_back(MP18Mesh[i]);

		if (i == 0) {  // MP18 -Gun model
			meshes.back()->setScaling(glm::vec3(1.0f));
			meshes.back()->setMaterial(m_materials[7]);
			meshes.back()->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}
	std::cout << "Amount of meshes in the scene: " << meshes.size() << std::endl;
}