#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include "material.h"
#include <unordered_map>

class Texture : public kgfw::Object {
public:
	Texture(int width, int height, int nrChannels, const GLubyte* data, const std::string filePath);
	~Texture();

	GLuint getTextureId() const;
	std::string getFilePath() const;
private:
	GLuint m_textureId;	// Texture id
	std::string m_filePath;
};
