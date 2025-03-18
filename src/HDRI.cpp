#include "HDRI.h"

HDRI::HDRI(Shader* cubemap, Shader* background, Shader* irradiance, Shader* prefilter, Shader* brdf) : 
	m_cubemapShader(cubemap), m_BackgroundShader(background), m_IrradianceShader(irradiance), m_Prefilter(prefilter), m_brdf(brdf), Object(__FUNCTION__) {
	// Set up framebuffer to render cubemap faces
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//ProcessHDRI(hdr);
}
HDRI::~HDRI() {
	delete m_cubemapShader;
	m_cubemapShader = 0;

	delete m_BackgroundShader;
	m_BackgroundShader = 0;

	delete m_Prefilter;
	m_Prefilter = 0;

	delete m_IrradianceShader;
	m_IrradianceShader = 0;

	delete m_brdf;
	m_brdf = 0;

	delete m_backgroundTexture;
	m_backgroundTexture = 0;
}

GLuint HDRI::getIrradiance(GLuint irradiance) {
	return m_irradianceMapTexture = irradiance;
}

GLuint HDRI::getPrefilter(GLuint prefilter) {
	return m_prefilterTexture = prefilter;
}

GLuint HDRI::getBRDF(GLuint BRDF) {
	return m_brdfTexture = BRDF;
}

GLuint HDRI::loadHDRTexture(const char* path) {
	// Load HDR image using stb_image
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
	GLuint hdrTexture = 0;

	if (data) {
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cerr << "Failed to load HDR image." << std::endl;
		stbi_image_free(data);
	}

	return hdrTexture;
}

GLuint HDRI::createCubemapFromHDR(GLuint hdrTexture) {
	// Create an empty cubemap texture
	GLuint cubemap;
	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Bind the cubemap shader and set up uniforms
	m_cubemapShader->bind();
	m_cubemapShader->setUniform("equirectangularMap", 0); // Set texture unit 0 to HDR texture
	m_cubemapShader->setUniform("projection", captureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);  // Bind HDR texture

	// Set the viewport to the resolution of the cubemap faces
	// Render each cubemap face
	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		m_cubemapShader->setUniform("view", captureViews[i]);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render a cube that will use the shader and project the HDR texture to the cubemap face
		m_meshRender->renderCube();
	}

	// Cleanup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return cubemap;
}

GLuint HDRI::createIrradianceMap(GLuint cubemap) {
	// Create an empty irradiance cubemap texture

	GLuint irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set up framebuffer and renderbuffer for rendering to cubemap faces
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	m_IrradianceShader->bind();
	m_IrradianceShader->setUniform("projection", captureProjection);
	m_IrradianceShader->setUniform("environmentMap", 0); // environment cubemap on texture unit 0

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	// Render each face of the irradiance map
	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		m_IrradianceShader->setUniform("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_meshRender->renderCube(); // Render cube to generate irradiance map
	}

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return irradianceMap;
}

GLuint HDRI::createPrefilterMap(GLuint cubemap) {
	// Create an empty irradiance cubemap texture
	// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
// --------------------------------------------------------------------------------
	GLuint prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	m_Prefilter->bind();
	m_Prefilter->setUniform("environmentMap", 0);
	m_Prefilter->setUniform("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		m_Prefilter->setUniform("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			m_Prefilter->setUniform("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_meshRender->renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return prefilterMap;
}

GLuint HDRI::createBRDF()
{
	GLuint brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	m_brdf->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_meshRender->renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return brdfLUTTexture;
}

void HDRI::ProcessHDRI(const char* hdr)
{
	// Reverse blending settings
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO); // Optional: Reset to default blend function

	// We load the HDR texture
	//GLuint texture = loadHDRTexture((std::string(ASSET_DIR) + "/HDRI/newport_loft.hdr").c_str());
	GLuint texture = loadHDRTexture((std::string(ASSET_DIR) + hdr).c_str());

	// Store texture for later use...
	m_texture = texture;

	// Convert the HDR texture to a cubemap
	GLint cubemap = createCubemapFromHDR(m_texture);

	// Store cubemap texture ID for later use (e.g., rendering)
	m_cubemapTexture = cubemap;

	// Last thing with cubemap creation: let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Generate the irradiance map from the cubemap
	GLuint irradianceMap = createIrradianceMap(cubemap);
	m_irradianceMapTexture = irradianceMap;

	// Generate the prefilter map from the cubemap
	GLuint prefilterMap = createPrefilterMap(cubemap);
	m_prefilterTexture = prefilterMap;

	// Generate the brdf map from the cubemap
	GLuint brdfMap = createBRDF();
	m_brdfTexture = brdfMap;

	// Enable alpha blending once again
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void HDRI::cleanUpHDRI()
{
	// Delete the old HDR texture/maps
	if (m_texture != 0) {
		glDeleteTextures(1, &m_texture);
		m_texture = 0;
	}

	if (m_cubemapTexture != 0) {
		glDeleteTextures(1, &m_cubemapTexture);
		m_cubemapTexture = 0;
	}

	if (m_irradianceMapTexture != 0) {
		glDeleteTextures(1, &m_irradianceMapTexture);
		m_irradianceMapTexture = 0;
	}

	if (m_prefilterTexture != 0) {
		glDeleteTextures(1, &m_prefilterTexture);
		m_prefilterTexture = 0;
	}

	if (m_brdfTexture != 0) {
		glDeleteTextures(1, &m_brdfTexture);
		m_brdfTexture = 0;
	}
}

void HDRI::cleanBackgroundTexture() {
	if (m_backgroundTexture) {
		delete m_backgroundTexture;
		m_backgroundTexture = 0;
	}
}

void HDRI::setHDRITextures(Shader* shader) {
	shader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapTexture);
	shader->setUniform("irradianceMap", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterTexture);
	shader->setUniform("prefilterMap", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_brdfTexture);
	shader->setUniform("brdfLUT", 2);
}

void HDRI::renderSkybox(Camera* m_camera) {
	glDepthFunc(GL_LEQUAL);  // Ensure skybox is drawn in the background'
	glm::mat4 view = glm::mat4(glm::mat3(m_camera->getViewMatrix()));  // Remove translation from the view matrix
	//std::cout << "View Matrix: " << glm::to_string(view) << std::endl;
	glm::mat4 projection = m_camera->getProjectionMatrix();

	m_BackgroundShader->bind();  // Bind the cubemap shader
	m_BackgroundShader->setUniform("view", view);
	m_BackgroundShader->setUniform("projection", projection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture);

	// Render the cube
	m_meshRender->renderCube();
	glDepthFunc(GL_LESS);
}

void HDRI::renderBackgroundImage(Camera* m_camera, Texture* backgroundImage, Shader* m_backImage) {
	// Decided to use "clip-space quad" method to render the background image for the background image
	// Since the quad is already in clip space (gl_Position = vec4(in_position, 1.0), just disable depth testing
	glDisable(GL_DEPTH_TEST);

	m_backImage->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backgroundImage->getTextureId());

	m_meshRender->renderQuad();
	glEnable(GL_DEPTH_TEST);
}