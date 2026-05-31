#pragma once
#include <vector>
#include <string>           // Inlude std::string

#include "material.h"
#include "camera.h"

#include <glad/gl.h>		// Include glad
#include <glm/glm.hpp>      // Include glm
#include <glm/gtc/matrix_transform.hpp>      // Include matrix transforms
#include <glm/gtc/type_ptr.hpp>
#include <kgfw/GLUtils.h>
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class

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
	void RenderGBuffer(Shader* shader, Camera* m_camera) const;
	void Render(Shader* shader, Camera* m_camera, const std::vector<glm::vec3> LightP, const std::vector<glm::vec3> LightColor, std::vector<float> LightStrength) const;
	void renderCube();
	void renderQuad();
	void setMaterial(Material* material);
	void setDisplayName(const std::string name);
	void setBackgroundName(const std::string name);
	void setModelPath(const std::string inPath);
	void setModelPathShort(const std::string inPath);

	std::string getDisplayName() const;
	std::string getBackgroundName() const;
	std::string getModelPath() const;
	std::string getModelPathShort() const;
	Material* getMaterial();

private:
	GLuint m_VAO = 0;
	GLuint m_EBO;
	GLuint m_VBO;
	size_t m_indiceCount;
	Material* m_material = 0;
	std::string m_meshDisplayName;
	std::string m_meshBackgroundName;
	std::string m_modelPath;
	std::string m_modelPathShort;
};