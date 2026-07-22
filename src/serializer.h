#pragma once
#include <fstream>
#include <vector>
#include <optional>

class Serializer {
public: 
	// Vectors
	template<typename T>
	static void write(std::ofstream& file, const std::vector<T>& vec) {
		size_t size = vec.size();
		file.write(reinterpret_cast<const char*>(&size), sizeof(size));
		file.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
	}

	template<typename T>
	static void read(std::ifstream& file, std::vector<T>& vec) {
		size_t size;
		file.read(reinterpret_cast<char*>(&size), sizeof(size));
		vec.resize(size);
		file.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
	}

	// Standard optional<string>
	static void write(std::ofstream& file, const std::optional<std::string> inString) {
		size_t nameLength = inString.value().size();
		file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
		file.write(inString.value().c_str(), nameLength);
	}

	static void read(std::ifstream& file, std::optional<std::string>& stringCache) {
		// Read 8 bytes from the file and copy into memory
		size_t nameLength;
		file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

		// Create a string buffer with the earlier name length. Using \0 to add empty for all the characters
		std::string outString(nameLength, '\0');
		file.read(&outString[0], nameLength);
		stringCache = outString;
	}
	/*
	// Possibly, I could try to make the optional take a templated data types as well, but it's good for now only for strings
	bool hasValue = value.has_value();
	write(hasValue);

	if (hasValue)
		write(*value);
	*/

	// Strings
	// Inline allows multiple identical definitions (normally this would trigger an error with identical defs), when each .cpp file gets it's own copy of this function
	static void write(std::ofstream& file, const std::string& inString) {
		size_t nameLength = inString.size();

		file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
		file.write(inString.c_str(), nameLength);
	}

	static void read(std::ifstream& file, std::string& stringCache) {
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
	static void write(std::ofstream& file, const T& inValue) {
		static_assert(std::is_trivially_copyable_v<T>);
		file.write(reinterpret_cast<const char*>(&inValue), sizeof(T));
	}

	template<typename T>
	static void read(std::ifstream& file, T& outValue) {
		static_assert(std::is_trivially_copyable_v<T>);
		file.read(reinterpret_cast<char*>(&outValue), sizeof(T));
	}
};