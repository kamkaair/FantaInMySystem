#include "GBuffer.h"
#include "utils.h"

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
		std::cout << "Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint GBuffer::createGPosition() {
	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	return gNormal;
}

GLuint GBuffer::createGAlbedo() {
	// Albedo color buffer (currently optional for SSAO, but useful for deferred in general)
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	return gAlbedo;
}

GLuint GBuffer::createGMetallicRoughness() {
	// Albedo color buffer (currently optional for SSAO, but useful for deferred in general)
	glGenTextures(1, &gMetalRough);
	glBindTexture(GL_TEXTURE_2D, gMetalRough);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMetalRough, 0);

	return gMetalRough;
}

GLuint GBuffer::createDepthBuffer() {
	// Depth buffer
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	return rboDepth;
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
}

void GBuffer::deconstructForwardShaders() {
	if (m_shader != 0) { utils::deleteObject(m_shader); }
}