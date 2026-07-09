#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include "savefileStructs.h"

template<typename T>
void writeVector(std::ofstream& file, const std::vector<T>& vec) {
	size_t size = vec.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));
	file.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
}

template<typename T>
void readVector(std::ifstream& file, std::vector<T>& vec) {
	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));
	vec.resize(size);
	file.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
}

// Inline allows multiple identical definitions (normally this would trigger an error with identical defs), when each .cpp file gets it's own copy of this function
inline void writeString(std::ofstream& file, const std::string inString) {
	size_t nameLength = inString.size();

	file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
	file.write(inString.c_str(), nameLength);
}

inline void readString(std::ifstream& file, std::string& stringCache) {
	// Read 8 bytes from the file and copy into memory
	size_t nameLength;
	file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

	// Create a string buffer with the earlier name length. Using \0 to add empty for all the characters
	std::string outString(nameLength, '\0');
	file.read(&outString[0], nameLength);
	stringCache = outString;
}

inline void writeStringVector(std::ofstream& file, const std::vector<MaterialPaths>& inVec) {
	size_t size = inVec.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));

	for (size_t i = 0; i < inVec.size(); i++) {
		writeString(file, inVec[i].materialName);
		writeString(file, inVec[i].diffuse.path);
		writeString(file, inVec[i].metallic.path);
		writeString(file, inVec[i].roughness.path);
		writeString(file, inVec[i].normalPath);
	}
}

inline void readStringVector(std::ifstream& file, std::vector<MaterialPaths>& inVec) {
	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));
	inVec.resize(size);
	std::cout << "Vector size: " << inVec.size() << std::endl;

	for (size_t i = 0; i < inVec.size(); i++) {
		readString(file, inVec[i].materialName);
		readString(file, inVec[i].diffuse.path);
		readString(file, inVec[i].metallic.path);
		readString(file, inVec[i].roughness.path);
		readString(file, inVec[i].normalPath);
	}
}

inline void writeMaterialVector(std::ofstream& file, const std::vector<MaterialPaths>& inVec) {
	size_t size = inVec.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));

	for (size_t i = 0; i < inVec.size(); i++) {
		writeString(file, inVec[i].materialName);

		writeString(file, inVec[i].diffuse.path);
		file.write(reinterpret_cast<const char*>(&inVec[i].diffuse.useMap), sizeof(inVec[i].diffuse.useMap));
		file.write(reinterpret_cast<const char*>(&inVec[i].diffuse.value), sizeof(inVec[i].diffuse.value));

		writeString(file, inVec[i].metallic.path);
		file.write(reinterpret_cast<const char*>(&inVec[i].metallic.useMap), sizeof(inVec[i].metallic.useMap));
		file.write(reinterpret_cast<const char*>(&inVec[i].metallic.value), sizeof(inVec[i].metallic.value));

		writeString(file, inVec[i].roughness.path);
		file.write(reinterpret_cast<const char*>(&inVec[i].roughness.useMap), sizeof(inVec[i].roughness.useMap));
		file.write(reinterpret_cast<const char*>(&inVec[i].roughness.value), sizeof(inVec[i].roughness.value));

		writeString(file, inVec[i].normalPath);
	}
}

inline void readMaterialVector(std::ifstream& file, std::vector<MaterialPaths>& inVec) {
	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));
	inVec.resize(size);
	std::cout << "Vector size: " << inVec.size() << std::endl;

	for (size_t i = 0; i < inVec.size(); i++) {
		readString(file, inVec[i].materialName);

		readString(file, inVec[i].diffuse.path);
		file.read(reinterpret_cast<char*>(&inVec[i].diffuse.useMap), sizeof(inVec[i].diffuse.useMap));
		file.read(reinterpret_cast<char*>(&inVec[i].diffuse.value), sizeof(inVec[i].diffuse.value));

		readString(file, inVec[i].metallic.path);
		file.read(reinterpret_cast<char*>(&inVec[i].metallic.useMap), sizeof(inVec[i].metallic.useMap));
		file.read(reinterpret_cast<char*>(&inVec[i].metallic.value), sizeof(inVec[i].metallic.value));

		readString(file, inVec[i].roughness.path);
		file.read(reinterpret_cast<char*>(&inVec[i].roughness.useMap), sizeof(inVec[i].roughness.useMap));
		file.read(reinterpret_cast<char*>(&inVec[i].roughness.value), sizeof(inVec[i].roughness.value));

		readString(file, inVec[i].normalPath);
	}
}

inline void writeFileMeshes(std::ofstream& file, std::vector<FileModels>& inVec) {
	size_t sizeModel = inVec.size();
	file.write(reinterpret_cast<const char*>(&sizeModel), sizeof(sizeModel));

	for (size_t i = 0; i < inVec.size(); i++) {
		writeString(file, inVec[i].modelPath);

		size_t sizeMesh = inVec[i].meshes.size();
		file.write(reinterpret_cast<const char*>(&sizeMesh), sizeof(sizeMesh));
		for (size_t j = 0; j < inVec[i].meshes.size(); j++) {
			writeString(file, inVec[i].meshes[j].modelName);

			file.write(reinterpret_cast<char*>(&inVec[i].meshes[j].pos), sizeof(inVec[i].meshes[j].pos));
			file.write(reinterpret_cast<char*>(&inVec[i].meshes[j].scaling), sizeof(inVec[i].meshes[j].scaling));
			file.write(reinterpret_cast<char*>(&inVec[i].meshes[j].rotation), sizeof(inVec[i].meshes[j].rotation));

			file.write(reinterpret_cast<char*>(&inVec[i].meshes[j].textureID), sizeof(inVec[i].meshes[j].textureID));
		}	
	}
}

inline void readFileMeshes(std::ifstream& file, std::vector<FileModels>& inVec) {
	size_t sizeModel;
	file.read(reinterpret_cast<char*>(&sizeModel), sizeof(sizeModel));
	inVec.resize(sizeModel);

	for (size_t i = 0; i < inVec.size(); i++) {
		readString(file, inVec[i].modelPath);

		size_t sizeMesh;
		file.read(reinterpret_cast<char*>(&sizeMesh), sizeof(sizeMesh));
		inVec[i].meshes.resize(sizeMesh);
		for (size_t j = 0; j < inVec[i].meshes.size(); j++) {
			readString(file, inVec[i].meshes[j].modelName);

			file.read(reinterpret_cast<char*>(&inVec[i].meshes[j].pos), sizeof(inVec[i].meshes[j].pos));
			file.read(reinterpret_cast<char*>(&inVec[i].meshes[j].scaling), sizeof(inVec[i].meshes[j].scaling));
			file.read(reinterpret_cast<char*>(&inVec[i].meshes[j].rotation), sizeof(inVec[i].meshes[j].rotation));

			file.read(reinterpret_cast<char*>(&inVec[i].meshes[j].textureID), sizeof(inVec[i].meshes[j].textureID));
		}
	}
}

inline void writeCameraData(std::ofstream& file, FileCamera& inCamera) {
	file.write(reinterpret_cast<const char*>(&inCamera), sizeof(inCamera));

	file.write(reinterpret_cast<const char*>(&inCamera.cameraPos), sizeof(inCamera.cameraPos));
	file.write(reinterpret_cast<const char*>(&inCamera.cameraFront), sizeof(inCamera.cameraFront));
	file.write(reinterpret_cast<const char*>(&inCamera.cameraFocus), sizeof(inCamera.cameraFocus));

	file.write(reinterpret_cast<const char*>(&inCamera.freeMovementEnabled), sizeof(inCamera.freeMovementEnabled));
}

inline void readCameraData(std::ifstream& file, FileCamera& outVec) {
	file.read(reinterpret_cast<char*>(&outVec), sizeof(outVec));

	file.read(reinterpret_cast<char*>(&outVec.cameraPos), sizeof(outVec.cameraPos));
	file.read(reinterpret_cast<char*>(&outVec.cameraFront), sizeof(outVec.cameraFront));
	file.read(reinterpret_cast<char*>(&outVec.cameraFocus), sizeof(outVec.cameraFocus));

	file.read(reinterpret_cast<char*>(&outVec.freeMovementEnabled), sizeof(outVec.freeMovementEnabled));
}

class SaveFile {
public:
	/*SaveFile(std::string name, int age, std::vector<glm::vec3> pos, std::vector<glm::vec3> color, std::vector<float> strength) : m_name(name), m_age(age),
		m_pos(pos), m_color(color), m_strength(strength) {}*/
	SaveFile(std::vector<FileLights> lightData, std::vector<MaterialPaths> pathNames, 
		std::vector<FileModels> fileMeshes, FileCamera fileCamera, std::string backgroundTextPath, std::string hdriPath)
		: m_lightData(lightData), m_pathNames(pathNames), m_fileModels(fileMeshes), m_fileCamera(fileCamera),
		m_backgroundTexPath(backgroundTextPath), m_hdriPath(hdriPath) {}

	void serialize(const std::string& filename) {
		std::ofstream file(filename, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Error: Failed to open file for writing." << std::endl;
			return;
		}
		
		// Write lightdata
		writeVector(file, m_lightData);

		// Write MaterialPaths
		writeMaterialVector(file, m_pathNames);

		// Write FileMeshes
		writeFileMeshes(file, m_fileModels);

		// Write backgroundTex
		writeString(file, m_backgroundTexPath);

		// Write HDRI
		writeString(file, m_hdriPath);

		// Write camera data
		writeCameraData(file, m_fileCamera);
		std::cout << std::endl;
		std::cout << filename << " - Object serialized successfully." << std::endl;
		file.close();
	}

	static SaveFile deserialize(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Error: Failed to open file for reading." << std::endl;
			return SaveFile(std::vector<FileLights>(0), std::vector<MaterialPaths>(), std::vector<FileModels>(), FileCamera(), "", "");
		}

		// Read lights
		std::vector<FileLights> lights;
		readVector(file, lights);

		// Read MaterialPaths strings
		std::vector<MaterialPaths> materialPaths;
		//readStringVector(file, materialPaths);
		readMaterialVector(file, materialPaths);

		std::vector<FileModels> fileModel;
		readFileMeshes(file, fileModel);

		std::string backgroundTex;
		readString(file, backgroundTex);

		std::string hdri;
		readString(file, hdri);

		FileCamera cameraData;
		readCameraData(file, cameraData);

		std::cout << filename << " - Object deserialized successfully." << std::endl;
		file.close();
		return SaveFile(lights, materialPaths, fileModel, cameraData, backgroundTex, hdri);
	}
	
	std::vector<FileLights> getLightData() const { return m_lightData; }
	std::vector<MaterialPaths> getPathNames() const { return m_pathNames; }
	std::vector<FileModels> getFileMeshes() const { return m_fileModels; }
	FileCamera getFileCamera() const { return m_fileCamera; }
	std::string getBackgroundTexPath() const { return m_backgroundTexPath; }
	std::string getHdriPath() const { return m_hdriPath; }

private:
	std::vector<FileLights> m_lightData;
	std::vector<MaterialPaths> m_pathNames;
	std::vector<FileModels> m_fileModels;
	FileCamera m_fileCamera;
	std::string m_backgroundTexPath;
	std::string m_hdriPath;
};

/* READING:
* 
	// Read 8 bytes from the file and copy into memory
	size_t nameLength;
	file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

	// Create a string buffer with the earlier name length. Using \0 to add empty for all the characters
	std::string name(nameLength, '\0');
	file.read(&name[0], nameLength);

	// Next 4 bytes for the integer memory with sizeof()
	int age;
	file.read(reinterpret_cast<char*>(&age), sizeof(age));

	// vec3
	glm::vec3 pos;
	file.read(reinterpret_cast<char*>(&pos), sizeof(pos));
*
*/

/* WRITING
* 
 	// Reading and writing takes in a pointer to the address of the value and the number of bytes
	// file.read(pointer, bytesNum)
	size_t nameLength = m_name.size();

	// std::string + "Alice". 8 + 5 = 13 bytes
	// char is treated as a raw byte in c++! The reinterpret_cast<char*> is treating the memory of a variable as raw bytes
	file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength)); // Get the memory address &nameLength (possibly nameLength has value of 5, stored in 0x1000) 

	// The string characters are written. name.c_str() points to the beginning of the string letters' address (0x2000, 0x2001, 0x2003...)
	file.write(m_name.c_str(), nameLength); // address pointer and how many bytes?

	std::cout << "nameLength: " << m_name.size() << " Sizeof size_t " << sizeof(size_t) << " m_name: " << sizeof(m_name) << std::endl;

	file.write(reinterpret_cast<char*>(&m_age), sizeof(m_age)); // integer's address and size in bytes
*
*/