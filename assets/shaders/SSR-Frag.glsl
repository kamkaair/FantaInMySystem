	#version 330 core
	//out vec4 FragColor;
	
	layout (location = 0) out vec4 reflectionColor;

	in vec2 texCoords;

	uniform sampler2D gNormal;
	uniform sampler2D colorBuffer;
	uniform sampler2D gDepth;

	uniform int width = 640, height = 480;
	uniform mat4 projection;
	uniform mat4 invProjection;
	
	float near = 0.1f, far = 100.0f;
	
	// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
	// float maxDistance = 15;
	// float resolution  = 0.3;
	// int   steps       = 10;
	// float thickness   = 0.5;

	float LinearizeDepth(float depth)
	{
		float z = depth * 2.0 - 1.0; // Back to NDC [-1, 1]
		return (2.0 * near * far) / (far + near - z * (far - near));
	}
	
	bool rayIsOutofScreen(vec2 ray){
	return (ray.x > 1.0 || ray.y > 1.0 || ray.x < 0.0 || ray.y < 0.0) ? true : false;
	}
	
	// // TraceRay adjusted to ray march in screen space with view-space depth check
	// vec3 TraceRayOld(vec3 rayPos, vec3 dir, int iterationCount) {
		// float sampleDepth;
		// vec3 hitColor = vec3(0);
		// bool hit = false;

		// for(int i = 0; i < iterationCount; i++){
			// rayPos += dir;
			// if(rayIsOutofScreen(rayPos.xy)){
				// break;
			// }

			// sampleDepth = texture(gDepth, rayPos.xy).r;
			// float depthDif = rayPos.z - sampleDepth;
			
			// //float thickness = 0.2;
			// //if(depthDif >= 0.0 && depthDif < thickness)
			// if(depthDif >= 0 && depthDif < 0.00001){ //we have a hit
				// hit = true;
				// hitColor = texture(colorBuffer, rayPos.xy).rgb;
				// break;
			// }
		// }
		// return hitColor;
	// }
	
	// TraceRay adjusted to ray march in screen space (UV-coords) with view-space depth check
	vec3 TraceRay(vec3 rayPos, vec3 rayDir, int steps) {
		vec3 hitColor = vec3(0.0);
		
		for (int i = 0; i < steps; ++i) {
			rayPos += rayDir;

			if (rayIsOutofScreen(rayPos.xy)) {
			break;
			}
				

			float sampledDepth = texture(gDepth, rayPos.xy).r;
			
			// Lineralize both sampled depth's and ray's positions
			float sampledViewDepth = LinearizeDepth(sampledDepth);
			float rayZView = LinearizeDepth(rayPos.z);

			// Compare depths in view space
			float depthDifference = rayZView - sampledViewDepth;

			float thickness = 0.1;
			if (depthDifference >= 0.0 && depthDifference < thickness) {
				hitColor = texture(colorBuffer, rayPos.xy).rgb;
				break;
			}
		}
		return hitColor;
	}

	void main() {
		float maxRayDistance = 100.0f;

		//View Space ray calculation
		//vec3 pixelPositionTexture;
		//pixelPositionTexture.xy = vec2(gl_FragCoord.x / width,  gl_FragCoord.y / height);
		//pixelPositionTexture.xy = texCoords;
		
		vec3 normalView = normalize(texture(gNormal, texCoords).rgb * 2.0 - 1.0);		
		vec3 debug = texture(colorBuffer, texCoords).rgb;
		float pixelDepth = texture(gDepth, texCoords).r;	// 0< <1
		
		// ----
		
		//pixelPositionTexture.z = pixelDepth;		
		//vec4 positionView = invProjection * vec4(pixelPositionTexture * 2 - vec3(1), 1);
		//positionView /= positionView.w;
		
		// ----
		
		// Convert to NDC coordinates [-1, 1]
		vec4 ndcPos = vec4(texCoords * 2.0 - vec2(1.0), pixelDepth * 2.0 - 1.0, 1.0);

		// Reconstruct view-space position
		vec4 positionView = invProjection * ndcPos;
		positionView /= positionView.w;
		
		// ----
		
		//vec3 reflectionView = normalize(reflect(positionView.xyz, normalView));
		
		vec3 viewDir = normalize(positionView.xyz);
		vec3 reflectionView = reflect(viewDir, normalView);
		
		if(reflectionView.z > 0.0){
			reflectionColor = vec4(0,0,0,1);
			return;
		}
		vec3 rayEndView = positionView.xyz + reflectionView * maxRayDistance;


		//Texture Space ray calculation
		vec4 rayEndClip = projection * vec4(rayEndView,1);
		rayEndClip /= rayEndClip.w;
		//rayEndClip.xyz = (rayEndClip.xyz + vec3(1)) / 2.0f;
		//vec3 rayDirectionTexture = rayEndClip.xyz - pixelPositionTexture;
		
		// Convert clip space to screen-space UV [0,1]
		vec3 rayEndUV = (rayEndClip.xyz * 0.5) + 0.5;
		
		// Initial ray in screen space (UV + depth)
		vec3 startUVZ = vec3(texCoords, pixelDepth);
		vec3 rayDirUVZ = rayEndUV - startUVZ;
		
		// Estimate screen-space length (in pixels) to determine ray steps
		ivec2 screenStart = ivec2(texCoords * vec2(width, height));
		ivec2 screenEnd   = ivec2(rayEndUV.xy * vec2(width, height));
		ivec2 screenDelta = screenEnd - screenStart;
		
		
		// ivec2 screenSpaceStartPosition = ivec2(pixelPositionTexture.x * width, pixelPositionTexture.y * height); 
		// ivec2 screenSpaceEndPosition = ivec2(rayEndClip.x * width, rayEndClip.y * height); 
		// ivec2 screenSpaceDistance = screenSpaceEndPosition - screenSpaceStartPosition;
		// int screenSpaceMaxDistance = max(abs(screenSpaceDistance.x), abs(screenSpaceDistance.y)) / 2;
		// rayDirectionTexture /= max(screenSpaceMaxDistance, 0.001f);

		//int steps = 50;
		//rayDirectionTexture /= float(steps);
		
		int steps = max(abs(screenDelta.x), abs(screenDelta.y));
		rayDirUVZ /= float(steps);

		//trace the ray
		//vec3 outColor = TraceRay(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
		vec3 outColor = TraceRay(startUVZ, rayDirUVZ, steps);
		reflectionColor = vec4(outColor, 1);
		
		// Debug
		//reflectionColor = vec4(vec3(pixelDepth), 1.0);
		//reflectionColor = vec4(debug, 1);
	}