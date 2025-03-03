#pragma once
#include <vector>
#include <string>           // Inlude std::string
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class

#include "gameobject.h"
#include "shader.h"
#include "material.h"
#include "texture.h"

#include <glad/gl.h>		// Include glad
#include <glm/glm.hpp>      // Include glm
#include <glm/gtc/matrix_transform.hpp>      // Include matrix transforms
#include <glm/gtc/type_ptr.hpp>
#include <kgfw/GLUtils.h>
#include <kgfw/Object.h>

class Shader;

struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoords;
	glm::vec3 normal;
};

class Mesh : public GameObject {
	
public:
	
	Mesh(const std::vector< Vertex >& vertices, const std::vector< unsigned int >& indices);
	~Mesh();
	//const std::vector<glm::vec3> 
	void Render(Shader* shader, const glm::vec3& viewPos, std::vector<glm::vec3> LightP, std::vector<glm::vec3> LightColor, const glm::mat4& viewMatrix = glm::mat4(1.0f), const glm::mat4& modelMatrix = glm::mat4(1.0f), const glm::mat4& projectionMatrix = glm::mat4(1.0f)) const;
    void renderCube();
	void renderQuad();
	void setMaterial(Material* material);
	void setDisplayName(const std::string name);
	void setBackgroundName(const std::string name);

	std::string getDisplayName() const;
	std::string getBackgroundName() const;
	Material* getMaterial();

private:
	GLuint m_VAO = 0;
	GLuint m_EBO;
	GLuint m_VBO;
	size_t m_indiceCount;
	Material* m_material = 0;
	std::string m_meshDisplayName;
	std::string m_meshBackgroundName;
};