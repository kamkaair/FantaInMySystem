#include "GBuffer.h"
#include "utils.h"
#include <iostream>

GBuffer::GBuffer(int inWidth, int inHeight) : width(inWidth), height(inHeight), Object(__FUNCTION__) {
	constructForwardShaders(); constructGBuffer();
}

// TODO: seems that the cleanup doesn't delete everything (memory usage rises up after reconstructing G-Buffer)
GBuffer::~GBuffer() {
	CleanUpGBuffer();
	deconstructForwardShaders();
	deconstructDeferredShaders();
}

void GBuffer::constructGBuffer() {
	// This constructor sets up the G-Buffer
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	gPosition = createGPosition();
	gNormal = createGNormal();
	gAlbedo = createGAlbedo();
	gMetalRough = createGMetallicRoughness();

	GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	rboDepth = createDepthBuffer();

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer GBuffers not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Lighting pass textures
	glGenFramebuffers(1, &lightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);

	m_lightDiff = createDiffuse();
	m_lightingSpec = createSpecular();
	m_lightingIndirectDiff = createIndirectDiffuse();
	m_lightingIndirectSpec = createIndirectSpecular();

	GLuint lightAttachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, lightAttachments);

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer lighting textures are not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint GBuffer::createGPosition() {
	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	return gPosition;
}

GLuint GBuffer::createGNormal() {
	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	return gNormal;
}

GLuint GBuffer::createGAlbedo() {
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	return gAlbedo;
}

GLuint GBuffer::createGMetallicRoughness() {
	glGenTextures(1, &gMetalRough);
	glBindTexture(GL_TEXTURE_2D, gMetalRough);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMetalRough, 0);

	return gMetalRough;
}

GLuint GBuffer::createDepthBuffer() {
	glGenTextures(1, &rboDepth);
	glBindTexture(GL_TEXTURE_2D, rboDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, rboDepth, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	return rboDepth;
}

GLuint GBuffer::createDiffuse() {
	glGenTextures(1, &m_lightDiff);
	glBindTexture(GL_TEXTURE_2D, m_lightDiff);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lightDiff, 0);

	return m_lightDiff;
}

GLuint GBuffer::createSpecular() {
	glGenTextures(1, &m_lightingSpec);
	glBindTexture(GL_TEXTURE_2D, m_lightingSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_lightingSpec, 0);

	return m_lightingSpec;
}

GLuint GBuffer::createIndirectDiffuse() {
	glGenTextures(1, &m_lightingIndirectDiff);
	glBindTexture(GL_TEXTURE_2D, m_lightingIndirectDiff);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_lightingIndirectDiff, 0);

	return m_lightingIndirectDiff;
}

GLuint GBuffer::createIndirectSpecular() {
	glGenTextures(1, &m_lightingIndirectSpec);
	glBindTexture(GL_TEXTURE_2D, m_lightingIndirectSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_lightingIndirectSpec, 0);

	return m_lightingIndirectSpec;
}

void GBuffer::CleanUpGBuffer() {
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (gBuffer != 0) { glDeleteFramebuffers(1, &gBuffer); gBuffer = 0; }
	if (gPosition != 0) { glDeleteTextures(1, &gPosition); gPosition = 0; }
	if (gNormal != 0) { glDeleteTextures(1, &gNormal); gNormal = 0; }
	if (gAlbedo != 0) { glDeleteTextures(1, &gAlbedo); gAlbedo = 0; }
	if (gMetalRough != 0) { glDeleteTextures(1, &gMetalRough); gMetalRough = 0; }
	if (rboDepth != 0) { glDeleteRenderbuffers(1, &rboDepth); rboDepth = 0; }

	if (m_lightDiff != 0) { glDeleteTextures(1, &m_lightDiff); m_lightDiff = 0; }
	if (m_lightingSpec != 0) { glDeleteTextures(1, &m_lightingSpec); m_lightingSpec = 0; }
	if (m_lightingIndirectDiff != 0) { glDeleteTextures(1, &m_lightingIndirectDiff); m_lightingIndirectDiff = 0; }
	if (m_lightingIndirectSpec != 0) { glDeleteTextures(1, &m_lightingIndirectSpec); m_lightingIndirectSpec = 0; }
}

void GBuffer::updateResolution() {
	// Reconstruct GBuffer
	CleanUpGBuffer();
	constructGBuffer();
}


void GBuffer::constructDeferredShaders() {
	if (m_geometryPass == 0)
		m_geometryPass = utils::makeShader("GeometryPassVert.glsl", "GeometryPassFrag.glsl");
	
	if (m_lightPass == 0)
	{
		setCurrentShader(0);
		m_lightPass = utils::makeShader("DeferredLightVert.glsl", "DeferredLightFrag.glsl");
		setCurrentShader(m_lightPass);
	}
	
	if (m_compositePass == 0)
		m_compositePass = utils::makeShader("SSAO-Vert.glsl", "CompositeFrag.glsl");
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