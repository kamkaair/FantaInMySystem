#include "mesh.h"
#include "shader.h"
#include "gameobject.h"

#include <GLFW/glfw3.h>		// Include glfw

Mesh::Mesh(const std::vector< Vertex >& vertices, const std::vector< unsigned int >& indices)
	: GameObject(__FUNCTION__) {
	m_indiceCount = indices.size();
	// create buffers/arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glBindVertexArray(0);
}

void Mesh::setMaterial(Material* material) {m_material = material;}
Material* Mesh::getMaterial() {return m_material;}

void Mesh::setDisplayName(const std::string name) { m_meshDisplayName = name;}
const std::string& Mesh::getDisplayName() const {return m_meshDisplayName;}

void Mesh::RenderGBuffer(Shader* shader, Camera* m_camera) const
{
	shader->bind();
	// Set uniform values to the shader
	// MVP Matrix (or I guess it's VP, since the model matrix is down there? :D)
	shader->setUniform("M", getModelMatrix());
	shader->setUniform("VP", m_camera->getProjectionMatrix() * m_camera->getViewMatrix());
	shader->setUniform("V", m_camera->getViewMatrix());

	// Bind material textures
	if (m_material) {
		// Set uniform values for material properties
		shader->setUniform("u_DiffuseColor", m_material->diffuseColor.x, m_material->diffuseColor.y, m_material->diffuseColor.z);
		shader->setUniform("u_Roughness", m_material->roughness);
		shader->setUniform("u_Metallic", m_material->metallic);
		shader->setUniform("u_emissionStrength", m_material->emission);

		shader->setUniform("useDiffuseTexture", m_material->useDiffuseTexture);
		shader->setUniform("useMetallicTexture", m_material->useMetallicTexture);
		shader->setUniform("useRoughnessTexture", m_material->useRoughnessTexture);
		shader->setUniform("useEmissionTexture", m_material->useEmissionTexture);

		const std::vector<GLuint>& textureIds = m_material->getTextures();
		// Ensure you bind the textures with the appropriate uniforms
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureIds[0]);  // DiffuseMap
		shader->setUniform("DiffuseMap", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureIds[1]);  // MetallicMap
		shader->setUniform("MetallicMap", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureIds[2]);  // RoughnessMap
		shader->setUniform("RoughnessMap", 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureIds[3]);  // NormalMap
		shader->setUniform("EmissionMap", 3);

		glActiveTexture(GL_TEXTURE4); 
		glBindTexture(GL_TEXTURE_2D, textureIds[5]);  // NormalMap
		shader->setUniform("NormalMap", 4);
	}

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indiceCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Unbind textures
	if (m_material) {
		const std::vector<GLuint>& textureIds = m_material->getTextures();
		for (int i = 0; i < textureIds.size(); ++i) {
			glActiveTexture(GL_TEXTURE1 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void Mesh::Render(Shader* shader, Camera* m_camera, const std::vector<FileLights> lights) const {
	// Bind the shader
	shader->bind();

	// Set uniform values to the shader
	// MVP Matrix (or I guess it's VP, since the model matrix is down there? :D)
	shader->setUniform("VP", m_camera->getProjectionMatrix() * m_camera->getViewMatrix());

	// Model matrix
	shader->setUniform("M", getModelMatrix());

	// Point light properties
	for (int i = 0; i < lights.size(); i++) {
		// Set the point light position
		shader->setUniform("pointLights[" + std::to_string(i) + "].position", lights[i].pos);

		// Set the point light color
		shader->setUniform("pointLights[" + std::to_string(i) + "].color", lights[i].color);

		// Set attenuation factors for the point light
		shader->setUniform("pointLights[" + std::to_string(i) + "].constant", 1.0f);   // Constant attenuation
		shader->setUniform("pointLights[" + std::to_string(i) + "].linear", 0.09f);    // Linear attenuation
		shader->setUniform("pointLights[" + std::to_string(i) + "].quadratic", 0.032f); // Quadratic attenuation
		shader->setUniform("pointLights[" + std::to_string(i) + "].strength", lights[i].strength); // Quadratic attenuation
	}
	int lightAmount = lights.size();
	shader->setUniform("NUM_POINT_LIGHTS", lightAmount);

	shader->setUniform("viewPos", m_camera->getPosition().x, m_camera->getPosition().y, m_camera->getPosition().z);
	
	// Bind material textures
	const std::vector<GLuint>& textureIds = m_material->getTextures();
	if (m_material) {

		// Set uniform values for material properties
		shader->setUniform("u_DiffuseColor", m_material->diffuseColor.x, m_material->diffuseColor.y, m_material->diffuseColor.z);
		shader->setUniform("u_Roughness", m_material->roughness);
		shader->setUniform("u_Metallic", m_material->metallic);
		shader->setUniform("u_emissionStrength", m_material->emission);
		shader->setUniform("u_opacity", m_material->opacity);

		shader->setUniform("useDiffuseTexture", m_material->useDiffuseTexture);
		shader->setUniform("useMetallicTexture", m_material->useMetallicTexture);
		shader->setUniform("useRoughnessTexture", m_material->useRoughnessTexture);
		shader->setUniform("useEmissionTexture", m_material->useEmissionTexture);
		shader->setUniform("useOpacityTexture", m_material->useOpacityTexture);

		// Ensure you bind the textures with the appropriate uniforms
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureIds[0]);  // DiffuseMap
		shader->setUniform("DiffuseMap", 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, textureIds[1]);  // MetallicMap
		shader->setUniform("MetallicMap", 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, textureIds[2]);  // RoughnessMap
		shader->setUniform("RoughnessMap", 5);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, textureIds[3]);  // NormalMap
		shader->setUniform("EmissionMap", 6);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, textureIds[5]);  // NormalMap
		shader->setUniform("NormalMap", 7);

		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, textureIds[4]);  // NormalMap
		shader->setUniform("OpacityMap", 8);
	}

	// draw mesh
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indiceCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Unbind textures
	if (m_material) {
		for (int i = 0; i < textureIds.size(); ++i) {
			glActiveTexture(GL_TEXTURE1 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

 //Function to render a 1x1 3D cube
void Mesh::renderCube() {
	static GLuint cubeVAO = 0;
	static GLuint cubeVBO = 0;
	static GLuint cubeEBO = 0;

	// Only initialize (generate) once
	if (cubeVAO == 0) {
		// Vertices for a 1x1 cube centered at the origin
		float skyboxVertices[] = {
			// Positions          
			-1.0f,  1.0f, -1.0f,  // 0
			-1.0f, -1.0f, -1.0f,  // 1
			 1.0f, -1.0f, -1.0f,  // 2
			 1.0f,  1.0f, -1.0f,  // 3
			-1.0f,  1.0f,  1.0f,  // 4
			-1.0f, -1.0f,  1.0f,  // 5
			 1.0f, -1.0f,  1.0f,  // 6
			 1.0f,  1.0f,  1.0f   // 7
		};

		unsigned int skyboxIndices[] = {
			// Front face
			0, 1, 2,
			2, 3, 0,
			// Back face
			7, 6, 5,
			5, 4, 7,
			// Left face
			4, 5, 1,
			1, 0, 4,
			// Right face
			3, 2, 6,
			6, 7, 3,
			// Top face
			4, 0, 3,
			3, 7, 4,
			// Bottom face
			1, 5, 6,
			6, 2, 1
		};

		// Skybox initialization
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glGenBuffers(1, &cubeEBO);  // New: Generate EBO

		glBindVertexArray(cubeVAO);

		// Bind VBO and send vertex data
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

		// Bind EBO and send index data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);  // New: Bind EBO
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		// Set vertex attribute pointer
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(6 * sizeof(float)));

		//glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Bind and render the cube
	glBindVertexArray(cubeVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::renderQuad()
{
	static GLuint quadVAO = 0, quadVBO, quadEBO;
	if (quadVAO == 0)
	{
		// Vertex data: positions and texture coordinates
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,   // Top-left
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,   // Bottom-left
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // Bottom-right
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f    // Top-right
		};

		// Indices for two triangles that form the quad
		unsigned int quadIndices[] = {
			0, 1, 2,   // First triangle
			0, 2, 3    // Second triangle
		};

		// Generate and bind VAO, VBO, and EBO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glGenBuffers(1, &quadEBO);  // New: Generate EBO

		glBindVertexArray(quadVAO);

		// Bind VBO and send vertex data
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		// Bind EBO and send index data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);  // New: Bind EBO
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

		// Set vertex attribute pointers
		// Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		// Texture coordinates
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindVertexArray(0);
	}

	// Bind and render the quad using glDrawElements
	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  // 6 indices for 2 triangles
	glBindVertexArray(0);
}

Mesh::~Mesh() {
	// Clean up OpenGL resources
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}