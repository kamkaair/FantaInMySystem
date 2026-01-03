	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	uniform sampler2D uSSR;
	
	uniform sampler2D uDirectDiffuse;
	uniform sampler2D uDirectSpec;
	uniform sampler2D uIndirectDiffuse;
	uniform sampler2D uIndirectSpecFallback;
	
	uniform bool useRoughnessMask;

	void main()
	{
		//vec3 lighting = texture(uLightingTex, texCoords).rgb;
		//vec3 lightingDiff = texture(uLightingDiffuse, texCoords).rgb;
		vec4 ssr = texture(uSSR, texCoords);
		float metallic = 1.0;
		float roughness = 0.0;

		vec3 directDiffuse  = texture(uDirectDiffuse, texCoords).rgb;
		vec3 directSpecular = texture(uDirectSpec, texCoords).rgb;
		vec3 indirectDiffuse = texture(uIndirectDiffuse, texCoords).rgb;
		vec3 fallbackSpec = texture(uIndirectSpecFallback, texCoords).rgb;
		
		vec3 indirectSpecular = mix(fallbackSpec.rgb, ssr.rgb, ssr.a);
		//vec3 indirectSpecular = ssr.rgb + fallbackSpec * (1.0 - ssr.a);

		vec3 color = directDiffuse + directSpecular + indirectDiffuse + indirectSpecular;
		
		color = color / (color + vec3(1.0));
		color = pow(color, vec3(1.0 / 2.2));
		
		FragColor = vec4(color, 1.0);
	}