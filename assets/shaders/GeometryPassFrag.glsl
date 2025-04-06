
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
	//uniform sampler2D NormalMap;

	void main()
	{    
		// Store the fragment position vector in the first gbuffer texture
		gPosition = fragPos;
		
		// Also store the per-fragment normals into the gbuffer
		gNormal = normalize(normal);
		
		// And the diffuse per-fragment color
		gAlbedoSpec.rgb = (pow(texture(DiffuseMap, texCoord).rgb, vec3(2.2)));
		
		// Metallic and Roughness values into a singular sampler2D
		gMetallicRoughness.r = texture(MetallicMap, texCoord).r;
		gMetallicRoughness.g  = texture(RoughnessMap, texCoord).r;
	}