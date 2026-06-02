#include "textureLoading.h"
#include <stb_image.h>
#include <kgfw/GLUtils.h>	// Include GLUtils for checkGLError
#include <iostream>
#include <filesystem>
#include "material.h"

TextureLoading::TextureLoading() : Object(__FUNCTION__) {}

void TextureLoading::cleanupMaterials() {
	for (Texture* tex : m_textures) {
		delete tex;
	}
	m_textures.clear();

	// Clean up all the materials
	for (Material* mat : m_materials) {
		delete mat;
	}
	m_materials.clear();
}

TextureLoading::~TextureLoading() {
	cleanupMaterials();
}

Texture* TextureLoading::loadTexture(const std::string& path, bool flipTexture) {
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(flipTexture);
	GLubyte* data = stbi_load((ASSET_DIR + path).c_str(), &width, &height, &nrChannels, 0);
	stbi_set_flip_vertically_on_load(false);

	if (data) {
		Texture* texture = new Texture(width, height, nrChannels, data);
		texture->setFilePathShort(path);
		texture->setFilePath(ASSET_DIR + path);
		stbi_image_free(data);  // Free image data after creating the texture

		return texture;
	}
	else {
		printf("Error loading texture file \"%s\"\n", (ASSET_DIR + path).c_str());

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
		m_materials.push_back(new Material(textureIds, materialName, m_materialIndex));  // Add the material to the list, last three are diffuse, metallic and roughness

		// Track the textures for later cleanup
		for (Texture* tex : textures) {
			m_textures.push_back(tex);
		}
		m_materialIndex++;

		std::cout << "Loaded material: " << materialName << std::endl;
	}
	else {
		std::cout << "Error loading material: " << materialName << std::endl;
	}

	return m_materials.back();  // Return the last added material
}

std::unordered_map<int, Material*> TextureLoading::loadMaterials(int presetMode) {
	//std::unordered_map<int, Material*> materialsMap;

	// mat 0
	materialsMap[0] = checkAndAddMaterial(loadTextureSet(
		ASSET_DIR + std::string("/textures/checkerboard.png"),
		ASSET_DIR + std::string("/textures/checkerboard.png"),
		ASSET_DIR + std::string("/textures/checkerboard.png"),
		ASSET_DIR + std::string("/textures/checkerboardNormal.png")
	), "TestMaterial");

	if (presetMode >= 1) {
		// mat 1
		materialsMap[1] = checkAndAddMaterial(loadTextureSet(
			ASSET_DIR + std::string("/textures/PresetMaterials/MP18/MP18Low_Metallic_BaseColor.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/MP18/MP18Low_Metallic_Metallic.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/MP18/MP18Low_Metallic_Roughness.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/MP18/MP18Low_Metallic_Normal.png")
		), "MP18");
	}

	if (presetMode >= 2) {
		// mat 2
		materialsMap[2] = checkAndAddMaterial(loadTextureSet(
			ASSET_DIR + std::string("/textures/PresetMaterials/Barrel/Barrel_BaseColor.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/Barrel/Barrel_Metallic.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/Barrel/Barrel_Roughness.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/Barrel/Barrel_Normal.png")
		), "Barrel");
	}

	if (presetMode >= 3) {
		//mat 3
		materialsMap[3] = checkAndAddMaterial(loadTextureSet(
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Blade1_BaseColor.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Blade1_Metallic.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Blade1_Roughness.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Blade1_Normal.png")
		), "Blade");

		//mat 4
		materialsMap[4] = checkAndAddMaterial(loadTextureSet(
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Grip_BaseColor.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Grip_Metallic.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Grip_Roughness.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Grip_Normal.png")
		), "Grip");

		//mat 5
		materialsMap[5] = checkAndAddMaterial(loadTextureSet(
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_GripDetail_BaseColor.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_GripDetail_Metallic.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_GripDetail_Roughness.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_GripDetail_Normal.png")
		), "Ornaments");

		// mat 6
		materialsMap[6] = checkAndAddMaterial(loadTextureSet(
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_BaseColor.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_Metallic.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_Roughness.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnifeStand_DefaultMaterial_Normal.png")
		), "Holder");

		// mat 7
		materialsMap[7] = checkAndAddMaterial(loadTextureSet(
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Screws_BaseColor.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Screws_Metallic.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Screws_Roughness.png"),
			ASSET_DIR + std::string("/textures/PresetMaterials/OrnamentKnife/GoofyAhhKnife_Screws_Normal.png")
		), "Screws");
	}
	return materialsMap;
}

std::vector<Material*> TextureLoading::MaterialsPushback(const std::vector<MaterialPaths>& materialList) {
	std::vector<Material*> matVec;
	for (int i = 0; i < materialList.size(); i++) {
		matVec.push_back(checkAndAddMaterial(loadTextureSet(
			(materialList[i].colorPath),
			(materialList[i].metallicPath),
			(materialList[i].roughnessPath),
			(materialList[i].normalPath)
		), materialList[i].materialName));
	}

	return matVec;
}

Mesh* TextureLoading::processMesh(aiMesh* mesh, const aiScene* scene, const std::string path) {
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
	if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < m_materials.size()) {
		meshMaterial = m_materials[mesh->mMaterialIndex];
	}

	// Create the Mesh object with the vertices, indices, and preloaded material
	Mesh* newMesh = new Mesh(vertices, indices);
	newMesh->setMaterial(meshMaterial);
	newMesh->setModelPath(ASSET_DIR + path);
	newMesh->setModelPath(path);
	std::cout << "Path to model: " << path << std::endl;

	// Push back all the model's vertex amounts
	vertexAmount.push_back(vertices.size());
	//std::cout << newMesh->getName() << " - " << vertices.size() << " Amount of Indices: " << indices.size() << std::endl;

	return newMesh;
}

void setName(Mesh* meshRef, const std::string& name) {
	if (meshRef) {
		meshRef->setDisplayName(name);
		//std::cout << name << std::endl;
	}
	else { printf("Error: Mesh reference is null\n"); }
}

void setNameBack(Mesh* meshRef, const std::string& name) {
	if (meshRef) {
		meshRef->setBackgroundName(name);
		//std::cout << name << std::endl;
	}
	else { printf("Error: Mesh reference is null\n"); }
}

void TextureLoading::processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene, const std::string path) {
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		// the node object only contains indices to index the actual objects in the scene.
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes->push_back(processMesh(mesh, scene, path)); // Pass scene here

		setNameBack(meshes->back(), node->mName.C_Str());

		// Use (*var)[i] to deference the pointer!!
		//setName((*meshes)[i], node->mName.C_Str());
		//std::cout << node->mName.C_Str() << std::endl;
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(meshes, node->mChildren[i], scene, path);
	}
}

std::vector<Mesh*> TextureLoading::loadMeshes(const std::string& path, const std::string& meshName) {
	std::vector<Mesh*> meshes; // Create the container that will be returned by this function

	//read file with Assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile((ASSET_DIR + path), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	//Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf("Error loading model file \"%s\": \"%s\" ", (ASSET_DIR + path).c_str(), importer.GetErrorString());
		return meshes;
	}

	// Process Assimp's root node recursively
	processNode(&meshes, scene->mRootNode, scene, path);

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

Texture* TextureLoading::findTexture(GLuint textureID) {
	for (int i = 0; i < m_textures.size(); i++) {
		if (m_textures[i]->getTextureId() == textureID)
			return m_textures[i];
	}

	return m_textures[0];

	/*auto iterator = std::find(m_textures.begin(), m_textures.end(), int(textureID));
	
	int index = distance(m_textures.begin(), iterator);
	return m_textures[index];*/
}

// Added & to pass a reference, silly dinky me...
void TextureLoading::loadAllMeshes(std::vector<Mesh*>& meshes, int presetMode) {

	auto PlaneMesh = loadMeshes((std::string(ASSET_DIR) + "/models/plane.obj"), "Plane");
	for (size_t i = 0; i < PlaneMesh.size(); ++i) {
		meshes.push_back(PlaneMesh[i]);

		if (i == 0) {  // Reflectivity test plane
			meshes.back()->setScaling(glm::vec3(6.0f));
			meshes.back()->setMaterial(m_materials[0]);
			meshes.back()->setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
		}
	}

	if (presetMode >= 1) {
		auto MP18Mesh = loadMeshes((std::string(ASSET_DIR) + "/models/MP18Low.obj"), "MP18");
		for (size_t i = 0; i < MP18Mesh.size(); ++i) {
			meshes.push_back(MP18Mesh[i]);

			if (i == 0) {  // MP18 -Gun model
				meshes.back()->setScaling(glm::vec3(1.0f));
				meshes.back()->setMaterial(m_materials[1]);
				meshes.back()->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}
	}

	if (presetMode >= 2) {
		auto BarrelMesh = loadMeshes((std::string(ASSET_DIR) + "/models/barrel.obj"), "Barrel");
		for (size_t i = 0; i < BarrelMesh.size(); ++i) {
			meshes.push_back(BarrelMesh[i]);

			if (i == 0) {  // Barrel
				meshes.back()->setScaling(glm::vec3(1.0f));
				meshes.back()->setMaterial(m_materials[2]);
				meshes.back()->setPosition(glm::vec3(0.0f, 2.0f, 0.0f));
			}
		}
	}

	if (presetMode >= 3) {
		auto OrnamentKnifeMesh = loadMeshes((std::string(ASSET_DIR) + "/models/OrnamentKnife/1.0OrnamentKnife.obj"), "OrnamentKnife");
		for (size_t i = 0; i < OrnamentKnifeMesh.size(); ++i) {
			meshes.push_back(OrnamentKnifeMesh[i]);
			// Set unique transformations for each object (grip, blade, ornaments)
			switch (i) {
			case 0: // Blade
				meshes.back()->setMaterial(m_materials[3]);
				meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				break;
			case 1: // Screws - not the actual texture, uses the blade's texture
				meshes.back()->setMaterial(m_materials[0]);
				meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				break;
			case 2: // Ornaments
				meshes.back()->setMaterial(m_materials[5]);
				meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				break;
			case 3: // Grip
				meshes.back()->setMaterial(m_materials[4]);
				meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				break;
			case 4: // Holder
				meshes.back()->setMaterial(m_materials[6]);
				meshes.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				break;
			default:
				std::cout << "LoadMeshes Index out of reach" << std::endl;
			}
		}
	}

	std::cout << "Amount of meshes in the scene: " << meshes.size() << std::endl;
}

/*Material findMaterial(std::vector<Material> vecMat, Material targetMat) {
	for (auto mat : vecMat) {
		if (targetMat.getTextures() == mat.getTextures()) {
			std::cout << "Found material" << mat.getName() << std::endl;
			return mat;
		}	
	}
	std::cout << "ERROR: Could not find the corresponding material" << std::endl;
	return;
}*/

// Added & to pass a reference, silly dinky me...
void TextureLoading::loadMeshes(std::vector<Mesh*>& meshes, std::vector<FileMeshes> fileMeshes) {
	for (int i = 0; i < fileMeshes.size(); i++) {
		//auto newMesh = loadMeshes((std::string(ASSET_DIR) + "/models/plane.obj"), m_materials, "Plane");
		auto newMesh = loadMeshes((fileMeshes[i].modelPath), fileMeshes[i].modelName);
		for (size_t j = 0; j < newMesh.size(); ++j) {
			meshes.push_back(newMesh[j]);

			meshes.back()->setPosition(fileMeshes[i].pos);
			meshes.back()->setRotation(fileMeshes[i].rotation);
			meshes.back()->setScaling(fileMeshes[i].scaling);
			meshes.back()->setMaterial(m_materials[fileMeshes[i].textureID]);			
		}
	}
	std::cout << "Amount of meshes in the scene: " << meshes.size() << std::endl;
}