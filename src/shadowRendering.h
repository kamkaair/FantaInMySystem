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
	void renderShadowMapping(const std::vector<Model*>& scene, const glm::vec3& lightPos);
	void renderPointLightShadows(const std::vector<Model*>& models, const glm::vec3& lightPos);
	void clearShadowMapping();

	glm::mat4& getLightSpaceMatrix() { return m_lightSpaceMatrix; }
	GLuint& getCameraDepthFBO() { return m_depthDirFBO; }
	GLuint& getCameraDepthBuffer() { return m_depthDirBuffer; }
	GLuint& getPointShadowCubeMap() { return m_depthCubeBuffer; }
	Shader* getDepthShader() { return m_shadowMapShader; }

private:
	void constructDirectionalLight();
	void constructPointLight();
	//void renderMeshes(const std::vector<Model*>& models, Shader* inShader);

	Shader* m_shadowMapShader;
	Shader* m_shadowPointShader;

	glm::mat4 m_lightSpaceMatrix;
	GLuint m_depthDirFBO, m_depthDirBuffer;
	GLuint m_depthCubeFBO, m_depthCubeBuffer;
	int m_shadowWidth = 1024, m_shadowHeight = 1024;
};