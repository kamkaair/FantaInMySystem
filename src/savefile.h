#include <fstream>
#include <iostream>
#include <string>

struct FileLights {
	glm::vec3 pos;
	glm::vec3 color;
	float strength;
};

class SaveFile {
public:
	SaveFile(std::string name, int age) : m_name(name), m_age(age) {}

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
		std::cout << "Object serialized successfully." << std::endl;
	}

	static SaveFile deserialize(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Error: Failed to open file for reading." << std::endl;
			return SaveFile("", 0);
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
		std::cout << "Object deserialized successfully." << std::endl;

		return SaveFile(name, age);
	}

	// Getter methods for the class
	std::string getName() const { return m_name; }
	int getAge() const { return m_age; }

private:
	std::string m_name;
	int m_age;
};

/*class SaveFile {
public:
	SaveFile() {
		//std::ofstream file(std::string(ASSET_DIR) + "/Saves/testSave.txt");
		file.open(std::string(ASSET_DIR) + "/Saves/testSave.txt");
		if (file.is_open())
		{
			std::string textCache;
			bool inBrackets = false;
			while (getline(file, textCache)) {
				std::string newData;
				for (int i = 0; i < textCache.size(); i++) {
					if (textCache[i] == '{')
						inBrackets = true;
					else if (textCache[i] == '}')
						inBrackets = false;

					if (inBrackets) {
						if (textCache[i] == '[') {
							int newIndex = 0, res = -1, dimension = 0;
							while ((res = textCache.find("(", res+1)) != std::string::npos)
							{
								std::vector<std::string> vecArray;
								FileLights pondTech;
								//allLights.push_back(readLines(textCache, res, newIndex));
								std::string separatedText = readLines(textCache, res, newIndex, vecArray);

								std::cout << "vecArray size: " << vecArray.size() << std::endl;

								//glm::vec3 newVec = floatToVec3(stof(vecArray[0]), stof(vecArray[1]), stof(vecArray[2]));
								std::vector<float> printVec;
								
								switch (vecArray.size()) {

									case 3:
										glm::vec3 newVec = floatToVec3(stof(vecArray[0]), stof(vecArray[1]), stof(vecArray[2]));
										if (dimension = 1)
											pondTech.pos = newVec;
										else
											pondTech.color = newVec;

										for (int i = 0; i < 3; i++)
											printVec.push_back(stof(vecArray[i]));
										break;
								
									case 1:
										pondTech.strength = stof(vecArray[0]);
										printVec.push_back(stof(vecArray[0]));
										break;
								
									default:
										std::cout << "ERROR " << vecArray.size() << std::endl;
										break;
								}

								allLights.push_back(pondTech);

								vecArray.clear();
							} // create int, float and vec3 conversions. str to int: stoi()
							for (auto l : allLights) {
								std::cout << "Pos: " << glm::to_string(l.pos) << std::endl;
								std::cout << "Color: " << glm::to_string(l.color) << std::endl;
								std::cout << "Strena:" << l.strength << std::endl;
							}
						}
						
					}
				}		
			}
			file.close();
		}
		
	}
private:
	glm::vec3 floatToVec3(float p1, float p2, float p3) {
		return glm::vec3(p1, p2, p3);
	}

	std::string readLines(std::string inText, int start, int& end, std::vector<std::string>& vecValues) {
		std::string textCache, numberCache;
		int dimension = 0;
		for (int i = start; i < inText.size(); i++) {
			if (inText[i] == ')') {
				end = i;
				break;
			}
			else if (inText[i] == '(' || inText[i] == ' ')
				continue;
			else {
				if (inText[i] == ',') {		
					vecValues.push_back(numberCache);
					numberCache.clear();
				}
				else {
					numberCache += inText[i];
				}
				textCache += inText[i];
			}
		}
		vecValues.push_back(numberCache);
		return textCache;
	}
	std::ifstream file;
	std::vector<FileLights> allLights;
};*/