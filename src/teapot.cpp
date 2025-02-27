#include "teapot.h"
#include <kgfw/GLUtils.h>           // Include GLUtils for checkGLError
#include "shader.h"
#include <glm/gtx/transform.hpp>	// glm transform functions.
#include <vector>
#include "teapot_data.h"


Teapot::Teapot()
    : GameObject(__FUNCTION__) {

	// TODO:
	// Create a buffer for pos, texcord and normal

		// 1. Create VAO
	// Create Vertex Array Object
	glGenVertexArrays(1, &m_VAO);
	checkGLError();

	// Create Vertex Buffer Object
	//Position buffer
	glGenBuffers(1, &m_posVBO);
	checkGLError();

	//Texture coordinates
	glGenBuffers(1, &m_TexCordVBO);
	checkGLError();

	// Create Normal Buffer Object
	glGenBuffers(1, &m_NormalVBO);
	checkGLError();

	//Bind vertex array
	glBindVertexArray(m_VAO);
	checkGLError();

	// 2. Create VBO for positions. Use TEAPOT_POSITIONS -variable (declader in teapot_data.h) for positions data.
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	checkGLError();
	glBufferData(GL_ARRAY_BUFFER, sizeof(TEAPOT_POSITIONS), TEAPOT_POSITIONS, GL_STATIC_DRAW);
	checkGLError();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	checkGLError();
	glEnableVertexAttribArray(0);
	checkGLError();

	// 3. Create VBO for texture coordinates Use TEAPOT_TEXTURE_COORDINATES -variable (declader in teapot_data.h) 
	//    for texture coordinates data.
	// Set buffer data to m_texCoordsVbo-object (bind buffer first and then set the data)
	glBindBuffer(GL_ARRAY_BUFFER, m_TexCordVBO);
	checkGLError();
	glBufferData(GL_ARRAY_BUFFER, sizeof(TEAPOT_TEXTURE_COORDINATES), TEAPOT_TEXTURE_COORDINATES, GL_STATIC_DRAW);
	checkGLError();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	checkGLError();
	glEnableVertexAttribArray(1);
	checkGLError();
	
	// 4. Create VBO for normals. Use TEAPOT_NORMALS variable (declader in teapot_data.h) 
	//    for normals data.
	glBindBuffer(GL_ARRAY_BUFFER, m_NormalVBO);
	checkGLError();
	glBufferData(GL_ARRAY_BUFFER, sizeof(TEAPOT_NORMALS), TEAPOT_NORMALS, GL_STATIC_DRAW);
	checkGLError();
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	checkGLError();
	glEnableVertexAttribArray(2);
	checkGLError();

}

Teapot::~Teapot() {
	// TODO: Delete created VAO and VBOs here.
	glDeleteBuffers(1, &m_posVBO);
	checkGLError();

	glDeleteBuffers(1, &m_TexCordVBO);
	checkGLError();

	glDeleteBuffers(1, &m_NormalVBO);
	checkGLError();

	glDeleteVertexArrays(1, &m_VAO);
	checkGLError();
}

void Teapot::render(Shader* shader, const glm::vec3& viewpos, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, GLuint textureId, GLuint textureId2) {
	// TODO: Bind the shader
	shader->bind();

#if 1
	shader->setUniform("ligthPos", 0.0f, 1.0f, 0.0f);
	shader->setUniform("lightColor", 1.0f, 1.0f, 1.0f);
	shader->setUniform("viewPos", viewpos.x, viewpos.y, viewpos.z);
#endif
	

	// TODO: Set uniform values to the shader (MVP Matrix)
	shader->setUniform("MVP", projectionMatrix * glm::inverse(viewMatrix) * getModelMatrix());
	
	shader->setUniform("M", getModelMatrix());
	checkGLError();
	
	// TODO: Set texture uniform to the shader
	if (textureId > 0) {
		shader->setUniform("texture0", 0);
		//GL_TEXTURE0, GL_TEXTURE1... so on...
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);

	}
	// TODO: Bind VAO.
	glBindVertexArray(m_VAO);
	checkGLError();
	// TODO: Draw teapot using glDrawArrays with TEAPOT_NUM_VERTICES.
	glDrawArrays(GL_TRIANGLES, 0, TEAPOT_NUM_VERTICES);
	checkGLError();
}

