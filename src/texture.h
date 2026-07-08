#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <unordered_map>

class Texture : public kgfw::Object {
public:
	Texture(int width, int height, int nrChannels, const GLubyte* data, bool generateMipmaps = true);
	~Texture();

	void setFilePath(std::string inPath);

	GLuint getTextureId() const;
	std::string getFilePath() const;
	std::string getTextureFilename() const;
private:
	GLuint m_textureId;	// Texture id
	std::string m_filePath = "";
	std::string m_textureFilename = "";
};
