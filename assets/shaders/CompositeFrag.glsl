	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	//uniform sampler2D uLightingTex;   // lighting buffer
	uniform sampler2D uSSR;
	//uniform sampler2D uRoughMetal;
	
	uniform sampler2D uDirectDiffuse;
	uniform sampler2D uDirectSpec;
	uniform sampler2D uIndirectDiffuse;
	uniform sampler2D uIndirectSpecFallback;
	
	//uniform bool useRoughnessMask;

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

		vec3 color = directDiffuse + directSpecular + indirectDiffuse + indirectSpecular;
		
		color = color / (color + vec3(1.0));
		color = pow(color, vec3(1.0 / 2.2));

		/*
		if (useRoughnessMask) {
			vec2 mr = texture(uGExtra, texCoords).rg;
			metallic = mr.r;
			roughness = mr.g;
		}
		*/
		
		// Scale SSR by metallic and glossiness (1 - roughness)
		//float mask = metallic * (1.0 - roughness);
		//vec3 result = ssr.rgb * mask; // * mask
		
		
		//FragColor = vec4(result, 1.0);
		FragColor = vec4(color, 1.0);
	}
	
	/*
		//Ambient + point lights
		vec3 color = ambient + Lo;

		// HDR tonemapping and gamma correct
		color = color / (color + vec3(1.0)) * HdrExposure;
		color = pow(color, vec3(1.0 / HdrContrast));
		
		//Color out
		FragColor = vec4(color, 1.0);
	*/