#include "shadowRendering.h"
#include <glm/glm.hpp>      // Include glm
#include <glm/gtc/matrix_transform.hpp>      // Include matrix transforms
#include <glm/gtc/type_ptr.hpp>
#include <kgfw/GLUtils.h>

ShadowRendering::ShadowRendering() {
	m_shadowMapShader = utils::makeShader("ShadowMappingVert.glsl", "ShadowMappingFrag.glsl");
	checkGLError();
	constructShadowMapping();
	checkGLError();
}

ShadowRendering::~ShadowRendering() {
	if (m_shadowMapShader != 0) { utils::deleteObject(m_shadowMapShader); }
}

void ShadowRendering::constructShadowMapping() {
	// Create camera's depth buffer
	glGenFramebuffers(1, &m_cameraDepthFBO);

	glGenTextures(1, &m_cameraDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_cameraDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowWidth, m_shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); // Set the border color 1.0f, should return a shadow value of 0.0f
	checkGLError();

	glBindFramebuffer(GL_FRAMEBUFFER, m_cameraDepthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_cameraDepthBuffer, 0);
	glDrawBuffer(GL_NONE); // kinda odd, why do we do these?
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGLError();
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