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
	
	// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
	float maxDistance = 15;
	float resolution  = 0.3;
	int   steps       = 10;
	float thickness   = 0.5;

	bool rayIsOutofScreen(vec2 ray){
	return (ray.x > 1 || ray.y > 1 || ray.x < 0 || ray.y < 0) ? true : false;
}

vec3 TraceRay(vec3 rayPos, vec3 dir, int iterationCount){
	float sampleDepth;
	vec3 hitColor = vec3(0);
	bool hit = false;

	for(int i = 0; i < iterationCount; i++){
		rayPos += dir;
		if(rayIsOutofScreen(rayPos.xy)){
			break;
		}

		sampleDepth = texture(gDepth, rayPos.xy).r;
		float depthDif = rayPos.z - sampleDepth;
		if(depthDif >= 0 && depthDif < 0.00001){ //we have a hit
			hit = true;
			hitColor = texture(colorBuffer, rayPos.xy).rgb;
			break;
		}
	}
	return hitColor;
}

void main(){
	float maxRayDistance = 100.0f;

	//View Space ray calculation
	vec3 pixelPositionTexture;
	//pixelPositionTexture.xy = vec2(gl_FragCoord.x / width,  gl_FragCoord.y / height);
	pixelPositionTexture.xy = texCoords;
	vec3 normalView = normalize(texture(gNormal, texCoords).rgb * 2.0 - 1.0);
	vec3 debug = texture(gDepth, texCoords).rgb;
	float pixelDepth = texture(gDepth, texCoords).r;	// 0< <1
	pixelPositionTexture.z = pixelDepth;		
	vec4 positionView = invProjection * vec4(pixelPositionTexture * 2 - vec3(1), 1);
	positionView /= positionView.w;
	vec3 reflectionView = normalize(reflect(positionView.xyz, normalView));
	if(reflectionView.z > 0){
		reflectionColor = vec4(0,0,0,1);
		return;
	}
	vec3 rayEndPositionView = positionView.xyz + reflectionView * maxRayDistance;


	//Texture Space ray calculation
	vec4 rayEndPositionTexture = projection * vec4(rayEndPositionView,1);
	rayEndPositionTexture /= rayEndPositionTexture.w;
	rayEndPositionTexture.xyz = (rayEndPositionTexture.xyz + vec3(1)) / 2.0f;
	vec3 rayDirectionTexture = rayEndPositionTexture.xyz - pixelPositionTexture;

	ivec2 screenSpaceStartPosition = ivec2(pixelPositionTexture.x * width, pixelPositionTexture.y * height); 
	ivec2 screenSpaceEndPosition = ivec2(rayEndPositionTexture.x * width, rayEndPositionTexture.y * height); 
	ivec2 screenSpaceDistance = screenSpaceEndPosition - screenSpaceStartPosition;
	int screenSpaceMaxDistance = max(abs(screenSpaceDistance.x), abs(screenSpaceDistance.y)) / 2;
	rayDirectionTexture /= max(screenSpaceMaxDistance, 0.001f);


	//trace the ray
	vec3 outColor = TraceRay(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
	//reflectionColor = vec4(outColor, 1);
	//reflectionColor = vec4(vec3(pixelDepth), 1.0);
	reflectionColor = vec4(debug, 1);
}