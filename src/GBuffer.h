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

private:
	int width, height;

	// Buffers
	GLuint gBuffer = 0, ssaoFBO = 0, rboDepth = 0;
	// Maps
	GLuint gPosition = 0, gNormal = 0, gAlbedo = 0;
};