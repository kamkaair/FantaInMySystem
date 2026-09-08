#include <glad/gl.h>

struct SSAO_SETTINGS {
	int kernelSize = 64;
	float radius = 0.5f;
	float bias = 0.025f;
	float occlusionStrength = 10.0f;

	bool clampedMidTones = false;
	bool useSSAO = true;
	bool dirty = false;
};

struct SSR_SETTINGS {
	int maxSteps = 5;
	float thickness = 0.00014;
	float rayDirMin = 0.001;

	bool useSSR = true;
	bool useTA = true;
	bool useRayScattering = true;
	bool useBinaryRefinement = false;
	bool dirty = false;
};

struct BLOOM_SETTINGS {
	int amount = 10;
	int distance = 5;
	bool useBloom = true;
	bool dirty = false;
};

struct SHADOW_SETTINGS {
	glm::vec3 pos;
	bool useShadowMap = true;
	bool useTimeSpin = true;
};