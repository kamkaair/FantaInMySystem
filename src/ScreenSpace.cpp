#include "ScreenSpace.h"

ScreenSpace::ScreenSpace(GBuffer* gbuffer)
	: m_GBuffer(gbuffer), Object(__FUNCTION__) {
	setupSSAO(); //constructSSAO(); constructed, when switching to the deferred rendering
}

ScreenSpace::~ScreenSpace() {
	deconstructSSAO();
	deconstructSSR();
	if (ssaoBlurFBO != 0) { glDeleteFramebuffers(1, &ssaoBlurFBO); ssaoBlurFBO = 0; }
	if (ssaoColorBufferBlur != 0) { glDeleteTextures(1, &ssaoColorBufferBlur); ssaoColorBufferBlur = 0; }

	if (ssaoFBO != 0) { glDeleteFramebuffers(1, &ssaoFBO); ssaoFBO = 0; }
	if (noiseTexture != 0) { glDeleteTextures(1, &noiseTexture); noiseTexture = 0; }
	if (ssaoColorBuffer != 0) { glDeleteTextures(1, &ssaoColorBuffer); ssaoColorBuffer = 0; }
	for (int i = 0; i < ssaoKernel.size(); i++) {
		ssaoKernel[i] = glm::vec3{ 0.0f };
	}

	if (m_gaussianBlur != 0) { utils::deleteObject(m_gaussianBlur); } // Deconstruct gaussian
	clearPingPongBuffer();
}

void ScreenSpace::deconstructSSAO() {
	if (m_SSAO != 0) { utils::deleteObject(m_SSAO); }
	if (m_blurSSAO != 0) { utils::deleteObject(m_blurSSAO); }
}

void ScreenSpace::deconstructSSR() {
	if (m_SSR != 0) { utils::deleteObject(m_SSR); }
	//if (m_blurSSR != 0) { utils::deleteObject(m_blurSSR); }
	if (m_SSR_TA != 0) { utils::deleteObject(m_SSR_TA); }
}

void ScreenSpace::clearPingPongBuffer() {
	if ((pingpongBuffer[0] != 0) && (pingpongBuffer[1] != 0)) {
		for (int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glDeleteTextures(1, &pingpongBuffer[i]);
			pingpongBuffer[i] = 0;
		}
	}
}

void ScreenSpace::constructSSAO() {
	// Load SSAO shaders
	if (m_SSAO == 0)
		m_SSAO = utils::makeShader("QuadVert.glsl", "SSAO-Frag.glsl");

	if (m_blurSSAO == 0)
		m_blurSSAO = utils::makeShader("QuadVert.glsl", "blurSSAO-Frag.glsl");
}

void ScreenSpace::constructSSR() {
	// Load SSAO shaders
	if (m_SSR == 0)
		m_SSR = utils::makeShader("QuadVert.glsl", "SSR-Frag.glsl");

	//if (m_blurSSR == 0)
		//m_blurSSR = utils::makeShader("QuadVert.glsl", "blurSSR-Frag.glsl");

	if (m_SSR_TA == 0)
		m_SSR_TA = utils::makeShader("QuadVert.glsl", "SSR-TAF.glsl");
}

void ScreenSpace::constructDeferredRendering() {
	glUseProgram(0); // Unbind any active shader
	glDisable(GL_BLEND);
	m_GBuffer->constructDeferredShaders();
	//m_GBuffer->deconstructForwardShaders();
	constructSSAO();
	constructSSR();
	
	if (m_gaussianBlur == 0)
		m_gaussianBlur = utils::makeShader("QuadVert.glsl", "GaussianBlurFrag.glsl");
}

void ScreenSpace::constructForwardRendering() {
	glUseProgram(0); // Unbind any active shader
	glEnable(GL_BLEND);

	deconstructSSAO();
	deconstructSSR();
	if (m_gaussianBlur != 0) { utils::deleteObject(m_gaussianBlur); } // Deconstruct gaussian

	m_GBuffer->setCurrentShader(m_GBuffer->getForwardShader());
	m_GBuffer->deconstructDeferredShaders();
}

void ScreenSpace::setupSSAO() {
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;

	// SSAO texture framebuffer
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	//ssaoColorBuffer = createSsaoColorBuffer();
	ssaoColorBuffer = m_GBuffer->createBuffer(GL_R16F, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);

	// SSR Color Buffer
	glGenFramebuffers(1, &ssrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	//ssrColorBuffer = createSsrSceneColorBuffer();
	ssrColorBuffer = m_GBuffer->createBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);

	// SSR Temporal Accumulation FBO
	createSSR_HistoryFramebuffer();
	createTemporalBuffers();

	//// SSR Blur framebuffer
	//glGenFramebuffers(1, &ssrBlurFBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, ssrBlurFBO);
	//ssrColorBufferBlur = createSsrSceneColorBufferBlur();

	// Blur framebuffer
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	//ssaoColorBufferBlur = createSsaoColorBufferBlur();
	ssaoColorBufferBlur = m_GBuffer->createBuffer(GL_R16F, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Gaussian blur
	createPingPongBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ssaoKernel = createSampleKernel(randomFloats, generator);
	noiseTexture = createNoiseTexture(randomFloats, generator);
}

void ScreenSpace::renderSSAO(Camera* m_camera, Mesh* m_meshRender, int samples) {
	updateSSAOUniforms();

	// SSAO texture
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
		
	m_SSAO->bind();
	utils::bindTexture(GL_TEXTURE0, m_SSAO, noiseTexture, "texNoise");
	utils::bindTexture(GL_TEXTURE1, m_SSAO, m_GBuffer->getGPosition(), "gPosition");
	utils::bindTexture(GL_TEXTURE2, m_SSAO, m_GBuffer->getGNormal(), "gNormal");

	// Send kernel + rotation 
	for (unsigned int i = 0; i < samples; ++i)
		m_SSAO->setUniform("samples[" + std::to_string(i) + "]", ssaoKernel[i]);

	m_SSAO->setUniform("projection", m_camera->getProjectionMatrix());

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Apply the blur to the SSAO texture
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	//glClear(GL_COLOR_BUFFER_BIT);
	m_blurSSAO->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenSpace::renderSSR(Camera* m_camera, Mesh* m_meshRender) {
	updateSSRUniforms();

	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	m_SSR->bind();

	utils::bindTexture(GL_TEXTURE0, m_SSR, m_GBuffer->getGNormal(), "gNormal");
	utils::bindTexture(GL_TEXTURE1, m_SSR, m_GBuffer->getLightIndirectSpec(), "colorBuffer");
	utils::bindTexture(GL_TEXTURE2, m_SSR, m_GBuffer->getGDepth(), "depthMap");
	utils::bindTexture(GL_TEXTURE3, m_SSR, m_GBuffer->getGMetallicRoughness(), "gMetallicRoughness");

	
	m_SSR->setUniform("SCR_WIDTH", float(m_GBuffer->getWidth()));
	m_SSR->setUniform("SCR_HEIGHT", float(m_GBuffer->getHeight()));

	m_SSR->setUniform("invProjection", glm::inverse(m_camera->getProjectionMatrix()));
	m_SSR->setUniform("projection", m_camera->getProjectionMatrix());

	m_SSR->setUniform("frameIndex", frameIndex);

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Temporal Accumulation
	if(m_ssrSettings.useTA) {
		glBindFramebuffer(GL_FRAMEBUFFER, getSSRHistoryWriteFBO());
		m_SSR_TA->bind();

		renderSSR_TA(m_camera, m_meshRender);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// BLUR
	////Apply the blur to the SSAO texture - SSAO blur technique
	//glBindFramebuffer(GL_FRAMEBUFFER, ssrBlurFBO);
	////glClear(GL_COLOR_BUFFER_BIT);
	//m_blurSSR->bind();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);
	//m_blurSSR->setUniform("colorTexture", 0);

	//m_meshRender->renderQuad();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenSpace::renderSSR_TA(Camera* m_camera, Mesh* m_meshRender) {

	// SSR Temporal Accumulation
	utils::bindTexture(GL_TEXTURE0, m_SSR_TA, ssrColorBuffer, "uSSRCurrent");
	utils::bindTexture(GL_TEXTURE1, m_SSR_TA, getSSRHistoryRead(), "uSSRHistory");
	utils::bindTexture(GL_TEXTURE2, m_SSR_TA, m_GBuffer->getGDepth(), "depthMap");
	utils::bindTexture(GL_TEXTURE3, m_SSR_TA, m_GBuffer->getGNormal(), "gNormal");
	utils::bindTexture(GL_TEXTURE4, m_SSR_TA, prevDepthTex, "uPrevDepth");
	utils::bindTexture(GL_TEXTURE5, m_SSR_TA, prevNormalTex, "uPrevNormal");

	m_SSR_TA->setUniform("near", m_camera->getNear());
	m_SSR_TA->setUniform("far", m_camera->getFar());

	glm::mat4 currView = m_camera->getViewMatrix();
	glm::mat4 currProj = m_camera->getProjectionMatrix();

	if (firstFrame)
	{
		prevView = currView;
		prevProj = currProj;
		firstFrame = false;
	}

	m_SSR_TA->setUniform("invProjection", glm::inverse(m_camera->getProjectionMatrix()));
	m_SSR_TA->setUniform("prevProjection", prevProj);

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// SSR History - Copying
	swapSSRHistory();

	// Copy depth to prevDepth
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->getGBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDepthFBO);

	glBlitFramebuffer(0, 0, m_GBuffer->getWidth(), m_GBuffer->getHeight(), 0, 0, m_GBuffer->getWidth(), m_GBuffer->getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	// Copy normal
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->getGBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevNormalFBO);

	glReadBuffer(GL_COLOR_ATTACHMENT1); // gNormal
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBlitFramebuffer(0, 0, m_GBuffer->getWidth(), m_GBuffer->getHeight(), 0, 0, m_GBuffer->getWidth(), m_GBuffer->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

	prevView = currView;
	prevProj = currProj;
	frameIndex++; // frame index used for the random
}

void ScreenSpace::renderBloom(Mesh* m_meshRender, const GLuint& inLightTexture) {
	static bool first_iteration = true;
	first_iteration = true;
	horizontal = true;

	updateBloomUniforms();

	m_gaussianBlur->bind();
	for (unsigned int i = 0; i < m_bloomSettings.amount; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? inLightTexture : pingpongBuffer[!horizontal]);
		m_gaussianBlur->setUniform("horizontal", horizontal);

		m_meshRender->renderQuad();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenSpace::renderCompositeShader(Mesh* m_meshRender) {
	// SSR Composite - Final Image
	glDisable(GL_DEPTH_TEST); // Disable depth test!

	m_GBuffer->getCompositeShader()->bind();
	utils::bindTexture(GL_TEXTURE0, m_GBuffer->getCompositeShader(), m_GBuffer->getLightPassBuffer(), "uLightPassTex");
	utils::bindTexture(GL_TEXTURE1, m_GBuffer->getCompositeShader(), m_GBuffer->getLightIndirectDiff(), "uIndirectDiff");
	utils::bindTexture(GL_TEXTURE2, m_GBuffer->getCompositeShader(), m_GBuffer->getLightIndirectSpec(), "uIndirectSpec");
	utils::bindTexture(GL_TEXTURE3, m_GBuffer->getCompositeShader(), m_GBuffer->getGEmission(), "uEmission");
	utils::bindTexture(GL_TEXTURE4, m_GBuffer->getCompositeShader(), getSSR_History(), "uSSR");
	utils::bindTexture(GL_TEXTURE5, m_GBuffer->getCompositeShader(), pingpongBuffer[!horizontal], "uBloom");

	m_meshRender->renderQuad();
	glEnable(GL_DEPTH_TEST); // Enable!
}

void ScreenSpace::updateSSAOUniforms() {
	if (!m_ssaoSettings.dirty)
		return;

	m_SSAO->bind();
	m_SSAO->setUniform("kernelSize", m_ssaoSettings.kernelSize);
	m_SSAO->setUniform("radius", m_ssaoSettings.radius);
	m_SSAO->setUniform("bias", m_ssaoSettings.bias);

	m_GBuffer->getLightPass()->bind();
	m_GBuffer->getLightPass()->setUniform("aoTone", m_ssaoSettings.clampedMidTones);
	m_GBuffer->getLightPass()->setUniform("useSSAO", m_ssaoSettings.useSSAO);
	m_GBuffer->getLightPass()->setUniform("aoStrength", m_ssaoSettings.occlusionStrength);

	m_ssaoSettings.dirty = false;
}

void ScreenSpace::updateSSRUniforms() {
	if (!m_ssrSettings.useTA) {
		resetTA_SSR();
		return;
	}

	if (!m_ssrSettings.dirty)
		return;

	m_SSR->bind();
	m_SSR->setUniform("maxSteps", m_ssrSettings.maxSteps);
	m_SSR->setUniform("thickness", m_ssrSettings.thickness);
	m_SSR->setUniform("rayDirMin", m_ssrSettings.rayDirMin);
	m_SSR->setUniform("useBinaryRefinement", m_ssrSettings.useBinaryRefinement);
	m_SSR->setUniform("useRoughnessScatterSSR", m_ssrSettings.useRayScattering);

	m_ssrSettings.dirty = false;
}

void ScreenSpace::updateBloomUniforms() {
	if (!m_bloomSettings.dirty)
		return;

	m_gaussianBlur->bind();
	m_gaussianBlur->setUniform("dist", m_bloomSettings.distance);
}

float SSAOLerp(float a, float b, float f)
{
	return a + f * (b - a);
}

std::vector<glm::vec3> ScreenSpace::createSampleKernel(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator) {
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0f;

		// scale samples s.t. they're more aligned to center of kernel
		scale = SSAOLerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
		//std::cout << glm::to_string(sample) << " also: " << i << std::endl;
	}
	return ssaoKernel;
}

GLuint ScreenSpace::createNoiseTexture(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator) {
	std::vector<glm::vec3> ssaoNoise;

	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return noiseTexture;
}

void ScreenSpace::createPingPongBuffer() {
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, m_GBuffer->getWidth(), m_GBuffer->getHeight(), 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
		);
	}
}

void ScreenSpace::createSSR_HistoryFramebuffer() {
	glGenFramebuffers(2, ssrHistoryFBO);
	glGenTextures(2, ssrTemporalBuffer);

	for (int i = 0; i < 2; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);

		glBindTexture(GL_TEXTURE_2D, ssrTemporalBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_GBuffer->getWidth(), m_GBuffer->getHeight(), 0, GL_RGBA, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrTemporalBuffer[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSR history FBO incomplete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenSpace::createTemporalBuffers()
{
	// prevDepth
	glGenTextures(1, &prevDepthTex);
	glBindTexture(GL_TEXTURE_2D, prevDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_GBuffer->getWidth(), m_GBuffer->getHeight(), 0,GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &prevDepthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, prevDepthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, prevDepthTex, 0);

	// prevNormal
	glGenTextures(1, &prevNormalTex);
	glBindTexture(GL_TEXTURE_2D, prevNormalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_GBuffer->getWidth(), m_GBuffer->getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &prevNormalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, prevNormalFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prevNormalTex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenSpace::recreateColorBuffer() {
	// Reconstruct G-Buffer
	m_GBuffer->updateResolution();

	// Reconstruct screen space color buffers
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	if (ssaoColorBuffer != 0) { glDeleteTextures(1, &ssaoColorBuffer); ssaoColorBuffer = 0; }
	ssaoColorBuffer = m_GBuffer->createBuffer(GL_R16F, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	if (ssaoColorBufferBlur != 0) { glDeleteTextures(1, &ssaoColorBufferBlur); ssaoColorBufferBlur = 0; }
	ssaoColorBufferBlur = m_GBuffer->createBuffer(GL_R16F, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	if (ssrColorBuffer != 0) { glDeleteTextures(1, &ssrColorBuffer); ssrColorBuffer = 0; }
	ssrColorBuffer = m_GBuffer->createBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	clearPingPongBuffer();
	createPingPongBuffer();

	if ((ssrTemporalBuffer[0] != 0) && (ssrTemporalBuffer[1] != 0)) { // Probably could just reuse clearPingPong for this one
		for (int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);
			glDeleteTextures(1, &ssrTemporalBuffer[i]);
			ssrTemporalBuffer[i] = 0;
		}
	}
	createSSR_HistoryFramebuffer();
	createTemporalBuffers();

	m_SSAO->setUniform("noiseScale", glm::vec2(m_GBuffer->getWidth() / 4.0f, m_GBuffer->getHeight() / 4.0f)); // new noiseScale resolution

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenSpace::resetTA_SSR() {
	ssrHistoryIndex = 0;
	frameIndex = 0;
	firstFrame = true;

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// Clear depth
	glBindFramebuffer(GL_FRAMEBUFFER, prevDepthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Clear normal
	glBindFramebuffer(GL_FRAMEBUFFER, prevNormalFBO);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/*if ((ssrTemporalBuffer[0] != 0) && (ssrTemporalBuffer[1] != 0)) {
		for (int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);
			glDeleteTextures(1, &ssrTemporalBuffer[i]);
			ssrTemporalBuffer[i] = 0;
		}
	}*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}