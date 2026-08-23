	#version 330 core

	//Variables
	out vec4 FragColor;

	in vec3 fragPos;
	in vec2 texCoord;
	in vec3 normal;
	in vec4 fragPosLightSpace;

	// HDRI
	uniform samplerCube irradianceMap, prefilterMap;
	uniform sampler2D brdfLUT;
	// PBR
	uniform sampler2D DiffuseMap, MetallicMap, RoughnessMap, EmissionMap, NormalMap, OpacityMap, shadowMap;
	
	// Use textures or basic colors/values?
	uniform bool useDiffuseTexture = true, useMetallicTexture = true, useRoughnessTexture = true, useEmissionTexture = false, useOpacityTexture = false;
	
	uniform vec3 u_DiffuseColor, objectColor, HDRIHue = vec3(1.0f), FinalColorHue = vec3(1.0f), sunDir;
	uniform float u_Roughness, u_Metallic, u_emissionStrength, u_opacity;
	uniform float HDRIExposure = 1.0f, HDRIContrast = 1.0f, FinalColorExposure = 1.0f, FinalColorContrast = 2.2f;
	uniform int NUM_POINT_LIGHTS;

	// Point light structure in GLSL
	struct PointLight {
		vec3 position;
		vec3 color;
		float constant;
		float linear;
		float quadratic;
		float strength;
	};
	
	//#define NUM_POINT_LIGHTS 4  // Adjust this as needed
	
	// Array of point lights
	uniform PointLight pointLights[12];
	
	uniform vec3 viewPos;
	const float PI = 3.14159265359;
	//float LampStrength = 0.8;
	float exposure = 1.0;
	
	//Normal map function
	vec3 getNormalFromMap()
	{
		vec3 tangentNormal = texture(NormalMap, texCoord).xyz * 2.0 - 1.0;

		vec3 Q1 = dFdx(fragPos);
		vec3 Q2 = dFdy(fragPos);
		vec2 st1 = dFdx(texCoord);
		vec2 st2 = dFdy(texCoord);

		vec3 N = normalize(normal);
		vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
		
		T = normalize(T - N * dot(N, T));
		vec3 B = cross(N, T);
		//vec3 B = -normalize(cross(N, T));

		mat3 TBN = mat3(T, B, N);

		return normalize(TBN * tangentNormal);
	}

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
	
	vec3 gammaCorrect(vec3 color, float exposure, float contrast) {
		color = color / (color + vec3(1.0)) * exposure;
		color = pow(color, vec3(1.0 / contrast));
		
		return color;
	}

	float ShadowCalculation(vec4 fragPosLightSpace) {
		// perform perspective divide
		vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		// transform to [0,1] range
		projCoords = projCoords * 0.5 + 0.5;
			
		// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
		float closestDepth = texture(shadowMap, projCoords.xy).r; 
		// get depth of current fragment from light's perspective
		float currentDepth = projCoords.z;
		// check whether current frag pos is in shadow
		float bias = 0.005;
		//vec3 normal = normalize(fs_in.Normal);
		//vec3 lightDir = normalize(lightPos - fs_in.FragPos);
		//float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); // Max 0.05, min 0.005. Dot product to get the angle
		float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
		
		vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
			}    
		}
		shadow /= 9.0;

		if(projCoords.z > 1.0) // Set the lightSpace outside frustum coordinates to 0.0f
			shadow = 0.0;

		return shadow;
	}

	//Main
	void main()
	{	
		float opacity = u_opacity; // Kind of a dinky way of implementing a switch between (texture OR float value)
		if (useOpacityTexture) {
			opacity = texture(OpacityMap, texCoord).r;
		}
		/*if(opacity < 0.1) {
			discard;
		}*/
		
		vec3 albedo = u_DiffuseColor;
		if (useDiffuseTexture) {
			albedo = (pow(texture(DiffuseMap, texCoord).rgb, vec3(2.2)));
		}
		
		float roughness = u_Roughness;
		if (useRoughnessTexture) {
			roughness = texture(RoughnessMap, texCoord).r;
		}

		float metallic = u_Metallic;
		if (useMetallicTexture) {
			metallic  = texture(MetallicMap, texCoord).r;
		}
		
		vec3 emission = vec3(albedo * u_emissionStrength); // Kind of a dinky way of implementing a switch between (texture OR float value)
		if (useEmissionTexture) {
			emission = texture(EmissionMap, texCoord).rgb;
		}
		
		//float distanceFactor = clamp(length(viewPos - fragPos) / 1, 0.0, 1.0);
		//float AmbientOcclusion = texture(ssao, texCoord).r;
		//vec3 AmbientOcclusion = texture(ssao, texCoord).rgb;

		vec3 N = getNormalFromMap();
		vec3 V = normalize(viewPos - fragPos);
		vec3 R = reflect(-V, N);	
		//vec3 R = mix(N, reflect(-V, N), distanceFactor);

		vec3 F0 = vec3(0.04);
		F0 = mix(F0, albedo, metallic);

		//For loop
		vec3 Lo = vec3(0.0);
		for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
		{
			// calculate per-light radiance - light calculations
			vec3 L = normalize(pointLights[i].position - fragPos);
			vec3 H = normalize(V + L);
			float distance = length(pointLights[i].position - fragPos);
			float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + 
										   pointLights[i].quadratic * (distance * distance));
			
			vec3 radiance = pointLights[i].color * attenuation * pointLights[i].strength;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(N, H, roughness);
			float G = GeometrySmith(N, V, L, roughness);
			
			//vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0); 

			vec3 numerator = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
			vec3 specular = numerator / denominator;

			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;
			float NdotL = max(dot(N, L), 0.0);

			Lo += (kD * albedo / PI + specular) * radiance * NdotL;

		}
		//vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
		// ambient lighting (we now use IBL as the ambient term)
		vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		
		vec3 irradiance = texture(irradianceMap, N).rgb * 1;
		
		//vec3 diffuse      = irradiance * vec3(albedo.r, chroma * cos(finalHue), chroma * sin(finalHue));
		vec3 diffuse 		= irradiance * vec3(albedo.r * HDRIHue.r, albedo.g * HDRIHue.g, albedo.b * HDRIHue.b);
		//vec3 diffuse      = irradiance * (vec3(albedo.r * Hue, albedo.g, albedo.b ) );
		
		// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
		//MAX_REFLECTION_LOD = 3.0; is quite nice :3
		const float MAX_REFLECTION_LOD = 3.0;
		vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;  
		vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
		vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * exposure;
		
		vec3 ambient = (kD * diffuse + specular) * HDRIExposure;
		ambient = gammaCorrect(ambient, HDRIExposure, HDRIContrast); // Ambient lighting tone mapping 

		float shadow = ShadowCalculation(fragPosLightSpace);
		vec3 color = (ambient + Lo * (1.0 - shadow)) + (emission * u_emissionStrength); 	//Ambient + point lights + emissive
		color = gammaCorrect(color, FinalColorExposure, FinalColorContrast); // HDR tonemapping and gamma correct
		
		// Color tweaking
		color = vec3(color.r * FinalColorHue.r, color.g * FinalColorHue.g, color.b * FinalColorHue.b);
		
		// Fun things
		//color = vec3(1.0) - color; // inverted colors
		//color = vec3((color.r + color.g + color.b) / 3.0f); // Black and white with average
		
		// Shadow mapping debugging
		//color = texture(shadowMap, texCoord).rgb;
		//FragColor = vec4(color, 1.0);
		
		//Color out
		FragColor = vec4(color, opacity);
	};