#include "shadowRendering.h"
#include <glm/glm.hpp>      // Include glm
#include <glm/gtc/matrix_transform.hpp>      // Include matrix transforms
#include <glm/gtc/type_ptr.hpp>
#include <kgfw/GLUtils.h>

ShadowRendering::ShadowRendering() : Object(__FUNCTION__) {
	m_shadowMapShader = utils::makeShader("DirShadowVert.glsl", "DirShadowFrag.glsl");
	m_shadowPointShader = utils::makeShader("PointShadowDepthVert.glsl", "PointShadowDepthFrag.glsl", "PointShadowDepthGeo.glsl");

	glGenFramebuffers(1, &m_depthCubeFBO); // Generate and bind fb, use the colorbuffer
	//constructPointLight();

	glGenFramebuffers(1, &m_depthDirFBO);
	constructDirectionalLight();
}

ShadowRendering::~ShadowRendering() {
	// Directional shadow objects
	utils::deleteFBO(m_depthDirFBO);
	utils::deleteTexture(m_depthDirBuffer);
	utils::deleteObject(m_shadowMapShader);

	// Point shadow objects
	utils::deleteFBO(m_depthCubeFBO);
	for (GLuint& cubemap : m_depthCubeBuffers)
		utils::deleteTexture(cubemap);
	utils::deleteObject(m_shadowPointShader);
}

void ShadowRendering::constructDirectionalLight() {
	// Create camera's depth buffer
	glGenTextures(1, &m_depthDirBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthDirBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowWidth, m_shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); // Set the border color 1.0f, should return a shadow value of 0.0f

	glBindFramebuffer(GL_FRAMEBUFFER, m_depthDirFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthDirBuffer, 0);
	glDrawBuffer(GL_NONE); // "Explicitly tell OpenGL this framebuffer object does not render to a color buffer". Only the depth values are important when generating a depth cubemap
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowRendering::constructPointLight() {
	GLuint newCubemap;
	glGenTextures(1, &newCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, newCubemap);
	for (unsigned int i = 0; i < 6; ++i) // (1024 * 1024) * 6... big
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			m_shadowWidth, m_shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthCubeFBO); // Moved the fbTexture attachment render loop due to the need of looping through buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_depthCubeBuffers.push_back(newCubemap);
}

void ShadowRendering::renderPointLightShadows(const std::vector<Model*>& models, const std::vector<FileLights>& lightVec) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthCubeFBO);

	for (int i = 0; i < lightVec.size(); i++) {
		glm::vec3 lightPos = lightVec[i].pos;
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
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthCubeBuffers[i], 0); // Attach the targeted depth buffer of a light!!
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Point shadow FBO incomplete!" << std::endl;
		}
		glClear(GL_DEPTH_BUFFER_BIT);

		m_shadowPointShader->bind();
		for (unsigned int i = 0; i < 6; ++i)
			m_shadowPointShader->setUniform("shadowMatrices[" + std::to_string(i) + "]", captureVP[i]);

		m_shadowPointShader->setUniform("far_plane", far_plane);
		m_shadowPointShader->setUniform("lightPos", lightPos);

		glCullFace(GL_FRONT); // Render the scene's meshes		
		if (!models.empty()) {
			for (Model* model : models)
				for (Mesh* mesh : model->getMeshes()) {
					mesh->renderMeshOnly(m_shadowPointShader);
				}
		}	
		glCullFace(GL_BACK);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowRendering::renderShadowMapping(const std::vector<Model*>& models, const glm::vec3& lightPos) {
	glViewport(0, 0, m_shadowWidth, m_shadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, getCameraDepthFBO());
	glClear(GL_DEPTH_BUFFER_BIT);

	// Shader and Matrices:
	// Light POV, matrices
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f); // near plane = 1.0f, far plane = 7.5f. Let's try 0.1f, 100.0f
	glm::mat4 lightView = glm::lookAt(lightPos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	m_lightSpaceMatrix = lightProjection * lightView;

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

void ShadowRendering::updatePointLights(const std::vector<FileLights>& lightVec) {
	if (!m_depthCubeBuffers.empty()) {
		for (auto& buff : m_depthCubeBuffers)
			utils::deleteTexture(buff);
		m_depthCubeBuffers.clear();
	}		

	for (int i = 0; i < lightVec.size(); i++) {
		constructPointLight();
	}
}

void ShadowRendering::clearShadowMapping() {
	glBindFramebuffer(GL_FRAMEBUFFER, getCameraDepthFBO());
	glClear(GL_DEPTH_BUFFER_BIT);
	m_lightSpaceMatrix = glm::mat4(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}