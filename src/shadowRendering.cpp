#include "shadowRendering.h"
#include <glm/glm.hpp>      // Include glm
#include <glm/gtc/matrix_transform.hpp>      // Include matrix transforms
#include <glm/gtc/type_ptr.hpp>
#include <kgfw/GLUtils.h>

ShadowRendering::ShadowRendering() {
	m_shadowMapShader = utils::makeShader("DirShadowVert.glsl", "DirShadowFrag.glsl");
	m_shadowPointShader = utils::makeShader("PointShadowDepthVert.glsl", "PointShadowDepthFrag.glsl", "PointShadowDepthGeo.glsl");
	constructPointLight();
	constructDirectionalLight();
	checkGLError();
}

ShadowRendering::~ShadowRendering() {
	if (m_shadowMapShader != 0) { utils::deleteObject(m_shadowMapShader); }
	if (m_shadowPointShader != 0) { utils::deleteObject(m_shadowMapShader); }
}

void ShadowRendering::constructDirectionalLight() {
	// Create camera's depth buffer
	glGenFramebuffers(1, &m_depthDirFBO);

	glGenTextures(1, &m_depthDirBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthDirBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowWidth, m_shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); // Set the border color 1.0f, should return a shadow value of 0.0f
	checkGLError();

	glBindFramebuffer(GL_FRAMEBUFFER, m_depthDirFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthDirBuffer, 0);
	glDrawBuffer(GL_NONE); // "Explicitly tell OpenGL this framebuffer object does not render to a color buffer". Only the depth values are important when generating a depth cubemap
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGLError();
}

void ShadowRendering::constructPointLight() {
	glGenTextures(1, &m_depthCubeBuffer);
	glGenFramebuffers(1, &m_depthCubeFBO); // Generate and bind fb, use the colorbuffer

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_depthCubeBuffer);
	for (unsigned int i = 0; i < 6; ++i) // (1024 * 1024) * 6... big
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			m_shadowWidth, m_shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthCubeFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthCubeBuffer, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/*void ShadowRendering::renderMeshes(const std::vector<Model*>& models) {
	
}*/

//, const std::vector<FileLights>& pointLights
void ShadowRendering::renderPointLightShadows(const std::vector<Model*>& models, const glm::vec3& lightPos) {
	// Load the cubemap projection and views for the 6 directions
	const float far_plane = 25.0f;
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), (float)m_shadowWidth / (float)m_shadowHeight, 1.0f, far_plane); // Originally near = 0.1, far = 10.0
	glm::mat4 captureVP[6] = {
		captureProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		captureProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		captureProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		captureProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		captureProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		captureProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	// 1. render scene to depth cubemap
	// --------------------------------
	glViewport(0, 0, m_shadowWidth, m_shadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthCubeFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_FRONT); // Render the scene's meshes
	m_shadowPointShader->bind();
	if (!models.empty()) {
		for (Model* model : models)
			for (Mesh* mesh : model->getMeshes()) {
				mesh->renderMeshOnly(m_shadowPointShader);
			}
	}

	for (unsigned int i = 0; i < 6; ++i)
		m_shadowPointShader->setUniform("shadowMatrices[" + std::to_string(i) + "]", captureVP[i]);

	m_shadowPointShader->setUniform("far_plane", far_plane);
	m_shadowPointShader->setUniform("lightPos", lightPos);
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowRendering::renderShadowMapping(const std::vector<Model*>& models, const glm::vec3& lightPos) {
	glViewport(0, 0, m_shadowWidth, m_shadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, getCameraDepthFBO());
	glClear(GL_DEPTH_BUFFER_BIT);
	checkGLError();

	// Shader and Matrices:
	// Light POV, matrices
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f); // near plane = 1.0f, far plane = 7.5f. Let's try 0.1f, 100.0f
	glm::mat4 lightView = glm::lookAt(lightPos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	m_lightSpaceMatrix = lightProjection * lightView;
	checkGLError();

	// Directional lighting
	glCullFace(GL_FRONT); // Render the scene's meshes
	getDepthShader()->bind();
	if (!models.empty()) {
		for (Model* model : models)
			for (Mesh* mesh : model->getMeshes()) {
				// Light space for shadow mapping
				getDepthShader()->setUniform("lightMatrix", m_lightSpaceMatrix);
				mesh->renderMeshOnly(getDepthShader());
			}
	}
	glCullFace(GL_BACK);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowRendering::clearShadowMapping() {
	glBindFramebuffer(GL_FRAMEBUFFER, getCameraDepthFBO());
	glClear(GL_DEPTH_BUFFER_BIT);
	m_lightSpaceMatrix = glm::mat4(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}