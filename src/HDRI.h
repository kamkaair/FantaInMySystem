#pragma once

#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <glm/glm.hpp>      // Include glm
#include "glm/gtx/string_cast.hpp" // Include for printing mats and vecs
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>
#include <iostream>
#include "shader.h"
#include "mesh.h"
#include "camera.h"

class HDRI : public kgfw::Object
{
public:
	HDRI(Shader* cubemap, Shader* background, Shader* irradiance, Shader* prefilter, Shader* brdf);
	~HDRI();

	GLuint createCubemapFromHDR(GLuint hdrTexture);
	GLuint createIrradianceMap(GLuint cubemap);
	GLuint createPrefilterMap(GLuint cubemap);
	GLuint createBRDF();
	void ProcessHDRI(const char* hdr);
	void cleanUpHDRI();
	static GLuint loadHDRTexture(const char* path);
	void renderSkybox(Camera* m_camera);
	void renderBackgroundImage(Camera* m_camera, Texture* backgroundImage, Shader* m_backImage);
	void setHDRITextures(Shader* shader);
	void setBackgroundTexture(Texture* backgroundTexture) { m_backgroundTexture = backgroundTexture; }
	void cleanBackgroundTexture();

	GLuint getIrradiance(GLuint irradiance);
	GLuint getPrefilter(GLuint prefilter);
	GLuint getBRDF(GLuint BRDF);
	Texture* getBackgroundTexture() { return m_backgroundTexture; }


private:
	GLuint	captureFBO = 0, captureRBO = 0;
	GLuint	m_cubemapTexture = 0, m_HDRTexture = 0, m_irradianceMapTexture = 0, m_prefilterTexture = 0, m_brdfTexture = 0;
	GLuint	m_texture = 0;

	Texture* m_backgroundTexture = 0;

	Shader* m_cubemapShader = 0;		// Pointer to the Shader object
	Shader* m_BackgroundShader = 0;		// Pointer to the Shader object
	Shader* m_IrradianceShader = 0;
	Shader* m_Prefilter = 0;
	Shader* m_brdf = 0;

	Mesh* m_meshRender = 0;

	// Load the cubemap projection and views for the 6 directions
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[6] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
};