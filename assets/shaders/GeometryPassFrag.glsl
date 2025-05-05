
	#version 330 core
	layout (location = 0) out vec3 gPosition;
	layout (location = 1) out vec3 gNormal;
	layout (location = 2) out vec4 gAlbedoSpec;
	layout (location = 3) out vec2 gMetallicRoughness;

	in vec3 fragPos;
	in vec2 texCoord;
	in vec3 normal;

	// Material properties	
	uniform sampler2D DiffuseMap;
	uniform sampler2D MetallicMap;
	uniform sampler2D RoughnessMap;
	uniform sampler2D NormalMap;
	
	uniform bool useDiffuseTexture = true, useMetallicTexture = true, useRoughnessTexture = true;
	uniform vec3 u_DiffuseColor;
	uniform float u_Roughness, u_Metallic;
	
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

	void main()
	{    
		// Store the fragment position vector in the first gbuffer texture
		gPosition = fragPos;
		
		// Also store the per-fragment normals into the gbuffer	
		vec3 N = texture(NormalMap, texCoord).rgb == vec3(0.0) ? normalize(normal) : getNormalFromMap();
		gNormal = N;
		
		// And the diffuse per-fragment color
		//gAlbedoSpec.rgb = (pow(texture(DiffuseMap, texCoord).rgb, vec3(2.2)));	
		gAlbedoSpec.rgb = u_DiffuseColor;
		if (useDiffuseTexture) {
			gAlbedoSpec.rgb = (pow(texture(DiffuseMap, texCoord).rgb, vec3(2.2)));
		}
		
		// Metallic and Roughness values into a singular sampler2D
		//gMetallicRoughness.r = texture(MetallicMap, texCoord).r;	
		gMetallicRoughness.r = u_Metallic;
		if (useMetallicTexture) {
			gMetallicRoughness.r = texture(MetallicMap, texCoord).r;
		}
		
		//gMetallicRoughness.g  = texture(RoughnessMap, texCoord).r;	
		gMetallicRoughness.g = u_Roughness;
		if (useRoughnessTexture) {
			gMetallicRoughness.g  = texture(RoughnessMap, texCoord).r;	
		}
	}