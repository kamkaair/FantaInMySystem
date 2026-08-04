#pragma once
#include "gameobject.h"     // Include base class
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad

class Shader;

class GBuffer : public kgfw::Object {
public:
	GBuffer(int inWidth, int inHeight);
	~GBuffer();

	// TODO: make a master creation method, there's way too much useless repetition
	GLuint createGPosition();
	GLuint createGAlbedo();
	GLuint createGNormal();
	GLuint createDepthBuffer();
	GLuint createGMetallicRoughness();
	GLuint createGEmission();

	void createLightPassBuffer();

	//GLuint createDiffuse();
	//GLuint createSpecular();
	//GLuint createIndirectDiffuse();
	//GLuint createIndirectSpecular();

	//GLuint getDiffuse() { return m_lightDiff; }
	//GLuint getSpecular() { return m_lightingSpec; }
	//GLuint getIndirectDiffuse() { return m_lightingIndirectDiff; }
	//GLuint getIndirectSpecular() { return m_lightingIndirectSpec; }

	void createTransparentPass();
	void createSkyBoxPass();

	GLuint createColorBuffer(int colorType, int colorChannels, int texDataType = GL_FLOAT, int colorAttachment = GL_COLOR_ATTACHMENT0);

	GLuint getGPosition() { return gPosition; }
	GLuint getGAlbedo() { return gAlbedo; }
	GLuint getGNormal() { return gNormal; }
	GLuint getGBuffer() { return gBuffer; }
	GLuint getGMetallicRoughness() { return gMetalRough; }
	GLuint getGEmission() { return gEmission; }
	GLuint getGDepth() { return gDepthTexture; }

	GLuint getLightingFBO() { return lightFBO; }
	GLuint getLightPassBuffer() { return m_LightPassTexture; }

	GLuint getTransFBO() { return m_transFBO; }
	GLuint getTransBuffer() { return m_transBuffer; }

	GLuint getSkyBoxBuffer() { return m_skyBoxTexture; }
	GLuint getSkyBoxFBO() { return m_skyBoxFBO; }

	float getWidth() const { return width; }
	float getHeight() const { return height; }

	void updateResolution();
	void setResolution(int inWidth, int inHeight) { width = inWidth; height = inHeight; }
	void CleanUpGBuffer();
	void constructGBuffer();

	void constructDeferredShaders();
	void constructForwardShaders();

	void deconstructDeferredShaders();
	void deconstructForwardShaders();

	Shader* getForwardShader() { return m_shader; }
	Shader* getLightPass() { return m_lightPass; } // TODO: Rename to hint the returning object to be a shader!!
	Shader* getGeometryPass() { return m_geometryPass; }
	Shader* getCurrentShader() { return m_currentShader; }
	Shader* getSkyBoxShader() { return m_skyBoxPass; }
	Shader* getCompositeShader() { return m_compositePass; }

	void setCurrentShader(Shader* inShader) { m_currentShader = inShader; } // Used for setting (and the other for getting) lighting shaders in the UI

private:
	int width, height;

	Shader* m_shader = 0; // Forward rendering lighting pass

	// Deferred rendering lighting passes
	Shader* m_lightPass = 0; // Not used: separated into "direct" and "indirect" shaders
	Shader* m_directPass = 0;
	Shader* m_indirectPass = 0;
	Shader* m_skyBoxPass = 0;
	Shader* m_compositePass = 0;

	Shader* m_geometryPass = 0;
	Shader* m_currentShader = 0;

	// FBOs
	GLuint gBuffer = 0, ssaoFBO = 0, lightFBO = 0, m_transFBO = 0, m_skyBoxFBO = 0;

	// Textures
	GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gMetalRough = 0, gEmission = 0, gDepthTexture = 0, m_transBuffer = 0, m_geometryMask = 0;
	GLuint m_LightPassTexture = 0; //m_lightDiff = 0, m_lightingSpec = 0, m_lightingIndirectDiff = 0, m_lightingIndirectSpec = 0;
	GLuint m_skyBoxTexture = 0;
};