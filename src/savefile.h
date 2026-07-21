#pragma once
#include <iostream>
#include <string>
#include "savefileStructs.h"

// Vectors
template<typename T>
inline void write(std::ofstream& file, const std::vector<T>& vec) {
	size_t size = vec.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));
	file.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
}

template<typename T>
inline void read(std::ifstream& file, std::vector<T>& vec) {
	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));
	vec.resize(size);
	file.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
}

// Standard optional
template<typename T>
inline void write(std::ofstream& file, const std::optional<T> inString) {
	size_t nameLength = inString.value().size();
	file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
	file.write(inString.value().c_str(), nameLength);
}

template<typename T>
inline void read(std::ifstream& file, std::optional<T>& stringCache) {
	// Read 8 bytes from the file and copy into memory
	size_t nameLength;
	file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

	// Create a string buffer with the earlier name length. Using \0 to add empty for all the characters
	std::string outString(nameLength, '\0');
	file.read(&outString[0], nameLength);
	stringCache = outString;
}

// Strings
// Inline allows multiple identical definitions (normally this would trigger an error with identical defs), when each .cpp file gets it's own copy of this function
inline void write(std::ofstream& file, const std::string& inString) {
	size_t nameLength = inString.size();

	file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
	file.write(inString.c_str(), nameLength);
}

inline void read(std::ifstream& file, std::string& stringCache) {
	// Read 8 bytes from the file and copy into memory
	size_t nameLength;
	file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

	// Create a string buffer with the earlier name length. Using \0 to add empty for all the characters
	std::string outString(nameLength, '\0');
	file.read(&outString[0], nameLength);
	stringCache = outString;
}

// Generic format
template<typename T>
inline void write(std::ofstream& file, const T& inValue) {
	static_assert(std::is_trivially_copyable_v<T>);
	file.write(reinterpret_cast<const char*>(&inValue), sizeof(T));
}

template<typename T>
inline void read(std::ifstream& file, T& outValue) {
	static_assert(std::is_trivially_copyable_v<T>);
	file.read(reinterpret_cast<char*>(&outValue), sizeof(T));
}

inline void writeStringVector(std::ofstream& file, const std::vector<MaterialPaths>& inVec) {
	size_t size = inVec.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));

	for (size_t i = 0; i < inVec.size(); i++) {
		write(file, inVec[i].materialName);
		write(file, inVec[i].diffuse.path);
		write(file, inVec[i].metallic.path);
		write(file, inVec[i].roughness.path);
		write(file, inVec[i].normalPath.path);
	}
}

inline void readStringVector(std::ifstream& file, std::vector<MaterialPaths>& inVec) {
	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));
	inVec.resize(size);
	std::cout << "Vector size: " << inVec.size() << std::endl;

	for (size_t i = 0; i < inVec.size(); i++) {
		read(file, inVec[i].materialName);
		read(file, inVec[i].diffuse.path);
		read(file, inVec[i].metallic.path);
		read(file, inVec[i].roughness.path);
		read(file, inVec[i].normalPath.path);
	}
}

inline void writeMaterialVector(std::ofstream& file, const std::vector<MaterialPaths>& inVec) {
	size_t size = inVec.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));

	for (auto mat : inVec) 
		mat.serialize(file, mat);
}

inline void readMaterialVector(std::ifstream& file, std::vector<MaterialPaths>& inVec) {
	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));
	inVec.resize(size);
	std::cout << "Vector size: " << inVec.size() << std::endl;

	for (auto& mat : inVec)
		mat.deserialize(file, mat);
}

inline void writeFileMeshes(std::ofstream& file, std::vector<FileModels>& inVec) {
	size_t sizeModel = inVec.size();
	file.write(reinterpret_cast<const char*>(&sizeModel), sizeof(sizeModel));

	for (size_t i = 0; i < inVec.size(); i++) {
		write(file, inVec[i].modelPath);

		size_t sizeMesh = inVec[i].meshes.size();		
		write(file, sizeMesh); // file.write(reinterpret_cast<const char*>(&sizeMesh), sizeof(sizeMesh));
		for (size_t j = 0; j < inVec[i].meshes.size(); j++) {
			write(file, inVec[i].meshes[j].modelName);

			write(file, inVec[i].meshes[j].pos);
			write(file, inVec[i].meshes[j].scaling);
			write(file, inVec[i].meshes[j].rotation);

			write(file, inVec[i].meshes[j].textureID);
		}	
	}
}

inline void readFileMeshes(std::ifstream& file, std::vector<FileModels>& inVec) {
	size_t sizeModel;
	file.read(reinterpret_cast<char*>(&sizeModel), sizeof(sizeModel));
	inVec.resize(sizeModel);

	for (size_t i = 0; i < inVec.size(); i++) {
		read(file, inVec[i].modelPath);

		size_t sizeMesh;	
		read(file, sizeMesh);//file.read(reinterpret_cast<char*>(&sizeMesh), sizeof(sizeMesh));
		inVec[i].meshes.resize(sizeMesh);
		for (size_t j = 0; j < inVec[i].meshes.size(); j++) {
			read(file, inVec[i].meshes[j].modelName);

			read(file, inVec[i].meshes[j].pos);
			read(file, inVec[i].meshes[j].scaling);
			read(file, inVec[i].meshes[j].rotation);

			read(file, inVec[i].meshes[j].textureID);
		}
	}
}

inline void writeCameraData(std::ofstream& file, FileCamera& inCamera) {
	// Vec3s
	write(file, inCamera.cameraPos);
	write(file, inCamera.cameraFront);
	write(file, inCamera.cameraFocus);

	// Floats
	write(file, inCamera.radius);
	write(file, inCamera.theta);
	write(file, inCamera.phi);

	write(file, inCamera.pitch);
	write(file, inCamera.yaw);

	write(file, inCamera.lastX);
	write(file, inCamera.lastY);

	// Doubles
	write(file, inCamera.xPos);
	write(file, inCamera.yPos);

	// Bool
	write(file, inCamera.freeMovementEnabled);
}

inline void readCameraData(std::ifstream& file, FileCamera& outCamera) {
	// Vec3s
	read(file, outCamera.cameraPos);
	read(file, outCamera.cameraFront);
	read(file, outCamera.cameraFocus);

	// Floats
	read(file, outCamera.radius);
	read(file, outCamera.theta);
	read(file, outCamera.phi);

	read(file, outCamera.pitch);
	read(file, outCamera.yaw);

	read(file, outCamera.lastX);
	read(file, outCamera.lastY);

	// Doubles
	read(file, outCamera.xPos);
	read(file, outCamera.yPos);

	// Bool
	read(file, outCamera.freeMovementEnabled);
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
		write(file, m_lightData);

		// Write MaterialPaths
		writeMaterialVector(file, m_pathNames);

		// Write FileMeshes
		writeFileMeshes(file, m_fileModels);

		// Write backgroundTex
		write(file, m_backgroundTexPath);

		// Write HDRI
		write(file, m_hdriPath);

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
		read(file, lights);

		// Read MaterialPaths strings
		std::vector<MaterialPaths> materialPaths;
		readMaterialVector(file, materialPaths);

		std::vector<FileModels> fileModel;
		readFileMeshes(file, fileModel);

		std::string backgroundTex;
		read(file, backgroundTex);

		std::string hdri;
		read(file, hdri);

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