	#version 330 core

	//Variables
	out vec4 FragColor;

	in vec3 fragPos;
	in vec2 texCoord;
	in vec3 normal;

	uniform samplerCube irradianceMap;
	uniform samplerCube prefilterMap;
	uniform sampler2D brdfLUT;

	uniform sampler2D DiffuseMap;
	uniform sampler2D MetallicMap;
	uniform sampler2D RoughnessMap;
	uniform sampler2D NormalMap;
	
	uniform float LampStrength;
	uniform float HdrExposure = 1.0f;
	uniform float HdrContrast = 2.2f;
	
	uniform vec3 u_DiffuseColor;
	uniform float u_Roughness;
	uniform float u_Metallic;
	
	uniform float HueChange;
	
	uniform bool useDiffuseTexture = true;
	uniform bool useMetallicTexture = true;
	uniform bool useRoughnessTexture = true;
	
	uniform int NUM_POINT_LIGHTS;
	
	uniform vec3 objectColor;

	// Point light structure in GLSL
	struct PointLight {
		vec3 position;
		vec3 color;
		float constant;
		float linear;
		float quadratic;
	};
	
	//#define NUM_POINT_LIGHTS 4  // Adjust this as needed
	
	// Array of point lights
	uniform PointLight pointLights[12];
	
	uniform vec3 viewPos;
	const float PI = 3.14159265359;
	//float LampStrength = 0.8;
	float exposure = 1.5;
	
	vec4 sharpen(in sampler2D tex, in vec2 coords, in vec2 renderSize) {
	  float dx = 1.0 / renderSize.x;
	  float dy = 1.0 / renderSize.y;
	  vec4 sum = vec4(0.0);
	  sum += -1. * texture2D(tex, coords + vec2( -1.0 * dx , 0.0 * dy));
	  sum += -1. * texture2D(tex, coords + vec2( 0.0 * dx , -1.0 * dy));
	  sum += 5. * texture2D(tex, coords + vec2( 0.0 * dx , 0.0 * dy));
	  sum += -1. * texture2D(tex, coords + vec2( 0.0 * dx , 1.0 * dy));
	  sum += -1. * texture2D(tex, coords + vec2( 1.0 * dx , 0.0 * dy));
	  return sum;
	}
	
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
	vec3 B = -normalize(cross(N, T));
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

	//Main
	void main()
	{
	
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
	
	//float distanceFactor = clamp(length(viewPos - fragPos) / 1, 0.0, 1.0);

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
	
	vec3 radiance = pointLights[i].color * attenuation * LampStrength;

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
	
	// Hue change for the diffuse color
	float originalHue = atan(albedo.b, albedo.g);
	float finalHue = originalHue + HueChange;
	float chroma = sqrt(albedo.b*albedo.b+albedo.g*albedo.g);
	
    //vec3 diffuse      = irradiance * (rgb2yiq * albedo);
	vec3 diffuse      = irradiance * vec3(albedo.r, chroma * cos(finalHue), chroma * sin(finalHue));
	//vec3 diffuse      = irradiance * (vec3(albedo.r * Hue, albedo.g, albedo.b ) );
	
	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	//MAX_REFLECTION_LOD = 3.0; is quite nice :3
    const float MAX_REFLECTION_LOD = 3.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;  
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * exposure;
	
    vec3 ambient = (kD * diffuse + specular);
	//vec3 ambient = ((kD * diffuse) + (specular * 0.1)) * ao;

	vec4 sharpening = sharpen(DiffuseMap, texCoord, texCoord) * 0.2; // Sharpening to FragColor
	//Ambient + point lights
	vec3 color = ambient + Lo;

	// HDR tonemapping and gamma correct
	color = color / (color + vec3(1.0)) * HdrExposure;
	color = pow(color, vec3(1.0 / HdrContrast));
	
	//Color out
	FragColor = vec4(color, 1.0);
	//FragColor = sharpen(DiffuseMap, texCoord, vec2(HueChange, HueChange));
	};