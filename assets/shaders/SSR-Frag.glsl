	#version 330 core
	//out vec4 FragColor;
	
	layout (location = 0) out vec4 reflectionColor;

	in vec2 texCoords;
	
	uniform sampler2D gNormal;
	uniform sampler2D ColorBuffer;
	uniform sampler2D gPosition;
	uniform sampler2D gExtraComponents;	
	
	uniform mat4 invView;
	uniform mat4 projection;
	//uniform mat4 invprojection;
	//uniform mat4 view;

	const float step = 0.1;
	const float minRayStep = 0.1;
	const float maxSteps = 30;
	const int numBinarySearchSteps = 5;
	const float reflectionSpecularFalloffExponent = 3.0;

	float Metallic;

	#define Scale vec3(.8, .8, .8)
	#define K 19.19

	vec3 PositionFromDepth(float depth);
	vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth); 
	vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth);
	vec3 fresnelSchlick(float cosTheta, vec3 F0);
	vec3 hash(vec3 a);

	void main()
	{

		vec2 MetallicEmmissive = texture2D(gExtraComponents, texCoords).rg;
		Metallic = MetallicEmmissive.r;

		if(Metallic < 0.01)
			discard;
	 
		//vec3 viewNormal = vec3(texture2D(gNormal, texCoords) * invView);
		vec3 normalVS = texture(gNormal, texCoords).xyz;  // assume stored in view-space
		vec3 viewNormal = mat3(invView) * normalVS;
		
		//vec3 viewPos = textureLod(gPosition, texCoords, 2).xyz;
		vec3 viewPos = texture(gPosition, texCoords).xyz;
		vec3 albedo = texture(ColorBuffer, texCoords).rgb;

		float spec = texture(ColorBuffer, texCoords).w;

		vec3 F0 = vec3(0.04); 
		F0      = mix(F0, albedo, Metallic);
		vec3 Fresnel = fresnelSchlick(max(dot(normalize(viewNormal), normalize(viewPos)), 0.0), F0);

		// Reflection vector
		vec3 reflected = normalize(reflect(normalize(viewPos), normalize(viewNormal)));
		//vec3 reflected = normalize(reflect(normalize(-viewPos), normalize(viewNormal)));


		vec3 hitPos = viewPos;
		float dDepth;
	 
		vec3 wp = vec3(vec4(viewPos, 1.0) * invView);
		vec3 jitt = mix(vec3(0.0), vec3(hash(wp)), spec);
		//vec3 jitt = mix(vec3(0.0), vec3(1.0), spec); // With constant jitter
		
		vec4 coords = RayMarch((vec3(jitt) + reflected * max(minRayStep, -viewPos.z)), hitPos, dDepth);
	 
	 
		vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
	 
	 
		float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

		float ReflectionMultiplier = pow(Metallic, reflectionSpecularFalloffExponent) * 
					screenEdgefactor * 
					-reflected.z;
	 
		// Get color
		vec3 SSR = textureLod(ColorBuffer, coords.xy, 0).rgb * clamp(ReflectionMultiplier, 0.0, 0.9) * Fresnel;  

		//reflectionColor = vec4(SSR, Metallic);
		reflectionColor = vec4(SSR, 1.0);
		//reflectionColor = vec4(1.0, 0.0, 1.0, 1.0);
	}

	// vec3 PositionFromDepth(float depth) {
		// float z = depth * 2.0 - 1.0;

		// vec4 clipSpacePosition = vec4(texCoords * 2.0 - 1.0, z, 1.0);
		// vec4 viewSpacePosition = invprojection * clipSpacePosition;

		// // Perspective division
		// viewSpacePosition /= viewSpacePosition.w;

		// return viewSpacePosition.xyz;
	// }

	vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
	{
		float depth;

		vec4 projectedCoord;
	 
		for(int i = 0; i < numBinarySearchSteps; i++)
		{

			projectedCoord = projection * vec4(hitCoord, 1.0);
			projectedCoord.xy /= projectedCoord.w;
			projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	 
			depth = textureLod(gPosition, projectedCoord.xy, 2).z;

	 
			dDepth = hitCoord.z - depth;

			dir *= 0.5;
			if(dDepth > 0.0)
				hitCoord += dir;
			else
				hitCoord -= dir;    
		}

			projectedCoord = projection * vec4(hitCoord, 1.0);
			projectedCoord.xy /= projectedCoord.w;
			projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	 
		return vec3(projectedCoord.xy, depth);
	}

	vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth)
	{

		dir *= step;
	 
	 
		float depth;
		int steps;
		vec4 projectedCoord;

	 
		for(int i = 0; i < maxSteps; i++)
		{
			hitCoord += dir;
	 
			projectedCoord = projection * vec4(hitCoord, 1.0);
			projectedCoord.xy /= projectedCoord.w;
			projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	 
			depth = textureLod(gPosition, projectedCoord.xy, 2).z;
			if(depth > 1000.0)
				continue;
	 
			dDepth = hitCoord.z - depth;

			if((dir.z - dDepth) < 1.2)
			{
				if(dDepth <= 0.0)
				{   
					vec4 Result;
					Result = vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);

					return Result;
				}
			}
			
			steps++;
		}
	 
		
		return vec4(projectedCoord.xy, depth, 0.0);
	}

	vec3 fresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	}


	vec3 hash(vec3 a)
	{
		a = fract(a * Scale);
		a += dot(a, a.yxz + K);
		return fract((a.xxy + a.yxx)*a.zyx);
	}
	
	// float near = 0.1f, far = 100.0f;

	// bool rayIsOutOfScreen(vec2 ray) {
    // return ray.x > 1.0 || ray.y > 1.0 || ray.x < 0.0 || ray.y < 0.0;
	// }

	// vec3 TraceRay(vec3 startViewPos, vec3 reflectionDirView, int steps) {
		// float thickness = 0.1;
		// vec3 stepView = reflectionDirView / float(steps);

		// for (int i = 0; i < steps; ++i) {
			// vec3 currentViewPos = startViewPos + stepView * float(i);

			// // Project to clip space
			// vec4 clipPos = projection * vec4(currentViewPos, 1.0);
			// clipPos /= clipPos.w;

			// // NDC -> screen UV
			// vec2 screenUV = clipPos.xy * 0.5 + 0.5;

			// if (rayIsOutOfScreen(screenUV)) {
				// break;
			// }

			// // Sample scene depth
			// vec3 sceneViewPos = texture(gPosition, screenUV).xyz;
			// float sceneDepth = -sceneViewPos.z;
			// float rayDepth = -currentViewPos.z;

			// float depthDiff = rayDepth - sceneDepth;

			// if (depthDiff >= 0.0 && depthDiff < thickness) {
				// return texture(colorBuffer, screenUV).rgb;
			// }
		// }

		// return vec3(0.0);
	// }

	// void main() {
		// float maxRayDistance = 100.0;

		// // Sample view-space normal and position
		// vec3 normalView = normalize(texture(gNormal, texCoords).rgb * 2.0 - 1.0);
		// vec3 viewPos = texture(gPosition, texCoords).xyz;

		// // View direction is from fragment to camera
		// vec3 viewDir = normalize(-viewPos);
		// vec3 reflectionDirView = reflect(viewDir, normalView);
		// // vec3 viewDir = normalize(viewPos.xyz);
		// // vec3 reflectionDirView = reflect(viewDir, normalize(normalView));

		// // Discard forward-facing reflections (avoid seeing behind camera)
		// if (reflectionDirView.z > 0.0) {
			// reflectionColor = vec4(0.0, 0.0, 0.0, 1.0);
			// return;
		// }

		// // Compute view-space endpoint of the ray
		// vec3 rayEndView = viewPos + reflectionDirView * maxRayDistance;

		// // Project endpoint to screen-space UV for estimating step count
		// vec4 clipEnd = projection * vec4(rayEndView, 1.0);
		// clipEnd /= clipEnd.w;
		// vec2 endUV = clipEnd.xy * 0.5 + 0.5;

		// // Estimate number of steps
		// ivec2 screenStart = ivec2(texCoords * vec2(width, height));
		// ivec2 screenEnd   = ivec2(endUV * vec2(width, height));
		// ivec2 delta = screenEnd - screenStart;
		// //int steps = max(abs(delta.x), abs(delta.y));
		// //steps = clamp(steps, 10, 100); // avoid over/under-shooting
		// int steps = clamp(max(abs(delta.x), abs(delta.y)), 20, 100);

		// vec3 resultColor = TraceRay(viewPos, reflectionDirView, steps);
		// reflectionColor = vec4(resultColor, 1.0);
	// }