#pragma once
#include "gameobject.h"     // Include base class
#include <glad/gl.h>		// Include glad
#include "shader.h"

class Shader;

class Teapot : public GameObject {
public:
	Teapot();
	~Teapot();

    void render(Shader* shader, const glm::vec3& viewpos, const glm::mat4& viewMatrix = glm::mat4(1.0f), const glm::mat4& projectionMatrix = glm::mat4(1.0f), GLuint textureId = 0, GLuint textureId2 = 0);
	//void render(Shader* shader, glm::vec3 viewpos, const glm::mat4& viewMatrix = glm::mat4(1.0f), const glm::mat4& projectionMatrix = glm::mat4(1.0f), GLuint textureId = 0);

	void values() {

	}

private:
	// TODO: decladre handles to the VAO, and VBOs to positions, normals and texture coordinates.
	GLuint m_posVBO;
	GLuint m_TexCordVBO;
	GLuint m_NormalVBO;
	GLuint m_VAO;

};
