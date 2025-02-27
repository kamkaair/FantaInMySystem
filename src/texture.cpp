#include "texture.h"
#include <stb_image.h>
#include <kgfw/GLUtils.h>	// Include GLUtils for checkGLError
#include <iostream>

Texture::Texture(int width, int height, int nrChannels, const GLubyte* data) : Object(__FUNCTION__) {
	//stbi_set_flip_vertically_on_load(false);
	// Create texture
	glGenTextures(1, &m_textureId);
	checkGLError();
	// Bind it for use
	glBindTexture(GL_TEXTURE_2D, m_textureId);
	checkGLError();

	if (nrChannels == 4) {
		// set the texture data as RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else if (nrChannels == 3) {
		// set the texture data as RGB
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else if (nrChannels == 1) {
		// set the texture data as RED (for grayscale)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	checkGLError();

	// Set texture wrap parameters to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// set the texture filtering to nearest (disabled filtering)
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//checkGLError();
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//checkGLError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	checkGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	checkGLError();

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	checkGLError();
}

Texture::~Texture() {
	//glDeleteTextures(1, &m_textureId);
	if (m_textureId) {
		glDeleteTextures(1, &m_textureId);
	}
}

GLuint Texture::getTextureId() const {
	return m_textureId;
}