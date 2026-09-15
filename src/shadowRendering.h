#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include "utils.h"
#include "models.h"
//#include "savefileStructs.h"

class Shader;

class ShadowRendering : public kgfw::Object {
public:
	ShadowRendering();
	~ShadowRendering();
	void renderShadowMapping(const std::vector<Model*>& scene, const glm::vec3& lightPos);
	void renderPointLightShadows(const std::vector<Model*>& models, const std::vector<FileLights>& lightVec);
	void updatePointLights(const std::vector<FileLights>& lightVec);
	void clearShadowMapping();

	glm::mat4& getLightSpaceMatrix() { return m_lightSpaceMatrix; }
	GLuint& getCameraDepthFBO() { return m_depthDirFBO; }
	GLuint& getCameraDepthBuffer() { return m_depthDirBuffer; }
	std::vector<GLuint>& getPointShadowCubeMap() { return m_depthCubeBuffers; }
	Shader* getDepthShader() { return m_shadowMapShader; }

private:
	void constructDirectionalLight();
	void constructPointLight();
	//void renderMeshes(const std::vector<Model*>& models, Shader* inShader);

	Shader* m_shadowMapShader;
	Shader* m_shadowPointShader;

	glm::mat4 m_lightSpaceMatrix;
	// Directional depth
	GLuint m_depthDirFBO, m_depthDirBuffer;
	// Point shadows
	GLuint m_depthCubeFBO;
	std::vector<GLuint> m_depthCubeBuffers;

	int m_shadowWidth = 1024, m_shadowHeight = 1024;
};