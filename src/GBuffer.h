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

	float getWidth() const { return width; }
	float getHeight() const { return height; }

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
	void setCurrentShader(Shader* inShader) { m_currentShader = inShader; } // Used for setting (and the other for getting) lighting shaders in the UI

private:
	int width, height;

	Shader* m_shader = 0;
	Shader* m_lightPass = 0;
	Shader* m_geometryPass = 0;
	Shader* m_currentShader = 0;

	// Buffers
	GLuint gBuffer = 0, ssaoFBO = 0, rboDepth = 0;
	// Maps
	GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gMetalRough = 0;
};