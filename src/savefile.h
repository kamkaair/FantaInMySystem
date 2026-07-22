#pragma once
#include <iostream>
#include <string>
#include "savefileStructs.h"

inline void writeMaterials(std::ofstream& file, std::vector<MaterialPaths>& inVec) {
	size_t size = inVec.size();
	Serializer::write(file, size);
	
	for (auto& mat : inVec) 
		mat.serialize(file);
}

inline void readMaterials(std::ifstream& file, std::vector<MaterialPaths>& inVec) {
	size_t size;
	Serializer::read(file, size);

	inVec.resize(size);
	std::cout << "Vector size: " << inVec.size() << std::endl;

	for (auto& mat : inVec)
		mat.deserialize(file);
}

inline void writeModels(std::ofstream& file, std::vector<FileModels>& inVec) {
	size_t sizeModel = inVec.size();
	Serializer::write(file, sizeModel);

	for (auto& model : inVec) 
		model.serialize(file);
}

inline void readModels(std::ifstream& file, std::vector<FileModels>& inVec) {
	size_t sizeModel;
	file.read(reinterpret_cast<char*>(&sizeModel), sizeof(sizeModel));
	inVec.resize(sizeModel);

	for (auto& model : inVec)
		model.deserialize(file);
}

inline void writeCameraData(std::ofstream& file, FileCamera& inCamera) {
	inCamera.serialize(file);
}

inline void readCameraData(std::ifstream& file, FileCamera& outCamera) {
	outCamera.deserialize(file);
}

class SaveFile {
public:
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
		Serializer::write(file, m_lightData);

		// Write MaterialPaths
		writeMaterials(file, m_pathNames);

		// Write FileMeshes
		writeModels(file, m_fileModels);

		// Write backgroundTex
		Serializer::write(file, m_backgroundTexPath);

		// Write HDRI
		Serializer::write(file, m_hdriPath);

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
		Serializer::read(file, lights);

		// Read MaterialPaths strings
		std::vector<MaterialPaths> materialPaths;
		readMaterials(file, materialPaths);

		std::vector<FileModels> fileModel;
		readModels(file, fileModel);

		std::string backgroundTex;
		Serializer::read(file, backgroundTex);

		std::string hdri;
		Serializer::read(file, hdri);

		FileCamera cameraData;
		readCameraData(file, cameraData);

		std::cout << filename << " - Object deserialized successfully." << std::endl;
		file.close();
		return SaveFile(lights, materialPaths, fileModel, cameraData, backgroundTex, hdri);
	}
	
	std::vector<FileLights>& getLightData() { return m_lightData; }
	const std::vector<MaterialPaths>& getPathNames() const { return m_pathNames; }
	const std::vector<FileModels>& getFileMeshes() const { return m_fileModels; }
	FileCamera& getFileCamera() { return m_fileCamera; }
	const std::string& getBackgroundTexPath() const { return m_backgroundTexPath; }
	const std::string& getHdriPath() const { return m_hdriPath; }

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