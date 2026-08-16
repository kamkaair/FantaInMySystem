#include "GBuffer.h"
#include "utils.h"
#include <iostream>

GBuffer::GBuffer(int inWidth, int inHeight) : width(inWidth), height(inHeight), Object(__FUNCTION__) {
	constructForwardShaders(); constructGBuffer();
}

GBuffer::~GBuffer() {
	CleanUpGBuffer();
	deconstructForwardShaders();
	deconstructDeferredShaders();
}

void GBuffer::constructGBuffer() {
	// GBuffer textures and depth
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	gPosition = createBuffer(GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);
	gNormal = createBuffer(GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT1);
	gAlbedo = createBuffer(GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2);
	gMetalRough = createBuffer(GL_RG16F, GL_RG, GL_FLOAT, GL_COLOR_ATTACHMENT3);
	gEmission = createBuffer(GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT4);

	GLuint attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);

	//gDepthTexture = createDepthBuffer(); // The depth buffer is also a texture
	gDepthTexture = createBuffer(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer GBUFFER not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ----------------------------------

	// Lighting pass textures and same depth as before
	glGenFramebuffers(1, &lightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);

	m_LightPassTexture = createBuffer(GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);
	m_LightIndirectDiff = createBuffer(GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT1);
	m_LightIndirectSpec = createBuffer(GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT2);

	GLuint lightAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, lightAttachments);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthTexture, 0);

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer LIGHT PASS textures are not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ----------------------------------
	
	// Lighting pass textures and same depth as before
	glGenFramebuffers(1, &m_compositeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_compositeFBO);

	m_CompositeTexture = createBuffer(GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthTexture, 0);

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer LIGHT PASS textures are not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint GBuffer::createBuffer(int colorType, int colorChannels, int texDataType, int colorAttachment) {
	// position color buffer
	GLuint newColorBuffer;

	glGenTextures(1, &newColorBuffer);
	glBindTexture(GL_TEXTURE_2D, newColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, colorType, width, height, 0, colorChannels, texDataType, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, newColorBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	return newColorBuffer;
}

void GBuffer::CleanUpGBuffer() {
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// GBuffer
	utils::deleteFBO(gBuffer);
	
	utils::deleteTexture(gPosition);
	utils::deleteTexture(gNormal);
	utils::deleteTexture(gAlbedo);
	utils::deleteTexture(gEmission);
	utils::deleteTexture(gMetalRough);
	utils::deleteTexture(gDepthTexture);

	// Light pass
	utils::deleteFBO(lightFBO);

	utils::deleteTexture(m_LightPassTexture);
	utils::deleteTexture(m_LightIndirectDiff);
	utils::deleteTexture(m_LightIndirectSpec);

	// Composite pass
	utils::deleteFBO(m_compositeFBO);

	utils::deleteTexture(m_CompositeTexture);
}

void GBuffer::updateResolution() {
	// Reconstruct GBuffer
	CleanUpGBuffer();
	constructGBuffer();
}

void GBuffer::constructDeferredShaders() {
	if (m_geometryPass == 0)
		m_geometryPass = utils::makeShader("GeometryPassVert.glsl", "GeometryPassFrag.glsl");
	
	if (m_lightPass == 0) {
		setCurrentShader(0);
		m_lightPass = utils::makeShader("QuadVert.glsl", "DeferredLightFrag.glsl");
		setCurrentShader(m_lightPass);
	}
	
	if (m_compositePass == 0)
		m_compositePass = utils::makeShader("QuadVert.glsl", "CompositeFrag.glsl");
}

void GBuffer::constructForwardShaders() {
	if (m_shader == 0) {
		setCurrentShader(0);
		m_shader = utils::makeShader("vertShader.glsl", "fragShader.glsl"); 
		setCurrentShader(m_shader);
	}
}

void GBuffer::deconstructDeferredShaders() {
	if (m_geometryPass != 0) { utils::deleteObject(m_geometryPass); }
	if (m_lightPass != 0) { utils::deleteObject(m_lightPass); }
	if (m_compositePass != 0) { utils::deleteObject(m_compositePass); }
}

void GBuffer::deconstructForwardShaders() {
	if (m_shader != 0) { utils::deleteObject(m_shader); }
}