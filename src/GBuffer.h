#pragma once
#include "gameobject.h"     // Include base class
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <iostream>
#include "shader.h"

class GBuffer : public kgfw::Object {
public:
	GBuffer(int inWidth, int inHeight);
	~GBuffer();

	GLuint createGPosition();
	GLuint createGAlbedo();
	GLuint createGNormal();
	GLuint createDepthBuffer();
	GLuint createGMetallicRoughness();

	GLuint getGPosition() { return gPosition; }
	GLuint getGAlbedo() { return gAlbedo; }
	GLuint getGNormal() { return gNormal; }
	GLuint getGBuffer() { return gBuffer; }
	GLuint getGMetallicRoughness() { return gMetalRough; }
	GLuint getGDepth() { return rboDepth; }

	GLuint getLightingFBO() { return lightFBO; }
	GLuint getLightingTex() { return lightColorBuffer; }

	float getWidth() const { return width; }
	float getHeight() const { return height; }

	void setResolution(int inWidth, int inHeight) { width = inWidth; height = inHeight; }
	void CleanUpGBuffer();
	void constructGBuffer();

	void constructDeferredShaders();
	void constructForwardShaders();
	void constructHistoryBuffers();

	void createLightingFramebuffer();

	void deconstructDeferredShaders();
	void deconstructForwardShaders();

	Shader* getForwardShader() { return m_shader; }
	Shader* getLightPass() { return m_lightPass; }
	Shader* getGeometryPass() { return m_geometryPass; }
	Shader* getCurrentShader() { return m_currentShader; }

	GLuint getHistoryFBO(int index) { return historyFBO[index]; }
	void setHistoryIndex(int newIndex) { historyIndex = newIndex; }
	int getHistoryIndex() { return historyIndex; }

	void setCurrentShader(Shader* inShader) { m_currentShader = inShader; } // Used for setting (and the other for getting) lighting shaders in the UI

private:
	int width, height, historyIndex = 0;

	Shader* m_shader = 0;
	Shader* m_lightPass = 0;
	Shader* m_geometryPass = 0;
	Shader* m_currentShader = 0;

	// FBOs
	GLuint gBuffer = 0, ssaoFBO = 0, rboDepth = 0, lightFBO = 0, historyFBO[2];

	// Textures
	GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gMetalRough = 0, lightColorBuffer = 0, historyColorBuffer[2];
};