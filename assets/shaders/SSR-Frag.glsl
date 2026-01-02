#version 330 core

layout (location = 0) out vec4 reflectionColor;

uniform sampler2D gNormal;
uniform sampler2D colorBuffer;
uniform sampler2D depthMap;

uniform float SCR_WIDTH;
uniform float SCR_HEIGHT;
uniform mat4 invProjection;
uniform mat4 projection;

uniform int maxSteps = 5;
uniform float thickness = 0.0001;
uniform float rayDirMin = 0.001;
uniform bool useBinaryRefinement = false;

bool rayIsOutofScreen(vec2 ray){
	return (ray.x > 1 || ray.y > 1 || ray.x < 0 || ray.y < 0) ? true : false;
}

vec3 binarySearch(vec3 rayPos, vec3 prevRayPos) {
	vec3 minRay = prevRayPos;
	vec3 maxRay = rayPos;

	for (int i = 0; i < maxSteps; i++) {
		vec3 midRay = (minRay + maxRay) * 0.5;
		float sampleDepth = texture(depthMap, midRay.xy).r;
		float depthDif = midRay.z - sampleDepth;
		
		if(depthDif > 0.0)
			maxRay = midRay;
		else
			minRay = midRay;
	}
	
	return (minRay + maxRay) * 0.5;
}

vec4 TraceRay(vec3 rayPos, vec3 dir, int iterationCount){
	float sampleDepth;
	vec3 hitColor = vec3(0);
	vec3 hitPos = vec3(0);
	vec3 prevRayPos = rayPos;
	float hit = 0.0;

	for(int i = 0; i < iterationCount; i++){
		prevRayPos = rayPos;
		rayPos += dir;
		if(rayIsOutofScreen(rayPos.xy)){
			hit = 0.0;
			break;
		}

		sampleDepth = texture(depthMap, rayPos.xy).r;
		float depthDif = rayPos.z - sampleDepth;
		
		float prevDepthDiff = prevRayPos.z - sampleDepth;
		
		//if(depthDif >= 0 && prevDepthDiff >= 0.0 && depthDif < thickness) {
		if(depthDif >= 0 && depthDif < thickness) { // we have a hit / ray has crossed the geometry
			hit = 1.0;
			
			if(useBinaryRefinement){
				//hitColor = texture(colorBuffer, binarySearch(rayPos, prevRayPos).xy).rgb;
				hitPos = binarySearch(rayPos, prevRayPos);
			}
			else{
				//hitColor = texture(colorBuffer, rayPos.xy).rgb;
				hitPos = rayPos;
			}
			hitColor = texture(colorBuffer, hitPos.xy).rgb;
			
			break;
		}
	}
	return vec4(hitColor, hit);
}

void main(){
	float maxRayDistance = 100.0f;

	//View Space ray calculation
	vec3 pixelPositionTexture;
	pixelPositionTexture.xy = vec2(gl_FragCoord.x / SCR_WIDTH,  gl_FragCoord.y / SCR_HEIGHT);
	vec3 normalView = texture(gNormal, pixelPositionTexture.xy).rgb;	
	float pixelDepth = texture(depthMap, pixelPositionTexture.xy).r;	// 0< <1
	pixelPositionTexture.z = pixelDepth;		
	vec4 positionView = invProjection * vec4(pixelPositionTexture * 2 - vec3(1), 1);
	positionView /= positionView.w;
	vec3 reflectionView = normalize(reflect(positionView.xyz, normalView));
	if(reflectionView.z > 0){
		reflectionColor = vec4(0,0,0,0);
		return;
	}
	vec3 rayEndPositionView = positionView.xyz + reflectionView * maxRayDistance;


	//Texture Space ray calculation
	vec4 rayEndPositionTexture = projection * vec4(rayEndPositionView,1);
	rayEndPositionTexture /= rayEndPositionTexture.w;
	rayEndPositionTexture.xyz = (rayEndPositionTexture.xyz + vec3(1)) / 2.0f;
	vec3 rayDirectionTexture = rayEndPositionTexture.xyz - pixelPositionTexture;

	ivec2 screenSpaceStartPosition = ivec2(pixelPositionTexture.x * SCR_WIDTH, pixelPositionTexture.y * SCR_HEIGHT); 
	ivec2 screenSpaceEndPosition = ivec2(rayEndPositionTexture.x * SCR_WIDTH, rayEndPositionTexture.y * SCR_HEIGHT); 
	ivec2 screenSpaceDistance = screenSpaceEndPosition - screenSpaceStartPosition;
	int screenSpaceMaxDistance = max(abs(screenSpaceDistance.x), abs(screenSpaceDistance.y)) / 2;
	rayDirectionTexture /= max(screenSpaceMaxDistance, rayDirMin);

	//trace the ray
	vec4 outColor = TraceRay(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
	reflectionColor = outColor;
	
	//reflectionColor = vec4(screenSpaceMaxDistance, 0.0, 0.0, 1.0);
}
