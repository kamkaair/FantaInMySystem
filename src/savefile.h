#include <fstream>
#include <iostream>
#include <string>

struct FileLights {
	glm::vec3 pos;
	glm::vec3 color;
	float strength;
};

template<typename T>
void readVector(std::ifstream& file, std::vector<T>& vec) {
	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size));
	vec.resize(size);
	file.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
}

template<typename T>
void writeVector(std::ofstream& file, const std::vector<T>& vec) {
	size_t size = vec.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));
	file.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
}

class SaveFile {
public:
	SaveFile(std::string name, int age, std::vector<glm::vec3> pos, std::vector<glm::vec3> color, std::vector<float> strength) : m_name(name), m_age(age),
		m_pos(pos), m_color(color), m_strength(strength) {}

	void serialize(const std::string& filename) {
		std::ofstream file(filename, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Error: Failed to open file for writing." << std::endl;
			return;
		}

		// Reading and writing takes in a pointer to the address of the value and the number of bytes
		// file.read(pointer, bytesNum)
		size_t nameLength = m_name.size();

		// char is treated as a raw byte in c++! The reinterpret_cast<char*> is treating the memory of a variable as raw bytes
		file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength)); // Get the memory address &nameLength (possibly nameLength has value of 5, stored in 0x1000) 

		// The string characters are written. name.c_str() points to the beginning of the string letters' address (0x2000, 0x2001, 0x2003...)
		file.write(m_name.c_str(), nameLength); // address pointer and how many bytes?

		file.write(reinterpret_cast<char*>(&m_age), sizeof(m_age)); // integer's address and size in bytes

		//float testFloat = 0.5;
		//std::cout << "vec3: " << sizeof(m_pos) << " nameLength: " << sizeof(size_t) << " m_name: " << sizeof(m_name) << " integer: " << sizeof(m_age) << " float: " << sizeof(testFloat) << std::endl;
		
		//file.write(reinterpret_cast<char*>(&m_pos), sizeof(m_pos));
		std::cout << "Array: " << sizeof(m_lightData) << " Vector: " << sizeof(m_lightDataVec) << std::endl;
		std::cout << "Pos: " << sizeof(m_pos) << " Color: " << sizeof(m_color) << " Strena: " << sizeof(m_strength) << std::endl;

		writeVector(file, m_pos);
		writeVector(file, m_color);
		writeVector(file, m_strength);

		std::cout << "Object serialized successfully." << std::endl;
	}

	static SaveFile deserialize(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Error: Failed to open file for reading." << std::endl;
			return SaveFile("", 0, std::vector<glm::vec3>(0), std::vector<glm::vec3>(0), std::vector<float>(0.0));
		}

		// Read 8 bytes from the file and copy into memory
		size_t nameLength;
		file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

		// Create a string buffer with the earlier name length. Using \0 to add empty for all the characters
		std::string name(nameLength, '\0');
		file.read(&name[0], nameLength);

		// Next 4 bytes for the integer memory with sizeof()
		int age;
		file.read(reinterpret_cast<char*>(&age), sizeof(age));

		//glm::vec3 pos;
		//file.read(reinterpret_cast<char*>(&pos), sizeof(pos));
		
		/*std::vector<glm::vec3> pos;
		size_t size;
		file.read(reinterpret_cast<char*>(&size), sizeof(size));
		pos.resize(size);
		file.read(reinterpret_cast<char*>(pos.data()), size * sizeof(glm::vec3));*/

		std::vector<glm::vec3> pos;
		readVector(file, pos);

		std::vector<glm::vec3> color;
		readVector(file, color);

		std::vector<float> strength;
		readVector(file, strength);

		std::cout << "Object deserialized successfully." << std::endl;

		return SaveFile(name, age, pos, color, strength);
	}

	// Getter methods for the class
	std::string getName() const { return m_name; }
	int getAge() const { return m_age; }
	std::vector<glm::vec3> getPosition() const { return m_pos; }
	std::vector<glm::vec3> getColor() const { return m_color; }
	std::vector<float> getStrength() const { return m_strength; }

private:
	std::string m_name;
	int m_age;
	std::vector<glm::vec3> m_pos;
	std::vector<glm::vec3> m_color;
	std::vector<float> m_strength;

	std::vector<FileLights> m_lightDataVec;
	FileLights m_lightData[3];

};