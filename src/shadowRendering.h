#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include "utils.h"
#include "models.h"

class Shader;

class ShadowRendering {
public:
	ShadowRendering();
	~ShadowRendering();
	void constructShadowMapping();
	void renderShadowMapping(const std::vector<Model*>& scene, const glm::vec3& lightPos);

	glm::mat4& getLightSpaceMatrix() { return m_lightSpaceMatrix; }
	GLuint& getCameraDepthFBO() { return m_cameraDepthFBO; }
	GLuint& getCameraDepthBuffer() { return m_cameraDepthBuffer; }
	Shader* getDepthShader() { return m_shadowMapShader; }
private:
	Shader* m_shadowMapShader;

	glm::mat4 m_lightSpaceMatrix;
	GLuint m_cameraDepthFBO, m_cameraDepthBuffer;
	int m_shadowWidth = 1024, m_shadowHeight = 1024;
};