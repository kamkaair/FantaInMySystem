#pragma once
#include "gameobject.h"     // Include base class
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad

class Shader;

class GBuffer : public kgfw::Object {
public:
	GBuffer(int inWidth, int inHeight);
	~GBuffer();

	GLuint createGPosition();
	GLuint createGAlbedo();
	GLuint createGNormal();
	GLuint createDepthBuffer();
	GLuint createGMetallicRoughness();

	GLuint createDiffuse();
	GLuint createSpecular();
	GLuint createIndirectDiffuse();
	GLuint createIndirectSpecular();

	GLuint getGPosition() { return gPosition; }
	GLuint getGAlbedo() { return gAlbedo; }
	GLuint getGNormal() { return gNormal; }
	GLuint getGBuffer() { return gBuffer; }
	GLuint getGMetallicRoughness() { return gMetalRough; }
	GLuint getGDepth() { return rboDepth; }

	GLuint getDiffuse() { return m_lightDiff; }
	GLuint getSpecular() { return m_lightingSpec; }
	GLuint getIndirectDiffuse() { return m_lightingIndirectDiff; }
	GLuint getIndirectSpecular() { return m_lightingIndirectSpec; }

	GLuint getLightingFBO() { return lightFBO; }

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
	Shader* getLightPass() { return m_lightPass; }
	Shader* getGeometryPass() { return m_geometryPass; }
	Shader* getCurrentShader() { return m_currentShader; }
	Shader* getCompositeShader() { return m_compositePass; }

	void setCurrentShader(Shader* inShader) { m_currentShader = inShader; } // Used for setting (and the other for getting) lighting shaders in the UI

private:
	int width, height;

	Shader* m_shader = 0; // Forward rendering lighting pass

	// Deferred rendering lighting passes
	Shader* m_lightPass = 0; // Not used: separated into "direct" and "indirect" shaders
	Shader* m_directPass = 0;
	Shader* m_indirectPass = 0;
	Shader* m_compositePass = 0;

	Shader* m_geometryPass = 0;
	Shader* m_currentShader = 0;

	// FBOs
	GLuint gBuffer = 0, ssaoFBO = 0, rboDepth = 0, 
		lightFBO = 0;

	// Textures
	GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gMetalRough = 0,
		m_lightDiff = 0, m_lightingSpec = 0, m_lightingIndirectDiff = 0, m_lightingIndirectSpec = 0;
};