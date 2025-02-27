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
	GLuint	captureFBO, captureRBO;
	GLuint	m_cubemapTexture, m_HDRTexture, m_irradianceMapTexture, m_prefilterTexture, m_brdfTexture;
	GLuint	m_texture;

	Texture* m_backgroundTexture;

	Shader* m_cubemapShader;		// Pointer to the Shader object
	Shader* m_BackgroundShader;		// Pointer to the Shader object
	Shader* m_IrradianceShader;
	Shader* m_Prefilter;
	Shader* m_brdf;

	Mesh* m_renderCube;
};