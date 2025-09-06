	#version 330 core
	//out vec4 FragColor;
	
	layout (location = 0) out vec4 reflectionColor;

	in vec2 texCoords;

	uniform sampler2D gNormal;
	uniform sampler2D colorBuffer;
	uniform sampler2D gDepth;
	uniform sampler2D gPosition;

	uniform int width = 640, height = 480;
	uniform mat4 projection;
	uniform mat4 invProjection;
	
	float near = 0.1f, far = 100.0f;

	bool rayIsOutOfScreen(vec2 ray) {
    return ray.x > 1.0 || ray.y > 1.0 || ray.x < 0.0 || ray.y < 0.0;
	}

	vec3 TraceRay(vec3 startViewPos, vec3 reflectionDirView, int steps) {
		float thickness = 0.1;
		vec3 stepView = reflectionDirView / float(steps);

		for (int i = 0; i < steps; ++i) {
			vec3 currentViewPos = startViewPos + stepView * float(i);

			// Project to clip space
			vec4 clipPos = projection * vec4(currentViewPos, 1.0);
			clipPos /= clipPos.w;

			// NDC -> screen UV
			vec2 screenUV = clipPos.xy * 0.5 + 0.5;

			if (rayIsOutOfScreen(screenUV)) {
				break;
			}

			// Sample scene depth
			vec3 sceneViewPos = texture(gPosition, screenUV).xyz;
			float sceneDepth = -sceneViewPos.z;
			float rayDepth = -currentViewPos.z;

			float depthDiff = rayDepth - sceneDepth;

			if (depthDiff >= 0.0 && depthDiff < thickness) {
				return texture(colorBuffer, screenUV).rgb;
			}
		}

		return vec3(0.0);
	}

	void main() {
		float maxRayDistance = 100.0;

		// Sample view-space normal and position
		vec3 normalView = normalize(texture(gNormal, texCoords).rgb * 2.0 - 1.0);
		vec3 viewPos = texture(gPosition, texCoords).xyz;

		// View direction is from fragment to camera
		vec3 viewDir = normalize(-viewPos);
		vec3 reflectionDirView = reflect(viewDir, normalView);
		// vec3 viewDir = normalize(viewPos.xyz);
		// vec3 reflectionDirView = reflect(viewDir, normalize(normalView));

		// Discard forward-facing reflections (avoid seeing behind camera)
		if (reflectionDirView.z > 0.0) {
			reflectionColor = vec4(0.0, 0.0, 0.0, 1.0);
			return;
		}

		// Compute view-space endpoint of the ray
		vec3 rayEndView = viewPos + reflectionDirView * maxRayDistance;

		// Project endpoint to screen-space UV for estimating step count
		vec4 clipEnd = projection * vec4(rayEndView, 1.0);
		clipEnd /= clipEnd.w;
		vec2 endUV = clipEnd.xy * 0.5 + 0.5;

		// Estimate number of steps
		ivec2 screenStart = ivec2(texCoords * vec2(width, height));
		ivec2 screenEnd   = ivec2(endUV * vec2(width, height));
		ivec2 delta = screenEnd - screenStart;
		//int steps = max(abs(delta.x), abs(delta.y));
		//steps = clamp(steps, 10, 100); // avoid over/under-shooting
		int steps = clamp(max(abs(delta.x), abs(delta.y)), 20, 100);

		vec3 resultColor = TraceRay(viewPos, reflectionDirView, steps);
		reflectionColor = vec4(resultColor, 1.0);
	}