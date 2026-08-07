#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>

#include "glm/gtx/string_cast.hpp" // Include for printing mats and vecs
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>				// Include glfw for windows

#include "shader.h"

namespace utils {

	static std::string loadShader(const std::string& filename)
	{
		//Shader loaded as a single string
		std::string result = "";
		std::string line = "";

		std::ifstream myFile(filename.c_str());

		if (myFile.is_open())
		{
			while (std::getline(myFile, line)) {
				result += line + "\n";
			}
			myFile.close();
		}

		return result;
	}

	static Shader* makeShader(const std::string vertex, const std::string frag) {
		// Load the main vertex and fragment shaders
		std::string vertexShaderSource = loadShader(std::string(ASSET_DIR) + "/shaders/" + vertex);
		std::string fragmentShaderSource = loadShader(std::string(ASSET_DIR) + "/shaders/" + frag);

		// Build and compile our shader program
		return new Shader(vertexShaderSource, fragmentShaderSource);
	}

	static void bindTexture(int glTexture, Shader* inShader, GLuint colorBuffer, std::string name, int value, int type = GL_TEXTURE_2D) {
		int GL_TEX_INDEX = glTexture - 33984; // 33984 is the integer number of the lowest GL_TEX (this is slightly crazy)

		glActiveTexture(glTexture);
		glBindTexture(type, colorBuffer);
		inShader->setUniform(name, GL_TEX_INDEX);
	}

	// Lambda  helper for deletion of objects. C++ doesn't support polymorphism, so this'll do
	template<typename T>
	inline void deleteObject(T*& ptr) {
		delete ptr;
		ptr = nullptr;
	}

	class utils {
	public:
		utils() : t0(glfwGetTime()), nbFrames(0), result(0.0f) {}

		float calculateFPS() {
			// Get current time
			double currentTime = glfwGetTime();

			// Check if one second has passed or this is the first frame
			if (currentTime - t0 >= 1.0 || nbFrames == 0) {
				// Print frame time and FPS
					//printf("%f ms/frame\n", 1000.0 / double(nbFrames)); This one in use!
				//printf("%g fps\n", 1 / int(nbFrames));
					//std::cout << "Frames per second: " << nbFrames << std::endl; This one in use!
				//printf("%g fps\n", 1000.0 / double(nbFrames));

				// Store the FPS result
				//result = 1000.0 / double(nbFrames);
				result = nbFrames;

				// Reset the timer and frame count
				t0 = currentTime;
				nbFrames = 0;
				//currentTime += 1.0;
			}

			// Increment frame count
			nbFrames++;

			// Return the FPS result (it updates every second)
			return result;
		}

	private:
		double t0;        // Time at the start of the current second
		float nbFrames;   // Number of frames since the last update
		float result;     // Last calculated FPS
	};
}