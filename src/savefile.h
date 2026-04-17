#include <fstream>
#include <iostream>
#include <string>

class SaveFile {
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
							int newIndex = 0, res = -1;
							while ((res = textCache.find("(", res+1)) != std::string::npos)
							{
								allLights.push_back(readLines(textCache, res, newIndex));
								//std::cout << res << std::endl;
							}
							
						}
						
					}
				}		
			}
			file.close();

			for (auto l : allLights) {
				std::cout << l << std::endl;
			}
		}
		
	}
private:
	std::string readLines(std::string inText, int start, int& end) {
		std::string textCache;
		for (int i = start; i < inText.size(); i++) {
			textCache += inText[i];

			if (inText[i] == ')') {
				end = i;
				break;
			}
		}
		return textCache;
	}
	std::ifstream file;
	std::vector<std::string> allLights;
};