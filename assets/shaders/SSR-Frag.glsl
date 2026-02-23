#version 330 core

layout (location = 0) out vec4 reflectionColor;

uniform sampler2D gNormal;
uniform sampler2D colorBuffer;
uniform sampler2D depthMap;
uniform sampler2D gMetallicRoughness;

uniform float SCR_WIDTH;
uniform float SCR_HEIGHT;
uniform mat4 invProjection;
uniform mat4 projection;

uniform int maxSteps = 5;
uniform float thickness = 0.00014;
uniform float rayDirMin = 0.001;
uniform bool useBinaryRefinement = false;
uniform bool useRoughnessScatterSSR = true;

uniform int frameIndex;

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
	
	//float thickness = max(0.00009, rayPos.z * 0.0001);

	for(int i = 0; i < iterationCount; i++){
		prevRayPos = rayPos;
		rayPos += dir;
		if(rayIsOutofScreen(rayPos.xy)){
			hit = 0.0;
			break;
		}

		sampleDepth = texture(depthMap, rayPos.xy).r;
		float depthDif = rayPos.z - sampleDepth;
		
		//float prevDepthDiff = prevRayPos.z - sampleDepth;
		//float prevDepthDiff = prevRayPos.z - texture(depthMap, prevRayPos.xy).r;
		
		//if(depthDif >= 0 && prevDepthDiff >= 0.0 && depthDif < thickness) {
		//if (depthDif >= 0.0 && prevDepthDiff <= 0.0 && depthDif < thickness) {
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
			
			// kill the pixels, which have been set to black at grazing angles
			if (hitColor == vec3(0.0, 0.0, 0.0))
				hit = 0.0;
			
			break;
		}
	}
	return vec4(hitColor, hit);
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 sampleHemisphereGGX(vec3 N, float roughness, vec2 Xi)
{
    float a = roughness * roughness;

    float phi = 2.0 * 3.14159265 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

vec3 F_Schlick(vec3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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
	
	vec2 gMetalRoughTex = texture(gMetallicRoughness, pixelPositionTexture.xy).rg;	

	float roughness = gMetalRoughTex.g;
		
	vec3 H, reflectionView, V = normalize(-positionView.xyz);
	vec2 Xi, jitter;
	float alphaPre, roughnessFade;
	if(useRoughnessScatterSSR) {
		alphaPre = 1.0;
		jitter = vec2(
			fract(sin(float(frameIndex) * 12.9898) * 43758.5453),
			fract(sin(float(frameIndex) * 78.233)  * 12345.6789));

		Xi = vec2(
			rand(pixelPositionTexture.xy + jitter),
			rand(pixelPositionTexture.yx + jitter));
		
		H = sampleHemisphereGGX(normalView, roughness, Xi);
		reflectionView = normalize(reflect(-V, H));
		
		roughnessFade = clamp(1.0 - roughness * roughness, 0.0, 1.0);
	}
	else {
		alphaPre = normalize(mix(1.0, 0.0, roughness)); // mix roughness, output opacity (kinda whack, but it's fine)	
		reflectionView = normalize(reflect(-V, normalView));
		
		roughnessFade = 1.0;
	}
	
	float metallic = gMetalRoughTex.r;

	vec3 albedo = texture(colorBuffer, pixelPositionTexture.xy).rgb;
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	float NdotV = clamp(dot(normalView, V), 0.0, 1.0);
	vec3 fresnel = F_Schlick(F0, NdotV);
	

	if(reflectionView.z >= 0.0){
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

	// Trace the ray
	vec4 outColor = TraceRay(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
	
	vec3 reflection = outColor.rgb * fresnel * roughnessFade;
	
	float alpha = outColor.a * alphaPre;
	
	reflectionColor = vec4(reflection, alpha);
	
	//float specEnergy = max(max(fresnel.r, fresnel.g), fresnel.b);
	//float alpha = outColor.a * roughnessFade * specEnergy;	
	
	//float angleFade = smoothstep(0.05, 0.3, NdotV);
	//float alpha = outColor.a * roughnessFade * angleFade;
	//float alpha = outColor.a * roughnessFade;
}
