#pragma once
#include "gameobject.h"     // Include base class
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <iostream>

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

private:
	int width, height;

	// Buffers
	GLuint gBuffer = 0, ssaoFBO = 0, rboDepth = 0;
	// Maps
	GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gMetalRough = 0;
};