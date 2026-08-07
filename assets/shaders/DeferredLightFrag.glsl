	// Fragment shader (lighting.frag)
	#version 330 core
	//out vec4 FragColor;
	
	layout (location = 0) out vec3 oLightPass;

	in vec2 texCoord;
	
	// HDRI
	uniform samplerCube irradianceMap, prefilterMap;
	uniform sampler2D brdfLUT;
	uniform bool worldCoords = true, aoTone = false, useSSAO = true;
	// G-Buffer
	uniform sampler2D gPosition, gNormal, gAlbedoSpec, gMetallicRoughness;
	// SSAO
	uniform sampler2D uSSAO, uSSR, uEmission;
	// General ImGui uniforms
	uniform float aoStrength = 10.0f;
	
	uniform float HDRIExposure = 1.0f, HDRIContrast = 1.0f, FinalColorExposure = 1.0f, FinalColorContrast = 2.2f;
	uniform vec3 HDRIHue = vec3(1.0f), FinalColorHue = vec3(1.0);
	
	uniform mat4 inverseView;
	const float PI = 3.14159265359;
	float exposure = 1.5;

	struct PointLight {
		vec3 position;
		vec3 color;
		float constant;
		float linear;
		float quadratic;
		float strength;
	};

	uniform PointLight pointLights[12];
	uniform int NUM_POINT_LIGHTS;

	// Camera is always at (0.0f, 0.0f, 0.0f), even after viewMatrix * cameraPos.
	// Works, but the IBL reflections have the same rotation in every angle.
	const vec3 view = vec3(0.0f, 0.0f, 0.0f);

	//1-----
	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = max(dot(N, H), 0.0);
		float NdotH2 = NdotH * NdotH;

		float nom = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;

		return nom / denom;
	}

	//2-----
	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0);
		float k = (r * r) / 8.0;

		float nom = NdotV;
		float denom = NdotV * (1.0 - k) + k;

		return nom / denom;
	}
	
	//3-----
	float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
	{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
	}
	
	//4-----
	vec3 fresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
	}
	
	//5
	vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
	{
		return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
	}
	
	vec3 gammaCorrect(vec3 color, float exposure, float contrast){
		color = color / (color + vec3(1.0)) * exposure;
		color = pow(color, vec3(1.0 / contrast));
		
		return color;
	}
	
	void main()
	{             
		// Retrieve data from gbuffer
		vec3 FragPos = texture(gPosition, texCoord).rgb;
		bool hasGeometry = length(FragPos) > 0.0001;
		if(!hasGeometry){ discard; return; }
		
		vec3 albedo = texture(gAlbedoSpec, texCoord).rgb;	
		float roughness = texture(gMetallicRoughness, texCoord).g;
		float metallic = texture(gMetallicRoughness, texCoord).r;
		vec3 N = texture(gNormal, texCoord).rgb;
		float AmbientOcclusion = texture(uSSAO, texCoord).r;
		vec4 ssr = texture(uSSR, texCoord).rgba;
		vec3 emission = texture(uEmission, texCoord).rgb;
		
		// PBR	
		// View direction
		vec3 NewR, NewN;
		vec3 V = normalize(view - FragPos);
		// Reflection
		vec3 R = reflect(-V, N);
		if(worldCoords){
			NewR = mat3(inverseView) * R;
			NewN = mat3(inverseView) * N;
		}
		else {
			NewR = R;
			NewN = N;
		}
		//R = mix(R, N, roughness * roughness); // bias reflection direction

		vec3 F0 = vec3(0.04);
		F0 = mix(F0, albedo, metallic);

		// Direct lighting loop
		//vec3 Lo = vec3(0.0);
		vec3 directDiff = vec3(0.0);
		vec3 directSpec = vec3(0.0);
		for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
		{
			// Skip the pixels, that are out of range
			if(length(pointLights[i].position - FragPos) > 50.0) // Hard coded distance = 50 (because linear 0.09, quadratic 0.032)
				continue;
		
			// calculate per-light radiance - light calculations
			// Light direction
			vec3 L = normalize(pointLights[i].position - FragPos);
			//vec3 diffuse = max(dot(N, L), 0.0) * albedo * pointLights[i].color;
			vec3 albedoBRDF = albedo / PI;
			
			// Halfway direction
			vec3 H = normalize(V + L);
			float spec = pow(max(dot(N, H), 0.0), 16.0);
			
			float distance = length(pointLights[i].position - FragPos);
			float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + 
										   pointLights[i].quadratic * (distance * distance));
			
			vec3 radiance = pointLights[i].color * attenuation * pointLights[i].strength;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(N, H, roughness);
			float G = GeometrySmith(N, V, L, roughness);	
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0); 

			vec3 numerator = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
			vec3 specular = numerator / denominator;

			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;
			float NdotL = max(dot(N, L), 0.0);

			//Lo += (kD * diffuse / PI + specular) * radiance * NdotL;
			//oDirectDiff += kD * diffuse / PI * radiance * NdotL;
			directDiff += kD * albedo / PI * radiance * NdotL;
			directSpec += specular * radiance * NdotL;
		}
		
		// ambient lighting (we now use IBL as the ambient term)
		vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		
		// HDRI
		vec3 irradiance = texture(irradianceMap, NewN).rgb; // N Set to world-space. See the magnificent lighting all around
		
		vec3 diffuse      = irradiance * vec3(albedo.r * HDRIHue.r, albedo.g * HDRIHue.g, albedo.b * HDRIHue.b);
		
		// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
		//MAX_REFLECTION_LOD = 3.0; is quite nice :3
		const float MAX_REFLECTION_LOD = 3.0;
		vec3 prefilteredColor = textureLod(prefilterMap, NewR, roughness * MAX_REFLECTION_LOD).rgb; // R Set to world-space. Reflect, reflect 360 degrees around my brother
		vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
		
		//vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * exposure;
		vec3 indirectSpec = prefilteredColor * (F * brdf.x + brdf.y) * exposure;
		indirectSpec = mix(indirectSpec.rgb, ssr.rgb, ssr.a);
		//vec3 ScreenSpaceColor = mix(indirectSpec.rgb, ssr.rgb, ssr.a);
		
		float ao = 1.0f;
		if(useSSAO){ 
			ao = pow(AmbientOcclusion, aoStrength);
		}
		//float ao = pow(AmbientOcclusion, aoStrength);
		if(aoTone) { ao = clamp((ao - 0.2) * 1.25, 0.0, 1.0); } // Remaps midtones. Adds contrast to the ambient occlusion
		//vec3 ambient = (kD * (diffuse * ao) + specular); // Replaced specular with the new finalSpecular

		vec3 indirectDiff = (kD * (diffuse * ao)); //kD * diffuse * albedo * ao
		
		vec3 finalColor = (directDiff + directSpec) + gammaCorrect((indirectDiff + indirectSpec), HDRIExposure, HDRIContrast) + emission;
		finalColor = vec3(finalColor.r * FinalColorHue.r, finalColor.g * FinalColorHue.g, finalColor.b * FinalColorHue.b);
		oLightPass = gammaCorrect(finalColor, FinalColorExposure, FinalColorContrast);
	}